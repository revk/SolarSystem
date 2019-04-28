// Port stuff

#include <sys/time.h>
#include <string.h>
#include <malloc.h>
#include <err.h>
#include <pthread.h>
#include "port.h"
#include "galaxybus.h"

port_p ports = NULL;

pthread_mutex_t portmutex;

extern void mqtt_output (port_p, int);

void
port_start (void)
{
   pthread_mutex_init (&portmutex, 0);
}

port_p
port_new_bus (unsigned char bus, unsigned char id, unsigned char isinput, unsigned char port)
{
   if (!port)
      isinput = 0;
   pthread_mutex_lock (&portmutex);
   port_p p;
   for (p = ports; p && (p->mqtt || p->bus != bus || p->id != id || p->port != port || p->isinput != isinput); p = p->next);
   if (!p)
   {
      p = malloc (sizeof (*p));
      memset ((void *) p, 0, sizeof (*p));
      if (!p)
         errx (1, "malloc");
      p->bus = bus;
      p->id = id;
      p->port = port;
      p->isinput = isinput;
      p->next = ports;
      ports = p;
   }
   pthread_mutex_unlock (&portmutex);
   return p;
}

port_p
port_new (const char *mqtt, unsigned char isinput, unsigned char port)
{
   if (!port)
      isinput = 0;
   pthread_mutex_lock (&portmutex);
   port_p p;
   for (p = ports; p && (!p->mqtt || p->port != port || p->isinput != isinput || strcmp (p->mqtt, mqtt)); p = p->next);
   if (!p)
   {
      p = malloc (sizeof (*p));
      memset ((void *) p, 0, sizeof (*p));
      if (!p)
         errx (1, "malloc");
      p->mqtt = strdup (mqtt);
      p->isinput = isinput;
      p->port = port;
      p->next = ports;
      ports = p;
   }
   pthread_mutex_unlock (&portmutex);
   return p;
}

port_p
port_new_base (port_p parent, unsigned char isinput, unsigned char port)
{
   if (parent->mqtt)
      return port_new (parent->mqtt, isinput, port);
   return port_new_bus (parent->bus, parent->id, isinput, port);
}

void
port_urgent_n (volatile port_p * w, int n)
{
   while (n--)
      if (w[n] && port_device (w[n]))
         device_urgent (port_device (w[n]));
}

void
port_output_n (volatile port_p * w, int n, int v)
{
   while (n--)
      if (w[n] && port_isoutput (w[n]))
      {
         if (v)
         {
            if (!w[n]->state)
            {
               w[n]->state = 1;
               if (port_device (w[n]))
                  device_output (port_device (w[n]), port_port (w[n]), 1);
               else
                  mqtt_output (w[n], 1);
               if (port_output_callback)
                  port_output_callback (w[n]);
            }
         } else
         {
            if (w[n]->state)
            {
               w[n]->state = 0;
               if (port_device (w[n]))
                  device_output (port_device (w[n]), port_port (w[n]), 0);
               else
                  mqtt_output (w[n], 0);
               if (port_output_callback)
                  port_output_callback (w[n]);
            }
         }
      }
}

int
port_defined_n (volatile port_p * w, int n)
{
   int q = 0;
   while (n--)
      if (w[n])
         q++;
   return q;
}

int
port_tamper_n (volatile port_p * w, int n)
{
   int q = 0;
   while (n--)
      if (w[n] && w[n]->tamper)
         q++;
   return q;
}

int
port_found_n (volatile port_p * w, int n)
{
   while (n--)
      if (w[n] && port_device (w[n]) && device_found (port_device (w[n])))
         return 0;
   return 1;
}

int
port_input_n (volatile port_p * w, int n)
{
   int q = 0;
   while (n--)
      if (w[n] && port_isinput (w[n]) && w[n]->state)
         q++;
   return q;
}
