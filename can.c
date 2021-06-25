// Security check

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "AJL/ajl.h"
#include "SQLlib/sqllib.h"
#include "login/redirect.h"

int main(int argc, const char *argv[])
{
   int us = 0,
       user = 0,
       organisation = 0,
       site = 0,
       aid = 0,
       class = 0;
   int redirect = 0;
   int reason = 0;
   const char *configfile = "../solarsystem.conf";
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         { "as", 0, POPT_ARG_INT, &us, 0, "Check as user", "N" },
         { "user", 'u', POPT_ARG_INT, &user, 0, "Check access to user", "N" },
         { "organisation", 'o', POPT_ARG_INT, &organisation, 0, "Check access to organisation", "N" },
         { "site", 's', POPT_ARG_INT, &site, 0, "Check access to site", "N" },
         { "aid", 'a', POPT_ARG_INT, &aid, 0, "Check access to aid", "N" },
         { "class", 'c', POPT_ARG_INT, &class, 0, "Check access to class", "N" },
         { "config-file", 0, POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &configfile, 0, "Config file", "filename" },
         { "redirect", 'r', POPT_ARG_NONE, &redirect, 0, "Redirect", NULL },
         { "reason", 0, POPT_ARG_NONE, &reason, 0, "Output reason allowed", NULL },
         { "debug", 'v', POPT_ARG_NONE, &sqldebug, 0, "Debug", NULL },
         POPT_AUTOHELP { }
      };

      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
      poptSetOtherOptionHelp(optCon, "{permissions}");

      int c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));

      if (poptPeekArg(optCon))
      {                         // TODO
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }
      poptFreeContext(optCon);
   }
   SQL sql;
   {                            // Load config file and extract settings
      j_t j = j_create();
      j_err(j_read_file(j, configfile));
      // Some housekeeping
      const char *sqlconfig = j_get(j, "sql.config");
      const char *sqldatabase = j_get(j, "sql.database");
      sql_cnf_connect(&sql, *sqlconfig ? sqlconfig : NULL);
      sql_select_db(&sql, sqldatabase); // Check database integrity
      j_delete(&j);
   }
   SQL_RES *resus = NULL;
   SQL_RES *resuser = NULL;
   SQL_RES *resorganisation = NULL;
   SQL_RES *ressite = NULL;
   SQL_RES *resaid = NULL;
   SQL_RES *resclass = NULL;
   const char *check(void) {
      if (!us)
         us = atoi(getenv("USER_ID") ? : "");
      if (!us)
      {
	      warnx("No --as/$USER_ID provided for can");
         return NULL;
      }
      resus=sql_safe_query_store_free(&sql,sql_printf("SELECT * FROM `user` WHERE `user`=%d",us));
      if(!sql_fetch_row(resus))
      {
	      warnx("User does not exist %d",us);
	      return NULL;
      }
      if(*sql_colz(resus,"admin")=='t')return "User is top level admin.";

      return NULL;              // Fail
   }
   const char *why = check();
   if (resus)
      sql_free_result(resus);
   if (resuser)
      sql_free_result(resuser);
   if (resorganisation)
      sql_free_result(resorganisation);
   if (ressite)
      sql_free_result(ressite);
   if (resaid)
      sql_free_result(resaid);
   if (resclass)
      sql_free_result(resclass);
   if (!why)
   { // not allowed
      if (redirect) sendredirect(NULL,"Sorry, access not allowed to this page");
   } else if(reason)
         printf("%s", why);
   sql_close(&sql);
   return why ? 0 : 1;
}
