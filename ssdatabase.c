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

void ssdatabase(SQL * sqlp,const char *sqldatabase)
{                               // Check database integrity
   if (sql_select_db(sqlp, sqldatabase))
   {
      warnx("Creating database %s", sqldatabase);
      sql_safe_query_free(sqlp, sql_printf("CREATE DATABASE `%#S`", sqldatabase));
      sql_select_db(sqlp, sqldatabase);
   }

   SQL_RES *res = NULL;
   void endtable(void) {
      if (res)
         sql_free_result(res);
      res = NULL;
   }

   void table(const char *name) {
      endtable();
      res = sql_query_store(sqlp, sql_printf("SELECT * FROM `%#S` LIMIT 0", name));
      if (res)
         return;                // Exists
      warnx("Creating table %s", name);
      sql_safe_query_free(sqlp, sql_printf("CREATE TABLE `%#S` (`%#S` int unsigned auto_increment primary key)", name, name));
      res = sql_query_store(sqlp, sql_printf("SELECT * FROM `%#S` LIMIT 0", name));
   }

   void link(const char *name) {
   }

   void text(const char *name) {
   }

#define table(n)	table(#n);
#define link(n)		link(#n);
#define	text(n)		text(#n);
   sql_transaction(sqlp);
#include "ssdatabase.h"
   endtable();
   sql_safe_commit(sqlp);
}
