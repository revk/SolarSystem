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
#include "ssdatabase.h"
#include "ssafile.h"
#include "fobcommand.h"

const char *cakey = "",
    *cacert = "";
const char *mqttkey = "",
    *mqttcert = "";

extern int sqldebug;
int dump = 0;                   // dump level debug
int mqttdump = 0;               // mqtt logging

const char *settings(SQL * sqlp, SQL_RES * res, slot_t id);
const char *security(SQL * sqlp, SQL * sqlkeyp, SQL_RES * res, slot_t id);

#define AES_STRING_LEN	35
char *getaes(SQL * sqlkeyp, char *target, const char *aid, const char *fob)
{                               // Get AES key (HEX ver and AES, so AES_STRING_LEN byte buffer)
   SQL_RES *res = sql_safe_query_store_free(sqlkeyp, sql_printf("SELECT * FROM `AES` WHERE `aid`=%#s AND `fob`=%#s ORDER BY `created` DESC LIMIT 1", aid ? : "", fob ? : ""));
   if (sql_fetch_row(res))
   {
      snprintf(target, AES_STRING_LEN, "%s%s", sql_col(res, "ver"), sql_col(res, "key"));
      return target;
   }
   unsigned char bin[17];
   randkey(bin);
   j_base16N(17, bin, AES_STRING_LEN, target);
   if (sql_query_free(sqlkeyp, sql_printf("INSERT INTO `AES` SET `aid`=%#s,`fob`=%#s,`ver`=%#.2s,`key`=%#s", aid ? : "", fob ? : "", target, target + 2)))
      *target = 0;
   return target;
}

const char *upgrade(SQL_RES * res, slot_t id)
{                               // Send upgrade if needed
   const char *upgrade = sql_col(res, "upgrade");
   if (!upgrade || j_time(upgrade) > time(0))
      return NULL;
   slot_send(id, "command", "upgrade", NULL);
   return upgrade;
}

const char *security(SQL * sqlp, SQL * sqlkeyp, SQL_RES * res, slot_t id)
{                               // Security settings
   j_t j = j_create();
   const char *aid = sql_colz(res, "aid");
   char nfc = (*sql_colz(res, "nfc") == 't');
   j_t aids = j_store_array(j, "aes");
   if (*aid && nfc)
   {                            // Security
      // Keys
      SQL_RES *r = sql_safe_query_store_free(sqlkeyp, sql_printf("SELECT * FROM `AES` WHERE `aid`=%#s AND `fob`='' order BY `created` DESC LIMIT 3", aid));
      while (sql_fetch_row(r))
         j_append_stringf(aids, "%s%s", sql_colz(r, "ver"), sql_colz(r, "key"));
      sql_free_result(r);
      if (!j_len(aids))
         j_append_string(aids, getaes(sqlkeyp, alloca(AES_STRING_LEN), aid, NULL));     // Make a key
   }
   if (!j_len(aids))
      aid = "";                 // We have not loaded any keys so no point in even trying an AID
   j_store_string(j, "aid", aid);
   j_t blacklist = j_store_array(j, "blacklist");
   if (*aid)
   {
      int organisation = atoi(sql_colz(res, "organisation"));
      SQL_RES *b = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `foborganisation` WHERE `organisation`=%d AND `blocked` IS NOT NULL AND `confirmed` IS NULL ORDER BY `blocked` DESC LIMIT 10", organisation));
      while (sql_fetch_row(b))
         j_append_string(blacklist, sql_colz(b, "fob"));
      sql_free_result(b);
   }
   if (!j_isnull(j))
      slot_send(id, "setting", NULL, &j);
   j_delete(&j);
   return NULL;
}

