// Make Afile

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include "AJL/ajl.h"
#include "SQLlib/sqllib.h"

unsigned int makeafile(SQL * sqlp, const char *fob, const char *aid, unsigned char *afile)
{                               // Make afile (max 256 characters) and return crc
   if (afile)
      *afile = 0;               // TODO
   return 0;                    // TODO
}
