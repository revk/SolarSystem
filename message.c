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
#include <openssl/evp.h>
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

   j_err(j_read_file(j, CONFIG_KEYS_FILE));
   const char *cakey = strdup(j_get(j, "ca") ? : "");
   j_delete(&j);
   const char *cacert = makecert(cakey, NULL, NULL, "SolarSystem");
   const char *key = makekey();
   const char *cert = makecert(key, cakey, cacert, "message");


   return 0;
}
