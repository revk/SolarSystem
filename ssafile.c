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

unsigned int makeafile(SQL_RES * res, unsigned char *afile)
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
   void addbcd(unsigned char v)
   {
	   add((('0'+(v/10))<<4)+(v%10));
   }
   void addbcd2(unsigned short v)
   {
	   addbcd(v/100);
	   addbcd(v%100);
   }

   if (!res || !sql_col(res, "access") || sql_col(res, "blocked"))
      add(0xFB);
   else
   {
      if (res && *sql_colz(res, "commit") == 't')
         add(0xF0);
      if (res && *sql_colz(res, "count") == 't')
         add(0xF1);
      if (res && *sql_colz(res, "override") == 't')
         add(0xFA);
      if (res && sql_col(res, "block"))
         add(0xFB);
      if (res && *sql_colz(res, "clock") == 't')
         add(0xFC);
      time_t expires = 0;
      int expiry = atoi(sql_colz(res, "expiry"));
      if (res)
         expires = sql_time(sql_colz(res, "expires"));
      if (!expires && expiry)
         expires = time(0) + 86400 * expiry;
      if (expires)
      {
	      struct tm t;

      }
   }
   // TODO arm and open
   if (p >= 256)
      return 0;                 // Too big
   *a = p - 1;
   memcpy(afile, a, p);
   return df_crc(*a, a + 1);
}
