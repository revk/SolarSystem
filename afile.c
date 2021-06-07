// Access file logic

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <malloc.h>
#include <time.h>
#include <openssl/evp.h>
#include <desfireaes.h>
#include <axl.h>

unsigned char *getafile(xml_t config, xml_t user, int debug, int forceallow)
{                               // Return malloc'd access file
   char *afile = NULL;
   size_t afilelen = 0;
   char *allow = NULL,
       *deny = NULL;
   size_t allowlen = 0,
       denylen = 0;
   FILE *allowf = open_memstream(&allow, &allowlen);
   FILE *denyf = open_memstream(&deny, &denylen);
   FILE *afilef = open_memstream(&afile, &afilelen);
   fputc(0x00, afilef);
   // Check doors
   const char *open = xml_get(user, "@open") ? : "*";
   xml_t door = NULL;
   while ((door = xml_element_next_by_name(config, door, "door")))
   {
      const char *devname = xml_get(door, "@max");
      if (!devname)
         devname = xml_get(door, "@min");
      if (!devname)
         devname = xml_get(door, "@i_fob");
      if (!devname)
      {
         if (debug)
            printf("Door with no device\n");
         continue;
      }
      xml_t device = NULL;
      while ((device = xml_element_next_by_name(config, device, "device")))
         if (!strcasecmp(xml_get(device, "@id") ? : "", devname) || !strcasecmp(xml_get(device, "@name") ? : "", devname))
            break;
      if (!device)
      {
         if (debug)
            printf("Door with device not found [%s]\n", devname);
         continue;
      }
      const char *devid = xml_get(device, "@id");
      if (!devid)
      {
         if (debug)
            printf("Door with no device id [%s]\n", devname);
         continue;
      }
      unsigned char id[3];
      if (df_hex(3, id, devid) != 3)
      {
         if (debug)
            printf("Door with bad id [%s] [%s]\n", devid, devname);
         continue;
      }
      const char *lock = xml_get(door, "@lock");
      if (lock && *open != '*')
      {                         // Check if allowed
         const char *c;
         for (c = open; *c && !strchr(lock, *c); c++);
         if (!*c)
            lock = NULL;        // Not allowed
      }
      if (*open == '*' || lock)
      {
         if (debug)
            fprintf(stderr, "Allow %s\n", devid);
         fwrite(id, 3, 1, allowf);
      } else
      {
         if (debug)
            fprintf(stderr, "Deny  %s\n", devid);
         fwrite(id, 3, 1, denyf);
      }
   }
   fclose(allowf);
   fclose(denyf);
   if (!allowlen && !forceallow)
      warnx("User is not allowed to use any doors");
   if (allowlen < denylen)
      forceallow = 1;           // Allow list is shorter
   if (denylen)
   {
      char *devs = allow;
      int devslen = allowlen;
      if (!forceallow)
      {                         // Set allowed
         devs = deny;
         devslen = denylen;
      }
      while (devslen)
      {
         int l = devslen;
         if (l > 14)
            l = 14;
         fputc((forceallow ? 0xA0 : 0xB0) + l, afilef);
         fwrite(devs, l, 1, afilef);
         devslen -= l;
         devs += l;
      }
   }
   // Times
   const char *t = xml_get(user, "@time-from") ? : xml_get(config, "system@time-from");
   if (t && *t)
   {
      unsigned char times[14];
      int l = df_hex(sizeof(times), times, t);
      if (l == 2 || l == 4 || l == 6 || l == 14)
      {
         fputc(0xF0 + l, afilef);
         fwrite(times, l, 1, afilef);
      }
   }
   t = xml_get(user, "@time-to") ? : xml_get(config, "system@time-to");
   if (t && *t)
   {
      unsigned char times[14];
      int l = df_hex(sizeof(times), times, t);
      if (l == 2 || l == 4 || l == 6 || l == 14)
      {
         fputc(0x20 + l, afilef);
         fwrite(times, l, 1, afilef);
      }
   }
   t = xml_get(user, "@time-override");
   if (t && !strcasecmp(t, "true"))
      fputc(0xC0, afilef);
   t = xml_get(user, "@deadlock-override");
   if (t && !strcasecmp(t, "true"))
      fputc(0xD0, afilef);
   const char *ex = xml_get(user, "@expiry") ? : xml_get(config, "system@expiry");
   if (ex)
   {
      time_t expiry = xml_time(ex);
      if (expiry)
      {                         // Explicit expiry
         struct tm t;
         localtime_r(&expiry, &t);
         char e[8];
         e[0] = 0xE7;
         e[1] = 0xE4;
         int v = t.tm_year + 1900;
         e[2] = (v / 1000) * 16 + (v / 100 % 10);
         e[3] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_mon + 1;
         e[4] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_mday;
         e[5] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_hour;
         e[6] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_min;
         e[7] = (v / 10 % 10) * 16 + (v % 10);
         v = t.tm_sec;
         e[8] = (v / 10 % 10) * 16 + (v % 10);
         fwrite(e, 8, 1, afilef);
      } else
      {
         int xdays = atoi(ex);
         if (xdays)
         {                      // Auto expiry
            struct tm t;
            time_t now = time(0) + 86400 * (xdays - debug);
            localtime_r(&now, &t);
            char e[7];
            e[0] = 0xE1;
            e[1] = xdays;
            e[2] = 0xE4;
            int v = t.tm_year + 1900;
            e[3] = (v / 1000) * 16 + (v / 100 % 10);
            e[4] = (v / 10 % 10) * 16 + (v % 10);
            v = t.tm_mon + 1;
            e[5] = (v / 10 % 10) * 16 + (v % 10);
            v = t.tm_mday;
            e[6] = (v / 10 % 10) * 16 + (v % 10);
            fwrite(e, 7, 1, afilef);
         }
      }
   }
   fclose(afilef);
   *afile = afilelen - 1;       // Store length in first byte
   if (afilelen > 256)
      errx(1, "Access file too long (%d)", (int) afilelen);
   if (allow)
      free(allow);
   if (deny)
      free(deny);
   return (unsigned char *) afile;
}
