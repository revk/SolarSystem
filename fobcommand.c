// Fob command thread

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include "AJL/ajl.h"
#include <openssl/evp.h>
#include "DESFireAES/include/desfireaes.h"
#include "mqttmsg.h"
#include "ssmqtt.h"
#include "fobcommand.h"

extern int mqttdump;

typedef struct nfc_s {
   int sock;
   slot_t id;                   // Us
   slot_t local;                // Local command
   slot_t device;               // Remote device
   const char *deviceid;
   unsigned char remote:1;
   unsigned char connected:1;
   unsigned char held:1;
   unsigned char gone:1;
   unsigned char done:1;
} nfc_t;

static j_t getmsg(nfc_t * f)
{                               // Get next message
   while (1)
   {
      fd_set r;
      FD_ZERO(&r);
      FD_SET(f->sock, &r);
      struct timeval to = { 60, 0 };
      if (select(f->sock + 1, &r, NULL, NULL, &to) <= 0)
         break;
      unsigned char buf[2000];
      int len = recv(f->sock, buf, sizeof(buf), 0);
      if (len <= 0)
      {
         f->done = 1;
         break;
      }
      if (*buf != 0x30)
         continue;
      j_t j = mqtt_decode(buf, len);
      if (!j || j_isnull(j))
         f->done = 1;
      else
      {
         const char *target = j_get(j, "_meta.target");
         if (!target || strcmp(target, f->deviceid))
         {
            j_delete(&j);
            continue;
         }
         const char *prefix = j_get(j, "_meta.prefix");
         const char *suffix = j_get(j, "_meta.suffix");
         if (prefix && target && suffix && !strcmp(prefix, "event") && !strcmp(suffix, "fob"))
         {
            if (j_test(j, "remote", 0))
               f->remote = 1;
            if (j_test(j, "held", 0))
               f->held = 1;
            if (j_test(j, "gone", 0))
               f->gone = f->done = 1;
            else
               f->connected = 1;
         }
      }
      return j;
   }
   return NULL;
}

static int dx(void *obj, unsigned int len, unsigned char *data, unsigned int max, const char **errstr)
{                               // DESFire DX
   nfc_t *f = obj;
   if (!f->done && f->connected)
   {                            // Send
      j_t j = j_create();
      j_string(j, j_base16(len, data));
      slot_send(f->device, "command", f->deviceid, "nfc", &j);
   }
   len = 0;
   while (!f->done && !len)
   {
      j_t j = getmsg(f);
      if (!f->done)
      {
         const char *prefix = j_get(j, "_meta.prefix");
         const char *suffix = j_get(j, "_meta.suffix");
         if (prefix && suffix && !strcmp(prefix, "info"))
         {
            if (!strcmp(suffix, "nfc"))
            {
               j_t jdata = j_find(j, "_data");
               if (j_isstring(jdata))
               {
                  ssize_t datalen = j_base16D(data, max, j_val(jdata));
                  if (datalen > max)
                     *errstr = "Too long";
                  else
                     len = datalen;
               }
            } else if (!strcmp(suffix, "nfcerror"))
               len = -1;
         }
      }
      j_delete(&j);
   }
   return len;
};

