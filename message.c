// Send a message to back end server

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "sscert.h"
#include "AJL/ajl.h"

int main(int argc, const char *argv[])
{
   const char *json = NULL;
   const char *topic = "";
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         { "topic", 't', POPT_ARG_STRING, &topic, 0, "Topic", "string" },
         POPT_AUTOHELP { }
      };

      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
      poptSetOtherOptionHelp(optCon, "<json>");

      int c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));

      json = poptGetArg(optCon);
      if (!json)
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }
      poptFreeContext(optCon);
   }

   j_t j = j_create();
   j_err(j_read_mem(j, json, strlen(json)));
   j_delete(&j);

   j = j_create();
   j_err(j_read_file(j, CONFIG_KEYS_FILE));
   const char *cacert = strdup(j_get(j, "ca.cert") ? : "");
   const char *msgkey = strdup(j_get(j, "msg.key") ? : "");
   const char *msgcert = strdup(j_get(j, "msg.cert") ? : "");
   j_delete(&j);

   int sock = -1;
 struct addrinfo base = { ai_family: AF_UNSPEC, ai_socktype:SOCK_STREAM };
   struct addrinfo *a = 0;
   if (getaddrinfo(CONFIG_MQTT_HOSTNAME, CONFIG_MQTT_PORT, &base, &a) || !a)
      errx(1, "Failed to find address for %s:%s", CONFIG_MQTT_HOSTNAME ? : "localhost", CONFIG_MQTT_PORT);
   for (struct addrinfo * p = a; p; p = p->ai_next)
   {
      sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
      if (sock < 0)
         continue;
      if (connect(sock, p->ai_addr, p->ai_addrlen))
      {
         close(sock);
         sock = -1;
         continue;
      }
      break;
   }
   if (sock < 0)
      err(1, "Cannot connect target address %s:%s", CONFIG_MQTT_HOSTNAME ? : "localhost", CONFIG_MQTT_PORT);
   SSL_CTX *ctx = NULL;
   ctx = SSL_CTX_new(SSLv23_client_method());
   if (!ctx)
      errx(1, "CTX fail");
   if (*msgkey)
   {
      EVP_PKEY *key = der2pkey(msgkey);
      SSL_CTX_use_PrivateKey(ctx, key);
      EVP_PKEY_free(key);
   }
   if (*msgcert)
   {
      X509 *cert = der2x509(msgcert);
      SSL_CTX_use_certificate(ctx, cert);
      X509_free(cert);
   }
   if (*cacert)
   {
      X509 *cert = der2x509(cacert);
      X509_STORE *ca = X509_STORE_new();
      X509_STORE_add_cert(ca, cert);
      SSL_CTX_set_cert_store(ctx, ca);
      X509_free(cert);
      SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
   }
   SSL *ssl = SSL_new(ctx);
   if (!ssl)
      errx(1, "Cannot make SSL");
   if (!SSL_set_fd(ssl, sock) || SSL_connect(ssl) != 1)
   {
      long e;
      while ((e = ERR_get_error()))
      {
         char temp[1000];
         ERR_error_string_n(e, temp, sizeof(temp));
         warnx("%s", temp);
      }
      errx(1, "SSL connect failed");
   }
   {                            // Send login
      unsigned char buf[100],
      *b = buf;
      *b++ = 0x10;              // connect
      *b++ = 0;                 // len
      *b++ = 0;                 // MQTT header
      *b++ = 4;
      *b++ = 'M';
      *b++ = 'Q';
      *b++ = 'T';
      *b++ = 'T';
      *b++ = 4;                 // ver
      *b++ = 0x02;              // clean, no user/pass/etc
      *b++ = 0;                 // keep alive
      *b++ = 60;
      buf[1] = (b - buf - 2);   // len
      SSL_write(ssl, buf, b - buf);
   }
   {                            // Connect reply
      unsigned char buf[100];
      size_t len = SSL_read(ssl, buf, sizeof(buf));
      if (len < 4 || *buf != 0x20 || buf[1] != 2)
         errx(1, "Bad reply to connect %d", (int) len);
   }
   unsigned short id = time(0);
   {                            // Send message
      int tlen = strlen(topic);
      int plen = strlen(json);
      if (tlen + plen + 7 > 2000)
         errx(1, "Message too long");
      unsigned char *buf = malloc(tlen + plen + 7),
          *b = buf;
      *b++ = 0x32;              // QoS 1
      *b++ = 0;                 // Len
      *b++ = 0;
      *b++ = (tlen >> 8);       // Topic
      *b++ = tlen;
      memcpy(b, topic, tlen);
      b += tlen;
      *b++ = (id >> 8);           // ID
      *b++ = id;
      memcpy(b, json, plen);
      b += plen;
      buf[1] = ((b - buf - 3) & 0x7F) + 0x80;
      buf[2] = ((b - buf - 3) >> 7);
      SSL_write(ssl, buf, b - buf);
   }
   {                            // Wait confirmation

      unsigned char buf[100];
      size_t len = SSL_read(ssl, buf, sizeof(buf));
      if (len < 4 || *buf != 0x40 || buf[1] != 2 || (buf[2] << 8) + buf[3] != id)
         errx(1, "Bad reply to message %d %02X %02X %02X %02X %04X", (int) len, buf[0], buf[1], buf[2], buf[3], id);
   }
   {                            // End
      unsigned char buf[100],
      *b = buf;
      *b++ = 0xE0;
      *b++ = 0;
      SSL_write(ssl, buf, b - buf);
   }
   SSL_shutdown(ssl);
   SSL_free(ssl);
   close(sock);

   return 0;
}
