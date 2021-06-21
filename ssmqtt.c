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
#include "AJL/ajl.h"

extern const char *cacert;
extern const char *mqtthost;
extern const char *mqttcert;
extern const char *mqttkey;
extern const char *mqttport;
extern int sqldebug;
SSL_CTX *ctx = NULL;

static void *server(void *arg)
{
   j_t j = arg;
   int s = atoi(j_get(j, "socket") ? : "");
   if (!s)
      errx(1, "socket not set");
   // TLS
   SSL *ssl = SSL_new(ctx);
   if (!ssl)
   {
      close(s);
      errx(1, "Cannot make SSL");
   }
   void fail(void) {
      close(s);
      long e;
      while ((e = ERR_get_error()))
      {
         char temp[1000];
         ERR_error_string_n(e, temp, sizeof(temp));
         warnx("%s", temp);
      }
      SSL_free(ssl);
      errx(1, "Failed SSL from %s", j_get(j, "address"));
   }
   if (!SSL_set_fd(ssl, s))
      fail();
   if (SSL_accept(ssl) != 1)
      fail();
   if (sqldebug)
      warnx("Connect from %s", j_get(j, "address"));
   // SSL_get_verify_result
   // SSL_get_peer_certificate

   warnx("TODO %s", j_get(j, "address"));
   SSL_shutdown(ssl);
   SSL_free(ssl);
   close(s);
   j_delete(&j);
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
      FILE *k = fmemopen((void *) mqttkey, strlen(mqttkey), "r");
      EVP_PKEY *key = PEM_read_PrivateKey(k, NULL, NULL, NULL); // No password
      fclose(k);
      SSL_CTX_use_PrivateKey(ctx, key);
      EVP_PKEY_free(key);
   }
   if (*mqttcert)
   {
      FILE *k = fmemopen((void *) mqttcert, strlen(mqttcert), "r");
      X509 *cert = PEM_read_X509(k, NULL, NULL, NULL);
      fclose(k);
      SSL_CTX_use_certificate(ctx, cert);
      X509_free(cert);
   }
   if (*cacert)
   {
      FILE *k = fmemopen((void *) cacert, strlen(cacert), "r");
      X509 *cert = PEM_read_X509(k, NULL, NULL, NULL);
      fclose(k);
      SSL_CTX_add_client_CA(ctx, cert);
      X509_STORE *ca=X509_STORE_new();
      X509_STORE_add_cert(ca,cert);
      SSL_CTX_set_cert_store(ctx,ca);
      X509_free(cert);
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
   {                            // Set up listen thread
      pthread_t t;
      if (pthread_create(&t, NULL, listener, NULL))
         err(1, "Cannot create listener thread");
      pthread_detach(t);
   }


   // Set up send thread

}

void command(j_t * jp)
{                               // Send command j."command" to j."device", and free j

   j_delete(jp);
}

void setting(j_t * jp)
{                               // Send setting to j."device", and free j

   j_delete(jp);
}
