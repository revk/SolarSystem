// Back end management / control for SolarSystem modules

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
#include <openssl/evp.h>
#include "SQLlib/sqllib.h"
#include "AJL/ajl.h"
#include "ssmqtt.h"
#include "sscert.h"

void ssdatabase(SQL *);
void sskeydatabase(SQL *);
void sstypes(const char *);

const char *cakey = "",
    *cacert = "";
const char *mqttkey = "",
    *mqttcert = "";

extern int sqldebug;
int dump = 0;                   // dump level debug

char *makeaes(SQL * sqlkeyp, const char *aid, const char *fob, const char *ver)
{                               // Make an new AES and return AES string (malloc'd)
   unsigned char bin[16];
   int f = open("/dev/urandom", O_RDONLY);
   if (f < 0)
      err(1, "Cannot open /dev/urandom");
   if (read(f, bin, sizeof(bin)) != sizeof(bin))
      errx(1, "Failed to read random");
   close(f);
   char *aes = j_base16(sizeof(bin), bin);
   if (sql_query_free(sqlkeyp, sql_printf("INSERT INTO `AES` SET `aid`=%#s,`fob`=%#s,`ver`=%#s,`key`=%#s", aid, fob, ver, aes)))
      aes = NULL;
   if (aes)
      return strdup(aes);       // was on stack
   return NULL;
}

const char *upgrade(SQL_RES * res, long long instance)
{                               // Send upgrade if needed
   const char *upgrade = sql_col(res, "upgrade");
   if (!upgrade || j_time(upgrade) > time(0))
      return NULL;
   command(instance, "upgrade", NULL);
   return upgrade;
}

const char *security(SQL * sqlp, SQL * sqlkeyp, SQL_RES * res, long long instance)
{                               // Security settings
   j_t j = j_create();
   const char *aid = sql_colz(res, "aid");
   int organisation = atoi(sql_colz(res, "organisation"));
   int doorauto = atoi(sql_colz(res, "doorauto"));
   j_t aids = j_store_array(j, "aes");
   if (*aid && strcmp(sql_colz(res, "nfctx"), "-"))
   {                            // Security
      // Keys
      SQL_RES *r = sql_safe_query_store_free(sqlkeyp, sql_printf("SELECT * FROM `AES` WHERE `aid`=%#s AND `fob` IS NULL order BY `created` DESC LIMIT 3", aid));
      while (sql_fetch_row(r))
         j_append_stringf(aids, "%s%s", sql_colz(r, "ver"), sql_colz(r, "key"));
      sql_free_result(r);
      if (!j_len(aids))
      {                         // Make first key
         char *aes = makeaes(sqlkeyp, aid, NULL, "01");
         if (aes)
         {
            j_append_stringf(aids, "01%s", aes);
            free(aes);
         }
      }
   }
   if (!j_len(aids))
      aid = "";                 // We have not loaded any keys so no point in even trying an AID
   j_store_string(j, "aid", aid);
   j_t blacklist = j_store_array(j, "blacklist");
   if (*aid && doorauto >= 3)
   {
      SQL_RES *b = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `foborganisation` LEFT JOIN `fobaid` USING (`fob`) WHERE `organisation`=%d AND `aid`=%#s AND `blocked` IS NOT NULL AND `confirmed` IS NULL ORDER BY `blocked` DESC LIMIT 10", organisation, aid));
      while (sql_fetch_row(b))
         j_append_string(blacklist, sql_colz(b, "fob"));
      sql_free_result(b);
   }
   if (!j_isnull(j))
      setting(instance, NULL, &j);
   j_delete(&j);
   return NULL;
}