void *fobcommand(void *arg)
{
   nfc_t f = { };
   int sock = -1;
   char provision = 0,
       identify = 0,
       adopt = 0,
       format = 0,
       hardformat = 0;
   unsigned char aid[3] = { };
   unsigned char masterkey[KEY_DATA_LEN] = { 0 };       // Type, Ver, Key
   unsigned char aid0key[KEY_DATA_LEN] = { 0 }; // Type, Ver, Key
   unsigned char aid1key[KEY_DATA_LEN] = { 0 }; // Type, Ver, Key
   unsigned char afile[256] = { 0 };
   int organisation = 0;
   int access = 0;
   char *fob = NULL;
   char *fobname = NULL;
   {                            // Get passed settings
      const char *v;
      j_t j = arg;
      if (mqttdump)
      {
         fprintf(stderr, "FC:");
         j_err(j_write(j, stderr));
         fprintf(stderr, "\n");
      }
      f.sock = atoi(j_get(j, "socket") ? : "");
      f.id = strtoll(j_get(j, "id") ? : "", NULL, 10);
      f.device = strtoll(j_get(j, "device") ? : "", NULL, 10);
      f.local = strtoll(j_get(j, "local") ? : "", NULL, 10);
      provision = j_test(j, "provision", 0);
      organisation = atoi(j_get(j, "organisation") ? : "");
      access = atoi(j_get(j, "access") ? : "");
      adopt = j_test(j, "adopt", 0);
      format = j_test(j, "format", 0);
      identify = j_test(j, "identify", 0);
      if ((hardformat = j_test(j, "hardformat", 0)))
         format = 1;
      if ((v = j_get(j, "deviceid")))
         f.deviceid = strdupa(v);
      if ((v = j_get(j, "fobname")))
         fobname = strdupa(v);
      v = j_get(j, "fob");
      if (v)
         fob = strdupa(v);
      j_base16D(masterkey, sizeof(masterkey), j_get(j, "masterkey"));
      j_base16D(aid0key, sizeof(aid0key), j_get(j, "aid0key"));
      j_base16D(aid1key, sizeof(aid1key), j_get(j, "aid1key"));
      j_base16D(afile, sizeof(afile), j_get(j, "afile"));
      j_base16D(aid, sizeof(aid), j_get(j, "aid"));
      if (!sock)
         errx(1, "socket not set");
      j_delete(&j);
   }
   if (f.deviceid && *f.deviceid)
   {
      void status(const char *msg) {
         if (mqttdump)
            warnx("Fob:%s", msg);
         if (!f.local)
            return;
         j_t j = j_create();
         j_store_string(j, "status", msg);
         slot_send(f.local, NULL, f.deviceid, NULL, &j);
      }
      void led(const char *led) {
         if (!f.device)
            return;
         j_t j = j_create();
         j_string(j, led);
         slot_send(f.device, "command", f.deviceid, "led", &j);
      }
      const char *e = NULL;
      const char *dfcheck(const char *res, const char *func, int line) {
         if (res)
         {
            e = res;
            if (mqttdump)
               warnx("DF fail (%s): %s line %d", *res ? res : "Gone", func, line);
            if (!f.gone)
               status("Failed, remove fob");
         }
         return res;
      }
#define df(x) if(dfcheck(df_##x,#x,__LINE__))return
      if (identify)
      {
         slot_send(f.device, "command", f.deviceid, "nfcremote", NULL);
         led("R+A-");
         char *id = NULL;
         while (!f.done && !f.connected && !id)
         {
            j_t j = getmsg(&f);
            const char *prefix = j_get(j, "_meta.prefix");
            const char *suffix = j_get(j, "_meta.suffix");
            if (prefix && suffix && !strcmp(prefix, "event") && !strcmp(suffix, "fob") && (id = (char *) j_get(j, "id")))
               id = strdupa(id);
            j_delete(&j);
         }
         if (id)
         {
            j_t j = j_create();
            j_store_string(j, "fobid", id);
            slot_send(f.local, NULL, f.deviceid, NULL, &j);
         }
      } else if (provision || adopt || format)
      {
         slot_send(f.device, "command", f.deviceid, "nfcremote", NULL);
         //if (fob) f.connected = 1;    // Already connected
         df_t d;
         e = df_init(&d, &f, &dx);
         if (!e)
         {
            if (!f.connected)
            {
               char *id = NULL;
               led("R+A-");
               status("Waiting for fob");
               // Wait fob connect...
               while (!f.done && !f.connected)
               {
                  j_t j = getmsg(&f);
                  const char *prefix = j_get(j, "_meta.prefix");
                  const char *suffix = j_get(j, "_meta.suffix");
                  if (prefix && suffix && !strcmp(prefix, "event") && !strcmp(suffix, "fob") && (id = (char *) j_get(j, "id")))
                     id = strdupa(id);
                  j_delete(&j);
               }
               if (fob && id && strcmp(fob, id))
               {
                  status(id);
                  e = "Not the expected fob";
               }
            }
            if (f.connected)
               led("A-");

            unsigned char uid[7];

            void doformat(void) {
               status("Formatting fob");
               df(format(&d, masterkey[1], masterkey + 2));
               df(get_uid(&d, uid));
               status(j_base16(sizeof(uid), uid));
               if (hardformat)
               {
                  status("Hard format fob - will need provisioning again");
                  df(change_key(&d, 0x80, 0, masterkey + 2, NULL));     // Hard reset to zero AES
                  df(authenticate(&d, 0, NULL));
               } else
               {
                  df(change_key_settings(&d, 0x09));
                  df(set_configuration(&d, 0));
               }
               unsigned int mem;
               df(free_memory(&d, &mem));
               {                // Tell system formatted
                  j_t j = j_create();
                  j_int(j_path(j, "_meta.loopback"), f.id);
                  j_true(j_path(j, "_meta.formatted"));
                  j_store_string(j, "fob", j_base16(sizeof(uid), uid));
                  if (aid[0] || aid[1] || aid[2])
                     j_store_string(j, "aid", j_base16(sizeof(aid), aid));
                  j_store_string(j, "deviceid", f.deviceid);
                  j_store_int(j, "capacity", mem);
                  j_store_int(j, "mem", mem);
                  mqtt_qin(&j);
               }
            }

            void doconnect(void) {
               status("Connecting to fob");
               df(select_application(&d, NULL));
               if (df_authenticate(&d, 0, masterkey + 2))
                  df(authenticate(&d, 0, NULL));
               df(get_uid(&d, uid));
               status(j_base16(sizeof(uid), uid));
               if (!fob)
                  return;
               if (strcmp(fob, j_base16(sizeof(uid), uid)))
                  e = "Fob mismatch";
            }

            void doadopt(void) {
               status("Adopting fob");
               if (!*aid0key)
                  randkey(aid0key);     // type 0 means not set
               unsigned int n;
               {
                  unsigned char aids[3 * 20];
                  df(get_application_ids(&d, &n, sizeof(aids), aids));
                  while (n && memcmp(aids + n * 3 - 3, aid, 3))
                     n--;
               }
               if (!n)
               {
                  status("Creating application");
                  df(create_application(&d, aid, 0xEB, 2));
               }
               df(select_application(&d, aid));
               if (df_authenticate(&d, 0, aid0key + 2))
               {                // Set key 0
                  status("Setting application key");
                  df(authenticate(&d, 0, NULL));        // own key to change it
                  df(change_key(&d, 0, aid0key[1], NULL, aid0key + 2));
               }
               df(authenticate(&d, 0, aid0key + 2));
               // Check files
               unsigned long long fids;
               df(get_file_ids(&d, &fids));
               if (!(fids & (1 << 0x0A)))
               {
                  status("Making access file");
                  df(create_file(&d, 0x0A, 'B', 1, 0x0010, 256, 0, 0, 0, 0, 0));
               }
               // File 0 - name
               // Not used - name is in access file
               // File 1 - log
               if (!(fids & (1 << 0x01)))
               {
                  status("Making log file");
                  df(create_file(&d, 0x01, 'C', 1, 0x0100, 13, 0, 0, 10, 0, 0));
               }
               // File 2 - counter
               if (!(fids & (1 << 0x02)))
               {
                  status("Making counter file");
                  df(create_file(&d, 0x02, 'V', 1, 0x0010, 0, 0, 0x7FFFFFFF, 0, 0, 0));
               }
               // File A - access
               status("Storing access file");
               df(write_data(&d, 0x0A, 'B', 1, 0, *afile + 1, afile));
               df(commit(&d));
               // This is last as it is what marks a fob as finally adopted
	       // TODO what if not current key version?
               if (df_authenticate(&d, 1, aid1key + 2))
               {                // Set key 1
                  status("Setting AID key");
                  df(authenticate(&d, 1, NULL));        // own key to change it
                  df(change_key(&d, 1, aid1key[1], NULL, aid1key + 2));
               }
               unsigned int mem;
               df(free_memory(&d, &mem));
               {                // Tell system adopted
                  j_t j = j_create();
                  j_int(j_path(j, "_meta.loopback"), f.id);
                  j_true(j_path(j, "_meta.adopted"));
                  j_store_string(j, "fob", j_base16(sizeof(uid), uid));
                  if (aid[0] || aid[1] || aid[2])
                     j_store_string(j, "aid", j_base16(sizeof(aid), aid));
                  j_store_string(j, "aid0key", j_base16(sizeof(aid0key), aid0key));
                  j_store_stringf(j, "ver", "%02X", aid1key[1]);
                  j_store_string(j, "deviceid", f.deviceid);
                  if (organisation)
                     j_store_int(j, "organisation", organisation);
                  if (access)
                     j_store_int(j, "access", access);
                  if (fobname)
                     j_store_string(j, "fobname", fobname);
                  j_store_int(j, "mem", mem);
                  mqtt_qin(&j);
               }
            }

            void doprovision(void) {    // Expects to have formatted and connected
               status("Setting key");
               if (!*masterkey)
                  randkey(masterkey);   // Type 0 is not set, so make a key
               if (adopt && !*aid0key)
                  randkey(aid0key);     // Type 0 is not set, so make a key
               unsigned int mem;
               df(free_memory(&d, &mem));
               {                // Tell system new key
                  j_t j = j_create();
                  j_int(j_path(j, "_meta.loopback"), f.id);
                  j_true(j_path(j, "_meta.provisioned"));
                  j_store_string(j, "fob", j_base16(sizeof(uid), uid));
                  j_store_string(j, "masterkey", j_base16(sizeof(masterkey), masterkey));
                  if (adopt)
                  {
                     if (aid[0] || aid[1] || aid[2])
                        j_store_string(j, "aid", j_base16(sizeof(aid), aid));
                     j_store_string(j, "aid0key", j_base16(sizeof(aid0key), aid0key));
                  }
                  j_store_int(j, "mem", mem);
                  mqtt_qin(&j);
               }
               df(change_key(&d, 0x80, masterkey[1], NULL, masterkey + 2));
               df(authenticate(&d, 0, masterkey + 2));
            }
            if (f.connected && !f.done)
            {
               if (!e && (format || provision))
                  doformat();
               if (!e)
                  doconnect();
               if (!e && provision)
                  doprovision();
               if (!e && adopt)
                  doadopt();
            }
            if (!e && !f.gone)
               status("Done, remove card");
         }
      }

      led(e ? "R" : "G");
      if (f.connected)
         while (!f.gone)
         {
            j_t j = getmsg(&f);
            j_delete(&j);
         }
      slot_send(f.device, "command", f.deviceid, "nfcdone", NULL);
      if (e)
         status(*e ? e : "Card gone");
      if (e && *e && mqttdump)
         warnx("Fob failed: %s", e);
   }
   // Finish
   slot_close(f.local);
   slot_destroy(f.id);
   return NULL;
}

void randblock(unsigned char *data, ssize_t len)
{
   int f = open("/dev/urandom", O_RDONLY);
   if (f < 0)
      err(1, "Cannot open random");
   if (read(f, data, len) != len)
      err(1, "Cannot read random");
   close(f);
}

void randkey(unsigned char key[KEY_DATA_LEN])
{
   *key = 1;                    // Default, non encrypted
   randblock(key + 1, KEY_DATA_LEN - 1);
}
