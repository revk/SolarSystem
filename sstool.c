// Tool for some database stuff...
#define	_GNU_SOURCE
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "SQLlib/sqllib.h"

int
main (int argc, const char *argv[])
{
   int organisation = atoi (getenv ("USER_ORGANISATION") ? : "");
   int site = atoi (getenv ("USER_SITE") ? : "");
   const char *provisiondevice = NULL;
   char *devicename = NULL;
   int pcb = 0;
   const char *aid = 0;
   const char *copydevice = NULL;
   poptContext optCon;          // context for parsing command-line options
   {                            // POPT
      const struct poptOption optionsTable[] = {
         {"provision-device", 0, POPT_ARG_STRING, &provisiondevice, 0, "Provision Device", "deviceID"},
         {"copy-device", 0, POPT_ARG_STRING, &copydevice, 0, "Copy Device", "deviceID"},
         {"device-name", 0, POPT_ARG_STRING, &devicename, 0, "Device name", "name"},
         {"pcb", 0, POPT_ARG_INT, &pcb, 0, "PCB", "ID"},
         {"organisation", 0, POPT_ARG_INT, &organisation, 0, "Organisation", "ID"},
         {"site", 0, POPT_ARG_INT, &site, 0, "Site", "ID"},
         {"aid", 0, POPT_ARG_STRING, &aid, 0, "AID", "ID"},
         {"debug", 'v', POPT_ARG_NONE, &sqldebug, 0, "Debug", NULL},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (poptPeekArg (optCon) || !provisiondevice)
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
   }
   SQL sql;
   sql_cnf_connect (&sql, CONFIG_SQL_SERVER_FILE);
   if (sql_select_db (&sql, CONFIG_SQL_DATABASE))
      errx (1, "Failed database select");
   SQL_RES *res;
   sql_transaction (&sql);

   if (provisiondevice)
   {
      SQL_RES *old = NULL;
      if (copydevice && *copydevice)
      {
         old = sql_safe_query_store_f (&sql, "SELECT * FROM `device` WHERE `device`=%#s", copydevice);
         if (!sql_fetch_row (old))
            errx (1, "Does not exist device %s", copydevice);
         if (!pcb)
            pcb = atoi (sql_colz (old, "pcb"));
         if (!organisation)
            organisation = atoi (sql_colz (old, "organisation"));
         if (!site)
            site = atoi (sql_colz (old, "site"));
         if (!aid)
            aid = strdupa (sql_colz (old, "aid"));
         if (!devicename || !*devicename)
            asprintf (&devicename, "%s-Copy", sql_colz (old, "devicename"));
      }
      if (!pcb)
         errx (1, "Specify --pcb");
      if (!organisation)
         errx (1, "Specify --organisation");
      if (!site)
         errx (1, "Specify --site");
      if (!aid)
         errx (1, "Specify --aid");
      if (!devicename)
         errx (1, "Specify --device-name");
      int nfc = 0,
         gps = 0;
      res = sql_safe_query_store_f (&sql, "SELECT * FROM `pcb` WHERE `pcb`=%d", pcb);
      if (sql_fetch_row (res))
      {
         nfc = strcmp (sql_colz (res, "nfctx"), "-");
         gps = strcmp (sql_colz (res, "gpstx"), "-");
      }
      sql_free_result (res);
      sql_s_t q = { 0 };
      sql_sprintf (&q, "INSERT INTO `device` SET `device`=%#s,", provisiondevice);
      void settings (void)
      {
         sql_sprintf (&q,
                      "`pcb`=%d,`organisation`=%d,`site`=%d,`aid`=%#s,`devicename`=%#s,`outofservice`='true',`upgrade`=now(),`nfc`=%#s,`gps`=%#s",
                      pcb, organisation, site, aid, devicename, nfc ? "true" : "false", gps ? "true" : "false");
         if (old)
         {                      // Copy some fields
            for (unsigned int n = 0; n < old->field_count; n++)
               if (old->fields[n].name  //
                   && strcasecmp (old->fields[n].name, "device")        //
                   && strcasecmp (old->fields[n].name, "pcb")   //
                   && strcasecmp (old->fields[n].name, "organisation")  //
                   && strcasecmp (old->fields[n].name, "site")  //
                   && strcasecmp (old->fields[n].name, "aid")   //
                   && strcasecmp (old->fields[n].name, "devicename")    //
                   && strcasecmp (old->fields[n].name, "outofservice")  //
                   && strcasecmp (old->fields[n].name, "upgrade")       //
                   && strcasecmp (old->fields[n].name, "nfc")   //
                   && strcasecmp (old->fields[n].name, "gps")   //
                   && strcasecmp (old->fields[n].name, "version")       //
                   && strcasecmp (old->fields[n].name, "build") //
                   && strcasecmp (old->fields[n].name, "build_suffix")  //
                   && strcasecmp (old->fields[n].name, "rst")   //
                   && strcasecmp (old->fields[n].name, "mem")   //
                   && strcasecmp (old->fields[n].name, "spi")   //
                   && strcasecmp (old->fields[n].name, "chan")  //
                   && strcasecmp (old->fields[n].name, "bssid") //
                   && strcasecmp (old->fields[n].name, "ssid")  //
                   && strcasecmp (old->fields[n].name, "encryptednvs")  //
                   && strcasecmp (old->fields[n].name, "secureboot")    //
                   && strcasecmp (old->fields[n].name, "flash") //
                  )
                  sql_sprintf (&q, ",`%#S`=%#s", old->fields[n].name, old->current_row[n]);
         }
      }
      settings ();
      sql_sprintf (&q, " ON DUPLICATE KEY UPDATE ");
      settings ();
      sql_safe_query_s (&sql, &q);
      sql_safe_query_f (&sql, "DELETE FROM `devicegpio` WHERE `device`=%#s", provisiondevice);
      if (old)
      {                         // Copy GPIO
         res = sql_safe_query_store_f (&sql, "SELECT * FROM `devicegpio` WHERE `device`=%#s", copydevice);
         while (sql_fetch_row (res))
         {
            SQL_RES *oldpin = sql_safe_query_store_f (&sql, "SELECT * FROM `gpio` WHERE `gpio`=%#s", sql_col (res, "gpio"));
            if (sql_fetch_row (oldpin))
            {
               SQL_RES *newpin = sql_safe_query_store_f (&sql, "SELECT * FROM `gpio` WHERE `pcb`=%d AND `initname`=%#s", pcb,
                                                         sql_col (oldpin, "initname"));
               if (sql_fetch_row (newpin))
               {
                  sql_s_t q = { 0 };
                  sql_sprintf (&q, "INSERT INTO `devicegpio` SET `device`=%#s,`gpio`=%#s", provisiondevice,
                               sql_col (newpin, "gpio"));
                  for (unsigned int n = 0; n < res->field_count; n++)
                     if (res->fields[n].name && strcasecmp (res->fields[n].name, "device")
                         && strcasecmp (res->fields[n].name, "gpio"))
                        sql_sprintf (&q, ",`%#S`=%#s", res->fields[n].name, res->current_row[n]);
                  sql_safe_query_s (&sql, &q);
               } else
                  sql_safe_query_f (&sql,
                                    "INSERT INTO `devicegpio` SET `device`=%#s,`gpio`=%#s,`type`=%#s,`name`=%#s,`hold`=%#s,`pulse`=%#s,`invert`=%#s,`func`=%#s",
                                    provisiondevice, sql_col (oldpin, "gpio"), sql_col (oldpin, "inittype"), sql_col (oldpin,
                                                                                                                      "initname"),
                                    sql_col (oldpin, "inithold"), sql_col (oldpin, "initpulse"), sql_col (oldpin, "initinvert"),
                                    sql_col (oldpin, "initfunc"));
               sql_free_result (newpin);
            }
            sql_free_result (oldpin);
         }
         sql_free_result (res);
      } else
      {
         res = sql_safe_query_store_f (&sql, "SELECT * FROM `gpio` WHERE `pcb`=%d", pcb);
         while (sql_fetch_row (res))
            sql_safe_query_f (&sql,
                              "INSERT INTO `devicegpio` SET `device`=%#s,`gpio`=%#s,`type`=%#s,`name`=%#s,`hold`=%#s,`pulse`=%#s,`invert`=%#s,`func`=%#s",
                              provisiondevice, sql_col (res, "gpio"), sql_col (res, "inittype"), sql_col (res, "initname"),
                              sql_col (res, "inithold"), sql_col (res, "initpulse"), sql_col (res, "initinvert"), sql_col (res,
                                                                                                                           "initfunc"));
         sql_free_result (res);
      }
      sql_safe_query_f (&sql, "UPDATE `device` SET `poke`=NOW() WHERE `site`=%d", site);
      if (old)
         sql_free_result (old);
   }
   if (sqldebug)
      sql_safe_rollback (&sql);
   else
      sql_safe_commit (&sql);
   sql_close (&sql);
   poptFreeContext (optCon);
   return 0;
}
