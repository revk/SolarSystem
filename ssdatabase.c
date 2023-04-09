// Database checking

#define _GNU_SOURCE             /* See feature_test_macros(7) */
typedef unsigned int uint32_t;
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
#include "ESP32/main/areas.h"
#include "ssdatabase.h"

void sstypes(const char *fn)
{                               // Create script types file
   FILE *f = fopen(fn, "w");
   int count = 0,
       started = 0;
   void done(void) {
      if (started)
         fprintf(f, "'\n");
      started = 0;
   }
   void start(const char *v) {
      if (started)
         done();
      fprintf(f, "setenv %s '", v);
      count = 0;
      started = 1;
   }
   void list(const char *v) {
      fprintf(f, "%s%s", count++ ? " " : "", v);
   }
   void out(const char *n, const char *v) {
      fprintf(f, "%s\"%s\"=\"%s\"", count++ ? " " : "", n, v);
   }
   void pick(const char *n, const char *v) {
      fprintf(f, "<option value=\"%s\">%s</option>", n, v);
   }
   void check(const char *p, const char *n, const char *v) {
      fprintf(f, "<input type=checkbox name=\"func\" value=\"%s\" id=\"%s%s\"><label for=\"%s%s\">%s</label>&nbsp;", n, p, n, p, n, v);
   }
   start("GPIOFUNCPICKI");
   pick("-", "No input func");
#define fi(g,t) pick(#g,#t);
#include "types.m"
   start("GPIOFUNCPICKO");
   pick("-", "No output func");
#define fo(g,t) pick(#g,#t);
#include "types.m"
   start("GPIOFUNCOUTI");
#define fi(g,t) out(#g," "#t);
#include "types.m"
   start("GPIOFUNCOUTO");
#define fo(g,t) out(#g,#t);
#include "types.m"
   start("GPIOFUNCSETI");
#define fi(g,t) check("I",#g,#t);
#include "types.m"
   start("GPIOFUNCSETO");
#define fo(g,t) check("O",#g,#t);
#include "types.m"
   start("GPIOTYPELIST");
#define i(g,t) list(#g);
#define o(g,t) list(#g);
#include "types.m"
   start("GPIOTYPELISTI");
#define i(g,t) list(#g);
#include "types.m"
   start("GPIOTYPELISTO");
#define o(g,t) list(#g);
#include "types.m"
   start("GPIOTYPEOUT");
   out("-", "Unused");
#define i(g,t) out(#g,#t);
#define o(g,t) out(#g,#t);
#include "types.m"
   start("GPIOTYPEPICK");
   pick("-", "-- Unused --");
#define i(g,t) pick(#g,#t);
#define o(g,t) pick(#g,#t);
#include "types.m"
   start("GPIOTYPEPICKI");
   pick("-", "-- Unused --");
#define i(g,t) pick(#g,#t);
#include "types.m"
   start("GPIOTYPEPICKO");
   pick("-", "-- Unused --");
#define o(g,t) pick(#g,#t);
#include "types.m"
   start("GPIONUMLIST");
#define g(g) list(#g);
#include "types.m"
#define n(g,t) list(#t);
#include "types.m"
#define l(g,t) list(#t);
#include "ESP32/main/logicalgpio.m"
   start("GPIONUMOUT");
   out("-", "Unused");
#define g(g) out(#g,#g);
#include "types.m"
#define n(g,t) out(#t,"NFC"#g);
#include "types.m"
#define l(g,t) out(#t,#g);
#include "ESP32/main/logicalgpio.m"
#define g(g) out("-"#g,#g" (active low)");
#include "types.m"
#define n(g,t) out("-"#t,"NFC"#g" (active low)");
#include "types.m"
   start("GPIONUMPICK");
   pick("-", "-- GPIO --");
#define g(g) pick(#g,#g);
#include "types.m"
#define n(g,t) pick(#t,"NFC"#g);
#include "types.m"
#define l(g,t) pick(#t,#g);
#include "ESP32/main/logicalgpio.m"
#define g(g) pick("-"#g,#g" (active low)");
#include "types.m"
#define n(g,t) pick("-"#t,"NFC"#g" (active low)");
#include "types.m"
   start("GPIONFCLIST");
#define n(g,t) list(#g);
#include "types.m"
   start("GPIONFCOUT");
   out("-", "Unused");
#define n(g,t) out(#g,#g);
#include "types.m"
#define n(g,t) out("-"#g,#g" (active low)");
#include "types.m"
   start("GPIONFCPICK");
   pick("-", "-- NFC GPIO --");
#define n(g,t) pick(#g,#g);
#include "types.m"
#define n(g,t) pick("-"#g,#g" (active low)");
#include "types.m"
   start("GPIOIOLIST");
#define io(g,t) list(#g);
#include "types.m"
   start("GPIOIOOUT");
#define io(g,t) out(#g,#t);
#include "types.m"
   start("GPIOIOPICK");
#define io(g,t) pick(#g,#t);
#include "types.m"
   start("AREALIST");
   int n = 0;
   for (char *a = AREAS; *a; a++)
      if (*a != a[1])
      {
         const char area[2] = { *a };
         list(area);
         n++;
      }
   start("STATELIST");
#define i(t,n,c) list(#n);
#define c(t,n) list(#n);
#define s(t,n,c) list(#n);
#include "ESP32/main/states.m"
   start("STATELISTO");
#define i(t,n,c) list(#n);
#define s(t,n,c) list(#n);
#include "ESP32/main/states.m"
   start("STATELISTI");
#define i(t,n,c) list(#n);
#define c(t,n) list(#n);
#include "ESP32/main/states.m"
   done();
   fprintf(f, "setenv AREACOUNT %d\n", n);
   fclose(f);
}

