// Port stuff

#include "port.h"
#include <string.h>
#include <malloc.h>
#include <err.h>

port_p ports = NULL;

port_p
port_new_bus (unsigned char bus, unsigned char id, unsigned char isinput, unsigned char port)
{
   port_p p;
   for (p = ports; p && (p->mqtt || p->bus != bus || p->id != id || p->port != port || p->isinput != isinput); p = p->next);
   if (p)
      return p;
   p = malloc (sizeof (*p));
   memset (p, 0, sizeof (*p));
   if (!p)
      errx (1, "malloc");
   p->bus = bus;
   p->id = id;
   p->port = port;
   p->isinput = isinput;
   p->next = ports;
   ports = p;
   //fprintf (stderr, "New bus port %d %02X %c%d\n", bus, id, port ? isinput ? 'I' : 'O' : '-', port);
   return p;
}

port_p
port_new (const char *mqtt, unsigned char isinput, unsigned char port)
{
   port_p p;
   for (p = ports; p && (!p->mqtt || p->port != port || p->isinput != isinput || (p->mqtt != mqtt && strcmp (p->mqtt, mqtt)));
        p = p->next);
   if (p)
      return p;
   p = malloc (sizeof (*p));
   memset (p, 0, sizeof (*p));
   if (!p)
      errx (1, "malloc");
   p->mqtt = mqtt;
   p->isinput = isinput;
   p->port = port;
   p->next = ports;
   ports = p;
   return p;
}