static void addwifi(j_t j, SQL_RES * s, const char *deviceid)
{
   const char *v = sql_colz(s, "device");
   if (!*v || strcmp(v, deviceid))
   {
      j_t ap = j_store_object(j, "ap");
      if (*v)
      {                         // we are root node as AP
         if ((v = sql_colz(s, "apssid")) && *v)
            j_store_string(ap, "ssid", v);
         if ((v = sql_colz(s, "appass")) && *v)
            j_store_string(ap, "pass", v);
         if ((v = sql_colz(s, "aplr")) && *v && *v == 't')
            j_store_string(ap, "lr", v);
      }
      j_t wifi = j_store_object(j, "wifi");
      if ((v = sql_colz(s, "wifissid")) && *v)
         j_store_string(wifi, "ssid", v);
      if ((v = sql_colz(s, "wifipass")) && *v)
         j_store_string(wifi, "pass", v);
   } else
   {                            // We are client or root node
      j_t ap = j_store_object(j, "ap");
      j_t wifi = j_store_object(j, "wifi");
      if ((v = sql_colz(s, "apssid")) && *v)
         j_store_string(wifi, "ssid", v);
      if ((v = sql_colz(s, "appass")) && *v)
         j_store_string(wifi, "pass", v);
      j_store_string(wifi,"mqtt",deviceid);
      if ((v = sql_colz(s, "aplr")) && *v && *v == 't')
         j_store_string(ap, "lr", v);
   }
}

const char *settings(SQL * sqlp, SQL_RES * res, slot_t id)
{                               // Send base settings
   j_t j = j_create();
   int door = (*sql_colz(res, "door") == 't');
   j_store_string(j, "name", sql_colz(res, "devicename"));
   if (*CONFIG_OTA_HOSTNAME)
      j_store_string(j, "otahost", CONFIG_OTA_HOSTNAME);
   j_store_int(j, "doorauto", door ? 5 : 0);
   int site = atoi(sql_colz(res, "site"));
   {                            // site
      SQL_RES *s = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `site` WHERE `site`=%d", site));
      if (sql_fetch_row(s))
      {
         addwifi(j, s, sql_colz(res, "device"));
         const char *host = sql_colz(s, "iothost");
         j_t iot = j_store_object(j, "iot");
         if (*host)
         {
            j_store_string(iot, "host", host);
            if (*sql_colz(res, "iotstatedoor") == 't')
               j_store_true(iot, "statedoor");
            if (*sql_colz(res, "iotstateinput") == 't')
               j_store_true(iot, "stateinput");
            if (*sql_colz(res, "iotstateoutput") == 't')
               j_store_true(iot, "stateoutput");
            if (*sql_colz(res, "iotstatefault") == 't')
               j_store_true(iot, "statefault");
            if (*sql_colz(res, "iotstatetamper") == 't')
               j_store_true(iot, "statetamper");
            if (*sql_colz(res, "ioteventfob") == 't')
               j_store_true(iot, "eventfob");
         }
      }
      sql_free_result(s);
   }
   int pcb = atoi(sql_colz(res, "pcb"));
   if (pcb)
   {                            // Main PCB settings
      SQL_RES *p = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `pcb` WHERE `pcb`=%d", pcb));
      if (sql_fetch_row(p))
      {
#define set(n) {const char *v=sql_colz(p,#n);if(!*v||!strcmp(v,"-"))j_store_string(j,#n,""); else j_store_literal(j,#n,v);}
         set(tamper);
#undef set
         j_t o = j_store_object(j, "keypad");
#define set(h,c) {const char *v=sql_colz(p,#h#c);if(v&&strcmp(v,"-"))j_store_literal(o,#c,v);}
         set(keypad, tx);
         set(keypad, rx);
         set(keypad, re);
         set(keypad, de);
         o = j_store_object(j, "nfc");
         set(nfc, tx);
         set(nfc, rx);
         set(nfc, power);
         set(nfc, red);
         set(nfc, amber);
         set(nfc, green);
         set(nfc, tamper);
         set(nfc, bell);
         set(nfc, card);
#undef set
         j_t blink = j_store_array(j, "blink");
#define led(n) {const char *v=sql_colz(p,#n);if(!*v||!strcmp(v,"-"))j_append_string(blink,""); else j_append_literal(blink,v);}
         if (strcmp(sql_colz(p, "leda"), "-"))
         {
            led(leda);
         } else
         {
            led(ledr);
            led(ledg);
            led(ledb);
         }
#undef led
      }
      sql_free_result(p);
      int i = (door ? 4 : 0),
          o = (door ? 4 : 0);
      j_t input = j_store_array(j, "input");
      j_t output = j_store_array(j, "output");
      j_t power = j_store_array(j, "power");
      SQL_RES *g = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `devicegpio` LEFT JOIN `gpio` USING (`gpio`) WHERE `device`=%#s", sql_col(res, "device")));
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
            const char *pin = sql_colz(g, "pin");
            int invert = (*sql_colz(g, "invert") == 't');
            if (*pin == '-')
            {
               pin++;
               invert = 1 - invert;
            }
            if (*pin)
            {
               j_store_literalf(gpio, "gpio", "%s%s", (invert ? "-" : ""), pin);
               void addstate(const char *state) {
                  const char *areas = sql_colz(g, state);
                  if (*areas)
                  {             // TODO do we allow commas?
                     j_store_string(gpio, state, areas);
                  }
               }
#define i(n) addstate(#n);
#define s(n) i(n)
#include "ESP32/main/states.m"
            }
         }
      }
      sql_free_result(g);
   }
   if (!j_isnull(j))
      slot_send(id, "setting", NULL, &j);
   j_delete(&j);
   return NULL;
}

