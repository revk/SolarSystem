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
#include "mqttmsg.h"
#include "ssmqtt.h"

#define	MAX_SETTING	1400    // Limit size of settings - allow for header and AES padding even

extern const char *cakey;
extern const char *cacert;
extern const char *mqttkey;
extern const char *mqttcert;
extern int sqldebug;
extern int dump;
extern int mqttdump;
SSL_CTX *ctx = NULL;

static void *
insecureserver (void *arg)
{                               // Non TLS server that just sends new setting to get secure connection
   int sock = -1;
   device_t device = "";
   char address[40] = "";
   {                            // Get passed settings
      j_t j = arg;
      sock = atoi (j_get (j, "socket") ? : "");
      if (!sock)
         errx (1, "server socket not set");
      strncpy (address, j_get (j, "address") ? : "", sizeof (address));
      j_delete (&j);
   }

   warnx ("Connect from %s (insecure)", address);

   uint8_t rx[10000];
   uint8_t tx[10000];
   int pos = 0,
      txp = 0;
   const char *fail = NULL;
   uint16_t keepalive = 0;
   time_t katimeout = time (0) + 10;
   int nfds = sock;
   nfds++;
   while (!fail)
   {
      time_t now = time (0);
      if (now >= katimeout)
      {
         fail = "Timeout";
         break;
      }
      fd_set r;
      FD_ZERO (&r);
      FD_SET (sock, &r);

      {
         struct timeval to = { pos ? 1 : katimeout - now, 0 };
         select (nfds, &r, NULL, NULL, &to);
      }

      if (!txp)
      {
         if (FD_ISSET (sock, &r))
         {                      // Next block
            int len = read (sock, rx + pos, sizeof (rx) - pos);
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
         int i,
           l = 0;               // Len is low byte first
         for (i = 1; i < pos && (rx[i] & 0x80); i++)
            l |= (rx[i] & 0x7F) << ((i - 1) * 7);
         l |= (rx[i] & 0x7F) << ((i - 1) * 7);
         i++;
         if (i > pos || (i == pos && (rx[i - 1] & 0x80)))
            continue;           // Need more data
         katimeout = time (0) + keepalive * 3 / 2;
         if (dump)
         {
            fprintf (stderr, "[insecure]%s<", device);
            for (int q = 0; q < i + l; q++)
               fprintf (stderr, " %02X", rx[q]);
            fprintf (stderr, "\n");
         }
         uint8_t *data = rx + i,
            *end = rx + i + l;
         // Process message
         const char *process (void)
         {
            switch (*rx >> 4)
            {
            case 1:            // Connect
               {
                  if (data + 6 > end || data[0] || data[1] != 4 || memcmp (data + 2, "MQTT", 4))
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
                  katimeout = time (0) + keepalive * 3 / 2;
                  data += 2;
                  // Client ID
                  int clientlen = (data[0] << 8) + data[1];
                  data += 2;
                  char *client = (char *) data;
                  data += clientlen;
                  // We don't really care about these
                  // Will Topic
                  // Will Message
                  // Username
                  // Password
                  // Send connect ack
                  tx[txp++] = 0x20;     // connack
                  tx[txp++] = 2;
                  tx[txp++] = 0;        // no session
                  tx[txp++] = 0;        // clean
                  if (clientlen != 15 || strncmp (client, "SS-", 3))
                     return "Bad client ID";
                  // Send new setting
                  char *topic = NULL;
                  if (asprintf (&topic, "setting/SS/%.12s", client + 3) <= 0)
                     errx (1, "malloc");
                  j_t j = j_create ();
                  j_t m = j_store_object (j, "mqtt");
                  j_store_string (m, "host", CONFIG_MQTT_HOSTNAME);
                  j_store_string (m, "cert", cacert);
                  txp += mqtt_encode (tx + txp, sizeof (tx) - txp, topic, j);
                  j_delete (&j);
                  free (topic);
               }
               break;
            case 3:            // Publish
               break;
            case 4:            // Puback
               break;
            case 5:            // Pubrec
               break;
            case 6:            // pubpel
               break;
            case 7:            // pubcomp
               break;
            case 8:            // sub
               {                // ACK the subscribe
                  if (data + 2 > end)
                     return "Too short";
                  unsigned short id = (data[0] << 8) + data[1];
                  data += 2;
                  // Ack
                  tx[txp++] = 0x90;
                  txp += 2;     // len
                  tx[txp++] = (id >> 8);
                  tx[txp++] = id;
                  while (data + 2 <= end)
                  {
                     int l = (data[0] << 8) + data[1];
                     data += 2;
                     data += l;
                     uint8_t qos = *data++;
                     tx[txp++] = qos;
                  }
                  tx[1] = 0x80 + ((txp - 3) & 0x7f);    // len
                  tx[2] = ((txp - 3) >> 7);
               }
               break;
            case 10:           // unsub
               return "Not expecting unsubscribe";
               break;
            case 12:           // pingreq
               {
                  tx[txp++] = 0xD0;
                  tx[txp++] = 0;
               }
               break;
            case 14:           // disconnect
               return "*Clean disconnect";
            default:
               warnx ("Message code %d", *rx >> 4);
               return "Unexpected MQTT message code";
            }
            return NULL;
         }
         if (!fail)
            fail = process ();
         if (i + l < pos)
            memmove (rx, rx + i + l, pos - i - l);      // more
         pos -= i + l;
      }
      if (!fail && txp)
      {                         // Send data
         if (dump)
         {
            fprintf (stderr, "[insecure]%s>", device);
            for (int i = 0; i < txp; i++)
               fprintf (stderr, " %02X", tx[i]);
            fprintf (stderr, "\n");
         }
         if (write (sock, tx, txp) <= 0)
            fail = "tx fail";
         txp = 0;
      }
   }
   if (fail)
      warnx ("Disconnect from %s (insecure): %s", address, fail);
   close (sock);
   return NULL;
}

static void *
server (void *arg)
{
   int sock = -1;
   device_t device = "";
   char address[40] = "";
   {                            // Get passed settings
      j_t j = arg;
      sock = atoi (j_get (j, "socket") ? : "");
      if (!sock)
         errx (1, "server socket not set");
      strncpy (address, j_get (j, "address") ? : "", sizeof (address));
      j_delete (&j);
   }

   warnx ("Connect from %s", address);

   int relaysock = -1;          // The socket we listen to for tx messages to other side
   slot_t us = slot_create (&relaysock, address);
   if (!us)
   {
      close (sock);
      return "Failed";
   }

   // TLS
   SSL *ssl = SSL_new (ctx);
   if (!ssl)
   {
      close (sock);
      errx (1, "Cannot make SSL");
   }
   if (!SSL_set_fd (ssl, sock) || SSL_accept (ssl) != 1)
   {
      close (sock);
      long e;
      while ((e = ERR_get_error ()))
      {
         char temp[1000];
         ERR_error_string_n (e, temp, sizeof (temp));
         warnx ("%s", temp);
      }
      SSL_shutdown (ssl);
      SSL_free (ssl);
      warnx ("Failed SSL from %s", address);
      slot_destroy (us);
      return NULL;
   }
   X509 *cert = SSL_get_peer_certificate (ssl);
   if (cert)
   {
      X509_NAME *subject = X509_get_subject_name (cert);
      if (subject)
      {
         device_t id;
         if (X509_NAME_get_text_by_NID (subject, NID_commonName, id, sizeof (id)) == sizeof (id) - 1)
            strcpy (device, id);
      }
      X509_free (cert);
   }
   void addq (j_t * jp)
   {
      j_t j = *jp;
      if (*device == '-')
         j_int (j_path (j, "_meta.local"), us);
      else
      {
         j_t meta = j_store_object (j, "_meta");
         j_store_int (meta, "id", us);
         if (*device)
            j_store_string (meta, "device", device);
         j_store_string (meta, "address", address);
         mqtt_topic (j, NULL, 0);
      }
      mqtt_qin (jp);
   }
   uint8_t rx[10000];
   uint8_t tx[10000];
   int pos = 0,
      txp = 0;
   const char *fail = NULL;
   uint16_t keepalive = 0;
   time_t katimeout = time (0) + 10;
   int nfds = sock;
   if (relaysock > sock)
      nfds = relaysock;
   nfds++;
   while (!fail)
   {
      time_t now = time (0);
      if (now >= katimeout)
      {
         fail = "Timeout";
         break;
      }
      fd_set r;
      FD_ZERO (&r);
      FD_SET (sock, &r);
      if (!SSL_has_pending (ssl))
      {
         FD_SET (relaysock, &r);
         struct timeval to = { pos ? 1 : katimeout - now, 0 };  // The 1 is a catch all for possible multiple messages, should not happen
         select (nfds, &r, NULL, NULL, &to);
         if (FD_ISSET (relaysock, &r))
         {
            txp = recv (relaysock, tx, sizeof (tx), 0);
            if (txp <= 0)
               break;           // Closed connection
         }
      }
      if (!txp && FD_ISSET (sock, &r))
      {
         {                      // Next block
            int len = SSL_read (ssl, rx + pos, sizeof (rx) - pos);
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
         int i,
           l = 0;               // Len is low byte first
         for (i = 1; i < pos && (rx[i] & 0x80); i++)
            l |= (rx[i] & 0x7F) << ((i - 1) * 7);
         l |= (rx[i] & 0x7F) << ((i - 1) * 7);
         i++;
         if (i > pos || (i == pos && (rx[i - 1] & 0x80)))
            continue;           // Need more data
         katimeout = time (0) + keepalive * 3 / 2;
         if (dump)
         {
            fprintf (stderr, "%s<", device);
            for (int q = 0; q < i + l; q++)
               fprintf (stderr, " %02X", rx[q]);
            fprintf (stderr, "\n");
         }
         uint8_t *data = rx + i,
            *end = rx + i + l;
         // Process message
         const char *process (void)
         {
            switch (*rx >> 4)
            {
            case 1:            // Connect
               {
                  if (data + 6 > end || data[0] || data[1] != 4 || memcmp (data + 2, "MQTT", 4))
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
                  katimeout = time (0) + keepalive * 3 / 2;
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
                  uint8_t qos = (*rx >> 1) & 3;
                  j_t j = mqtt_decode (rx, i + l);
                  if (j)
                  {
                     if (qos)
                     {
                        int id = atoi (j_get (j, "_meta.id") ? : "");
                        tx[txp++] = (qos == 1 ? 0x40 : 0x50);   // puback/pubrec
                        tx[txp++] = 2;
                        tx[txp++] = (id >> 8);
                        tx[txp++] = id;
                     }
                     addq (&j);
                  }
               }
               break;
            case 4:            // Puback
               break;
            case 5:            // Pubrec
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
               break;
            case 8:            // sub
               {
                  if (data + 2 > end)
                     return "Too short";
                  unsigned short id = (data[0] << 8) + data[1];
                  data += 2;
                  // Ack
                  tx[txp++] = 0x90;
                  txp += 2;     // len
                  tx[txp++] = (id >> 8);
                  tx[txp++] = id;
                  j_t j = j_create ();
                  j_t s = j_array (j_path (j, "_meta.subscribe"));
                  while (data + 2 <= end)
                  {
                     int l = (data[0] << 8) + data[1];
                     data += 2;
                     if (data + l <= end)
                        j_append_stringn (s, (char *) data, l);
                     data += l;
                     uint8_t qos = *data++;
                     tx[txp++] = qos;
                  }
                  addq (&j);
                  tx[1] = 0x80 + ((txp - 3) & 0x7f);    // len
                  tx[2] = ((txp - 3) >> 7);
               }
               break;
            case 10:           // unsub
               {
                  if (data + 2 > end)
                     return "Too short";
                  unsigned short id = (data[0] << 8) + data[1];
                  data += 2;
                  j_t j = j_create ();
                  j_t s = j_array (j_path (j, "_meta.unsubscribe"));
                  while (data + 2 <= end)
                  {
                     int l = (data[0] << 8) + data[1];
                     data += 2;
                     if (data + l <= end)
                        j_append_stringn (s, (char *) data, l);
                     data += l;
                  }
                  addq (&j);
                  // Ack
                  tx[txp++] = 0xB0;
                  tx[txp++] = 2;
                  tx[txp++] = (id >> 8);
                  tx[txp++] = id;
               }
               break;
            case 12:           // pingreq
               {
                  tx[txp++] = 0xD0;
                  tx[txp++] = 0;
               }
               break;
            case 14:           // disconnect
               return "*Clean disconnect";
            default:
               return "Unexpected MQTT message code";
            }
            return NULL;
         }
         if (!fail)
            fail = process ();
         if (i + l < pos)
            memmove (rx, rx + i + l, pos - i - l);      // more - but we don't expect that as TLS packets should be one message
         pos -= i + l;
      }
      if (!fail && txp)
      {                         // Send data
         if (dump)
         {
            fprintf (stderr, "%s>", device);
            for (int i = 0; i < txp; i++)
               fprintf (stderr, " %02X", tx[i]);
            fprintf (stderr, "\n");
         }
         SSL_write (ssl, tx, txp);
         txp = 0;
      }
   }
   if (*device != '-')
   {                            // Down message if it sent and up and not internal
      j_t j = j_create ();
      addq (&j);
   }
   slot_t linked = slot_linked (us);
   if (linked)
      slot_close (linked);      // tell linked to close (e.g. fobcommand task)
   slot_destroy (us);
   if (fail && *fail != '*')
      warnx ("Fail from %s (%s)", address, fail);
   else
      warnx ("Closed from %s", address);
   SSL_shutdown (ssl);
   SSL_free (ssl);
   close (sock);
   return NULL;
}

static void *
insecurelistener (void *arg)
{                               // Listen thread
   arg = arg;
   int slisten = -1;
 struct addrinfo base = { ai_flags: AI_PASSIVE, ai_family:
#ifdef	CONFIG_MQTT_IPV4
      AF_INET
#else
      AF_UNSPEC
#endif
    , ai_socktype:SOCK_STREAM
   };
   struct addrinfo *a = 0,
      *p;
   if (getaddrinfo (CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_INSECURE_PORT, &base, &a) || !a)
      errx (1, "Failed to find address for %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_INSECURE_PORT);
   for (p = a; p; p = p->ai_next)
   {
      slisten = socket (p->ai_family, p->ai_socktype, p->ai_protocol);
      if (slisten < 0)
         continue;
      int on = 1;
      setsockopt (slisten, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
      if (bind (slisten, p->ai_addr, p->ai_addrlen) || listen (slisten, 10))
      {                         // failed to connect
         close (slisten);
         slisten = -1;
         continue;
      }
      break;
   }
   freeaddrinfo (a);
   if (slisten < 0)
      err (1, "Cannot bind local address %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_INSECURE_PORT);
   if (sqldebug)
      warnx ("Bind %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_INSECURE_PORT);
   while (1)
   {
      struct sockaddr_in6 addr = { 0 };
      socklen_t len = sizeof (addr);
      int s = accept (slisten, (void *) &addr, &len);
      if (s < 0)
      {
         warn ("Bad accept");
         continue;
      }
      char from[INET6_ADDRSTRLEN + 1] = "";
      if (addr.sin6_family == AF_INET)
         inet_ntop (addr.sin6_family, &((struct sockaddr_in *) &addr)->sin_addr, from, sizeof (from));
      else
         inet_ntop (addr.sin6_family, &addr.sin6_addr, from, sizeof (from));
      if (!strncmp (from, "::ffff:", 7) && strchr (from, '.'))
         memmove (from, from + 7, strlen (from + 7) + 1);
      j_t j = j_create ();
      j_store_string (j, "address", from);
      j_store_int (j, "socket", s);
      pthread_t t;
      if (pthread_create (&t, NULL, insecureserver, j))
         err (1, "Cannot create insecure server thread");
      pthread_detach (t);
   }
   return NULL;
}

static void *
listener (void *arg)
{                               // Listen thread
   arg = arg;
   ctx = SSL_CTX_new (SSLv23_server_method ());
   if (!ctx)
      errx (1, "CTX fail");
   if (*mqttkey)
   {
      EVP_PKEY *key = der2pkey (mqttkey);
      if (SSL_CTX_use_PrivateKey (ctx, key) != 1)
         errx (1, "Failed to set private key");
      EVP_PKEY_free (key);
   }
   if (*mqttcert)
   {
      X509 *cert = der2x509 (mqttcert);
      if (SSL_CTX_use_certificate (ctx, cert) != 1)
         errx (1, "Failed to set certificate");
      X509_free (cert);
   }
   if (*cacert)
   {
      X509 *cert = der2x509 (cacert);
      if (SSL_CTX_add_client_CA (ctx, cert) != 1)
         errx (1, "Failed to set client CA");
      X509_STORE *ca = X509_STORE_new ();
      X509_STORE_add_cert (ca, cert);
      SSL_CTX_set_cert_store (ctx, ca);
      X509_free (cert);
      SSL_CTX_set_verify (ctx, SSL_VERIFY_PEER, NULL);
   }
   int slisten = -1;
 struct addrinfo base = { ai_flags: AI_PASSIVE, ai_family:
#ifdef	CONFIG_MQTT_IPV4
      AF_INET
#else
      AF_UNSPEC
#endif
    , ai_socktype:SOCK_STREAM
   };
   struct addrinfo *a = 0,
      *p;
   if (getaddrinfo (CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT, &base, &a) || !a)
      errx (1, "Failed to find address for %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT);
   for (p = a; p; p = p->ai_next)
   {
      slisten = socket (p->ai_family, p->ai_socktype, p->ai_protocol);
      if (slisten < 0)
         continue;
      int on = 1;
      setsockopt (slisten, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
      if (bind (slisten, p->ai_addr, p->ai_addrlen) || listen (slisten, 10))
      {                         // failed to connect
         close (slisten);
         slisten = -1;
         continue;
      }
      break;
   }
   freeaddrinfo (a);
   if (slisten < 0)
      err (1, "Cannot bind local address %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT);
   if (sqldebug)
      warnx ("Bind %s:%s", CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT);
   while (1)
   {
      struct sockaddr_in6 addr = { 0 };
      socklen_t len = sizeof (addr);
      int s = accept (slisten, (void *) &addr, &len);
      if (s < 0)
      {
         warn ("Bad accept");
         continue;
      }
      char from[INET6_ADDRSTRLEN + 1] = "";
      if (addr.sin6_family == AF_INET)
         inet_ntop (addr.sin6_family, &((struct sockaddr_in *) &addr)->sin_addr, from, sizeof (from));
      else
         inet_ntop (addr.sin6_family, &addr.sin6_addr, from, sizeof (from));
      if (!strncmp (from, "::ffff:", 7) && strchr (from, '.'))
         memmove (from, from + 7, strlen (from + 7) + 1);
      j_t j = j_create ();
      j_store_string (j, "address", from);
      j_store_int (j, "socket", s);
      pthread_t t;
      if (pthread_create (&t, NULL, server, j))
         err (1, "Cannot create server thread");
      pthread_detach (t);
   }
   return NULL;
}

static pthread_mutex_t rxq_mutex;
static sem_t rxq_sem;
static pthread_mutex_t slot_mutex;

void
mqtt_start (void)
{                               // Start MQTT server (not a real MQTT server, just talks MQTT)
   OPENSSL_init_ssl (0, NULL);
   pthread_mutex_init (&slot_mutex, NULL);
   pthread_mutex_init (&rxq_mutex, NULL);
   sem_init (&rxq_sem, 0, 0);
   {                            // Set up listen thread (secure)
      pthread_t t;
      if (pthread_create (&t, NULL, listener, NULL))
         err (1, "Cannot create listener thread");
      pthread_detach (t);
   }
   {                            // Set up listen thread (insecure)
      pthread_t t;
      if (pthread_create (&t, NULL, insecurelistener, NULL))
         err (1, "Cannot create listener thread");
      pthread_detach (t);
   }
}

// Incoming queue
typedef struct rxq_s rxq_t;
struct rxq_s
{
   rxq_t *next;
   j_t j;
};

static rxq_t *rxq = NULL,
   *rxqhead = NULL;
void
mqtt_qin (j_t * jp)
{                               // Queue incoming
   j_t j = *jp;
   rxq_t *q = malloc (sizeof (*q));
   q->j = j;
   *jp = NULL;
   q->next = NULL;
   pthread_mutex_lock (&rxq_mutex);
   if (rxqhead)
      rxqhead->next = q;
   else
      rxq = q;
   rxqhead = q;
   sem_post (&rxq_sem);
   pthread_mutex_unlock (&rxq_mutex);
}

j_t
incoming (void)
{                               // Wait for next rx message
   const struct timespec to = { time (0) + 60, 0 };
   if (sem_timedwait (&rxq_sem, &to))
      return NULL;
   pthread_mutex_lock (&rxq_mutex);
   rxq_t *q = rxq;
   if (q)
   {
      rxq = q->next;
      if (!rxq)
         rxqhead = NULL;
   }
   pthread_mutex_unlock (&rxq_mutex);
   if (!q)
      return NULL;              // Should not happen
   j_t j = q->j;
   free (q);
   if (mqttdump)
   {
      j_t meta = j_find (j, "_meta");
      if (meta)
         j_detach (meta);
      if (meta)
      {
         slot_t id = strtoll (j_get (meta, "id") ? : j_get (meta, "local") ? : "", NULL, 10);
         if (id)
            fprintf (stderr, "%lld", id);
      }
      fprintf (stderr, "<:");
      if (meta)
      {
         const char *topic = j_get (meta, "topic");
         if (topic)
            fprintf (stderr, "%s ", topic);
         else
         {
            if ((topic = j_get (meta, "prefix")))
               fprintf (stderr, "%s/", topic);
            if ((topic = j_get (meta, "target")))
               fprintf (stderr, "%s/", topic);
            if ((topic = j_get (meta, "suffix")))
               fprintf (stderr, "%s", topic);
            fprintf (stderr, " ");
         }
      }
      if (j_isobject (j) && !j_len (j) && meta)
      {
         fprintf (stderr, "*");
         j_err (j_write (meta, stderr));
      } else
         j_err (j_write (j, stderr));
      fprintf (stderr, "\n");
      if (meta)
         j_store_json (j, "_meta", &meta);
   }
   return j;
}

// Server slot

#define	SLOT_MAX	65536
static int slot_count = 0;
struct slots_s
{
   slot_t id;                   // Slot ID
   slot_t linked;               // Linked ID
   int txsock;                  // Socket for sending data
   int rxsock;                  // Socket for receiving data
};
typedef struct slots_s slots_t;
static slots_t slots[SLOT_MAX] = { };

slot_t
slot_create (int *sockp, const char *address)
{                               // Create a slot, and allocated the non blocking linked sockets, returns server socket
   static slot_t slot_id = 1;   // Next id to allocate
   pthread_mutex_lock (&slot_mutex);
   if (slot_count >= SLOT_MAX)
   {
      warnx ("Too many connections");
      pthread_mutex_unlock (&slot_mutex);
      return 0;
   }
   slots_t *slot;
   while ((slot = &slots[slot_id % SLOT_MAX])->id)
      slot_id++;                // One has to exist based on slot_count - maybe one day make a free slot linkage
   {
      int sp[2];
      if (socketpair (AF_LOCAL, SOCK_DGRAM, 0, sp) < 0)
         err (1, "socketpair");
      slot->txsock = sp[0];
      slot->rxsock = sp[1];
      if (sockp)
         *sockp = sp[1];
   }
   slot->id = slot_id;
   slot->linked = 0;
   slot_count++;
   pthread_mutex_unlock (&slot_mutex);
   if (mqttdump)
      warnx ("Create %lld, slot count %d, %s", slot_id, slot_count, address);
   return slot_id;
}

slot_t
slot_linked (slot_t id)
{                               // Return the linked slot (cleared if linked slot has been destroyed)
   if (!id)
      return 0;
   slot_t linked = 0;
   slots_t *s = &slots[id % SLOT_MAX];
   pthread_mutex_lock (&slot_mutex);
   if (s->id == id && (linked = s->linked) && slots[linked % SLOT_MAX].id != linked)
      s->linked = linked = 0;   // Bad far end
   pthread_mutex_unlock (&slot_mutex);
   return linked;
}

void
slot_link (slot_t id, slot_t linked)
{                               // Link two slots, both show slot_linked as the other
   if (!id)
      return;
   slots_t *s = &slots[id % SLOT_MAX];
   pthread_mutex_lock (&slot_mutex);
   if (s->linked && slots[s->linked % SLOT_MAX].id && slots[s->linked % SLOT_MAX].linked == id)
      slots[s->linked % SLOT_MAX].linked = 0;   // Unlink
   if (s->id == id)
      s->linked = linked;
   pthread_mutex_unlock (&slot_mutex);
}

void
slot_close (slot_t id)
{                               // Sends the server a zero length message to the server asking it to close and destroy
   if (!id)
      return;
   slots_t *s = &slots[id % SLOT_MAX];
   pthread_mutex_lock (&slot_mutex);
   if (s->id == id)
      send (s->txsock, NULL, 0, 0);     // Could, in theory, hang...
   pthread_mutex_unlock (&slot_mutex);
   if (mqttdump)
      warnx ("Close request %lld", id);
}

void
slot_destroy (slot_t id)
{                               // Destroy a slot, called by server when closing, handles socket closing, unlinks
   if (!id)
      return;
   slots_t *s = &slots[id % SLOT_MAX];
   pthread_mutex_lock (&slot_mutex);
   if (s->id == id)
   {
      slot_t linked = s->linked;
      if (slots[linked % SLOT_MAX].id == linked && slots[linked % SLOT_MAX].linked == id)
         slots[linked % SLOT_MAX].linked = 0;
      s->id = 0;
      s->linked = 0;
      close (s->txsock);
      close (s->rxsock);
      s->txsock = -1;
      s->rxsock = -1;
      slot_count--;
   }
   pthread_mutex_unlock (&slot_mutex);
   if (mqttdump)
      warnx ("Destroy %lld, slot count %d", id, slot_count);
}

const char *
slot_send (slot_t id, const char *prefix, const char *deviceid, const char *suffix, j_t * jp)
{
   char *topic = NULL;
   j_t j = NULL;
   if (jp)
   {
      j = *jp;
      *jp = NULL;
   }
   const char *process (j_t j)
   {
      if (!topic
          && ((!prefix && !(topic = strdup ("")))
              || (prefix
                  && asprintf (&topic, "%s/SS/%s%s%s", prefix, (deviceid && *deviceid) ? deviceid : "*", suffix ? "/" : "",
                               suffix ? : "") < 0)))
         return "malloc";

      uint8_t tx[2048];         // Sane limit
      unsigned int txp = mqtt_encode (tx, sizeof (tx), topic, j);

      pthread_mutex_lock (&slot_mutex);
      if (slots[id % SLOT_MAX].id != id)
      {
         pthread_mutex_unlock (&slot_mutex);
         warnx ("Slot Send %lld but slot is %lld", id, slots[id % SLOT_MAX].id);
         return slots[id % SLOT_MAX].id ? "Recycled id" : "Closed id";
      }
      if (send (slots[id % SLOT_MAX].txsock, tx, txp, 0) < 0)
      {
         pthread_mutex_unlock (&slot_mutex);
         return "Failed to send";
      }
      pthread_mutex_unlock (&slot_mutex);
      if (mqttdump)
      {
         fprintf (stderr, "%lld>:%s ", id, topic);
         if (j)
            j_err (j_write (j, stderr));
         fprintf (stderr, "\n");
      }
      return NULL;
   }
   const char *fail = NULL;
#ifdef	MAX_SETTING
   if (prefix && !strcmp (prefix, "setting"))
   {
      int len (j_t j)
      {
         char *t = j_write_str (j);
         if (!t)
         {
            fail = "JSON failed";
            return -1;
         }
         int len = strlen (t);
         free (t);
         return len;
      }
      while (!fail && len (j) > MAX_SETTING)
      {
         j_t n = j_create (),
            c;
         j_object (n);
         int l = 3;
         while ((c = j_first (j)))
         {
            int ln = strlen (j_name (c)) + 3;
            int lc = len (c);
            if (l + ln + lc > MAX_SETTING)
               break;
            j_store_json (n, j_name (c), &c);
            l += ln + lc + 1;
         }
         if (j_len (n))
         {
            process (n);
            j_delete (&n);
         } else
         {                      // Did not fit, so have to send anyway
            warnx ("Setting too long");
            j_err (j_write (j, stderr));
            j_delete (&n);
            break;
         }
      }
      if (j_len (j))
         process (j);
   } else
      fail = process (j);       // Not a setting
#else
   fail = process (j);          // Just process it
#endif
   if (fail)
      warnx ("tx MQTT fail: %s", fail);
   free (topic);
   j_delete (&j);
   return fail;
}
