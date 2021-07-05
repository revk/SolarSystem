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
   start("GPIONUMOUT");
   out("-", "Unused");
#define g(g) out(#g,#g);
#include "types.m"
#define g(g) out("-"#g,#g" (active low)");
#include "types.m"
   start("GPIONUMPICK");
   pick("-", "-- GPIO --");
#define g(g) pick(#g,#g);
#include "types.m"
#define g(g) pick("-"#g,#g" (active low)");
#include "types.m"
   start("GPIONFCLIST");
#define n(g) list(#g);
#include "types.m"
   start("GPIONFCOUT");
   out("-", "Unused");
#define n(g) pick(#g,#g);
#include "types.m"
#define n(g) pick("-"#g,#g" (active low)");
#include "types.m"
   start("GPIONFCPICK");
   pick("-", "-- NFC GPIO --");
#define n(g) pick(#g,#g);
#include "types.m"
#define n(g) pick("-"#g,#g" (active low)");
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
   for (char *a = AREAS; *a; a++)
      if (*a != '-')
      {
         const char area[2] = { *a };
         list(area);
      }
   start("STATELIST");
#define i(n) list(#n);
#define s(n) list(#n);
#include "ESP32/main/states.m"
   start("STATELISTI");
#define i(n) list(#n);
#include "ESP32/main/states.m"
   done();
   fclose(f);
}

void sskeydatabase(SQL * sqlp)
{
   if (sql_select_db(sqlp, CONFIG_SQL_KEY_DATABASE))
   {
      warnx("Creating database %s", CONFIG_SQL_KEY_DATABASE);
      sql_safe_query_free(sqlp, sql_printf("CREATE DATABASE `%#S`", CONFIG_SQL_KEY_DATABASE));
      sql_select_db(sqlp, CONFIG_SQL_KEY_DATABASE);
   }
   SQL_RES *res = sql_query_store(sqlp, "DESCRIBE `AES`");
   if (res)
      sql_free_result(res);
   else
   {
      sql_safe_query(sqlp, "CREATE TABLE `AES` (" "`created` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,`aid` char(6) NOT NULL DEFAULT '',`fob` char(14) NOT NULL DEFAULT '',`ver` char(2) NOT NULL DEFAULT '',`key` char(32) NOT NULL,UNIQUE KEY `key` (`aid`,`fob`,`ver`)" ")");
   }
}

void ssdatabase(SQL * sqlp)
{                               // Check database integrity
   if (sql_select_db(sqlp, CONFIG_SQL_DATABASE))
   {
      warnx("Creating database %s", CONFIG_SQL_DATABASE);
      sql_safe_query_free(sqlp, sql_printf("CREATE DATABASE `%#S`", CONFIG_SQL_DATABASE));
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
      res = sql_query_store_free(sqlp, sql_printf("DESCRIBE `%S`", name));
      if (res)
      {                         // Exists
         sql_free_result(res);
         res = NULL;
         return;
      }
      warnx("Creating table %s", name);
      if (l)
         sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` char(%d) NOT NULL PRIMARY KEY)", name, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` int unsigned NOT NULL AUTO_INCREMENT PRIMARY KEY)", name, name));
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
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` char(%d) DEFAULT NULL", tablename, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` int unsigned DEFAULT NULL", tablename, name));
   }

   void unique(const char *a, const char *b) {
      char *key;
      if (asprintf(&key, "UNIQUE KEY `%s_%s_%s`", tablename, a, b) < 0)
         errx(1, "malloc");
      if (!strstr(tabledef, key))
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s (`%#S`,`%#S`)", tablename, key, a, b));
      free(key);
   }

   void getrows(const char *name) {     // Get rows
      endtable();
      addtable(name);
      tablename = name;
      res = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `%#S` LIMIT 0", name));
   }

   void getdefs(const char *name) {     // Get tabledef
      addtable(name);
      tablename = name;
      SQL_RES *res = sql_safe_query_store_free(sqlp, sql_printf("SHOW CREATE TABLE `%#S`", name));
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
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s (%#S) REFERENCES `%#S` (%#S) ON DELETE RESTRICT ON UPDATE CASCADE", tablename, constraint, name, tab, tab));
      free(constraint);
   }

   void join(const char *name, const char *a, const char *b) {
      res = sql_query_store_free(sqlp, sql_printf("DESCRIBE `%S`", name));
      if (res)
      {                         // Exists
         sql_free_result(res);
         res = NULL;
         return;
      }
      warnx("Creating table %s", name);
      sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` int unsigned NOT NULL AUTO_INCREMENT PRIMARY KEY)", name, name));
      getrows(name);
      getdefs(name);
      linked(a, a);
      linked(b, b);
      unique(a, b);
      sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` DROP `%#S`", name, name));
   }

   void key(const char *name, int l) {
      char *key;
      if (asprintf(&key, "UNIQUE KEY `%s_%s`", tablename, name) < 0)
         errx(1, "malloc");
      if (!strstr(tabledef, key))
      {
         if (l)
            sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s (`%#S`(%d))", tablename, key, name, l));
         else
            sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s (`%#S`)", tablename, key, name));
      }
      free(key);
   }

   void index(const char *name) {
      char *key;
      if (asprintf(&key, "KEY `%s_%s`", tablename, name) < 0)
         errx(1, "malloc");
      if (!strcasestr(tabledef, key))
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s (`%#S`)", tablename, key, name));
      free(key);
   }

   void text(const char *name, int l) {
      addfield(name);
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      warnx("Creating field %s/%s", tablename, name);
      if (l)
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` char(%d) DEFAULT NULL", tablename, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` text DEFAULT NULL", tablename, name));
   }

   void field(const char *name, const char *type, const char *deflt) {
      addfield(name);
      char *def;
      if (asprintf(&def, "`%s` %s %sDEFAULT %s", name, type, strcmp(deflt, "NULL") ? "NOT NULL " : "", deflt) < 0)
         errx(1, "malloc");
      if (sql_colnum(res, name) < 0)
      {
         warnx("Creating field %s/%s", tablename, name);
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s", tablename, def));
      } else if (!strcasestr(tabledef, def))
      {
         warnx("Updating field %s/%s", tablename, name);
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` MODIFY %s", tablename, def));
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
         if (*p != '-')
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
#define	gpio(n)		field(#n,"enum('-','2','4','5','12','13','14','15','16','17','18','19','21','22','23','25','26','27','32','33','34','35','36','39','-2','-4','-5','-12','-13','-14','-15','-16','-17','-18','-19','-21','-22','-23','-25','-26','-27','-32','-33','-34','-35','-36','-39')","'-'");
#define	gpionfc(n)	field(#n,"enum('-','30','31','32','33','34','35','71','72','-30','-31','-32','-33','-34','-35','-71','-72')","'-'");
#define	gpiotype(n)	field(#n,"enum('-','I','O','P','I1','I2','I3','I4','I8','O1','O2','O3','O4')","'-'");
#define	gpiopcb(n)	field(#n,"enum('-','IO','I','O')","'-'");
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
      SQL_RES *res = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `%#S` LIMIT 1", dbs[t]));
      for (size_t f = 0; f < res->field_count; f++)
      {
         int q;
         for (q = 0; q < tbn[t] && strcasecmp(tbs[t][q], res->fields[f].name); q++);
         if (q == tbn[t])
         {
            warnx("Dropping field %s/%s", dbs[t], res->fields[f].name);
            sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` DROP `%#S`", dbs[t], res->fields[f].name));
         }
      }
      sql_free_result(res);
   }
   free(dbs);
   free(tbs);
   free(tbn);
   sql_safe_commit(sqlp);
}