void sskeydatabase(SQL * sqlp)
{                               // Selects database
   if (sql_select_db(sqlp, CONFIG_SQL_KEY_DATABASE))
   {
      warnx("Creating database %s", CONFIG_SQL_KEY_DATABASE);
      sql_safe_query_f(sqlp, "CREATE DATABASE `%#S`", CONFIG_SQL_KEY_DATABASE);
      sql_select_db(sqlp, CONFIG_SQL_KEY_DATABASE);
   }
   SQL_RES *res = sql_query_store_f(sqlp, "DESCRIBE `AES`");
   if (res)
   {
      // Crude update, if we do any more then use same mechanism as ssdatabase()
      char hastype = 0;
      SQL_ROW row;
      while ((row = sql_fetch_row(res)))
         if (row[0] && !strcmp(row[0], "type"))
            hastype = 1;
      sql_free_result(res);
      if (!hastype)
         sql_safe_query(sqlp, "ALTER TABLE `AES` ADD `type` char(2) NOT NULL default '01' AFTER `fob`");
   } else
   {
      sql_safe_query(sqlp, "CREATE TABLE `AES` (" "`created` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,`aid` char(6) NOT NULL DEFAULT '',`fob` char(14) NOT NULL DEFAULT '',`type` char(2) NOT NULL DEFAULT '00',`ver` char(2) NOT NULL DEFAULT '',`key` char(32) NOT NULL,UNIQUE KEY `key` (`aid`,`fob`,`ver`)" ")");
   }
}

