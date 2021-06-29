// Make Afile

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
#include <openssl/evp.h>
#include "DESFireAES/include/desfireaes.h"

unsigned int makeafile(SQL * sqlp, int access, unsigned char *afile)
{                               // Make afile (max 256 characters) and return crc
   if (afile)
   {                            // Default if error
      afile[0] = 1;
      afile[1] = 0xFB;          // Block
   }
   unsigned char a[256];
   int p = 1;
   void add(unsigned char v) {
      if (p < 256)
         a[p] = v;
      p++;
   }
   SQL_RES *res = sql_safe_query_store_free(sqlp, sql_printf("SELECT * FROM `access` WHERE `access`=%d", access));
   if (!sql_fetch_row(res))
      add(0xFB);                // Block
   else
   {
      if (*sql_colz(res, "commit") == 't')
         add(0xF0);
      if (*sql_colz(res, "count") == 't')
         add(0xF1);
      if (*sql_colz(res, "override") == 't')
         add(0xFA);
      if (sql_col(res, "block"))
         add(0xFB);
      if (*sql_colz(res, "clock") == 't')
         add(0xFC);
   }
   sql_free_result(res);
   if (p >= 256)
      return 0;                 // Too big
   *a = p - 1;
   memcpy(afile, a, p);
   return df_crc(*a, a+1);
}
