// MQTT handling for Solar System

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <semaphore.h>
#include "AJL/ajl.h"
#include "sscert.h"
#include "ssmqtt.h"

extern const char *cakey;
extern const char *cacert;
extern const char *mqttkey;
extern const char *mqttcert;
extern int sqldebug;
SSL_CTX *ctx = NULL;

#define	MAXSLOTS	65536

typedef struct slot_s slot_t;
struct slot_s {
   long long instance;          // Instance for checking, (mod MAXSLOT is slot) 0 if free
   int txsock;                  // Socket for sending data
};

static slot_t slots[MAXSLOTS] = { };

static long long instance = 1;  // Next instance to allocate
static int slotcount = 0;
static pthread_mutex_t slot_mutex;

typedef struct rxq_s rxq_t;
struct rxq_s {
   rxq_t *next;
   j_t j;
};

static rxq_t *rxq = NULL,
    *rxqhead = NULL;
static pthread_mutex_t rxq_mutex;
static sem_t rxq_sem;

static void *server(void *arg)
{
   int sock = -1;
   device_t device = "";
   char address[40] = "";
   {                            // Get passed settings
      j_t j = arg;
      sock = atoi(j_get(j, "socket") ? : "");
      if (!sock)
         errx(1, "socket not set");
      strncpy(address, j_get(j, "address") ? : "", sizeof(address));
      j_delete(&j);
   }
   if (sqldebug)
      warnx("Connect from %s", address);
   int txsock = -1;             // Rx socket
   pthread_mutex_lock(&slot_mutex);
   if (slotcount >= MAXSLOTS)
   {
      warnx("Too many connections %s", address);
      close(sock);
      pthread_mutex_unlock(&slot_mutex);
      return NULL;
   }
   slot_t *slot;
   while ((slot = &slots[instance % MAXSLOTS])->instance)
      instance++;
   {
      int sp[2];
      if (socketpair(AF_LOCAL, SOCK_DGRAM | SOCK_NONBLOCK, 0, sp) < 0)
         err(1, "socketpair");
      slot->txsock = sp[0];
      txsock = sp[1];
   }
   slot->instance = instance;
   slotcount++;
   pthread_mutex_unlock(&slot_mutex);
   // TLS
   SSL *ssl = SSL_new(ctx);
   if (!ssl)
   {
      close(sock);
      errx(1, "Cannot make SSL");
   }
   if (!SSL_set_fd(ssl, sock) || SSL_accept(ssl) != 1)
   {
      close(sock);
      long e;
      while ((e = ERR_get_error()))
      {
         char temp[1000];
         ERR_error_string_n(e, temp, sizeof(temp));
         warnx("%s", temp);
      }
      SSL_free(ssl);
      warnx("Failed SSL from %s", address);
      return NULL;
   }
   X509 *cert = SSL_get_peer_certificate(ssl);
   if (cert)
   {
      X509_NAME *subject = X509_get_subject_name(cert);
      if (subject)
      {
         device_t id;
         if (X509_NAME_get_text_by_NID(subject, NID_commonName, id, sizeof(id)) == sizeof(id) - 1)
            strcpy(device, id);
      }
      X509_free(cert);
   }
   long long message = 0;
   void addq(j_t j, int tlen, char *topic) {
      j_t meta = j_find(j, "_meta");
      if (meta)
         j_delete(&meta);       // Naughty
      meta = j_store_object(j, "_meta");
      j_store_int(meta, "instance", slot->instance);
      j_store_int(meta, "message", message);
      if (*device)
         j_store_string(meta, "device", device);
      j_store_string(meta, "address", address);
      if (topic)
      {
         j_store_stringn(meta, "topic", topic, tlen);
         int n;
         for (n = 0; n < tlen && topic[n] != '/'; n++);
         if (n < tlen)
         {
            j_store_stringn(meta, "prefix", topic, n);
            n++;
            while (n < tlen && topic[n] != '/')
               n++;             // SS
            if (n < tlen)
            {
               n++;
               while (n < tlen && topic[n] != '/')
                  n++;          // ID
               if (n < tlen)
               {
                  n++;
                  if (n < tlen)
                     j_store_stringn(meta, "suffix", topic + n, tlen - n);
               }
            }
         }
      }
      if (!message && (strcmp(j_get(meta, "prefix") ? : "", "state") || j_find(meta, "suffix")))
      {                         // First message has to be system state message, else ignore
         warnx("Unexpected initial message from %s %s", address, device);
         j_delete(&j);
         return;                // Not sent
      }
      message++;
      rxq_t *q = malloc(sizeof(*q));
      q->j = j;
      q->next = NULL;
      pthread_mutex_lock(&rxq_mutex);
      if (rxqhead)
         rxqhead->next = q;
      else
         rxq = q;
      rxqhead = q;
      sem_post(&rxq_sem);
      pthread_mutex_unlock(&rxq_mutex);
   }
   uint8_t rx[10000];
   uint8_t tx[10000];
   uint32_t pos = 0,
       txp;
   const char *fail = NULL;
   uint16_t keepalive = 0;
   time_t katimeout = time(0) + 10;
   int nfds = sock;
   if (txsock > sock)
      nfds = txsock;
   nfds++;
   while (!fail)
   {
      time_t now = time(0);
      if (now >= katimeout)
      {
         fail = "Timeout";
         break;
      }
      fd_set r;
      FD_ZERO(&r);
      FD_SET(sock, &r);
      if (!SSL_has_pending(ssl))
      {
         FD_SET(txsock, &r);
         struct timeval to = { katimeout - now, 0 };
         select(nfds, &r, NULL, NULL, &to);
         if (FD_ISSET(txsock, &r))
            txp = recv(txsock, tx, sizeof(tx), 0);
      }
      if (FD_ISSET(sock, &r))
      {
         {                      // Next block
            int len = SSL_read(ssl, rx + pos, sizeof(rx) - pos);
            if (len <= 0)
            {
               fail = "Closed";
               break;
            }
            pos += len;
         }
         if (fail)
            break;
         if (pos < 2)
            continue;
         uint32_t i,
          l = 0;                // Len is low byte first
         for (i = 1; i < pos && (rx[i] & 0x80); i++)
            l |= (rx[i] & 0x7F) << ((i - 1) * 7);
         l |= (rx[i] & 0x7F) << ((i - 1) * 7);
         i++;
         if (i > pos || (i == pos && (rx[i - 1] & 0x80)))
            continue;           // Need more data
         katimeout = time(0) + keepalive * 3 / 2;
         txp = 0;
         if (sqldebug)
         {
            fprintf(stderr, "%s<", device);
            for (uint32_t q = 0; q < i + l; q++)
               fprintf(stderr, " %02X", rx[q]);
            fprintf(stderr, "\n");
         }
         if (i + l < pos)
            fail = "Bad message len";
         uint8_t *data = rx + i,
             *end = rx + i + l;
         // Process message
         const char *process(void) {
            switch (*rx >> 4)
            {
            case 1:            // Connect
               {
                  if (data + 6 > end || data[0] || data[1] != 4 || memcmp(data + 2, "MQTT", 4))
                     return "Not MQTT format";
                  data += 6;
                  if (data + 1 > end || *data != 4)
                     return "Bad MQTT version";
                  data++;
                  if (data + 1 > end)
                     return "Too short";
                  data++;       // Flags (we don't care)
                  if (data + 2 > end)
                     return "Too short";
                  keepalive = (data[0] << 8) + data[1];
                  katimeout = time(0) + keepalive * 3 / 2;
                  data += 2;
                  // We don't really care about these
                  // Client ID
                  // Will Topic
                  // Will Message
                  // Username
                  // Password
                  // Send connect ack
                  tx[txp++] = 0x20;     // connack
                  tx[txp++] = 2;
                  tx[txp++] = 0;        // no session
                  tx[txp++] = 0;        // clean
               }
               break;
            case 3:            // Publish
               {
                  //uint8_t dup = (*rx >> 3) & 1;
                  uint8_t qos = (*rx >> 1) & 3;
                  //uint8_t retain = *rx & 1;
                  // topic
                  if (data + 2 > end)
                     return "Too short";
                  uint16_t tlen = (data[0] << 8) + data[1];
                  uint16_t id = 0;
                  data += 2;
                  char *topic = (void *) data;
                  data += tlen;
                  if (data > end)
                     return "Too short";
                  if (qos)
                  {
                     if (data + 2 > end)
                        return "Too short";
                     id = (data[0] << 8) + data[1];
                     data += 2;
                  }
                  int plen = end - data;
                  if (sqldebug)
                     warnx("%.*s: %.*s\n", tlen, topic, plen, data);
                  j_t j = j_create();
                  if (j_read_mem(j, (void *) data, plen))
                  {
                     warnx("Non JSON from %s: %.*s", device, plen, data);
                     j_delete(&j);
                  } else
                  {
                     if (!j_isobject(j))
                     {
                        j_t n = j_create();
                        j_store_json(n, "_data", &j);
                        j = n;
                     }
                     addq(j, tlen, topic);
                  }
                  if (qos)
                  {
                     tx[txp++] = (qos == 1 ? 0x40 : 0x50);      // puback/pubrec
                     tx[txp++] = 2;
                     tx[txp++] = (id >> 8);
                     tx[txp++] = id;
                  }
               }
               break;
            case 4:            // Puback
               {
               }
               break;
            case 5:            // Pubrec
               {
               }
               break;
            case 6:            // pubpel
               {
                  if (data + 2 > end)
                     return "Too short";
                  tx[txp++] = 0x70;     // pubcomp
                  tx[txp++] = 2;
                  tx[txp++] = data[0];
                  tx[txp++] = data[1];
               }
               break;
            case 7:            // pubcomp
               {
               }
               break;
            case 8:            // sub
               {
                  if (data + 2 > end)
                     return "Too short";
                  tx[txp++] = 0x90;
                  tx[txp++] = 3;
                  tx[txp++] = data[0];
                  tx[txp++] = data[1];
                  tx[txp++] = 0;        //QoS
               }
               break;
            case 10:           // unsub
               {
               }
               break;
            case 12:           // pingreq
               {
                  tx[txp++] = 0xD0;
                  tx[txp++] = 0;
               }
               break;
            case 14:           // disconnect
               return "Disconnected";
            default:
               return "Unexpected MQTT message code";
            }
            return NULL;
         }
         if (!fail)
            fail = process();
         if (i + l < pos)
            memmove(rx, rx + i + pos, pos - i - l);     // more
         pos -= i + l;
      }
      if (!fail && txp)
      {                         // Send data
         if (sqldebug)
         {
            fprintf(stderr, "%s>", device);
            for (uint32_t i = 0; i < txp; i++)
               fprintf(stderr, " %02X", tx[i]);
            fprintf(stderr, "\n");
         }
         SSL_write(ssl, tx, txp);
      }
   }
   if(message)
   {                            // Down
      j_t j = j_create();
      addq(j, 0, NULL);
   }
   pthread_mutex_lock(&slot_mutex);
   close(slot->txsock);
   slot->txsock = -1;
   slot->instance = 0;
   slotcount--;
   pthread_mutex_unlock(&slot_mutex);
   close(txsock);
   if (fail)
      warnx("Fail from %s (%s)", address, fail);
   if (sqldebug)
      warnx("Closed from %s", address);
   SSL_shutdown(ssl);
   SSL_free(ssl);
   close(sock);
   return NULL;
}

