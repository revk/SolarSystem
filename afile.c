// Access file logic library
// Creates the afile to use, based on settings in JSON

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <err.h>
#include <malloc.h>
#include <time.h>
#include <openssl/evp.h>
#include <desfireaes.h>
#include <ajl.h>

uint8_t *makeafile(j_t j)
{                               // Return malloc'd access file
   uint8_t *afile = NULL;
   size_t afilelen = 0;
   FILE *afilef = open_memstream((void *) &afile, &afilelen);
   fputc(0x00, afilef);         // Len place holder
   if (j)
   {
      int debug = j_test(j, "debug", 0);
      if (debug)
         j_err(j_write(j, stderr));
      // Flags
      if (j_test(j, "commit", 0))
         fputc(0xF0, afilef);
      if (j_test(j, "log", 0))
         fputc(0xF1, afilef);
      if (j_test(j, "count", 0))
         fputc(0xF2, afilef);
      if (j_test(j, "block", 0))
         fputc(0xFB, afilef);
      if (j_test(j, "clock", 0))
         fputc(0xFC, afilef);
      // Times
      const char *t = j_get(j, "from");
      if (t && *t)
      {
         unsigned char times[14];
         int l = df_hex(sizeof(times), times, t);
         if (l == 2 || l == 4 || l == 6 || l == 14)
         {
            fputc(0x10 + l, afilef);
            fwrite(times, l, 1, afilef);
         } else if (debug)
            warnx("from time not valid size");
      }
      t = j_get(j, "to");
      if (t && *t)
      {
         unsigned char times[14];
         int l = df_hex(sizeof(times), times, t);
         if (l == 2 || l == 4 || l == 6 || l == 14)
         {
            fputc(0x20 + l, afilef);
            fwrite(times, l, 1, afilef);
         } else if (debug)
            warnx("to time not valid size");
      }

      j_t ex = j_find(j, "expiry");
      if (j_isnumber(ex))
      {                         // Expiry days
         int xdays = atoi(j_val(ex));
         struct tm t;
         time_t now = time(0) + 86400 * (xdays - debug);        // Note debug sets yesterday
         localtime_r(&now, &t);
         char e[7];
         e[0] = 0xE1;
         e[1] = xdays;
         e[2] = 0xE4;           // Date only, so end of day
         int v = t.tm_year + 1900;
         e[3] = (v / 1000) * 16 + (v / 100 % 10);
         e[4] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_mon + 1;
         e[5] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_mday;
         e[6] = (v / 10 % 10) * 16 + (v % 10);
         fwrite(e, 7, 1, afilef);

      } else if (j_isstring(ex))
      {                         // Expiry date/time
         const char *t = j_val(ex);
         uint8_t e[8],
          n = 1;
         while (*t && n < (uint8_t) sizeof(e))
         {
            if (isdigit(*t) && isdigit(t[1]))
            {
               e[n++] = ((*t - '0') << 4) + (t[1] - '0');
               t += 2;
            }
            if (*t && !isdigit(*t))
               t++;
         }
         if (n > 1 && !*t)
         {
            e[0] = 0xE1 + n;
            fwrite(e, n, 1, afilef);
         } else if (debug)
            warnx("Bad expiry");
      }
      void area(uint8_t f, const char *tag) {
         const char *p = j_get(j, tag);
         if (!p)
            return;
         uint32_t a = 0;
         while (*p)
         {
            if (isalpha(*p))
               a |= (1 << (32 - (*p & 0x1F)));
            p++;
         }
         uint8_t e[5],
          n = 1;
         while (a && n < (uint8_t) sizeof(e))
         {
            e[n++] = (a >> 24);
            a <<= 8;
         }
         e[0] = f + 1 + n;
         fwrite(e, n, 1, afilef);
      }
      area(0xA0, "allow");
      area(0xD0, "deadlock");
   }
   fclose(afilef);
   *afile = afilelen - 1;       // Store length in first byte
   if (afilelen > 256)
      errx(1, "Access file too long (%d)", (int) afilelen);
   return afile;
}
