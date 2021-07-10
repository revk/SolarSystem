// MQTT message formatting

#include <string.h>
#include <stdlib.h>
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
      else if (tlen)
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
         int id = n;
         while (topic[n] && topic[n] != '/')
            n++;                // ID
         j_store_stringn(meta, "target", topic + id, n - id);
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

size_t mqtt_encode(unsigned char *tx, size_t max, const char *topic, j_t j)
{
   // Make publish
   unsigned int txp = 0;
   tx[txp++] = 0x30;            // Not dup, qos=0, no retain
   tx[txp++] = 0;               // Len TBA
   tx[txp++] = 0;
   unsigned int l = strlen(topic ? : "");
   if (txp + l > max)
      return 0;
   tx[txp++] = (l >> 8);
   tx[txp++] = l;
   if (l)
      memcpy(tx + txp, topic, l);
   txp += l;
   // QoS 0 so not packet ID
   size_t len = 0;
   char *buf = NULL;
   if (j && !j_isnull(j) && j_write_mem(j, &buf, &len))
      return 0;
   if (txp + len > max)
   {
      free(buf);
      return 0;
   }
   if (len)
      memcpy(tx + txp, buf, len);
   txp += len;
   free(buf);
   // Store len
   tx[1] = ((txp - 3) & 0x7F) + 0x80;
   tx[2] = ((txp - 3) >> 7);
   return txp;
}
