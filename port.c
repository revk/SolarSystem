// Port stuff

#include "port.h"
#include "malloc.h"
#include "err.h"

port_p ports = NULL;

port_p
port_new_bus (unsigned int b)
{
   port_p p;
   for (p = ports; p && p->busid != b; p = p->next);
   if (p)
      return p;
   p = malloc (sizeof (*p));
   if (!p)
      errx (1, "malloc");
   p->busid = b;
   p->next = ports;
   ports = p;
   return p;
}
