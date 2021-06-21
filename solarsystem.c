// Back end management / control for SolarSystem modules

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
   umask(0007);
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
      // TODO new certs every time maybe?
      if (!*cacert||sqldebug)
         j_string(j_path(j, "ca.cert"), changed = cacert = makecert(cakey, NULL, NULL, "SolarSystem"));
      if (!*mqttcert||sqldebug)
         j_string(j_path(j, "mqtt.cert"), changed = mqttcert = makecert(mqttkey, cakey, cacert, mqtthost));
      // Update config file if needed
      if (changed)
      {
         FILE *f = fopen(configfile, "w");
         if (!f)
            err(1, "Cannot write config file %s", configfile);
         j_err(j_write_pretty_close(j, f));
      }
      j_delete(&j);
   }
   if (sqldebug)
   {
      char *key=makekey();
      char *cert=makecert(key,cakey,cacert,"112233445566");
      printf("CA cert:\n%s\nTest key:\n%s\nTest cert:\n%s\n", cacert,key,cert);
   }

   mqtt_start();
   while (1)
      sleep(1);
   sql_close(&sql);
   return 0;
}
