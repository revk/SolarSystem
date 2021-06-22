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
   }
   syslog(LOG_INFO, "Starting");
   mqtt_start();
   while (1)
      sleep(1);
   sql_close(&sql);
   return 0;
}
