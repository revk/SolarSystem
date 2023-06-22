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
#include "ESP32/main/areas.h"

unsigned int
makeafile (SQL_RES * res, unsigned char *afile)
{                               // Make afile (max 256 characters) and return crc
   if (afile)
   {                            // Default if error
      afile[0] = 1;
      afile[1] = 0xFB;          // Block
   }
   unsigned char a[256];
   int p = 1;
   void add (unsigned char v)
   {
      if (p < 256)
         a[p] = v;
      p++;
   }
   void addbcd (unsigned char v)
   {
      add (((v / 10) << 4) + (v % 10));
   }
   void addbcd2 (unsigned short v)
   {
      addbcd (v / 100);
      addbcd (v % 100);
   }
   const char *name = sql_colz (res, "fobname");
   if (*name)
   {
      int l = strlen (name);
      if (l > 15)
         l = 15;
      add (0x40 + l);
      for (int i = 0; i < l; i++)
         add (name[i]);
   }
   const char *sms = sql_colz (res, "fobsms");
   if (*sms == '+')
      sms++;
   if (*sms)
   {
      int l = strlen (sms);
      if (l > 30)
         l = 30;
      if (l & 1)
         l++;
      add (0x90 + l / 2);
      for (int i = 0; i < l; i += 2)
         add (((sms[i] - '0') << 4) + ((sms[i + 1] ? : '?') - '0'));
   }
   if (!res || !sql_col (res, "access") || sql_col (res, "blocked"))
      add (0xFB);
   else
   {
      if (res && *sql_colz (res, "commit") == 't')
         add (0xF0);
      if (res && *sql_colz (res, "count") == 't')
         add (0xF1);
      if (res && *sql_colz (res, "armlate") == 't')
         add (0xFA);
      if (res && sql_col (res, "block"))
         add (0xFB);
      if (res && *sql_colz (res, "clock") == 't')
         add (0xFC);
      if (res && *sql_colz (res, "override") == 't')
         add (0xFF);
      time_t expires = 0;
      int expiry = atoi (sql_colz (res, "expiry"));
      if (expiry > 255)
         expiry = 255;
      if (res)
         expires = sql_time_utc (sql_colz (res, "expires"));
      if (expires)
         expiry = 0;            // Hard expiry instead
      if (expiry)
         expires = time (0) + 86400 * expiry;
      if (expires)
      {
         if (expiry)
         {
            add (0x31);
            add (expiry);
         }
         struct tm tm;
         gmtime_r (&expires, &tm);
         if (expiry || (!tm.tm_hour && !tm.tm_min && !tm.tm_sec) || (tm.tm_hour == 23 && tm.tm_min == 59 && tm.tm_sec == 59))
         {                      // date only
            if (!expiry)
               tm.tm_sec--;
            expires = timegm (&tm);
            gmtime_r (&expires, &tm);
            add (0x34);
            addbcd2 (tm.tm_year + 1900);
            addbcd (tm.tm_mon + 1);
            addbcd (tm.tm_mday);
         } else
         {
            add (0x37);
            addbcd2 (tm.tm_year + 1900);
            addbcd (tm.tm_mon + 1);
            addbcd (tm.tm_mday);
            addbcd (tm.tm_hour);
            addbcd (tm.tm_min);
            addbcd (tm.tm_sec);
         }
      }
      static const char *days[] = { "sun", "mon", "tue", "wed", "thu", "fri", "sat" };
      unsigned short from[7] = { }, to[7] = { };
      int d;
      for (d = 0; d < 7; d++)
      {
         char day[10];
         sprintf (day, "%sfrom", days[d]);
         const char *v = sql_colz (res, day);
         if (strlen (v) >= 5)
            from[d] = ((v[0] - '0') * 1000) + ((v[1] - '0') * 100) + ((v[3] - '0') * 10) + (v[4] - '0');
         sprintf (day, "%sto", days[d]);
         v = sql_colz (res, day);
         if (strlen (v) >= 5)
         {
            to[d] = ((v[0] - '0') * 1000) + ((v[1] - '0') * 100) + ((v[3] - '0') * 10) + (v[4] - '0');
            if (to[d] != from[d])
            {
               to[d]++;
               if ((to[d] % 100) == 60)
                  to[d] += 40;
            }
         }
      }
      void addtimes (unsigned short day[7], unsigned char tag)
      {
         for (d = 1; d < 7 && day[d] == day[0]; d++);
         if (d == 7)
         {                      // All same so one entry
            add (tag + 2);
            addbcd2 (day[0]);
         } else
         {
            for (d = 2; d < 6 && day[d] == day[1]; d++);
            if (d == 6)
            {                   // weekdays same
               if (day[0] == day[6])
               {                // weekend same
                  add (tag + 4);
                  addbcd2 (day[0]);
                  addbcd2 (day[1]);
               } else
               {                // different weekend days
                  add (tag + 6);
                  addbcd2 (day[0]);
                  addbcd2 (day[1]);
                  addbcd2 (day[6]);
               }
            } else if (d == 5)
            {                   // Sun,Mon-Thu,Fri,Say
               add (tag + 8);
               addbcd2 (day[0]);
               addbcd2 (day[1]);
               addbcd2 (day[5]);
               addbcd2 (day[6]);
            } else
            {                   // different days
               add (tag + 14);
               for (d = 0; d < 7; d++)
                  addbcd2 (day[d]);
            }
         }
      }
      for (d = 0; d < 7 && !from[d] && to[d] == 0x2400; d++);
      if (d < 7)
      {
         addtimes (from, 0x10);
         addtimes (to, 0x20);
      }
   }
   void addarea (unsigned char tag, const char *name)
   {
      const char *v = sql_col (res, name);
      if (v)
      {
         unsigned int a = 0;
         while (*v)
         {
            const char *p = strchr (AREAS, *v);
            if (p)
               a |= (0x80000000 >> (p - AREAS));
            v++;
         }
         if (!p)
            add (tag);
         else if (!(a & 0xFFFFFF))
         {
            add (tag + 1);
            add (a >> 24);
         } else if (!(a & 0xFFFF))
         {
            add (tag + 2);
            add (a >> 24);
            add (a >> 16);
         } else if (!(a & 0xFF))
         {
            add (tag + 3);
            add (a >> 24);
            add (a >> 16);
            add (a >> 8);
         } else
         {
            add (tag + 4);
            add (a >> 24);
            add (a >> 16);
            add (a >> 8);
            add (a);
         }
      }
   }
   addarea (0xA0, "arm");
   addarea (0xB0, "strong");
   addarea (0xC0, "prop");
   addarea (0xD0, "disarm");
   addarea (0xE0, "enter");
   if (p >= 256)
      return 0;                 // Too big
   *a = p - 1;
   memcpy (afile, a, p);
   return df_crc (*a, a + 1);
}
