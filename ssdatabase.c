// Database checking

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "SQLlib/sqllib.h"

void ssdatabase(SQL * sqlp, const char *sqldatabase)
{                               // Check database integrity
   if (sql_select_db(sqlp, sqldatabase))
   {
      warnx("Creating database %s", sqldatabase);
      sql_safe_query_free(sqlp, sql_printf("CREATE DATABASE `%#S`", sqldatabase));
      sql_select_db(sqlp, sqldatabase);
   }

   SQL_RES *res = NULL;
   const char *tablename = NULL;
   void endtable(void) {
      if (res)
         sql_free_result(res);
      res = NULL;
      tablename = NULL;
   }

   void create(const char *name, int l) {
      res = sql_query_store_free(sqlp, sql_printf("DESCRIBE `%S`", name));
      if (res)
      {                         // Exists
         sql_free_result(res);
         res = NULL;
         return;
      }
      warnx("Creating table %s", name);
      if (l)
         sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` CHAR(%d) NOT NULL PRIMARY KEY)", name, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` INT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY)", name, name));
   }

   void table(const char *name, int l) {
      endtable();
      tablename = name;
      l = l;
      res = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `%#S` LIMIT 0", name));
   }

   void link(const char *name) {
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      int l = 0;
#define table(n,len) if(!strcmp(name,#n))l=len;
#include "ssdatabase.h"
      warnx("Creating link %s/%s", tablename, name);
      if (l)
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` CHAR(%d) DEFAULT NULL", tablename, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` INT UNSIGNED DEFAULT NULL", tablename, name));
      sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD CONSTRAINT `%#S_%#S` FOREIGN KEY (%#S) REFERENCES `%#S` (%#S) ON DELETE CASCADE ON UPDATE CASCADE", tablename, tablename, name, name, name, name));
   }

   void text(const char *name, int l) {
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      warnx("Creating field %s/%s", tablename, name);
      if (l)
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` CHAR(%d) DEFAULT NULL", tablename, name, l));
      else
         sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` TEXT DEFAULT NULL", tablename, name));
   }

   void field(const char *name, const char *type) {
      if (sql_colnum(res, name) >= 0)
         return;                // Exists - we are not updating type for now
      warnx("Creating field %s/%s", tablename, name);
      sql_safe_query_free(sqlp, sql_printf("ALTER TABLE `%#S` ADD `%#S` %s DEFAULT NULL", tablename, name, type));
   }


   sql_transaction(sqlp);
#define table(n,l)	create(#n,l);   // Make tables first
#include "ssdatabase.h"
#define table(n,l)	table(#n,l);
#define link(n)		link(#n);
#define	text(n,l)	text(#n,l);
#define	num(n)		field(#n,"INT");
#define	ip(n)		field(#n,"VARCHAR(39)");
#define	time(n)		field(#n,"DATETIME");
#include "ssdatabase.h"
   endtable();
   sql_safe_commit(sqlp);
}
