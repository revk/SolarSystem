// MQTT message formatting
//
#include "AJL/ajl.h"

j_t mqtt_decode(unsigned char *buf, size_t len)
{                               // Decode and MQTT message, return JSON payload, with topic in _meta.topic
   if ((*buf & 0xF0) != 0x30)
      return NULL;
   unsigned char *b = buf,
       *e = buf + len;
   unsigned char qos = (*b >> 1) & 3;
   unsigned char retain = (*b & 1);
   unsigned char dup = ((*b >> 3) & 1);
   b++;
   int l = 0,
       s = 0;
   while (b < e && (*b & 0x80))
   {
      l |= (*b & 0x7F) << s;
      s += 7;
      b++;
   }
   l |= (*b++ << s);
   if (b + l != e)
      return NULL;
   int id = 0;
   int tlen = (b[0] << 8) + b[1];
   b += 2;
   char *topic = (char *) b;
   b += tlen;
   if (qos)
   {
      id = (b[0] << 8) + b[1];
      b += 2;
   }
   int plen = len - (b - buf);
   j_t j = j_create();
   const char *fail = j_read_mem(j, (char *) b, plen);
   if (fail)
   {
      warnx("Parse error %s %.*s", fail, plen, b);
      j_delete(&j);
      return NULL;
   }
   if (!j_isobject(j) && !j_isnull(j))
   {
      j_t n = j_create();
      j_store_json(n, "_data", &j);
      j = n;
   }
   if (tlen)
      j_stringn(j_path(j, "_meta.topic"), topic, tlen);
   if (qos)
   {
      j_int(j_path(j, "_meta.id"), id);
      j_int(j_path(j, "_meta.qos"), qos);
      if (retain)
         j_true(j_path(j, "_meta.retain"));
      if (dup)
         j_true(j_path(j, "_meta.dup"));
   }
   return j;
}

size_t mqtt_encode(unsigned char *buf, size_t max, j_t j)
{
   return 0;
}

size_t mqtt_login(unsigned char *buf, size_t max)
{
   return 0;
}
