// Back end management / control for SolarSystem modules

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "AJL/ajl.h"

// System wide settings, mostly taken from config file - these define defaults as well
int debug = 0;
const char *configfile="solarsystem.conf";
#define s(n,d,h)	const char *n=#d;
#define i(n,d,h)	int n=d;
#include "ssconfig.h"

int
main (int argc, const char *argv[])
{
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
      {"string-default", 'S', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &configfile, 0, "Config file", "filename"},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug",NULL},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      if (poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
      poptFreeContext (optCon);
   }
   // Load config file and extract settings
   j_t j=j_create();
   j_err(j_read_file(j,configfile));
#define s(n,d,h) {j_t e=j_find(j,#n);if(e){if(!j_isstring(e))errx(1,#n" should be a string");n=j_val(e);}if(debug)warnx(#h" set to:\t%s",n);}
#define i(n,d,h) {j_t e=j_find(j,#n);if(e){if(!j_isnumber(e))errx(1,#n" should be a number");n=atoi(j_val(e));}if(debug)warnx(#h" set to:\t%d",n);}
#include "ssconfig.h"
		   j_delete(&j);

   return 0;
}