void bogus(slot_t id)
{                               // This is bogus auth
   j_t m = j_create();
   j_store_string(m, "clientkey", "");
   j_store_string(m, "clientcert", "");
   slot_send(id, "setting", NULL, &m);
}

void daily(SQL * sqlp)
{                               // Daily tasks and clean up
   sql_safe_query(sqlp, "DELETE FROM `session` WHERE `expires`<NOW()"); // Old sessions
}

void dopoke(SQL * sqlp, SQL * sqlkeyp)
{                               // Poking that may need doing
   SQL_RES *res = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `device` WHERE `poke` IS NOT NULL AND `id` IS NOT NULL"));
   while (sql_fetch_row(res))
   {
      sql_safe_query_free(sqlp, sql_printf("UPDATE `device` SET `poke`=NULL WHERE `device`=%#s", sql_col(res, "device")));
      slot_t id = strtoull(sql_colz(res, "id"), NULL, 10);
      settings(sqlp, res, id);
      security(sqlp, sqlkeyp, res, id);
   }
}

const char *forkcommand(j_t * jp, slot_t device, slot_t local)
{
   j_t j = *jp;
   *jp = 0;
   int relaysock;
   slot_t id = slot_create(&relaysock, "local");
   if (!id)
      return "Link failed";
   slot_link(device, id);
   j_store_int(j, "id", id);
   if (device)
      j_store_int(j, "device", device);
   if (local)
      j_store_int(j, "local", local);
   j_store_int(j, "socket", relaysock);
   pthread_t t;
   if (pthread_create(&t, NULL, fobcommand, j))
      err(1, "Cannot create fob adopt thread");
   pthread_detach(t);
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
         { "mqtt", 'm', POPT_ARG_NONE, &mqttdump, 0, "Debug mqtt", NULL },
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
   if (!sqldebug && !mqttdump && !dump)
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
      {
         cakey = makekey();
         cacert = "";           // Certs no longer valid if key changes
         mqttcert = "";
      }
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
      warnx("New " CONFIG_MSG_KEY_FILE " may need group change for www-data");
   }
   // Connect
   sstypes("types");
   SQL sqlkey;
   sql_cnf_connect(&sqlkey, CONFIG_SQL_KEY_CONFIG_FILE);
   sskeydatabase(&sqlkey);
   SQL sql;
   sql_cnf_connect(&sql, CONFIG_SQL_CONFIG_FILE);
   ssdatabase(&sql);
   syslog(LOG_INFO, "Starting");
   sql_safe_query(&sql, "DELETE FROM `pending` WHERE `id` IS NOT NULL");
   sql_safe_query(&sql, "UPDATE `device` SET `id`=NULL,`online`=NULL WHERE `id` IS NOT NULL");
   mqtt_start();
   // Main loop getting messages (from MQTT or websocket)
   int poke = 1;
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
      if (poke)
      {
         poke = 0;
         dopoke(&sql, &sqlkey);
      }
      j_t j = incoming();
      if (!j)
         warnx("WTF");
      else
      {
         if (dump)
            j_err(j_write_pretty(j, stderr));
         SQL_RES *device = NULL;
         const char *deviceid = NULL;
         SQL_RES *checkdevice(void) {
            if (device || !deviceid || *deviceid == '-')
               return device;
            device = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", deviceid));
            if (!sql_fetch_row(device))
            {
               sql_free_result(device);
               device = NULL;
               deviceid = NULL;
            }
            return device;
         }
         j_t meta = j_find(j, "_meta");
         if (meta)
            meta = j_detach(meta);
         char forked = 0;
         const char *local(slot_t local) {      // Commands sent to us from local system
            slot_t id = 0;      // Device id
            const char *v;
            {                   // Identify the device we want to talk to...
               SQL_RES *res = NULL;
               if ((v = j_get(meta, "device")))
                  res = sql_safe_query_store_free(&sql, sql_printf("SELECT `id` FROM `device` WHERE `device`=%#s", v));
               else if ((v = j_get(meta, "pending")))
                  res = sql_safe_query_store_free(&sql, sql_printf("SELECT `id` FROM `pending` WHERE `pending`=%#s", v));
               if (res)
               {                // Check device on line, find id
                  deviceid = v;
                  if (sql_fetch_row(res))
                     id = strtoll(sql_colz(res, "id") ? : "", NULL, 10);
                  else
                     sql_free_result(res);
                  if (!id)
                     return "Device not on line";
               }
            }
            if (j_find(meta, "poke"))
               poke = 1;
            else if (j_find(meta, "provision") && deviceid)
            {                   // JSON is rest of settings to send
               char *key = makekey();
               char *cert = makecert(key, cakey, cacert, v);
               j_t j = j_create();
               j_store_string(j, "clientcert", cert);
               free(cert);
               const char *fail = slot_send(id, "setting", NULL, &j);
               j = j_create();
               j_store_string(j, "clientkey", key);
               free(key);
               if (fail)
                  return fail;
               fail = slot_send(id, "setting", NULL, &j);
               const char *aid = j_get(meta, "aid");
               if (aid && !fail)
               {
                  SQL_RES *s = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `aid` LEFT JOIN `site` USING (`site`) WHERE `aid`=%#s", aid));
                  if (sql_fetch_row(s))
                  {
                     addwifi(j, s, deviceid);
                     fail = slot_send(id, "setting", NULL, &j);
                  }
                  sql_free_result(s);
               }
               // Set online later to remove from pending lists in UI
               if (!fail)
                  sql_safe_query_free(&sql, sql_printf("UPDATE `pending` SET `online`=%#T WHERE `pending`=%#s", time(0) + 60, deviceid));
               return fail;
            } else if ((v = j_get(meta, "deport")))
            {
               j_store_object(j, "client");     // Clear client
               j_t mqtt = j_store_object(j, "mqtt");
               j_store_string(mqtt, "host", v);
               j_store_string(mqtt, "cert", "");        // Cleared as typically in factory default
               const char *fail = slot_send(id, "setting", NULL, &j);
               // Set online later to remove from pending lists in UI
               if (!fail)
                  sql_safe_query_free(&sql, sql_printf("UPDATE `pending` SET `online`=%#T WHERE `pending`=%#s", time(0) + 60, deviceid));
               return fail;
            } else if ((v = j_get(meta, "prefix")))
            {                   // Send to device
               const char *suffix = j_get(meta, "suffix");
               if (!id)
                  return "No id";

               const char *fail = NULL;
               j_t data = j_find(j, "_data");
               if (data)
                  slot_send(id, v, suffix, &data);
               else
                  slot_send(id, v, suffix, &j);
               return fail;
            } else if (j_find(meta, "fobprovision"))
            {
               j_t init = j_create();
               if (j_find(meta, "format"))
                  j_store_true(init, "format"); // Format as well
               if (j_find(meta, "hardformat"))
                  j_store_true(init, "hardformat");     // Format as well
               j_store_true(init, "provision");
               const char *aid = j_get(meta, "aid");
               if (aid)
               {                // Adopt as well
                  j_store_true(init, "adopt");
                  j_store_string(init, "aid", aid);
                  char temp[AES_STRING_LEN];
                  // Not sending aid0key means has to finish else not going to adopt later
                  j_store_string(init, "aid1key", getaes(&sqlkey, temp, aid, NULL));
               }
               j_store_int(init, "device", j_get(meta, "device"));
               forked = 1;
               return forkcommand(&init, id, local);
            } else
               return "Unknown local request";
            return NULL;
         }
         const char *loopback(void) {   // From linked
            const char *v;
            const char *fob = j_get(j, "fob");
            const char *aid = j_get(j, "aid");
            if (fob)
               sql_safe_query_free(&sql, sql_printf("INSERT IGNORE INTO `fob` SET `fob`=%#s,`provisioned`=NOW()", fob));
            if ((v = j_get(j, "mem")) && fob)
               sql_safe_query_free(&sql, sql_printf("UPDATE `fob` SET `mem`=%#s WHERE `fob`=%#s AND `mem`!=%#s", v, fob, v));
            if (j_find(meta, "provisioned") && fob && (v = j_get(j, "masterkey")))
            {
               sql_safe_query_free(&sqlkey, sql_printf("REPLACE INTO `AES` SET `fob`=%#s,`aid`='',`ver`=%#.2s,`key`=%#s", fob, v, v + 2));
               if (aid && (v = j_get(j, "aid0key")))
                  sql_safe_query_free(&sqlkey, sql_printf("REPLACE INTO `AES` SET `fob`=%#s,`aid`=%#s,`ver`=%#.2s,`key`=%#s", fob, aid, v, v + 2));
            }
            if (j_find(meta, "adopted") && fob && aid)
            {
               if ((v = j_get(j, "aid0key")))
                  sql_safe_query_free(&sqlkey, sql_printf("REPLACE INTO `AES` SET `fob`=%#s,`aid`=%#s,`ver`=%#.2s,`key`=%#s", fob, aid, v, v + 2));
               if ((v = j_get(j, "organisation")))
                  sql_safe_query_free(&sql, sql_printf("INSERT IGNORE INTO `foborganisation` SET `fob`=%#s,`organisation`=%s", fob, v));
               sql_safe_query_free(&sql, sql_printf("INSERT IGNORE INTO `fobaid` SET `fob`=%#s,`aid`=%#s,`adopted`=NOW()", fob, aid));
               sql_safe_query_free(&sql, sql_printf("UPDATE `fobaid` SET `adopted`=NOW() WHERE `fob`=%#s AND `aid`=%#s", fob, aid));
               if ((v = j_get(j, "deviceid")))
                  sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `adoptnext`='false' WHERE `device`=%#s", v));
            }
            if (j_find(meta, "formatted") && fob)
            {
               sql_safe_query_free(&sql, sql_printf("DELETE FROM `fobaid` WHERE `fob`=%#s", fob));
               if ((v = j_get(j, "deviceid")))
                  sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `formatnext`='false' WHERE `device`=%#s", v));
            }
            return NULL;
         }
         const char *process(void) {
            if (!meta)
               return "No meta data";
            if (j_find(meta, "loopback"))
               return loopback();
            if (j_find(meta, "local"))
            {
               slot_t id = strtoull(j_get(meta, "local") ? : "", NULL, 10);
               const char *reply = local(id);
               if (id)
               {                // Send response
                  if (reply)
                  {             // Send reply
                     j_t j = j_create();
                     j_string(j, reply);
                     slot_send(id, NULL, NULL, &j);     // reply
                  }
                  if (!forked)
                  {             // Tell command we are closed
                     j_t j = j_create();
                     slot_send(id, NULL, NULL, &j);
                  }
               }
               return reply;
            }

            slot_t message = strtoull(j_get(meta, "message") ? : "", NULL, 10);
            slot_t id = strtoull(j_get(meta, "id") ? : "", NULL, 10);
            if (!id)
               return "No id";
            deviceid = j_get(meta, "device");
            const char *address = j_get(meta, "address");
            const char *prefix = j_get(meta, "prefix");
            const char *suffix = j_get(meta, "suffix");
            if (!message && (!deviceid || *deviceid != '-'))
            {                   // Connect (first message ID 0) - *MUST* be a top level state message unless local
               const char *claimedid = j_get(j, "id");
               if (!claimedid)
                  return "No id";
               if (!prefix || strcmp(prefix, "state") || suffix || !j_find(j, "up"))
                  return "Bad initial message";
               if (deviceid)
               {
                  const char *claimedid = j_get(j, "id");
                  if (claimedid && strcmp(claimedid, deviceid))
                     bogus(id);
               }
               sql_string_t s = { };
               if (checkdevice())
               {
                  sql_sprintf(&s, "UPDATE `device` SET ");      // known, update
                  sql_sprintf(&s, "`lastonline`=NOW(),`poke`=NULL,");
                  if (!upgrade(device, id) && !settings(&sql, device, id))
                     security(&sql, &sqlkey, device, id);
               } else           // pending - update pending
               {
                  sql_sprintf(&s, "REPLACE INTO `pending` SET ");
                  sql_sprintf(&s, "`pending`=%#s,", claimedid);
                  if (deviceid)
                     sql_sprintf(&s, "`authenticated`=%#s,", "true");   // TODO zapping
                  SQL_RES *res = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", claimedid));
                  if (sql_fetch_row(res))
                     upgrade(res, id);
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
               sql_sprintf(&s, "`id`=%lld,", id);
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
               if (checkdevice())
               {                // known
                  slot_t i = strtoull(sql_colz(device, "id"), NULL, 10);
                  if (i == id)
                     sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `online`=NULL,`id`=NULL,`lastonline`=NOW() WHERE `device`=%#s AND `id`=%lld", deviceid, id));
               } else           // pending
                  sql_safe_query_free(&sql, sql_printf("DELETE FROM `pending` WHERE `id`=%lld", id));
               return NULL;
            }
            if (prefix && !strcmp(prefix, "state"))
            {                   // State
               if (suffix && (!strcmp(suffix, "fault") || !strcmp(suffix, "tamper")) && checkdevice())
               {
                  char *buf;
                  size_t len;
                  j_err(j_write_mem(j, &buf, &len));
                  if (strcmp(buf, sql_colz(device, suffix)))
                     sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `%#S`=%#s WHERE `device`=%#s", suffix, buf, deviceid));
                  free(buf);
               }
            } else if (prefix && !strcmp(prefix, "event"))
            {
               if (!strcmp(suffix, "fob") && checkdevice())
               {                // Fob usage - loads of options
                  int organisation = atoi(sql_colz(device, "organisation"));
                  const char *aid = sql_colz(device, "aid");
                  const char *fobid = j_get(j, "id");
                  char held = j_test(j, "held", 0);
                  char gone = j_test(j, "gone", 0);
                  char secure = j_test(j, "secure", 0);
                  char block = j_test(j, "block", 0);
                  char remote = j_test(j, "remote", 0);
                  char updated = j_test(j, "updated", 0);
                  char blacklist = j_test(j, "blacklist", 0);
                  if (!held && !gone && !remote)
                  {             // Initial fob use
                     SQL_RES *fa = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `fobaid` LEFT JOIN `foborganisation` USING (`fob`) LEFT JOIN `access` USING (`access`) WHERE `fob`=%#s AND `aid`=%#s", fobid, aid));
                     if (!sql_fetch_row(fa))
                     {
                        sql_free_result(fa);
                        fa = NULL;
                     }
                     if ((block || (updated && blacklist)) && secure)
                     {          // Confirm blocked
                        sql_safe_query_free(&sql, sql_printf("UPDATE `foborganisation` SET `confirmed`=NOW() WHERE `organisation`=%d AND `fob`=%#s AND `confirmed` IS NULL", organisation, fobid));
                        if (sql_affected_rows(&sql))
                        {
                           sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `poke`=NOW() WHERE `organisation`=%d", organisation));
                           poke = 1;
                        }
                     }
                     if (secure)
                     {
                        if (fa)
                        {
                           const char *ver = j_get(j, "ver");
                           if (ver && strcmp(ver, sql_colz(fa, "ver")))
                              sql_safe_query_free(&sql, sql_printf("UPDATE `fobaid` SET `ver`=%#s WHERE `fob`=%#s AND `aid`=%#s", ver, fobid, aid));
                           if (!sql_col(fa, "adopted"))
                              sql_safe_query_free(&sql, sql_printf("UPDATE `fobaid` SET `adopted`=NOW() WHERE `fob`=%#s AND `aid`=%#s", fobid, aid));
                        }
                        // Check afile
                        const char *crc = j_get(j, "crc");
                        if (crc)
                        {       // Check afile
                           unsigned was = strtoull(crc, NULL, 16);
                           unsigned char afile[256] = { };
                           unsigned new = makeafile(fa, afile);
                           if (was != new)
                           {    // Send afile
                              if (sqldebug)
                                 warnx("CRC mismatch %08X %08X", was, new);
                              j_t a = j_create();
                              j_string(a, j_base16a(*afile + 1, afile));
                              slot_send(id, "command", "access", &a);
                           }
                        }
                     }
                     if ((!secure && ((*sql_colz(device, "adoptnext") == 't') || (fa && !sql_col(fa, "adopted")))) || (*sql_colz(device, "formatnext") == 't'))
                     {          // Create fob record if necessary, if we have a key
                        char masterkey[AES_STRING_LEN] = "";
                        if (!fa)
                        {       // Do we know the key, if so, we can add to this aid now
                           SQL_RES *res = sql_safe_query_store_free(&sqlkey, sql_printf("SELECT * FROM `AES` WHERE `fob`=%#s AND `aid`='' ORDER BY `created` DESC LIMIT 1", fobid));
                           if (sql_fetch_row(res))
                           {    // We know this fob...
                              snprintf(masterkey, sizeof(masterkey), "%s%s", sql_colz(res, "ver"), sql_colz(res, "key"));
                              sql_safe_query_free(&sql, sql_printf("INSERT IGNORE INTO `fob` SET `fob`=%#s", fobid));
                              sql_safe_query_free(&sql, sql_printf("INSERT INTO `fobaid` SET `fob`=%#s,`aid`=%#s", fobid, aid));
                              fa = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `fobaid` LEFT JOIN `foborganisation` USING (`fob`) LEFT JOIN `access` USING (`access`) WHERE `fob`=%#s AND `aid`=%#s", fobid, aid));
                              sql_fetch_row(fa);
                           }
                           sql_free_result(res);
                        }
                        if (!*masterkey)
                           getaes(&sqlkey, masterkey, NULL, fobid);
                        if (*sql_colz(device, "formatnext") == 't')
                        {       // Format a fob (even if secure)
                           j_t init = j_create();
                           j_store_true(init, sqldebug ? "hardformat" : "format");
                           j_store_string(init, "masterkey", masterkey);
                           j_store_int(init, "device", id);
                           j_store_string(init, "deviceid", deviceid);
                           j_store_string(init, "fob", fobid);
                           forkcommand(&init, id, 0);
                        } else if (fa && !secure && ((*sql_colz(device, "adoptnext") == 't') || (fa && !sql_col(fa, "adopted"))))
                        {       // Adopt
                           if (sql_col(fa, "adopted"))
                              sql_safe_query_free(&sql, sql_printf("UPDATE `fobaid` SET `adopted`=NULL WHERE `fob`=%#s AND `aid`=%#s", fobid, aid));
                           unsigned char afile[256] = { };
                           makeafile(fa, afile);
                           j_t init = j_create();
                           j_store_true(init, "adopt");
                           j_store_string(init, "afile", j_base16a(*afile + 1, afile));
                           j_store_string(init, "fob", fobid);
                           j_store_int(init, "organisation", organisation);
                           j_store_string(init, "aid", aid);
                           j_store_string(init, "deviceid", deviceid);
                           char temp[AES_STRING_LEN];
                           j_store_string(init, "masterkey", masterkey);
                           j_store_string(init, "aid0key", getaes(&sqlkey, temp, aid, fobid));
                           j_store_string(init, "aid1key", getaes(&sqlkey, temp, aid, NULL));
                           j_store_int(init, "device", id);
                           forkcommand(&init, id, 0);
                        }
                     }
                     if (fa)
                        sql_free_result(fa);
                  }
               }
            } else if (prefix && !strcmp(prefix, "error"))
            {
            } else if (prefix && !strcmp(prefix, "info"))
            {
               if (suffix && !strcmp(suffix, "upgrade") && j_find(j, "complete") && checkdevice())
                  sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `upgrade`=NULL WHERE `device`=%#s", deviceid));     // Upgrade done
            } else
               return "Unknown message";
            if (j)
            {
               slot_t l = slot_linked(id);
               if (l)
                  slot_send(l, prefix, suffix, &j);     // Send to linked session
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
