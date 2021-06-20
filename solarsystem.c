// Back end management / control for SolarSystem modules

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>

// System wide settings, mostly taken from config file - these define defaults as well
int debug = 0;


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

   return 0;
}