const char *settings(SQL * sqlp, SQL_RES * res, long long instance)
{                               // Send base settings
   j_t j = j_create();
   int doorauto = atoi(sql_colz(res, "doorauto"));
   if (*CONFIG_OTA_HOSTNAME)
      j_store_string(j, "otahost", CONFIG_OTA_HOSTNAME);
   j_store_string(j, "name", sql_colz(res, "description"));
   j_store_int(j, "doorauto", doorauto);
   int pcb = atoi(sql_colz(res, "pcb"));
   if (pcb)
   {                            // Main PCB settings
      SQL_RES *p = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `pcb` WHERE `pcb`=%d", pcb));
      if (sql_fetch_row(p))
      {
#define set(n) {const char *v=sql_colz(p,#n);if(!strcmp(v,"-"))v="";j_store_string(j,#n,v);}
         set(tamper);
         set(blink);
         set(nfctx);
         set(nfcrx);
         set(nfcpower);
         set(nfcred);
         set(nfcamber);
         set(nfcgreen);
         set(nfctamper);
         set(nfcbell);
         set(nfccard);
#undef set
      }
      sql_free_result(p);
      int i = (doorauto ? 4 : 0),
          o = (doorauto ? 4 : 0);
      j_t input = j_store_array(j, "input");
      j_t output = j_store_array(j, "output");
      j_t power = j_store_array(j, "power");
      SQL_RES *g = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `devicegpio` WHERE `device`=%#s", sql_col(res, "device")));
      while (sql_fetch_row(g))
      {
         const char *type = sql_colz(g, "type");
         j_t gpio = NULL;
         int n = 0;
         if (*type == 'P')
            gpio = j_append_null(power);
         else if (*type == 'I')
         {
            if (type[1])
               n = atoi(type + 1) - 1;
            else
               n = i++;
            gpio = j_index(j_extend(input, n + 1), n);
         } else if (*type == 'O')
         {
            if (type[1])
               n = atoi(type + 1) - 1;
            else
               n = o++;
            gpio = j_index(j_extend(output, n + 1), n);
         }
         if (gpio)
         {
            j_store_string(gpio, "gpio", sql_colz(g, "gpio"));
            void addstate(const char *state) {
               const char *areas = sql_colz(g, state);
               if (*areas)
               {
                  j_store_string(gpio, state, areas);
               }
            }
#define i(n) addstate(#n);
#define s(n) i(n)
#include "ESP32/main/states.m"
         }
      }
      sql_free_result(g);
   }
   if (!j_isnull(j))
      setting(instance, NULL, &j);
   j_delete(&j);
   return NULL;
}

void bogus(long long instance)
{                               // This is bogus auth
   j_t m = j_create();
   j_store_string(m, "clientkey", "");
   j_store_string(m, "clientcert", "");
   setting(instance, NULL, &m);
}

void daily(SQL * sqlp)
{
   sql_safe_query(sqlp, "DELETE FROM `session` WHERE `expires`<NOW()");
   // TODO afile updates?
}

static void *fobcommand(void *arg)
{
   warnx("Started fobcommand");
   int sock = -1;
   long long instance = 0;
   {                            // Get passed settings
      j_t j = arg;
      sock = atoi(j_get(j, "socket") ? : "");
      instance = strtoll(j_get(j, "instance"), NULL, 10);
      if (!sock)
         errx(1, "socket not set");
      j_delete(&j);
   }
   while (1)
   {
      fd_set r;
      FD_ZERO(&r);
      FD_SET(sock, &r);
      struct timeval to = { 3000, 0 };
      if (select(sock + 1, &r, NULL, NULL, &to) <= 0)
         break;
      unsigned char buf[2000];
      int len = recv(sock, buf, sizeof(buf), 0);
      if (len <= 0)
         break;
      if (*buf != 0x30)
         continue;
      j_t j = mqtt_decode(buf, len);
      if(j_isnull(j))
      { // End
	      j_delete(&j);
	      break;
      }
      j_err(j_write_pretty(j, stderr)); // TODO
      j_delete(&j);
   }
   {                            // unlink
      j_t j = j_create();
      j_int(j_path(j, "_meta.loopback"), instance);
      mqtt_qin(&j);
   }
   warnx("Ended fobcommand");
   return NULL;
}