static void *listener(void *arg)
{                               // Listen thread
   arg = arg;
   ctx = SSL_CTX_new(SSLv23_server_method());
   if (!ctx)
      errx(1, "CTX fail");
   if (*mqttkey)
   {
      EVP_PKEY *key = der2pkey(mqttkey);
      SSL_CTX_use_PrivateKey(ctx, key);
      EVP_PKEY_free(key);
   }
   if (*mqttcert)
   {
      X509 *cert = der2x509(mqttcert);
      SSL_CTX_use_certificate(ctx, cert);
      X509_free(cert);
   }
   if (*cacert)
   {
      X509 *cert = der2x509(cacert);
      SSL_CTX_add_client_CA(ctx, cert);
      X509_STORE *ca = X509_STORE_new();
      X509_STORE_add_cert(ca, cert);
      SSL_CTX_set_cert_store(ctx, ca);
      X509_free(cert);
      SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
   }
   int slisten = -1;
 struct addrinfo base = { ai_flags: AI_PASSIVE, ai_family:
#ifdef	CONFIG_MQTT_IPV4
      AF_INET
#else
      AF_ANY
#endif
    , ai_socktype:SOCK_STREAM
   };
   struct addrinfo *a = 0,
       *p;
   if (getaddrinfo(CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT, &base, &a) || !a)
      errx(1, "Failed to find address for %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT);
   for (p = a; p; p = p->ai_next)
   {
      slisten = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (slisten < 0)
         continue;
      int on = 1;
      setsockopt(slisten, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
      if (bind(slisten, p->ai_addr, p->ai_addrlen) || listen(slisten, 10))
      {                         // failed to connect
         close(slisten);
         slisten = -1;
         continue;
      }
      break;
   }
   if (slisten < 0)
      err(1, "Cannot bind local address %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT);
   if (sqldebug)
      warnx("Bind %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT);
   while (1)
   {
      struct sockaddr_in6 addr = { 0 };
      socklen_t len = sizeof(addr);
      int s = accept(slisten, (void *) &addr, &len);
      if (s < 0)
      {
         warn("Bad accept");
         continue;
      }
      char from[INET6_ADDRSTRLEN + 1] = "";
      if (addr.sin6_family == AF_INET)
         inet_ntop(addr.sin6_family, &((struct sockaddr_in *) &addr)->sin_addr, from, sizeof(from));
      else
         inet_ntop(addr.sin6_family, &addr.sin6_addr, from, sizeof(from));
      if (!strncmp(from, "::ffff:", 7) && strchr(from, '.'))
         memmove(from, from + 7, strlen(from + 7) + 1);
      j_t j = j_create();
      j_store_string(j, "address", from);
      j_store_int(j, "socket", s);
      pthread_t t;
      if (pthread_create(&t, NULL, server, j))
         err(1, "Cannot create server thread");
      pthread_detach(t);
   }
   return NULL;
}

void mqtt_start(void)
{                               // Start MQTT server (not a real MQTT server, just talks MQTT)
   pthread_mutex_init(&slot_mutex, NULL);
   pthread_mutex_init(&rxq_mutex, NULL);
   sem_init(&rxq_sem, 0, 0);
   {                            // Set up listen thread
      pthread_t t;
      if (pthread_create(&t, NULL, listener, NULL))
         err(1, "Cannot create listener thread");
      pthread_detach(t);
   }
}

const char *mqtt_send(long long instance, const char *prefix, const char *suffix, j_t * jp)
{
   char *buf = NULL;
   char *topic = NULL;
   j_t j = NULL;
   if (jp)
   {
      j = *jp;
      *jp = NULL;
   }
   const char *process(void) {
      if (asprintf(&topic, "%s/SS/*%s%s", prefix, suffix ? "/" : "", suffix ? : "") < 0)
         return "malloc";
      // Make publish
      uint8_t tx[2048];         // Sane limit
      unsigned int txp = 0;
      tx[txp++] = 0x30;         // Not dup, qos=0, no retain
      tx[txp++] = 0;            // Len TBA
      tx[txp++] = 0;
      unsigned int l = strlen(topic);
      if (txp + l > sizeof(tx))
         return "Topic too big";
      tx[txp++] = (l >> 8);
      tx[txp++] = l;
      if (l)
         memcpy(tx + txp, topic, l);
      txp += l;
      // QoS 0 so not packet ID
      size_t len = 0;
      if (j && j_write_mem(j, &buf, &len))
         return "Bad JSON make";
      if (txp + len > sizeof(tx))
         return "Payload too big";
      if (len)
         memcpy(tx + txp, buf, len);
      txp += len;
      // Store len
      tx[1] = ((txp - 3) & 0x7F) + 0x80;
      tx[2] = ((txp - 3) >> 7);
      pthread_mutex_lock(&slot_mutex);
      if (slots[instance % MAXSLOTS].instance != instance)
      {
         pthread_mutex_unlock(&slot_mutex);
         return "Old instance";
      }
      if (send(slots[instance % MAXSLOTS].txsock, tx, txp, 0) < 0)
      {
         pthread_mutex_unlock(&slot_mutex);
         return "Failed to send";
      }
      pthread_mutex_unlock(&slot_mutex);
      if (sqldebug)
         fprintf(stderr, "Send to %lld: %s %s\n", instance, topic, buf);
      return NULL;
   }
   if (buf)
      free(buf);
   if (topic)
      free(topic);
   const char *fail = process();
   if (fail)
      warnx("tx MQTT fail: %s", fail);
   if (j)
      j_delete(&j);
   return fail;
}

const char *command(long long instance, const char *suffix, j_t * jp)
{                               // Send command (expects _meta.instance to be set)
   return mqtt_send(instance, "command", suffix, jp);
}

const char *setting(long long instance, const char *suffix, j_t * jp)
{                               // Send setting (expects _meta.instance to be set)
   return mqtt_send(instance, "setting", suffix, jp);
}

j_t incoming(void)
{                               // Wait for next rx message
   sem_wait(&rxq_sem);
   pthread_mutex_lock(&rxq_mutex);
   rxq_t *q = rxq;
   if (q)
   {
      rxq = q->next;
      if (!rxq)
         rxqhead = NULL;
   }
   pthread_mutex_unlock(&rxq_mutex);
   if (!q)
      return NULL;              // Should not happen
   j_t j = q->j;
   free(q);
   return j;
}
