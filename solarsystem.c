// Back end management / control for SolarSystem modules

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
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

void ssdatabase(SQL *, const char *);

// System wide settings, mostly taken from config file - these define defaults as well
extern int sqldebug;
const char *configfile = "solarsystem.conf";
#define s(p,n,d,h)	const char *p##n=#d;
#define i(p,n,d,h)	int p##n=d;
#include "ssconfig.h"

const char *deport(SQL_RES * res, long long instance)
{                               // Check if deport needed
   const char *deport = sql_col(res, "deport");
   if (!deport || !*deport)
      return NULL;
   j_t m = j_create();
   j_store_string(m, "clientkey", "");
   j_store_string(m, "clientcert", "");
   j_store_string(m, "mqttcert", "");
   j_store_string(m, "mqtthost", deport);
   setting(instance, NULL, &m);
   return deport;
}

const char *upgrade(SQL_RES * res, long long instance)
{                               // Send upgrade if needed
   const char *upgrade = sql_col(res, "upgrade");
   if (!upgrade || j_time(upgrade) > time(0))
      return NULL;
   command(instance, "upgrade", NULL);
   return upgrade;
}

void bogus(long long instance)
{                               // This is bogus auth
   j_t m = j_create();
   j_store_string(m, "clientkey", "");
   j_store_string(m, "clientcert", "");
   setting(instance, NULL, &m);
}


int main(int argc, const char *argv[])
{
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         { "string-default", 'S', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &configfile, 0, "Config file", "filename" },
         { "debug", 'v', POPT_ARG_NONE, &sqldebug, 0, "Debug", NULL },
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
   umask(0077);                 // Owner only
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
   SQL sql;
   {                            // Load config file and extract settings
      const char *changed = NULL;
      j_t j = j_create();
      if (access(configfile, R_OK | W_OK))
      {
         j_object(j);
         changed = "";
      } else
         j_err(j_read_file(j, configfile));
#define s(p,n,d,h) {j_t e=j_find(j,#p"."#n);if(e){if(!j_isstring(e))errx(1,#p"."#n" should be a string");p##n=strdup(j_val(e)?:"");}}
#define i(p,n,d,h) {j_t e=j_find(j,#p"."#n);if(e){if(!j_isnumber(e))errx(1,#p"."#n" should be a number");p##n=atoi(j_val(e));}}
#include "ssconfig.h"
      // Some housekeeping
      sql_cnf_connect(&sql, *sqlconfig ? sqlconfig : NULL);
      ssdatabase(&sql, sqldatabase);    // Check database integrity
      if (!*cakey)
         j_string(j_path(j, "ca.key"), changed = cakey = makekey());
      if (!*mqttkey)
         j_string(j_path(j, "mqtt.key"), changed = mqttkey = makekey());
      // May as well make certs anyway
      j_string(j_path(j, "ca.cert"), changed = cacert = makecert(cakey, NULL, NULL, "SolarSystem"));
      j_string(j_path(j, "mqtt.cert"), changed = mqttcert = makecert(mqttkey, cakey, cacert, mqtthost));
      // Update config file if needed
      if (changed)
      {
         char *temp;
         if (asprintf(&temp, "%s+", configfile) < 0)
            errx(1, "malloc");
         FILE *f = fopen(temp, "w");
         if (!f)
            err(1, "Cannot write config file %s", configfile);
         j_err(j_write_pretty_close(j, f));
         if (rename(temp, configfile))
            err(1, "Cannot make config file");
         free(temp);
      }
      j_delete(&j);
   }
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
   // TODO start websocket (proxy from apache)
   // Main loop getting messages (from MQTT or websocket)
   while (1)
   {
      j_t j = incoming();
      if (!j)
         warnx("WTF");
      else
      {
         if (sqldebug)
            j_err(j_write_pretty(j, stderr));
         SQL_RES *device = NULL;
         const char *process(void) {
            j_t meta = j_find(j, "_meta");
            if (!meta)
               return "No meta data";

            long long message = strtoull(j_get(meta, "message") ? : "", NULL, 10);
            long long instance = strtoull(j_get(meta, "instance") ? : "", NULL, 10);
            if (!instance)
               return "No instance";
            const char *deviceid;
            if ((deviceid = j_get(meta, "device")) && *deviceid && (device = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", deviceid))) && !sql_fetch_row(device))
            {                   // Not found - new device
               sql_free_result(device);
               sql_safe_query_free(&sql, sql_printf("INSERT INTO `device` SET `device`=%#s", deviceid));
               device = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", deviceid));
               sql_fetch_row(device);;
            }
            const char *address = j_get(meta, "address");
            const char *prefix = j_get(meta, "prefix");
            const char *suffix = j_get(meta, "suffix");
            if (!message)
            {                   // Connect (first message ID 0)
               if (device)
               {                // known
                  long long i = strtoull(sql_colz(device, "instance"), NULL, 10);
                  if (i != instance)
                     sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `online`=NOW(),`lastonline`=NOW(),`instance`=%lld,`address`=%#s WHERE `device`=%#s", instance, address, deviceid));
                  if (deport(device, instance))
                     return NULL;
                  if (upgrade(device, instance))
                     return NULL;
                  // TODO settings update (should this included deport?)
               } else           // pending
               {
                  const char *id = j_get(j, "id");
                  if (!id)
                     return NULL;
                  sql_safe_query_free(&sql, sql_printf("REPLACE INTO `pending` SET `pending`=%#s,`online`=NOW(),`address`=%#s,`instance`=%lld", id, address, instance));
                  SQL_RES *res = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `device` WHERE `device`=%#s", id));
                  if (sql_fetch_row(res))
                  {
                     if (!upgrade(res, instance))
                        deport(res, instance);
                  }
                  sql_free_result(res);
               }
               // Continue as the message could be anything
            }
            if (!prefix)
            {                   // Down (all other messages have a topic)
               if (device)
               {                // known
                  long long i = strtoull(sql_colz(device, "instance"), NULL, 10);
                  if (i == instance)
                     sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `online`=NULL,`instance`=NULL,`lastonline`=NOW() WHERE `device`=%#s AND `instance`=%lld", deviceid, instance));
               } else           // pending
                  sql_safe_query_free(&sql, sql_printf("DELETE FROM `pending` WHERE `instance`=%lld", instance));
               return NULL;
            }
            if (prefix && !strcmp(prefix, "state"))
            {                   // State
               if (!device)
                  return NULL;  // Not authenticated
               if (!suffix)
               {                // System level
                  const char *id = j_get(j, "id");
                  if (id && strcmp(id, deviceid))
                     bogus(instance);
                  return NULL;
               }
               return NULL;
            }
            if (prefix && !strcmp(prefix, "event"))
            {
               if (!device)
                  return NULL;  // Not authenticated
               return NULL;
            }
            if (prefix && !strcmp(prefix, "error"))
            {
               if (!device)
                  return NULL;  // Not authenticated
               return NULL;
            }
            if (prefix && !strcmp(prefix, "info"))
            {
               if (!device)
                  return NULL;  // Not authenticated
               if (suffix && !strcmp(suffix, "upgrade") && j_find(j, "complete"))
                  sql_safe_query_free(&sql, sql_printf("UPDATE `device` SET `upgrade`=NULL WHERE `device`=%#s", deviceid));     // Upgrade done

               return NULL;
            }
            return "Unknown message";
         }
         const char *fail = process();
         if (device)
            sql_free_result(device);
         if (fail)
            warnx("Failed to process message: %s", fail);
         j_delete(&j);
      }
   }
   sql_close(&sql);
   return 0;
}
