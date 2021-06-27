// Fob command thread

#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include "AJL/ajl.h"
#include "ssmqtt.h"

void *fobcommand(void *arg)
{
   warnx("Started fobcommand");
   int sock = -1;
   long long instance = 0;
   {                            // Get passed settings
      j_t j = arg;
      sock = atoi(j_get(j, "socket") ? : "");
      instance = strtoll(j_get(j, "instance"), NULL, 10);
      if (!sock)
         errx(1, "socket not set");
      j_delete(&j);
   }
   while (1)
   {
      fd_set r;
      FD_ZERO(&r);
      FD_SET(sock, &r);
      struct timeval to = { 3000, 0 };
      if (select(sock + 1, &r, NULL, NULL, &to) <= 0)
         break;
      unsigned char buf[2000];
      int len = recv(sock, buf, sizeof(buf), 0);
      if (len <= 0)
         break;
      if (*buf != 0x30)
         continue;
      j_t j = mqtt_decode(buf, len);
      if(j_isnull(j))
      { // End
	      j_delete(&j);
	      break;
      }
      j_err(j_write_pretty(j, stderr)); // TODO
      j_delete(&j);
   }
   {                            // unlink
      j_t j = j_create();
      j_int(j_path(j, "_meta.loopback"), instance);
      mqtt_qin(&j);
   }
   warnx("Ended fobcommand");
   close(sock);
   return NULL;
}
