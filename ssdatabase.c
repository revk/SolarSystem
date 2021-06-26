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

void sskeydatabase(SQL * sqlp)
{
   if (sql_select_db(sqlp, CONFIG_SQL_KEY_DATABASE))
   {
      warnx("Creating database %s", CONFIG_SQL_KEY_DATABASE);
      sql_safe_query_free(sqlp, sql_printf("CREATE DATABASE `%#S`", CONFIG_SQL_KEY_DATABASE));
      sql_select_db(sqlp, CONFIG_SQL_KEY_DATABASE);
   }
      SQL_RES *res = sql_query_store(sqlp, "DESCRIBE `AES`");
      if(res)sql_free_result(res);
      else
      {
	      sql_safe_query(sqlp,"CREATE TABLE `AES` (" \
			      "`created` datetime DEFAULT CURRENT_TIMESTAMP," \
			      "`aid` char(6) DEFAULT NULL," \
			      "`fob` char(14) DEFAULT NULL," \
			      "`ver` char(2) DEFAULT NULL," \
			      "`key` char(32) DEFAULT NULL," \
			      "UNIQUE KEY `key` (`aid`,`fob`,`ver`)" \
			      ")");
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

   void create(const char *name, int l) {       // Make table
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

   void link(const char *name) {
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      int l = 0;
#define table(n,len) if(!strcmp(name,#n))l=len;
#include "ssdatabase.h"
      warnx("Creating link %s/%s", tablename, name);
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

   void getrows(const char *name) {       // Get rows
      endtable();
      tablename = name;
      res = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `%#S` LIMIT 0", name));
   }

   void getdefs(const char *name) {     // Get tabledef
      tablename = name;
      SQL_RES *res = sql_safe_query_store_free(sqlp, sql_printf("SHOW CREATE TABLE `%#S`", name));
      if (!sql_fetch_row(res))
         errx(1, "WTF %s", name);
      tabledef = strdup(res->current_row[1]);
      sql_free_result(res);
   }

   void foreign(const char *name) {
      char *constraint;
      if (asprintf(&constraint, "CONSTRAINT `%s_%s` FOREIGN KEY", tablename, name) < 0)
         errx(1, "malloc");
      if (!strstr(tabledef, constraint))
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD %s (%#S) REFERENCES `%#S` (%#S) ON DELETE RESTRICT ON UPDATE CASCADE", tablename, constraint, name, name, name));
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
      sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` int unsigned NOT NULL AUTO_INCREMENT PRIMARY KEY)", name,name));
      getrows(name);
      getdefs(name);
      link(a);
      link(b);
      unique(a,b);
      sql_safe_query_free(sqlp,sql_printf("ALTER TABLE `%#S` DROP `%#S`",name,name));
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
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      warnx("Creating field %s/%s", tablename, name);
      if (l)
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` char(%d) DEFAULT NULL", tablename, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` text DEFAULT NULL", tablename, name));
   }

   void field(const char *name, const char *type,const char *deflt) {
      char *def;
      if (asprintf(&def, "`%s` %s %sDEFAULT %s", name, type,strcmp(deflt,"NULL")?"NOT NULL ":"",deflt) < 0)
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
#include "ssdatabase.h"
#define table(n,l)	getrows(#n);getdefs(#n);  // Get table info
#define	join(a,b)	getrows(#a#b);getdefs(#a#b);
#define link(n)		link(#n);       // Foreign key
#define	text(n,l)	text(#n,l);
#define	num(n)		field(#n,"int(10)","NULL");
#define	ip(n)		field(#n,"varchar(39)","NULL");
#define	time(n)		field(#n,"datetime","NULL");
#define	gpio(n)		field(#n,"enum('','2','4','5','12','13','14','15','16','17','18','19','21','22','23','25','26','27','32','33','34','35','36','39','-2','-4','-5','-12','-13','-14','-15','-16','-17','-18','-19','-21','-22','-23','-25','-26','-27','-32','-33','-34','-35','-36','-39')","''");
#define	gpionfc(n)	field(#n,"enum('','30','31','32','33','34','35','71','72','-30','-31','-32','-33','-34','-35','-71','-72')","''");
#define	gpiotype(n)	field(#n,"enum('','I','O','P','I1','I2','I3','I4','I8','O1','O2','O3','O4')","''");
#define	gpiopcb(n)	field(#n,"enum('','IO','I','O')","''");
#define	bool(n)		field(#n,"enum('false','true')","'false'");
#define	areas(n)	field(#n,areastype,"NULL");
#define	area(n	)	field(#n,areatype,"NULL");
#include "ssdatabase.h"
#define table(n,l)	getdefs(#n);    // Get table info
#define join(a,b)	getdefs(#a#b);foreign(#a);foreign(#b);  // Get table info
#define link(n)		foreign(#n);    // Foreign key
#define unique(a,b)	unique(#a,#b);  // Make extra keys
#define key(n,l)	key(#n,l);      // Make extra key
#define index(n)	index(#n);      // Make extra index
#include "ssdatabase.h"
   endtable();
   sql_safe_commit(sqlp);
}
