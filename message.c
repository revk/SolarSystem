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
#include "mqttmsg.h"

int main(int argc, const char *argv[])
{
   int ret = 0;
   char *json = NULL;
   const char *device = NULL;
   const char *pending = NULL;
   const char *topic = "";
   const char *command = NULL;
   int fobidentify = 0;
   int fobprovision = 0;
   const char *fobadopt = 0;
   const char *fobformat = 0;
   const char *deport = NULL;
   const char *rollover = NULL;
   const char *aid = NULL;
   const char *status = NULL;
   const char *arm = NULL;
   const char *strong = NULL;
   const char *disarm = NULL;
   const char *fobname = NULL;
   const char *api = NULL;
   int debug = 0;
   int setting = 0;
   int silent = 0;
   int provision = 0;
   int poke = 0;
   int site = 0;
   int organisation = 0;
   int access = 0;
   int jsonstdin = 0;
   int print = 0;
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         { "fob-identify", 0, POPT_ARG_NONE, &fobidentify, 0, "Fob identify", NULL },
         { "fob-adopt", 0, POPT_ARG_STRING, &fobadopt, 0, "Fob adopt", NULL },
         { "fob-format", 0, POPT_ARG_STRING, &fobformat, 0, "Fob format", NULL },
         { "fob-provision", 0, POPT_ARG_NONE, &fobprovision, 0, "Fob provision", NULL },
         { "fob-name", 0, POPT_ARG_STRING, &fobname, 0, "Fob name", "name" },
         { "command", 0, POPT_ARG_STRING, &command, 0, "Command", "tag" },
         { "settings", 0, POPT_ARG_NONE, &setting, 0, "Setting", NULL },
         { "aid", 0, POPT_ARG_STRING, &aid, 0, "AID", "XXXXXX" },
         { "access", 0, POPT_ARG_INT, &access, 0, "Access", "N" },
         { "deport", 0, POPT_ARG_STRING, &deport, 0, "Deport", "mqtthost" },
         { "rollover", 0, POPT_ARG_STRING, &rollover, 0, "Rollover", "AID" },
         { "device", 'd', POPT_ARG_STRING, &device, 0, "Device", "XXXXXXXXXXXX" },
         { "site", 's', POPT_ARG_INT, &site, 0, "Site", "N" },
         { "organisation", 's', POPT_ARG_INT, &organisation, 0, "Organisation", "N" },
         { "arm", 0, POPT_ARG_STRING, &arm, 0, "Arm", "A...Z" },
         { "strong", 0, POPT_ARG_STRING, &strong, 0, "Strong", "A...Z" },
         { "disarm", 0, POPT_ARG_STRING, &disarm, 0, "Disarm", "A...Z" },
         { "api", 0, POPT_ARG_STRING, &api, 0, "API", "User (needs JSON also)" },
         { "pending", 'p', POPT_ARG_STRING, &pending, 0, "Pending device", "XXXXXXXXXXXX" },
         { "status", 's', POPT_ARG_STRING, &status, 0, "Status", "div ID" },
         { "provision", 0, POPT_ARG_NONE, &provision, 0, "Provision", NULL },
         { "print", 0, POPT_ARG_NONE, &print, 0, "Print", NULL },
         { "poke", 0, POPT_ARG_NONE, &poke, 0, "Poke", NULL },
         { "stdin", 0, POPT_ARG_NONE, &jsonstdin, 0, "JSON from stdin", NULL },
         { "silent", 'q', POPT_ARG_NONE, &silent, 0, "Silent", NULL },
         { "debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug", NULL },
         POPT_AUTOHELP { }
      };

      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
      poptSetOtherOptionHelp(optCon, "<json>");

      int c;
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));

      if (poptPeekArg(optCon) && !jsonstdin)
         json = (char *) poptGetArg(optCon);

      if (poptPeekArg(optCon))
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }

      poptFreeContext(optCon);
   }

   j_t j = j_create();
   if (json)
      j_err(j_read_mem(j, json, strlen(json)));
   else if (jsonstdin)
      j_err(j_read_fd(j, fileno(stdin)));
   if (!j_isobject(j) && !j_isnull(j))
   {
      j_t n = j_create();
      j_store_json(n, "_data", &j);
      j = n;
   }
   j_t meta = j_store_object(j, "_meta");
   if (provision)
      j_store_true(meta, "provision");
   if (print)
      j_store_true(meta, "print");
   if (poke)
      j_store_true(meta, "poke");
   if (deport)
      j_store_string(meta, "deport", deport);
   if (rollover && strlen(rollover) == 6)
      j_store_string(meta, "rollover", rollover);
   if (fobprovision)
   {
      if (!device)
         errx(1, "Specify device to use for fob provisioning");
      j_store_true(meta, "fobprovision");
   }
   if (fobidentify)
   {
      if (!device)
         errx(1, "Specify device to use for fob identify");
      j_store_true(meta, "fobidentify");
      silent = 1;
   }
   if (fobformat)
   {
      if (!device)
         errx(1, "Specify device to use for fob format");
      j_store_string(meta, "fob", fobformat);
      j_store_true(meta, "fobformat");
   }
   if (fobadopt)
   {
      if (!device)
         errx(1, "Specify device to use for fob adopt");
      if (!aid)
         errx(1, "Specify aid to use for fob adopt");
      j_store_string(meta, "fob", fobadopt);
      j_store_string(meta, "fobname", fobname);
      j_store_true(meta, "fobadopt");
   }
   if (command)
   {
      j_store_string(meta, "prefix", "command");
      if (*command)
         j_store_string(meta, "suffix", command);
   }
   if (setting)
      j_store_string(meta, "prefix", "setting");
   if (organisation)
      j_store_int(meta, "organisation", organisation);
   if (site)
      j_store_int(meta, "site", site);
   if (access)
      j_store_int(meta, "access", access);
   if (api)
      j_store_string(meta, "api", api);
   if (arm && *arm)
   {
      j_store_string(meta, "prefix", "command");
      j_store_string(meta, "suffix", "arm");
      j_store_string(j, "_data", arm);
   } else if (strong && *strong)
   {
      j_store_string(meta, "prefix", "command");
      j_store_string(meta, "suffix", "strong");
      j_store_string(j, "_data", strong);
   } else if (disarm && *disarm)
   {
      j_store_string(meta, "prefix", "command");
      j_store_string(meta, "suffix", "disarm");
      j_store_string(j, "_data", disarm);
   }
   if (device && *device)
      j_store_string(meta, "device", device);
   if (aid && *aid)
      j_store_string(meta, "aid", aid);
   if (pending && *pending)
      j_store_string(meta, "pending", pending);

   j_store_string(meta, "user_id", getenv("USER_ID"));
   j_store_string(meta, "user_username", getenv("USER_USERNAME"));
   j_store_string(meta, "remote_addr", getenv("REMOTE_ADDR"));

   if (debug)
      j_err(j_write_pretty(j, stderr));

   const char *cacert = NULL,
       *msgkey = NULL,
       *msgcert = NULL;
   {                            // Get security
      j_t j = j_create();
      const char *fail = j_read_file(j, CONFIG_MSG_KEY_FILE);
      if (fail)
         fail = j_read_file(j, "../" CONFIG_MSG_KEY_FILE);      // Gets run from www dir
      if (fail)
         errx(1, "Cannot read keys %s", CONFIG_MSG_KEY_FILE);
      cacert = strdup(j_get(j, "ca.cert") ? : "");
      msgkey = strdup(j_get(j, "msg.key") ? : "");
      msgcert = strdup(j_get(j, "msg.cert") ? : "");
      j_delete(&j);
   }

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
   freeaddrinfo(a);
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
   {                            // Send message
      unsigned char buf[2000];
      int len = mqtt_encode(buf, sizeof(buf), topic, j);
      SSL_write(ssl, buf, len);
   }
   char done = 0;
   while (!done)
   {                            // Wait reply
      unsigned char buf[1000];
      size_t len = SSL_read(ssl, buf, sizeof(buf));
      if (len < 2 && *buf != 0x30)
         errx(1, "Bad reply");
      j_t j = mqtt_decode(buf, len);
      mqtt_dataonly(j);
      if (j && !j_isnull(j))
      {
         if (fobidentify)
         {
            const char *f = j_get(j, "fobid");
            if (f && *f)
            {
               printf("%s", f);
               done = 1;
            }
         }
         if (status)
         {
            const char *s = j_get(j, "status");
            if (s)
               printf("<script>e=document.createElement('li');e.textContent='%s';document.getElementById('%s').append(e);</script>\n", s, status);
         }
         ret = 1;
         if (j_isstring(j))
         {
            if (status)
               printf("<script>e=document.createElement('li');e.textContent='%s';document.getElementById('%s').append(e);</script>\n", j_val(j), status);
            if (!silent)
               printf("%s", j_val(j));
         } else if (!silent)
            j_err(j_write_pretty(j, stdout));
         fflush(stdout);
      } else
         done = 1;
      j_delete(&j);
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

   return ret;
}
