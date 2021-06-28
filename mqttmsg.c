// MQTT message formatting

#include <string.h>
#include "AJL/ajl.h"
#include "mqttmsg.h"

void mqtt_topic(j_t j, const char *topic, int tlen)
{                               // Break down _meta.topic in to prefix and suffix (optionally store it too)
   if (!j)
      return;
   j_t meta = j_path(j, "_meta");
   if (topic)
   {
      if (tlen < 0)
         j_store_string(meta, "topic", topic);
      else
         j_store_stringn(meta, "topic", topic, tlen);
   }
   topic = j_get(meta, "topic");
   if (!topic)
      return;
   int n;
   for (n = 0; topic[n] && topic[n] != '/'; n++);
   if (topic[n])
   {
      j_store_stringn(meta, "prefix", topic, n);
      n++;
      while (topic[n] && topic[n] != '/')
         n++;                   // SS
      if (topic[n])
      {
         n++;
         while (topic[n] && topic[n] != '/')
            n++;                // ID
         if (topic[n])
         {
            n++;
            if (topic[n])
               j_store_string(meta, "suffix", topic + n);
         }
      }
   }
}

void mqtt_dataonly(j_t j)
{                               // Remove meta
   j_t meta = j_find(j, "_meta");
   if (meta)
      j_delete(&meta);
   j_t data = j_find(j, "_data");
   if (data)
      j_replace(j, &data);
}

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
   if (plen)
   {
      const char *fail = j_read_mem(j, (char *) b, plen);
      if (fail)
      {
         warnx("Parse error %s %.*s", fail, plen, b);
         j_delete(&j);
         return NULL;
      }
   }
   if (!j_isobject(j))
   {
      j_t n = j_create();
      j_store_json(n, "_data", &j);
      j = n;
   }
   mqtt_topic(j, topic, tlen);
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
   // TODO
   return 0;
}

size_t mqtt_login(unsigned char *buf, size_t max)
{
   // TODO
   return 0;
}
