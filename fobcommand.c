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

extern int mqttdump;

typedef struct nfc_s {
   int sock;
   slot_t id;                   // Us
   slot_t local;                // Local command
   slot_t device;               // Remote device
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
         const char *prefix = j_get(j, "_meta.prefix");
         const char *suffix = j_get(j, "_meta.suffix");
         if (prefix && suffix && !strcmp(prefix, "event") && !strcmp(suffix, "fob"))
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
      slot_send(f->device, "command", "nfc", &j);
   }
   len = 0;
   while (!f->done && !len)
   {
      j_t j = getmsg(f);
      if (!f->done)
      {
         const char *prefix = j_get(j, "_meta.prefix");
         const char *suffix = j_get(j, "_meta.suffix");
         if (prefix && suffix && !strcmp(prefix, "info") && !strcmp(suffix, "nfc"))
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
         }
      }
      j_delete(&j);
   }
   return len;
};

void *fobcommand(void *arg)
{
   nfc_t f = { };
   warnx("Started fobcommand");
   int sock = -1;
   char provision = 0,
       adopt = 0,
       format = 0;
   unsigned char aid[3] = { };
   unsigned char masterkey[17] = { };   // Keys with version on front
   unsigned char aid0key[17] = { };
   unsigned char aid1key[17] = { };
   unsigned char afile[256];
   int organisation=0;
   char *deviceid = NULL;
   char *fob = NULL;
   {                            // Get passed settings
      j_t j = arg;
      j_err(j_write_pretty(j, stderr)); // TODO
      f.sock = atoi(j_get(j, "socket") ? : "");
      f.id = strtoll(j_get(j, "id") ? : "", NULL, 10);
      f.device = strtoll(j_get(j, "device") ? : "", NULL, 10);
      f.local = strtoll(j_get(j, "local") ? : "", NULL, 10);
      provision = j_test(j, "provision", 0);
      organisation=atoi(j_get(j,"organisation"));
      adopt = j_test(j, "adopt", 0);
      format = j_test(j, "format", 0);
      const char *v = j_get(j, "deviceid");
      if (v)
         deviceid = strdupa(v);
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
   void status(const char *msg) {
      if (!f.local)
         return;
      j_t j = j_create();
      j_store_string(j, "status", msg);
      slot_send(f.local, NULL, NULL, &j);
   }
   void led(const char *led) {
      if (!f.device)
         return;
      j_t j = j_create();
      j_string(j, led);
      slot_send(f.device, "command", "led", &j);
   }
   const char *e = NULL;
   if (provision || adopt || format)
   {
      slot_send(f.device, "command", "nfcremote", NULL);
      if (fob)
         f.connected = 1;       // Already connected
      df_t d;
      if ((e = df_init(&d, &f, &dx)))
         status(e);
      else
      {
         if (!f.connected)
         {
            led("A");
            status("Waiting for fob");
            // Wait fob connect...
            while (!f.done && !f.connected)
            {
               j_t j = getmsg(&f);
               j_delete(&j);
            }
         }
         if (f.connected)
            led("A-");

         unsigned char uid[7];

         void doconnect(void) {
            if (mqttdump)
               warnx("Fob connect");
            if ((e = df_select_application(&d, NULL)))
               return;
            if ((e = df_authenticate(&d, 0, masterkey + 1)))
               return;
            if ((e = df_get_uid(&d, uid)))
               return;
            if (!fob)
               return;
            if (strcmp(fob, j_base16(sizeof(uid), uid)))
               e = "Fob mismatch";
            warnx("Connect done");
         }
         void doformat(void) {
            if (mqttdump)
               warnx("Fob format");
            if ((e = df_format(&d, masterkey + 1)) && (e = df_format(&d, NULL)))
               return;
            unsigned int mem;
            if ((e = df_free_memory(&d, &mem)))
               return;
            {                   // Tell system formatted
               j_t j = j_create();
               j_int(j_path(j, "_meta.loopback"), f.id);
               j_true(j_path(j, "_meta.formatted"));
               j_store_string(j, "fob", j_base16(sizeof(uid), uid));
               j_store_string(j, "aid", j_base16(sizeof(aid), aid));
               j_store_string(j, "deviceid", deviceid);
               j_store_int(j, "mem", mem);
               mqtt_qin(&j);
            }
         }

         void doadopt(void) {
            if (mqttdump)
               warnx("Fob adopt");
            // TODO checking key versions to work out if part adopted
            unsigned int n;
            {
               unsigned char aids[3 * 20];
               if ((e = df_get_application_ids(&d, &n, sizeof(aids), aids)))
                  errx(1, "Application list: %s", e);
               while (n && memcmp(aids + n * 3 - 3, aid, 3))
                  n--;
            }
            if (!n)
            {                   // Create
               if ((e = df_create_application(&d, aid, DF_SET_DEFAULT, 2)))
                  return;
               if ((e = df_select_application(&d, aid)))
                  return;
               if ((e = df_authenticate(&d, 0, NULL)))
                  return;
               if ((e = df_change_key(&d, 0, *aid0key, NULL, aid0key + 1)))
                  return;
               if ((e = df_authenticate(&d, 0, aid0key + 1)))
                  return;
               if ((e = df_change_key(&d, 1, *aid1key, NULL, aid1key + 1)))
                  return;
               if ((e = df_change_key_settings(&d, 0xEB)))
                  return;
               if ((e = df_create_file(&d, 0x0A, 'B', 1, 0x0010, 256, 0, 0, 0, 0, 0)))
                  return;
            } else
            {                   // Check auth is right
               if ((e = df_select_application(&d, aid)))
                  return;
               if ((e = df_authenticate(&d, 1, aid1key + 1)))
                  return;
               if ((e = df_authenticate(&d, 0, aid0key + 1)))
                  return;
            }
            if ((e = df_write_data(&d, 0x0A, 'B', 1, 0, *afile + 1, afile)))
               return;
            if ((e = df_commit(&d)))
               return;

            unsigned int mem;
            if ((e = df_free_memory(&d, &mem)))
               return;
            {                   // Tell system adopted
               j_t j = j_create();
               j_int(j_path(j, "_meta.loopback"), f.id);
               j_true(j_path(j, "_meta.adopted"));
               j_store_string(j, "fob", j_base16(sizeof(uid), uid));
               j_store_string(j, "aid", j_base16(sizeof(aid), aid));
               j_store_string(j, "deviceid", deviceid);
               if(organisation)j_store_int(j, "organisation", organisation);
               j_store_int(j, "mem", mem);
               mqtt_qin(&j);
            }
         }

         void doprovision(void) {
            if (mqttdump)
               warnx("Fob provision");
            status("Provisioning fob");
            unsigned char version;
            if ((e = df_get_key_version(&d, 0, &version)))
               return;
            if (!version)
            {                   // Formatting
               status("Formatting card");
               if ((e = df_format(&d, masterkey + 1)))
                  return;
               memset(masterkey, 0, sizeof(masterkey)); // Format does that
               if ((e = df_authenticate(&d, 0, NULL)))
                  return;
               if ((e = df_change_key_settings(&d, 0x09)))
                  return;
               if ((e = df_set_configuration(&d, 0)))
                  return;       // Not random or lock...
            } else if ((e = df_authenticate(&d, 0, masterkey + 1)))
               return;
            unsigned char uid[7];
            if ((e = df_get_uid(&d, uid)))
               return;
            status("Setting key");
            unsigned char key[16];
            {                   // Key key
               int f = open("/dev/urandom", O_RDONLY);
               if (f < 0)
                  err(1, "Cannot open random");
               if (read(f, key, sizeof(key)) != sizeof(key))
                  err(1, "Cannot read random");
               close(f);
            }
            unsigned int mem;
            if ((e = df_free_memory(&d, &mem)))
               return;          // setting key does not change this
            {                   // Tell system new key
               j_t j = j_create();
               j_int(j_path(j, "_meta.loopback"), f.id);
               j_true(j_path(j, "_meta.provisioned"));
               j_store_string(j, "fob", j_base16(sizeof(uid), uid));
               j_store_string(j, "key", j_base16(sizeof(key), key));
               j_store_int(j, "mem", mem);
               mqtt_qin(&j);
            }
            if ((e = df_change_key(&d, 0x80, 0x01, NULL, key)))
               return;
            if ((e = df_authenticate(&d, 0, key)))
               return;
            status("Done, remove card");
         }
         if (f.connected && !f.done)
         {
            doconnect();
            if (!e)
            {
               if (adopt)
                  doadopt();
               else if (provision)
                  doprovision();
               else if (format)
                  doformat();
            }
         }

      }
      slot_send(f.device, "command", "nfcdone", NULL);
   }
   led(e ? "R" : "G");
   if (e)
      status(*e ? e : "Card gone");
   if (e && *e && mqttdump)
      warnx("Fob failed: %s", e);
   // Finish
   slot_close(f.local);
   slot_destroy(f.id);
   warnx("Ended fobcommand");
   return NULL;
}
