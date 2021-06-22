// MQTT handling for Solar System
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

extern const char *cacert;
extern const char *mqtthost;
extern const char *mqttcert;
extern const char *mqttkey;
extern const char *mqttport;
extern int sqldebug;
SSL_CTX *ctx = NULL;

// TODO may want hash or something
typedef struct client_s client_t;
struct client_s {
   client_t *next;              // Double linked list
   client_t **prev;
   device_t device;             // Device ID
   int txsock;                  // Socket for sending data
};

static client_t *clients = NULL;
static pthread_mutex_t clients_mutex;

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
   char *address;
   {                            // Get passed settings
      j_t j = arg;
      sock = atoi(j_get(j, "socket") ? : "");
      if (!sock)
         errx(1, "socket not set");
      address = strdup(j_get(j, "address") ? : "");
      j_delete(&j);
   }
   if (sqldebug)
      warnx("Connect from %s", address);
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
      free(address);
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
   int txsock = -1;             // Rx socket
   client_t *c = malloc(sizeof(*c));
   if (!c)
      errx(1, "malloc");
   {
      int sp[2];
      if (socketpair(AF_LOCAL, SOCK_DGRAM, 0, sp) < 0)
         err(1, "socketpair");
      c->txsock = sp[0];
      txsock = sp[1];
   }
   strcpy(c->device, device);
   pthread_mutex_lock(&clients_mutex);
   c->next = clients;
   c->prev = &clients;
   if (c->next)
      c->next->prev = &c->next;
   clients = c;
   pthread_mutex_unlock(&clients_mutex);
   void addq(j_t j, int tlen, void *topic) {

      j_t meta = j_store_object(j, "_meta");
      if (*device)
         j_store_string(meta, "device", device);
      j_store_string(meta, "address", address);
      if (topic)
         j_store_stringn(meta, "topic", topic, tlen);
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
               break;
            pos += len;
         }
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
                  uint8_t *topic = data;
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
   j_t j=j_create();
   j_store_false(j,"up");
   addq(j,0,NULL);
   pthread_mutex_lock(&clients_mutex);
   *c->prev = c->next;
   if (c->next)
      c->next->prev = c->prev;
   pthread_mutex_unlock(&clients_mutex);
   close(c->txsock);
   free(c);
   close(txsock);
   if (fail)
      warnx("Fail from %s (%s)", address, fail);
   if (sqldebug)
      warnx("Closed from %s", address);
   SSL_shutdown(ssl);
   SSL_free(ssl);
   close(sock);
   free(address);
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
 struct addrinfo base = { ai_flags: AI_PASSIVE, ai_family: AF_UNSPEC, ai_socktype:SOCK_STREAM };
   struct addrinfo *a = 0,
       *p;
   if (getaddrinfo(*mqtthost ? mqtthost : NULL, mqttport, &base, &a) || !a)
      errx(1, "Failed to find address for %s:%s", mqtthost, mqttport);
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
      err(1, "Cannot bind local address %s:%s", mqtthost, mqttport);
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
   pthread_mutex_init(&clients_mutex, NULL);
   pthread_mutex_init(&rxq_mutex, NULL);
   sem_init(&rxq_sem, 0, 0);
   {                            // Set up listen thread
      pthread_t t;
      if (pthread_create(&t, NULL, listener, NULL))
         err(1, "Cannot create listener thread");
      pthread_detach(t);
   }
}

void command(j_t * jp)
{                               // Send command j."command" to j."device", and free j

   j_delete(jp);
}

void setting(j_t * jp)
{                               // Send setting to j."device", and free j

   j_delete(jp);
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