void ssdatabase(SQL * sqlp)
{                               // Check database integrity - selects database
   if (sql_select_db(sqlp, CONFIG_SQL_DATABASE))
   {
      warnx("Creating database %s", CONFIG_SQL_DATABASE);
      sql_safe_query_f(sqlp, "CREATE DATABASE `%#S`", CONFIG_SQL_DATABASE);
      sql_select_db(sqlp, CONFIG_SQL_DATABASE);
   }

   SQL_RES *res = NULL;
   const char *tablename = NULL;
   char *tabledef = NULL;
   void endtable(void) {
      if (res)
         sql_free_result(res);
      res = NULL;
      tablename = NULL;
      if (tabledef)
         free(tabledef);
      tabledef = NULL;
   }

   int db = 0;
   const char **dbs = NULL;
   int dbn = 0;
   const char ***tbs = NULL;
   int *tbn = NULL;
   void addtable(const char *name) {
      for (db = 0; db < dbn && strcmp(dbs[db], name); db++);
      if (db < dbn)
         return;
      dbn++;
      dbs = realloc(dbs, sizeof(*dbs) * dbn);
      dbs[db] = name;
      tbs = realloc(tbs, sizeof(*tbs) * dbn);
      tbs[db] = NULL;
      tbn = realloc(tbn, sizeof(*tbn) * dbn);
      tbn[db] = 0;
   }
   void addfield(const char *field) {
      tbn[db]++;
      tbs[db] = realloc(tbs[db], sizeof(*tbs) * tbn[db]);
      tbs[db][tbn[db] - 1] = field;
   }

   void create(const char *name, int l) {       // Make table
      addtable(name);
      addfield(name);
      res = sql_query_store_f(sqlp, "DESCRIBE `%S`", name);
      if (res)
      {                         // Exists
         sql_free_result(res);
         res = NULL;
         return;
      }
      warnx("Creating table %s", name);
      if (l)
         sql_safe_query_f(sqlp, "CREATE TABLE `%#S` (`%#S` char(%d) NOT NULL PRIMARY KEY)", name, name, l);
      else
         sql_safe_query_f(sqlp, "CREATE TABLE `%#S` (`%#S` int unsigned NOT NULL AUTO_INCREMENT PRIMARY KEY)", name, name);
   }

   void linked(const char *tab, const char *name) {
      addfield(name);
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      int l = 0;
#define table(n,len) if(!strcmp(tab,#n))l=len;
#include "ssdatabase.m"
      warnx("Creating link %s/%s/%s", tablename, tab, name);
      if (l)
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD `%#S` char(%d) DEFAULT NULL", tablename, name, l);
      else
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD `%#S` int unsigned DEFAULT NULL", tablename, name);
   }

   void unique(const char *a, const char *b) {
      char *key;
      if (asprintf(&key, "UNIQUE KEY `%s_%s_%s`", tablename, a, b) < 0)
         errx(1, "malloc");
      if (!strstr(tabledef, key))
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s (`%#S`,`%#S`)", tablename, key, a, b);
      free(key);
   }

   void getrows(const char *name) {     // Get rows
      endtable();
      addtable(name);
      tablename = name;
      res = sql_safe_query_store_f(sqlp, "SELECT * FROM `%#S` LIMIT 0", name);
   }

   void getdefs(const char *name) {     // Get tabledef
      addtable(name);
      tablename = name;
      SQL_RES *res = sql_safe_query_store_f(sqlp, "SHOW CREATE TABLE `%#S`", name);
      if (!sql_fetch_row(res))
         errx(1, "WTF %s", name);
      tabledef = strdup(res->current_row[1]);
      sql_free_result(res);
   }

   void foreign(const char *tab, const char *name) {
      addfield(name);
      char *constraint;
      if (asprintf(&constraint, "CONSTRAINT `%s_%s` FOREIGN KEY", tablename, name) < 0)
         errx(1, "malloc");
      if (!strstr(tabledef, constraint))
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s (%#S) REFERENCES `%#S` (%#S) ON DELETE RESTRICT ON UPDATE CASCADE", tablename, constraint, name, tab, tab);
      free(constraint);
   }

   void join(const char *name, const char *a, const char *b) {
      res = sql_query_store_f(sqlp, "DESCRIBE `%S`", name);
      if (res)
      {                         // Exists
         sql_free_result(res);
         res = NULL;
         return;
      }
      warnx("Creating table %s", name);
      sql_safe_query_f(sqlp, "CREATE TABLE `%#S` (`%#S` int unsigned NOT NULL AUTO_INCREMENT PRIMARY KEY)", name, name);
      getrows(name);
      getdefs(name);
      linked(a, a);
      linked(b, b);
      unique(a, b);
      sql_safe_query_f(sqlp, "ALTER TABLE `%#S` DROP `%#S`", name, name);
   }

   void key(const char *name, int l) {
      char *key;
      if (asprintf(&key, "UNIQUE KEY `%s_%s`", tablename, name) < 0)
         errx(1, "malloc");
      if (!strstr(tabledef, key))
      {
         if (l)
            sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s (`%#S`(%d))", tablename, key, name, l);
         else
            sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s (`%#S`)", tablename, key, name);
      }
      free(key);
   }

   void index(const char *name) {
      char *key;
      if (asprintf(&key, "KEY `%s_%s`", tablename, name) < 0)
         errx(1, "malloc");
      if (!strcasestr(tabledef, key))
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s (`%#S`)", tablename, key, name);
      free(key);
   }

   void text(const char *name, int l) {
      addfield(name);
      char *def;
      if (l)
      {
         if (asprintf(&def, "`%s` char(%d) DEFAULT NULL", name, l) < 0)
            errx(1, "malloc");
      } else
      {
         if (asprintf(&def, "`%s` text DEFAULT NULL", name) < 0)
            errx(1, "malloc");
      }

      if (sql_colnum(res, name) < 0)
      {
         warnx("Creating field %s/%s", tablename, name);
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s", tablename, def);
      } else if (!strcasestr(tabledef, def))
      {
         warnx("Updating field %s/%s", tablename, name);
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` MODIFY %s", tablename, def);
      }
      free(def);

   }

   void field(const char *name, const char *type, const char *deflt) {
      addfield(name);
      char *def;
      if (asprintf(&def, "`%s` %s %sDEFAULT %s", name, type, strcmp(deflt, "NULL") ? "NOT NULL " : "", deflt) < 0)
         errx(1, "malloc");
      if (sql_colnum(res, name) < 0)
      {
         warnx("Creating field %s/%s", tablename, name);
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` ADD %s", tablename, def);
      } else if (!strcasestr(tabledef, def))
      {
         warnx("Updating field %s/%s", tablename, name);
         sql_safe_query_f(sqlp, "ALTER TABLE `%#S` MODIFY %s", tablename, def);
      }
      free(def);
   }

   char *areatype = NULL;
   char *areastype = NULL;
   {
      char *a = NULL;
      size_t l;
      FILE *f = open_memstream(&a, &l);
      char *p;
      for (p = AREAS; *p; p++)
         if (*p != p[1])
            fprintf(f, ",'%c'", *p);
      fclose(f);
      if (asprintf(&areatype, "enum(%s)", a + 1) < 0)
         errx(1, "malloc");
      if (asprintf(&areastype, "set(%s)", a + 1) < 0)
         errx(1, "malloc");
      free(a);
   }

   sql_transaction(sqlp);
#define table(n,l)	create(#n,l);   // Make tables first
#define join(a,b)	join(#a#b,#a,#b);
#include "ssdatabase.m"
#define table(n,l)	getrows(#n);getdefs(#n);        // Get table info
#define	join(a,b)	getrows(#a#b);getdefs(#a#b);
#define link(n)		linked(#n,#n);  // Foreign key
#define link2(a,b)	linked(#a,#b);  // Foreign key under different name
#define	text(n,l)	text(#n,l);
#define	num(n)		field(#n,"int(10)","NULL");
#define	ip(n)		field(#n,"varchar(39)","NULL");
#define	datetime(n)	field(#n,"datetime","NULL");
#define	date(n)		field(#n,"date","NULL");
#define	time0000(n)	field(#n,"time","'00:00:00'");
#define	time2359(n)	field(#n,"time","'23:59:00'");
#define	gpio(n)		field(#n,"enum('-','0','2','4','5','7','8','12','13','14','15','16','17','18','19','20','21','22','23','25','26','27','32','33','34','35','36','39','48','49','50','51','52','53','54','55','56','57','58','59','60','61','62','63','-0','-2','-4','-5','-7','-8','-12','-13','-14','-15','-16','-17','-18','-19','-20','-21','-22','-23','-25','-26','-27','-32','-33','-34','-35','-36','-39','-48','-49','-50','-51','-52','-53','-54','-55','-56','-57','-58','-59','-60','-61','-62','-63')","'-'");
#define	gpionfc(n)	field(#n,"enum('-','30','31','32','33','34','35','71','72','-30','-31','-32','-33','-34','-35','-71','-72')","'-'");
#define	gpiotype(n)	field(#n,"enum('-','I','O','P')","'-'");
#define	gpiopcb(n)	field(#n,"enum('-','IO','I','O')","'-'");
#define	gpiofunc(n)	field(#n,"set('L','D','E','B','C','O','M')","''");
#define	bool(n)		field(#n,"enum('false','true')","'false'");
#define	areas(n)	field(#n,areastype,"''");
#define	area(n)		field(#n,areatype,"'A'");
#include "ssdatabase.m"
#define table(n,l)	getdefs(#n);    // Get table info
#define join(a,b)	getdefs(#a#b);foreign(#a,#a);foreign(#b,#b);    // Get table info
#define link(n)		foreign(#n,#n); // Foreign key
#define link2(a,b)	foreign(#a,#b); // Foreign key
#define unique(a,b)	unique(#a,#b);  // Make extra keys
#define key(n,l)	key(#n,l);      // Make extra key
#define index(n)	index(#n);      // Make extra index
#include "ssdatabase.m"
   endtable();
   // Delete extras
   for (int t = 0; t < dbn; t++)
   {
      SQL_RES *res = sql_safe_query_store_f(sqlp, "SELECT * FROM `%#S` LIMIT 1", dbs[t]);
      for (size_t f = 0; f < res->field_count; f++)
      {
         int q;
         for (q = 0; q < tbn[t] && strcasecmp(tbs[t][q], res->fields[f].name); q++);
         if (q == tbn[t])
         {
            warnx("Dropping field %s/%s", dbs[t], res->fields[f].name);
            sql_safe_query_f(sqlp, "ALTER TABLE `%#S` DROP `%#S`", dbs[t], res->fields[f].name);
         }
      }
      sql_free_result(res);
   }
   free(dbs);
   free(tbs);
   free(tbn);
   sql_safe_commit(sqlp);
}
