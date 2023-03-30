// Make an AID

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "AJL/ajl.h"
#include "SQLlib/sqllib.h"
#include "login/redirect.h"

int main(int argc, const char *argv[])
{
#ifdef CONFIG_SQL_DEBUG
   sqldebug = 1;
#endif
   int silent = 0;
   int tries = 100;
   int site = 0;
   const char *name = "Default AID";
   char aid[7] = "";
   {
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         { "site", 'o', POPT_ARG_INT, &site, 0, "Site ID", "N" },
         { "tries", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &tries, 0, "Tries", "N" },
         { "name", 'd', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &name, 0, "Description", "text" },
         { "silent", 'q', POPT_ARG_NONE, &silent, 0, "Silent", NULL },
         { "debug", 'v', POPT_ARG_NONE, &sqldebug, 0, "Debug", NULL },
         POPT_AUTOHELP { }
      };

      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);

      int c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));

      if (poptPeekArg(optCon) || !site)
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }

      poptFreeContext(optCon);
   }

   SQL sql;
   sql_cnf_connect(&sql, CONFIG_SQL_WEBUSER_FILE);
   if (*CONFIG_SQL_DATABASE)
      sql_safe_select_db(&sql, CONFIG_SQL_DATABASE);
   int f = open("/dev/urandom", O_RDONLY);
   if (f < 0)
      err(1, "Cannot open /dev/urandom");
   SQL_RES *res = sql_safe_query_store_free(&sql, sql_printf("SELECT * FROM `site` WHERE `site`=%d", site));
   if (!sql_fetch_row(res))
      errx(1, "Unknown site");
   while (tries--)
   {
      unsigned char bin[3];
      if (read(f, bin, sizeof(bin)) != sizeof(bin))
         err(1, "Cannot read random");
      if (!bin[0] && !bin[1] && bin[2])
         continue;              // cannot be 0
      sprintf(aid, "%02X%02X%02X", bin[0], bin[1], bin[2]);
      if (sql_query_free(&sql, sql_printf("INSERT INTO `aid` SET `aid`=%#s,`site`=%d,`aidname`=%#s,organisation=%d", aid, site, name, atoi(sql_colz(res, "organisation")))))
      {
         *aid = 0;
         continue;
      }
      break;
   }
   sql_free_result(res);
   close(f);
   sql_close(&sql);

   if (!*aid)
      return 1;
   if (!silent)
      printf("%s", aid);
   return 0;

}