int main(int argc, const char *argv[])
{
#ifdef	SQL_DEBUG
   sqldebug = 1;
#endif
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         { "debug", 'v', POPT_ARG_NONE, &sqldebug, 0, "Debug", NULL },
         { "dump", 'V', POPT_ARG_NONE, &dump, 0, "Debug dump", NULL },
         POPT_AUTOHELP { }
      };
      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");
      int c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));
      if (poptPeekArg(optCon))
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }
      poptFreeContext(optCon);
   }
   if (!sqldebug)
      daemon(1, 1);
   // Get started
   signal(SIGPIPE, SIG_IGN);    // Don't crash on pipe errors
   openlog("SS", LOG_CONS | LOG_PID, LOG_USER); // Main logging is to syslog
   {                            // File limits - allow lots of connections at once
      struct rlimit l = {
      };
      if (!getrlimit(RLIMIT_NOFILE, &l))
      {
         l.rlim_cur = l.rlim_max;
         if (setrlimit(RLIMIT_NOFILE, &l))
            syslog(LOG_INFO, "Could not increase files");
      }
      if (!getrlimit(RLIMIT_NPROC, &l))
      {
         l.rlim_cur = l.rlim_max;
         if (setrlimit(RLIMIT_NPROC, &l))
            syslog(LOG_INFO, "Could not increase threads");
      }
   }
   // Load (or make) keys
   const char *msgkey = "",
       *msgcert = "";
   if (!access(CONFIG_KEYS_FILE, R_OK))
   {
      j_t j = j_create();
      if (!j_read_file(j, CONFIG_KEYS_FILE))
      {
         cakey = strdup(j_get(j, "ca.key") ? : "");
         mqttkey = strdup(j_get(j, "mqtt.key") ? : "");
      }
      j_delete(&j);
   }
   if (!access(CONFIG_MSG_KEY_FILE, R_OK))
   {
      j_t j = j_create();
      if (!j_read_file(j, CONFIG_MSG_KEY_FILE))
      {
         msgkey = strdup(j_get(j, "msg.key") ? : "");
         cacert = strdup(j_get(j, "ca.cert") ? : "");
         msgcert = strdup(j_get(j, "msg.cert") ? : "");
         mqttcert = strdup(j_get(j, "mqtt.cert") ? : "");
      }
      j_delete(&j);
   }
   if (!*cakey || !*mqttkey || !*msgkey || !*cacert || !*mqttcert || !*msgcert)
   {
      if (!*cakey)
         cakey = makekey();
      if (!*mqttkey)
         mqttkey = makekey();
      if (!*msgkey)
         msgkey = makekey();
      if (!*cacert)
         cacert = makecert(cakey, NULL, NULL, "SolarSystem");
      if (!*mqttcert)
         mqttcert = makecert(mqttkey, cakey, cacert, CONFIG_MQTT_HOSTNAME);
      if (!*msgcert)
         msgcert = makecert(msgkey, cakey, cacert, "-LOCAL--MSG-");
      unlink(CONFIG_KEYS_FILE);
      j_t j = j_create();
      j_object(j);
      j_string(j_path(j, "ca.key"), cakey);
      j_string(j_path(j, "mqtt.key"), mqttkey);
      int f = open(CONFIG_KEYS_FILE, O_CREAT | O_WRONLY, 0400);
      if (f < 0)
         err(1, "Cannot make %s", CONFIG_KEYS_FILE);
      j_err(j_write_fd(j, f));
      close(f);
      j_delete(&j);
      unlink(CONFIG_MSG_KEY_FILE);
      j = j_create();
      j_object(j);
      j_string(j_path(j, "msg.key"), msgkey);
      j_string(j_path(j, "ca.cert"), cacert);
      j_string(j_path(j, "mqtt.cert"), mqttcert);
      j_string(j_path(j, "msg.cert"), msgcert);
      f = open(CONFIG_MSG_KEY_FILE, O_CREAT | O_WRONLY, 0440);
      if (f < 0)
         err(1, "Cannot make %s", CONFIG_KEYS_FILE);
      j_err(j_write_fd(j, f));
      close(f);
      j_delete(&j);
   }
   // Connect
   sstypes("types");
   SQL sqlkey;
   sql_cnf_connect(&sqlkey, CONFIG_SQL_KEY_CONFIG_FILE);
   sskeydatabase(&sqlkey);
   SQL sql;
   sql_cnf_connect(&sql, CONFIG_SQL_CONFIG_FILE);
   ssdatabase(&sql);
   if (sqldebug)
   {
      char *key = makekey();
      char *cert = makecert(key, cakey, cacert, "112233445566");
      printf("CA cert:\n%s\nTest key:\n%s\nTest cert:\n%s\n", cacert, key, cert);
      free(key);
      free(cert);
   }
   syslog(LOG_INFO, "Starting");
   sql_safe_query(&sql, "DELETE FROM `pending` WHERE `instance` IS NOT NULL");
   sql_safe_query(&sql, "UPDATE `device` SET `instance`=NULL,`online`=NULL WHERE `instance` IS NOT NULL");
   mqtt_start();
   // Main loop getting messages (from MQTT or websocket)
   while (1)
   {
      {                         // Daily jobs
         static int today = 0;
         time_t now = time(0);
         if (now / 86400 != today)
         {
            today = now / 86400;
            daily(&sql);
         }
      }
      j_t j = incoming();
      if (!j)
         warnx("WTF");
      else
      {
         if (dump)
            j_err(j_write_pretty(j, stderr));
         SQL_RES *device = NULL;
         j_t meta = j_find(j, "_meta");
         if (meta)
            meta = j_detach(meta);
         const char *local(void) {      // Commands sent to us from local system
            const char *v;
            SQL_RES *res = NULL;
            if ((v = j_get(meta, "device")))
               res = sql_safe_query_store_free(&sql, sql_printf("SELECT `instance` FROM `device` WHERE `device`=%#s", v));
            else if ((v = j_get(meta, "pending")))
               res = sql_safe_query_store_free(&sql, sql_printf("SELECT `instance` FROM `pending` WHERE `pending`=%#s", v));
            if (!res)
               return "No device specified";
            if (!sql_fetch_row(res))
            {
               sql_free_result(res);
               return "Device not found";
            }
            long long instance = strtoll(sql_colz(res, "instance") ? : "", NULL, 10);
            if (!instance)
               return "Device not online";
            sql_free_result(res);
            if ((v = j_get(meta, "provision")))
            {                   // JSON is rest of settings to send
               char *key = makekey();
               char *cert = makecert(key, cakey, cacert, v);
               j_store_string(j, "clientcert", cert);
               free(cert);
               const char *fail = mqtt_send(instance, "setting", NULL, &j);
               j = j_create();
               j_store_string(j, "clientkey", key);
               free(key);
               if (fail)
                  return fail;
               fail = mqtt_send(instance, "setting", NULL, &j);
               // Set online later to remove from pending lists in UI
               if (!fail)
                  sql_safe_query_free(&sql, sql_printf("UPDATE `pending` SET `online`=%#T WHERE `pending`=%#s", time(0) + 60, v));
               return fail;
            } else if ((v = j_get(meta, "deport")))
            {
               j_store_null(j, "clientcert");
               j_store_null(j, "clientkey");
               j_store_string(j, "mqtthost", v);
               j_store_string(j, "mqttcert", "");
               j_store_null(j, "mqttport");
               const char *fail = mqtt_send(instance, "setting", NULL, &j);
               // Set online later to remove from pending lists in UI
               if (!fail)
                  sql_safe_query_free(&sql, sql_printf("UPDATE `pending` SET `online`=%#T WHERE `pending`=%#s", time(0) + 60, v));
               return fail;
            } else if ((v = j_get(meta, "prefix")))
            {                   // Send to device
               const char *suffix = j_get(meta, "suffix");
               if (!instance)
                  return "No instance";

               const char *fail = NULL;
               j_t data = j_find(j, "_data");
               if (data)
                  mqtt_send(instance, v, suffix, &data);
               else
                  mqtt_send(instance, v, suffix, &j);
               return fail;
            } else if (j_find(meta, "fobprovision"))
            {
               int txsock;
               slot_t *s = mqtt_slot(&txsock);
               if (!s)
                  return "Link failed";
               slot_link(instance, s);
               j_t j = j_create();
               j_store_int(j, "instance", instance);
               j_store_int(j, "socket", txsock);
               pthread_t t;
               if (pthread_create(&t, NULL, fobcommand, j))
                  err(1, "Cannot create fobcommand thread");
               pthread_detach(t);
            } else
               return "Unknown local request";
            return NULL;
         }
         const char *loopback(void) {   // From linked
            long long instance = strtoll(j_get(meta, "loopback"), NULL, 10);
            // TODO
            return NULL;
         }
         const char *process(void) {
            if (!meta)
               return "No meta data";

            if (j_find(meta, "loopback"))
               return loopback();
            if (j_find(meta, "local"))
            {
               const char *reply = local();
               long long instance = strtoull(j_get(meta, "local") ? : "", NULL, 10);
               if (instance)
               {
                  j_t j = j_create();
                  if (reply)
                     j_string(j, reply);
                  mqtt_send(instance, NULL, NULL, &j);  // reply
               }
               return reply;
            }

            long long message = strtoull(j_get(meta, "message") ? : "", NULL, 10);
            long long instance = strtoull(j_get(meta, "instance") ? : "", NULL, 10);
            if (!instance)
               return "No instance";
            const char *deviceid;
            if ((deviceid = j_get(meta, "device")) && *deviceid && (device = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", deviceid))) && !sql_fetch_row(device))
            {                   // Not found - treat as pending even though athenticated
               sql_free_result(device);
               device = NULL;
            }
            const char *address = j_get(meta, "address");
            const char *prefix = j_get(meta, "prefix");
            const char *suffix = j_get(meta, "suffix");
            if (!message && (!deviceid || *deviceid != '-'))
            {                   // Connect (first message ID 0) - *MUST* be a top level state message
               const char *id = j_get(j, "id");
               if (!id)
                  return "No id";
               if (!prefix || strcmp(prefix, "state") || suffix)
                  return "Bad initial message";
               sql_string_t s = { };
               if (device)
               {
                  sql_sprintf(&s, "UPDATE `device` SET ");      // known, update
                  sql_sprintf(&s, "`lastonline`=NOW(),");
                  if (!upgrade(device, instance) && !settings(&sql, device, instance))
                     security(&sql, &sqlkey, device, instance);
               } else           // pending - update pending
               {
                  sql_sprintf(&s, "REPLACE INTO `pending` SET ");
                  sql_sprintf(&s, "`pending`=%#s,", id);
                  if (deviceid)
                     sql_sprintf(&s, "`authenticated`=%#s,", "true");
                  SQL_RES *res = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", id));
                  if (sql_fetch_row(res))
                     upgrade(res, instance);
                  sql_free_result(res);
               }
               if (!device || (address && strcmp(sql_colz(device, "address"), address)))
                  sql_sprintf(&s, "`address`=%#s,", address);
               const char *version = j_get(j, "version");
               if (!device || (version && strcmp(sql_colz(device, "version"), version)))
                  sql_sprintf(&s, "`version`=%#s,", version);
               const char *secureboot = (j_test(j, "secureboot", 0) ? "true" : "false");
               if (!device || (secureboot && strcmp(sql_colz(device, "secureboot"), secureboot)))
                  sql_sprintf(&s, "`secureboot`=%#s,", secureboot);
               const char *encryptednvs = (j_test(j, "encryptednvs", 0) ? "true" : "falencryptednvs");
               if (!device || (encryptednvs && strcmp(sql_colz(device, "encryptednvs"), encryptednvs)))
                  sql_sprintf(&s, "`encryptednvs`=%#s,", secureboot);
               int flash = atoi(j_get(j, "flash") ? : "");
               if (flash && (!device || (flash != atoi(sql_colz(device, "flash")))))
                  sql_sprintf(&s, "`flash`=%d,", flash);
               sql_sprintf(&s, "`online`=NOW(),");
               sql_sprintf(&s, "`instance`=%lld,", instance);
               if (sql_back_s(&s) == ',')
               {
                  if (device)
                     sql_sprintf(&s, " WHERE `device`=%#s", deviceid);
                  sql_safe_query_s(&sql, &s);
               } else
                  sql_free_s(&s);
               // Continue as the message could be anything
            }
            if (!prefix)
            {                   // Down (all other messages have a topic)
               long long i = strtoull(sql_colz(device, "instance"), NULL, 10);
               if (device)
               {                // known
                  if (i == instance)
                     sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `online`=NULL,`instance`=NULL,`lastonline`=NOW() WHERE `device`=%#s AND `instance`=%lld", deviceid, instance));
                  long long l = slot_linked(instance);
                  if (l)
                  {
                     warnx("Close linked");     // TODO
                     mqtt_send(l, NULL, NULL, NULL);    // Tell linked we are closed
                  }
               } else           // pending
                  sql_safe_query_free(&sql, sql_printf("DELETE FROM `pending` WHERE `instance`=%lld", instance));
               mqtt_close_slot(i);
               return NULL;
            }
            if (prefix && !strcmp(prefix, "state"))
            {                   // State
               if (device)
               {
                  if (!suffix)
                  {             // System level
                     const char *id = j_get(j, "id");
                     if (id && strcmp(id, deviceid))
                        bogus(instance);
                  }
               }
            } else if (prefix && !strcmp(prefix, "event"))
            {
               if (device)
               {
               }
            } else if (prefix && !strcmp(prefix, "error"))
            {
               if (device)
               {
               }
            } else if (prefix && !strcmp(prefix, "info"))
            {
               if (device)
               {
                  if (suffix && !strcmp(suffix, "upgrade") && j_find(j, "complete"))
                     sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `upgrade`=NULL WHERE `device`=%#s", deviceid));  // Upgrade done
               }
            } else
               return "Unknown message";
            if (j)
            {
               long long l = slot_linked(instance);
               if (l)
                  mqtt_send(l, prefix, suffix, &j);     // Send to linked session
            }
            return NULL;
         }
         const char *fail = process();
         if (device)
            sql_free_result(device);
         if (fail)
            warnx("Failed to process message: %s", fail);
         j_delete(&j);
         j_delete(&meta);
      }
   }
   sql_close(&sql);
   sql_close(&sqlkey);
   return 0;
}
