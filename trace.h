// Include at start of main()
#if 0
#include <syslog.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <execinfo.h>
#endif
// assumes argv[0] is code
{
   void trace (int sig)
   {
      void *stack[100];
      size_t size;
      size = backtrace (stack, 100);
      fprintf (stderr, "\nSignal: %d\n", sig);
      char *cmd = NULL;
      int e = asprintf (&cmd, "addr2line -f -i -e %s", argv[0]);
      int i;
      for (i = 2; i < size; i++)
         e += asprintf (&cmd, "%s %p", cmd, stack[i]);
      FILE *al = popen (cmd, "r");
      char buf[1000];
      while (fgets (buf, sizeof (buf), al) > 0)
         fprintf (stderr, "%s", buf);
      pclose (al);
      openlog (argv[0], LOG_CONS | LOG_PID, LOG_USER);
      for (i = 2; i < size; i++)
         syslog (LOG_INFO, "%p\n", stack[i]);
      exit (1);
   }
   signal (SIGSEGV, trace);     // install our handler
   signal (SIGILL, trace);      // install our handler
   signal (SIGFPE, trace);      // install our handler
}
