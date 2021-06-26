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
#include "sscert.h"
#include "AJL/ajl.h"

int main(int argc, const char *argv[])
{
   const char *json = NULL;
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
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

   j=j_create();
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
   ctx = SSL_CTX_new(SSLv23_server_method());
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
      SSL_CTX_add_client_CA(ctx, cert);
      X509_STORE *ca = X509_STORE_new();
      X509_STORE_add_cert(ca, cert);
      SSL_CTX_set_cert_store(ctx, ca);
      X509_free(cert);
      SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
   }





   close(sock);

   return 0;
}
