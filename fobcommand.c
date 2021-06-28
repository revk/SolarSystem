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
#include "ssmqtt.h"

typedef struct nfc_s {
   int sock;
   long long instance;
   long long local;
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
      mqtt_send(f->instance, "command", "nfc", &j);
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
               unsigned char *buf = NULL;
               ssize_t datalen = j_base16d(j_val(jdata), &buf);
               if (datalen > max)
                  *errstr = "Too long";
               else
                  memcpy(data, buf, len = datalen);
               if (buf)
                  free(buf);
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
   {                            // Get passed settings
      j_t j = arg;
      j_err(j_write_pretty(j, stderr)); // TODO
      f.sock = atoi(j_get(j, "socket") ? : "");
      f.instance = strtoll(j_get(j, "instance"), NULL, 10);
      f.local = strtoll(j_get(j, "local"), NULL, 10);
      provision = j_test(j, "provision", 0);
      adopt = j_test(j, "adopt", 0);
      format = j_test(j, "format", 0);
      if (!sock)
         errx(1, "socket not set");
      j_delete(&j);
   }
   void status(const char *msg) {
      if (!f.local)
         return;
      j_t j = j_create();
      j_store_string(j, "status", msg);
      mqtt_send(f.local, NULL, NULL, &j);
   }
   void led(const char *led) {
      if (!f.instance)
         return;
      j_t j = j_create();
      j_string(j, led);
      mqtt_send(f.instance, "command", "led", &j);
   }
   const char *e = NULL;
   if (provision || adopt)
   {
      mqtt_send(f.instance, "command", "nfc", NULL);
      if (adopt)
         f.connected = 1;       // Already connected for adopt
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

         void doadopt(void) {

         }

         void doprovision(void) {
            status("Provisioning fob");
            // Select application (root)
            if ((e = df_select_application(&d, NULL)))
               return;
            unsigned char version;
            if ((e = df_get_key_version(&d, 0, &version)))
               return;
            if (!version || format)
            {                   // Formatting
               status("Formatting card");
               if ((e = df_format(&d, NULL)))
                  return;
               if ((e = df_authenticate(&d, 0, NULL)))
                  return;
               if ((e = df_change_key_settings(&d, 0x09)))
                  return;
               if ((e = df_set_configuration(&d, 0)))
                  return;       // Not random or lock...
            } else if ((e = df_authenticate(&d, 0, NULL)))
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
            {                   // Tell system new key
               j_t j = j_create();
               j_int(j_path(j, "_meta.loopback"), f.instance);
               j_store_true(j, "_meta.provision");
               j_store_string(j, "fob", j_base16(sizeof(uid), uid));
               j_store_string(j, "key", j_base16(sizeof(key), key));
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
            if (adopt)
               doadopt();
            if (provision)
               doprovision();
         }

      }
   }
   led(e ? "R" : "G");
   if (e)
      status(*e ? e : "Card gone");
   // Finish
   {                            // unlink
      j_t j = j_create();
      j_int(j_path(j, "_meta.loopback"), f.instance);
      j_store_true(j, "_meta.close");
      mqtt_qin(&j);
      if (f.local)
         mqtt_send(f.local, NULL, 0, NULL);     // Close local
   }
   warnx("Ended fobcommand");
   close(sock);
   return NULL;
}
