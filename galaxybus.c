// Library for managing galaxy RS485 bus logic

// Designed to work specifically with things like Max readers, RIOs, and keypads
// Command line provides some basic test / tools

    /*
       A complete alarm panel using devices compatible with Honeywell/Galaxy RS485 buses
       Copyright (C) 2017  RevK and Andrews & Arnold Ltd

       This program is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       This program is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with this program.  If not, see <http://www.gnu.org/licenses/>.
     */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <err.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <pthread.h>
#include <galaxybus.h>

const char maxeeprom[] = {      // Max reader EEPROM. Add ID to locations 0C and 1F
   0x72, 0x15, 0xE7, 0x30, 0x78, 0x00, 0x00, 0x0E, 0x26, 0x50, 0xAA, 0x00,
   0x00, 0x11, 0x10, 0x01,
   0x13, 0x34, 0x33, 0x34, 0x33, 0x00, 0x45, 0xAF, 0x20, 0x00, 0x91, 0x00,
   0x00, 0x00, 0x22, 0x5F,
   0x04, 0x17, 0x22, 0x06, 0x01, 0x06, 0xF8, 0x22, 0x06, 0x81, 0x90, 0xC7,
   0x22, 0x40, 0xA3, 0x20,
   0xE8, 0x10, 0x28, 0xA0, 0xA1, 0x10, 0x16, 0x01, 0x70, 0xF8, 0x08, 0xC8,
   0xA8, 0xB8, 0xC3, 0x06,
   0xC1, 0x10, 0x70, 0x71, 0x10, 0x30, 0x61, 0x90, 0xB0, 0xB0, 0x22, 0x50,
   0x01, 0x10, 0x48, 0x63,
   0x22, 0x80, 0x83, 0x10, 0x80, 0x27, 0x22, 0x20, 0x43, 0x60, 0x28, 0xC3,
   0x06, 0x27, 0x82, 0x40,
   0x04, 0xF8, 0x48, 0x30, 0x04, 0xF8, 0xC8, 0x42, 0x00, 0x62, 0x00, 0x03,
   0x97, 0xE2, 0x30, 0x03,
   0xC3, 0x06, 0x08, 0xC3, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
};

// Data
int dump = 0;
int debug = 0;

pthread_t busthreads[MAX_BUS];

device_t device[MAX_DEVICE] = { };

event_t *event = NULL,
    **eventp = NULL;

pthread_mutex_t qmutex;
pthread_mutex_t outputmutex;
int qpipe[2];

char *WATCHDOG = NULL;          // Optional watchdog device

#define t(x) #x,
const char *type_name[MAX_TYPE] = { TYPE };

#undef t
#define e(x) #x,
const char *event_name[EVENT_COUNT] = { EVENT };

#undef e

#define RIO_DEFAULT_RESPONSE    300     // ms

const rio_threshold_t rio_thresholds[3] = {
   { "1k", 8, 9, 12, 13, 120 },
   { "2k2", 18, 20, 25, 27, 120 },
   { "4k7", 37, 42, 55, 65, 190 },
};

int device_found(int id)
{
   if (id < 0 || id >= MAX_DEVICE)
      return 0;
   return device[id].found;
}

void device_urgent(int id)
{
   if (id < 0 || id >= MAX_DEVICE)
      return;
   device[id].urgent = 1;
}

void device_output(int id, int port, int value)
{
   if (id < 0 || id >= MAX_DEVICE || port < 1 || port >= MAX_OUTPUT)
      return;
   pthread_mutex_lock(&outputmutex);
   if (value)
      device[id].output |= (1 << (port - 1));
   else
      device[id].output &= ~(1 << (port - 1));
   pthread_mutex_unlock(&outputmutex);
}

int device_input(int id, int port)
{
   if (id < 0 || id >= MAX_DEVICE || port < 1 || port >= MAX_INPUT)
      return -1;
   if (device[id].input & (1 << (port - 1)))
      return 1;
   return 0;
}

void device_led(int id, int led)
{
   if (id < 0 || id >= MAX_DEVICE)
      return;
   device[id].led = led;
}

void postevent(event_t * e)
{
   if (e)
   {
      if (e->port)
      {                         // Reflect states in  port object
         if (e->event == EVENT_INPUT)
            e->port->state = e->state;
         else if (e->event == EVENT_TAMPER)
            e->port->tamper = e->state;
         else if (e->event == EVENT_FAULT)
            e->port->fault = e->state;
         else if (e->event == EVENT_FOUND)
            e->port->state = 1;
         else if (e->event == EVENT_MISSING)
            e->port->state = 0;
      }
      pthread_mutex_lock(&qmutex);
      if (event)
         *eventp = e;
      else
         event = e;
      eventp = (void *) &e->next;
      pthread_mutex_unlock(&qmutex);
   }
   // Use pipe to signal that event is waiting, but it will get events anyway so non blocking
   char x = 0;
   if (write(qpipe[1], &x, 1) < 0)
      perror("queue send");
}

void *poller(void *d)
{
   int busid = (long) d;
   if (busid < 0 || busid >= MAX_BUS)
      errx(1, "Bad bus ID to start %d", busid);
   device_t *dev = device + busid * 256;
   char devname[100];
   {                            // Latency
      snprintf(devname, sizeof(devname), "/sys/bus/usb-serial/devices/ttyUSB%d/latency_timer", busid);
      FILE *f = fopen(devname, "w");
      if (!f)
         warn("Cannot open %s", devname);
      else
      {
         fprintf(f, "1");
         fclose(f);
      }
   }
   snprintf(devname, sizeof(devname), "/dev/ttyUSB%d", busid);
   int f = open(devname, O_RDWR);
   if (f < 0)
   {
      warn("Cannot open %s", devname);
      return NULL;
   }
   if (flock(f, LOCK_EX | LOCK_NB) < 0)
   {
      warn("Cannot lock %s", devname);
      return NULL;
   }
   struct termios t = { 0
   };
   t.c_cflag = B9600 | CS8 | CREAD | CLOCAL;    // 9600Baud 1n8
   t.c_cc[VTIME] = 1;
   ioctl(f, TCSETS, &t);
   dev[US].disabled = 1;        // Don't poll ourselves
   // Internal data structures
   struct {
      input_t input;            // confirmed inputs
      output_t output;          // confirmed outputs
      output_t invert;          // Invert outputs
      tamper_t tamper;          // confirmed tamper (device tamper is 1<<8)
      fault_t fault;            // confirmed fault
      unsigned char laststatus; // last status simple F4 for debug reporting of changes
      unsigned char polling:1;
      unsigned char restart:1;
      unsigned char send00;     // Need to send 0x00 (1<<input)
      unsigned char send07:1;   // Need to send 0x07
      unsigned char send0B:1;   // Need to send 0x0B
      unsigned char send0C:1;   // Need to send 0x0C
      unsigned char send0D:1;   // Need to send 0x0D
      unsigned char send19:1;   // Need to send 0x19
      unsigned char send01:1;   // Send 01 message
      unsigned char send02:1;   // Send 02 message
      unsigned char toggle00:1; // Toggle for 0x00 message
      unsigned char toggle07:1; // Toggle for 0x07 message
      unsigned char toggle0B:1; // Toggle for 0x0B message
      unsigned char toggle0C:1; // Toggle for 0x0C message
      union {
         struct keypad_data_s k;
         struct {               // Max
            unsigned char disable:1;    // Reader disabled
            unsigned char config:1;     // We are doing config
            unsigned char led;
            unsigned char addr; // Programming address
            unsigned long long fobheld; // time for held
         };
         struct {               // RIO
            unsigned char response[MAX_INPUT];
            unsigned char threshold[MAX_INPUT][5];
         };
      };
   } mydev[256] = {
   };
//
   struct timeval now = { 0
   };
   struct timezone tz = { 0
   };
   unsigned char id = 0,
       idleid = 0;
   unsigned char idlecheck = 0;
   event_t *newevent(int etype, unsigned char isinput, unsigned char port) {    // port from 1 else assumed device level
      event_t *e = malloc(sizeof(event_t));
      if (!e)
         errx(1, "malloc");
      memset((void *) e, 0, sizeof(*e));
      unsigned int did = ((busid << 8) + id);
      if (port < sizeof(device[did].port) / sizeof(device[did].port[0]))
      {                         // Port ID reporting
         if (!device[did].port[port])
            device[did].port[port] = port_new_bus(busid, id, isinput, port);
         e->port = device[did].port[port];
      }
      e->when = now;
      e->event = etype;
      return e;
   }
   unsigned char cmd[256];      // The command we are sending
   unsigned int cmdlen = 0;
   unsigned char res[256];      // The response we are receiving
   unsigned int reslen = 0;
   int errors = 0;              // Protocol errors
   int stalled = 0;             // Stuck sending
   int retries = 0;             // retries
   int rx = 0;                  // Rx byte count
   int tx = 0;                  // Tx byte count
   int retry = 0;               // Retry count
   int more = 1;                // More to same device
   int stall = 0;               // Stall count
   time_t nextka = time(0) + 10;        // next keep alive
   time_t lastsec = 0;          // One second events
   long long gap = 0;           // Time to first byte response for logging
   struct timeval timeout = { 0
   };
   void sendcmd(void) {         // Send command
      if (!dev[id].type || dev[id].type != TYPE_PAD)
         usleep(3000);
      if (write(f, cmd, cmdlen) != (int) cmdlen)
         errors++;
      // Delay for the sending of the command - we do not rx data at this point (though can pick up a break sometimes)
      usleep(1000000 * (10 * cmdlen + 1) / 9600);       // We send one stop bit, then data with 1 stop on each byte
      // Timeout for reply
      timeout.tv_usec = 15000;  // 10ms inter message gap, and then some extra for slower devices
      if (dev[id].type == TYPE_MAX)
         timeout.tv_usec += 10000;      // And more for max!!
      tx += cmdlen;
      // Debug/dump
      if (dump || (debug && (cmd[1] != 0x06 || cmdlen > 3) && (cmd[1] != 0x01 || cmdlen > 3) && (cmd[1] != 0x00 || cmdlen > 4)))
      {                         // Debug does not dump boring polls
         unsigned int n;
         printf("%s%X%02X >", type_name[dev[id].type], busid + 1, id);
         for (n = 0; n < cmdlen - 1; n++)
            printf(" %02X", cmd[n]);
         printf("\n");
         fflush(stdout);
      }
   }
   // Main polling loop
   int wd = -1;
   if (WATCHDOG)
      wd = open(WATCHDOG, O_RDWR);
   while (1)
   {
      unsigned char type = dev[id].type;
      fd_set readfds;
      FD_ZERO(&readfds);
      FD_SET(f, &readfds);
      gettimeofday(&now, &tz);
      long long reftime = now.tv_sec * 1000000ULL + now.tv_usec;
      int s = select(f + 1, &readfds, NULL, NULL, &timeout);
      if (s > 0)
         s = read(f, &res[reslen], 1);
      if (s < 0)
         warn("Bad rx");
      if (s)
      {                         // we have a character
         if (!reslen && !*res)
            continue;           // An initial break is seeing tail end of us sending
         timeout.tv_usec = 6000;        // Inter message gap typically 9 to 10ms
         if (!reslen && (debug || dump))
         {                      // Timing for debug
            gettimeofday(&now, &tz);
            gap = now.tv_sec * 1000000ULL + now.tv_usec - reftime;
         }
         rx++;
         if (reslen == sizeof(res))
         {                      // Not sensible
            errors++;
            reslen = 0;
            continue;
         }
         reslen++;
         continue;
      }
      // Timeout
      if (reslen)
      {                         // Dump
         if (dump || (debug && (res[0] != US || ((reslen != 4 || res[1] != 0xF4 || res[2] != mydev[id].laststatus) && (reslen != 3 || res[1] != 0xFE) && (type != TYPE_RIO || (res[1] != 0xF1 && res[1] != 0xF7)) && (type != TYPE_RFR || (res[1] != 0xF7 && res[1] != 0xFE && res[1] != 0xFD))))))
         {                      // debug tries not to log boring
            unsigned int n;
            printf("%s%X%02X <", type_name[dev[id].type], busid + 1, id);
            for (n = 0; n < reslen - 1; n++)
               printf(" %02X", res[n]);
            if (reslen == 1)
               printf(" ? %02X", res[n]);       // too short?
            printf(" (%lld.%lldms)\n", gap / 1000, gap / 100 % 10);
            fflush(stdout);
         }
         if (reslen == 4 && res[0] == US && res[1] == 0xF4)
            mydev[id].laststatus = res[2];
      }
      int fail(void) {          // Failed rx
         if (cmdlen && cmd[1])
         {
            retries++;
            if (debug)
            {
               unsigned int n;
               printf("%s%X%02X *", type_name[dev[id].type], busid + 1, id);
               for (n = 0; n < cmdlen - 1; n++)
                  printf(" %02X", cmd[n]);
               printf("\n");
               fflush(stdout);
            }
         }
         if (retry++ >= MAX_RETRY)
         {                      // give up
            if (dev[id].type && !mydev[id].polling && !dev[id].missing)
            {
               dev[id].missing = 1;
               if (debug)
                  printf("%s%X%02X lost\n", type_name[dev[id].type], busid + 1, id);
               postevent(newevent(EVENT_MISSING, 0, 0));
            }
            mydev[id].polling = 0;
            cmdlen = 0;         // we move on to next device
            return 0;
         }
         if (cmdlen)
            sendcmd();
         reslen = 0;            // Ready again
         return 1;
      }
      if (cmdlen && reslen < 2 && fail())
         continue;              // Too short
      if (reslen)
      {                         // Check checksum
         unsigned int c = 0xAA,
             n;
         for (n = 0; n < reslen - 1; n++)
            c += res[n];
         while (c > 0xFF)
            c = (c >> 8) + (c & 0xFF);
         if (res[reslen - 1] != c && fail())
            continue;           // Bad checksum
      }
      if (reslen && res[0] != US && fail())
         continue;              // Not to us - should we consider this some sort of tamper?
      if (reslen == 3 && res[1] == 0xF2)
      {                         // Did not understand
         errors++;
         if (fail())
            continue;
      }
      retry = 0;
      // Process response in context of cmd sent - resets cmdlen, and may have cmdlen set again as part of response to response
      if (reslen && cmdlen)
      {
         if (!mydev[id].polling)
         {                      // Started responding
            mydev[id].polling = 1;      // poll it now
            mydev[id].restart = 1;      // update it completely
         }
         if (cmd[1] == 0x00 && cmdlen == 4 && res[1] == 0xFF && reslen > 5)
         {                      // Restart response
            more = 1;
            if (res[2] & 0x08)
            {
               if (res[4])
                  type = TYPE_PAD;
               else
               {
                  type = TYPE_MAX;
                  dev[id].pad = 1;
               }
            } else if (!res[2])
            {
               type = TYPE_MAX;
               dev[id].pad = 0;
            } else if (res[2] & 0x20)
               type = TYPE_RIO;
            else if (res[2] == 0x10)
               type = TYPE_RFR;
            if (!dev[id].type)
            {                   // New
               if (debug)
                  printf("%s%X%02X found new\n", type_name[type], busid + 1, id);
               dev[id].missing = 1;     // report found
            } else if (dev[id].type == type)
            {
               if (debug)
                  printf("%s%X%02X found\n", type_name[type], busid + 1, id);
            } else
            {                   // Changed
               if (debug)
                  printf("%s%X%02X clash %s %02X %02X %02X\n", type_name[type], busid + 1, id, type_name[dev[id].type], res[2], res[3], res[4]);
               memset((void *) &dev[id], 0, sizeof(dev[id]));
               memset(&mydev[id], 0, sizeof(mydev[id]));
               dev[id].disabled = 1;
               postevent(newevent(EVENT_DISABLED, 0, 0));
            }
            dev[id].type = type;
            if (dev[id].missing)
            {                   // Report found
               dev[id].missing = 0;
               postevent(newevent(EVENT_FOUND, 0, 0));
            }
         }
         cmdlen = 0;            // Reset.. Possible that some responses need immediate reply.
         switch (type)
         {
         case TYPE_MAX:        // Data from max
            {
               if ((res[1] == 0xF4 || res[1] == 0xFC) && reslen > 3)
               {
                  dev[id].found = 1;
                  unsigned long was = mydev[id].tamper;
                  if (res[1] == 0xFC)
                     mydev[id].tamper |= ((1 << MAX_INPUT) | (1 << INPUT_MAX_OPEN) | (1 << INPUT_MAX_EXIT));
                  else
                     mydev[id].tamper &= ~((1 << MAX_INPUT) | (1 << INPUT_MAX_OPEN) | (1 << INPUT_MAX_EXIT));
                  if (res[2] & 0x10)
                     mydev[id].input &= ~(1 << INPUT_MAX_OPEN);
                  else
                     mydev[id].input |= (1 << INPUT_MAX_OPEN);
                  if (res[2] & 0x20)
                     mydev[id].input |= (1 << INPUT_MAX_EXIT);
                  else
                     mydev[id].input &= ~(1 << INPUT_MAX_EXIT);
                  if (reslen > 4)
                  {
                     unsigned long n = (res[2] & 0x0F),
                         q;
                     for (q = 3; q < reslen - 1; q++)
                        n = n * 100 + (res[q] >> 4) * 10 + (res[q] & 0xF);
                     if (!(mydev[id].input & (1 << INPUT_MAX_FOB)))
                     {          // Fob starts
                        event_t *e = newevent(EVENT_FOB, 0, 0);
                        snprintf((char *) e->fob, sizeof(e->fob), "%lu", n);
                        postevent(e);
                        mydev[id].input |= (1 << INPUT_MAX_FOB);
                        if (dev[id].fob_hold)
                           mydev[id].fobheld = now.tv_sec * 1000000ULL + now.tv_usec + dev[id].fob_hold * 100000ULL;
                        else
                           mydev[id].fobheld = 0;
                     } else if (mydev[id].fobheld && now.tv_sec * 1000000ULL + now.tv_usec > mydev[id].fobheld)
                     {          // Fob held
                        mydev[id].input |= (1 << INPUT_MAX_FOB_HELD);
                        mydev[id].fobheld = 0;
                        event_t *e = newevent(EVENT_FOB_HELD, 0, 0);
                        snprintf((char *) e->fob, sizeof(e->fob), "%lu", n);
                        postevent(e);
                     }
                  } else
                     mydev[id].input &= ~((1 << INPUT_MAX_FOB) | (1 << INPUT_MAX_FOB_HELD));
                  if (was != mydev[id].tamper)
                     mydev[id].send07 = 1;      // Override internal LEDs
               }
               break;
            }
         case TYPE_PAD:        // Data from keypad
            {
               if (res[1] == 0xF4 && reslen > 3)
               {                // Status
                  dev[id].found = 1;
                  if (res[2] & 0x40)
                     mydev[id].tamper |= (1 << MAX_INPUT);
                  else
                     mydev[id].tamper &= ~(1 << MAX_INPUT);
                  if (res[2] != 0x7F)
                  {             // key
                     event_t *e = newevent((res[2] & 0x80) ? EVENT_KEY_HELD : EVENT_KEY, 0, 0);
                     e->key = "0123456789BA\n\e*#"[res[2] & 0x0F];
                     postevent(e);
                     // Acknowledge key
                     cmd[++cmdlen] = 0x0B;
                     cmd[++cmdlen] = (mydev[id].toggle0B ? 2 : 0);
                     mydev[id].toggle0B ^= 1;
                  }
                  break;
               }
               if (res[1] == 0xFE)
                  mydev[id].tamper &= ~(1 << MAX_INPUT);        // idle (no tamper)
               break;
            }
         case TYPE_RIO:        // Date from RIO
            {
               if (res[1] == 0xF8 && reslen > 11 && (res[6] & 0x01))
                  mydev[id].tamper |= (1 << MAX_INPUT); // tamper
               else if (res[1] == 0xFE || (res[1] == 0xF8 && reslen > 11 && !(res[6] & 0x01)))
                  mydev[id].tamper &= ~(1 << MAX_INPUT);        // no tamper
               if (res[1] == 0xF1)
               {                // Voltages
                  unsigned int p = 2;
                  unsigned int n;
                  for (n = 0; n < MAX_INPUT && p + 1 < reslen; n++)
                  {
                     dev[id].ri[n].resistance = (res[p] << 8) + res[p + 1];
                     p += 2;
                  }
                  for (; n < MAX_INPUT; n++)
                     dev[id].ri[n].resistance = 0;
                  for (n = 0; n < sizeof(dev[id].voltage) / sizeof(*dev[id].voltage) && p + 1 < reslen; n++)
                  {
                     dev[id].voltage[n] = (res[p] << 8) + res[p + 1];
                     p += 2;
                  }
                  if (n < sizeof(dev[id].voltage) / sizeof(*dev[id].voltage))
                     for (; n < sizeof(dev[id].voltage) / sizeof(*dev[id].voltage); n++)
                        dev[id].voltage[n] = 0;
                  else if (p + 1 < reslen)
                  {             // Flags
                     if (res[p] & 0x80)
                        mydev[id].fault &= ~(1 << FAULT_RIO_NO_PWR);
                     else
                        mydev[id].fault |= (1 << FAULT_RIO_NO_PWR);
                     if (res[p] & 0x40)
                        mydev[id].fault &= ~(1 << FAULT_RIO_NO_BAT);
                     else
                        mydev[id].fault |= (1 << FAULT_RIO_NO_BAT);
#if 0                           // Looks like 0x20 may be charging, and stops briefly every hour
                     if (res[p] & 0x20)
                        mydev[id].fault |= (1 << FAULT_RIO_BAD_BAT);
                     else
                        mydev[id].fault &= ~(1 << FAULT_RIO_BAD_BAT);
#endif
                  }
               }
               if (res[1] == 0xF8 || res[1] == 0xFD)
               {                // Status
                  dev[id].found = 1;
                  mydev[id].input = res[2];
                  if (reslen > 7)
                  {
                     mydev[id].fault = ((mydev[id].fault & ~0xFF) | res[4] | res[5]);
                     dev[id].low = (res[5] | res[7]);   // combined with tamper and fault can work out what state it is...
                     mydev[id].tamper = ((mydev[id].tamper & ~0xFF) | res[3]);
                  } else
                  {             // Short message when no fault or tamper
                     mydev[id].fault = (mydev[id].fault & ~0xFF);
                     dev[id].low = 0;
                     mydev[id].tamper = (mydev[id].tamper & ~0xFF);
                  }
               }
               break;
            }
         case TYPE_RFR:        // Data from RF RIO
            {
               if (res[1] == 0xFE && res[2] == 0x0F)
                  mydev[id].tamper |= (1 << MAX_INPUT); // tamper
               else if (res[1] == 0xFE)
                  mydev[id].tamper &= ~(1 << MAX_INPUT);        // no tamper
               if (res[1] == 0xF7)
               {                // Status including voltage?
                  // TODO
               }
               if (res[1] == 0xFE)
               {                // Status
                  // TODO
               }
               if (res[1] == 0xFD && reslen > 12)
               {                // RF signal
                  // find device by serial, or create new device object
                  // TODO
                  switch (res[10])
                  {             // Device types we know
                  case 0x03:   // PIR V2GY or Flood V2GY
                     // TODO
                     break;
                  case 0x05:   // Fob V2GY (simple)
                     // TODO
                     break;
                  case 0x06:   // Smoke detector, V2GY or Alpha
                     // TODO
                     break;
                  case 0x08:   // Fob Alpha
                     {
                        // TODO
                     }
                     break;
                  case 0x12:   // Flood Alpha
                     // TODO
                     break;
                  case 0x18:   // PIR Alpha
                     // TODO
                     break;
                  }
                  // Set up response message to device
                  // TODO
                  // TODO temp for now
                  event_t *e = newevent(EVENT_RF, 0, 0);
                  e->rfserial = ((res[2] << 24) | (res[3] << 16) | (res[4] << 8) | res[5]);
                  e->rfstatus = ((res[6] << 24) | (res[7] << 16) | (res[8] << 8) | res[9]);
                  e->rftype = res[10];
                  e->rfsignal = res[11];
                  postevent(e);
               }
               break;
            }
         }
      }
      if (cmdlen)
         more = 1;              // Have set up new command in response to response
      if (more && stall++ >= MAX_STALL)
      {
         stalled++;
         more = 0;
      }
      if (!more)
      {                         // Break cycle for urgent updates
         int n;
         for (n = 0; n < 256 && !dev[n].urgent; n++);
         if (n < 256)
         {
            if (debug)
               printf("%s%X%02X URGENT\n", type_name[dev[n].type], busid + 1, n);
            dev[n].urgent = 0;
            id = n;
            more = 1;
         }
      }
      if (!more)
      {                         // Move on to next
         stall = 0;
         int m = 2;
         while (m)
         {
            if (!++id)
            {                   // End of scan cycle
               idlecheck = 0;
               if (lastsec != now.tv_sec || !m--)
               {                // Idle checking
                  lastsec = now.tv_sec;
                  int m = 2;
                  do
                  {
                     if (!++idleid)
                        m--;
                  }
                  while (m && mydev[idleid].polling);
                  idlecheck = 1;
                  if (wd >= 0)
                     write(wd, &idlecheck, 1);  // Poke watchdog
               }
            }
            if (dev[id].disabled)
               continue;
            if (dev[id].type && (idlecheck || !dev[id].missing))
               break;           // found a suitable device
            if (idlecheck && idleid == id)
               break;           // Checking an idle device
         }
      }
      type = dev[id].type;
// Command for this device
// Note, this mostly works by comparing dev to mydev, and sending new required settings from dev and copying to mydev
// Take care to avoid race conditions, e.g. read dev, send command, then copy (changed) dev to mydev - that is bad
      more = 0;
      if (type && mydev[id].polling)
         switch (type)
         {
         case TYPE_MAX:        // Output to Max
            {
               if (mydev[id].restart)
               {                // Max specific resets
                  mydev[id].restart = 0;
                  mydev[id].send0C = 1;
                  mydev[id].send07 = 1;
                  mydev[id].toggle0C = 1;
               }
               if (mydev[id].send0C || mydev[id].output != dev[id].output || mydev[id].invert != dev[id].invert || mydev[id].disable != dev[id].disable)
               {                // Settings changed
                  if (cmdlen)
                     more++;
                  else
                  {
                     mydev[id].send0C = (mydev[id].toggle0C || mydev[id].output != dev[id].output || mydev[id].invert != dev[id].invert || mydev[id].disable != dev[id].disable) ? 1 : 0;       // Send twice
                     mydev[id].output = dev[id].output;
                     mydev[id].invert = dev[id].invert;
                     mydev[id].disable = dev[id].disable;
                     mydev[id].led = dev[id].led;
                     cmd[++cmdlen] = 0x0C;
                     // The logic here is not 100% clear, seems we have to send the toggle on and off to latch something. Seems to work now
                     cmd[++cmdlen] = ((mydev[id].toggle0C ? 0x09 : 0x00) | (((mydev[id].output ^ mydev[id].invert ^ 2) & 3) << 1) | ((mydev[id].disable) ? 0 : 0x80));
                     cmd[++cmdlen] = dev[id].led;
                     mydev[id].send07 = 0;      // Includes LED
                     mydev[id].toggle0C ^= 1;
                  }
               }
               if (mydev[id].send07 || mydev[id].led != dev[id].led)
               {
                  if (cmdlen)
                     more++;
                  else
                  {
                     cmd[++cmdlen] = 0x07;
                     mydev[id].led = cmd[++cmdlen] = dev[id].led;
                     mydev[id].send07 = 0;
                  }
               }
               if (!mydev[id].config && dev[id].config)
               {                // Start config device
                  dev[id].config = 0;   // Clear
                  mydev[id].config = 1;
                  mydev[id].addr = 0;
                  event_t *e = newevent(EVENT_CONFIG, 0, 0);
                  postevent(e);
               }
               if (mydev[id].config)
               {                // Programming EEPROM
                  if (cmdlen)
                     more++;
                  else
                  {
                     if (mydev[id].addr >= sizeof(maxeeprom))
                     {
                        mydev[id].config = 0;   // Done - no reply to reset
                        mydev[id].addr = 0;
                        cmd[++cmdlen] = 0x0A;   // Reset
                     } else
                     {          // Program 4 bytes of EEPROM
                        cmd[++cmdlen] = 0x08;
                        cmd[++cmdlen] = 0x00;
                        cmd[++cmdlen] = mydev[id].addr;
                        unsigned char a0 = 0,
                            a3 = 0;
                        if (mydev[id].addr == 0x0C)
                           a0 = dev[id].newid;  // Device ID at 0x0C
                        if (mydev[id].addr + 3 == 0x1F)
                           a3 = dev[id].newid;  // Checksum at 0x1F
                        cmd[++cmdlen] = maxeeprom[mydev[id].addr++] + a0;
                        cmd[++cmdlen] = maxeeprom[mydev[id].addr++];
                        cmd[++cmdlen] = maxeeprom[mydev[id].addr++];
                        cmd[++cmdlen] = maxeeprom[mydev[id].addr++] + a3;
                     }
                  }
               }
               break;
            }
         case TYPE_PAD:        // output to Keypad
            {
               if (mydev[id].restart)
               {                // Pad specific resets
                  mydev[id].restart = 0;
                  mydev[id].toggle0B = 0;
                  mydev[id].toggle07 = 1;
                  mydev[id].send07 = 1; // Send stuff ASAP
                  mydev[id].send0B = 1;
                  mydev[id].send0C = 1;
                  mydev[id].send0D = 1;
                  mydev[id].send19 = 1;
               }
               if (mydev[id].send0B)
               {                // Next key - needs to be first else we get the key status again and think we have another key
                  if (cmdlen)
                     more++;
                  else
                  {
                     cmd[++cmdlen] = 0x0B;
                     cmd[++cmdlen] = (mydev[id].toggle0B ? 2 : 0);
                     mydev[id].send0B = 0;
                     mydev[id].toggle0B ^= 1;
                  }
               }
               if (mydev[id].send07 || mydev[id].k.blink != dev[id].k.blink || mydev[id].k.cursor != dev[id].k.cursor || memcmp(mydev[id].k.text[0], (void *) dev[id].k.text[0], 16) || memcmp(mydev[id].k.text[1], (void *) dev[id].k.text[1], 16))
               {                // Text change
                  int l,
                   p;
                  if (cmdlen)
                     more++;
                  else
                  {             // Updating display, try to track the change in mydev text as we go and only send what we need to
                     cmd[++cmdlen] = 0x07;
                     cmd[++cmdlen] = 0x01 | ((mydev[id].k.blink = dev[id].k.blink) ? 0x08 : 0x00) | (mydev[id].toggle07 ? 0x80 : 0);
                     unsigned int maketext(int dummy, int space) {
                        int q = cmdlen;
                        if (space)
                           cmd[++q] = 0x17;     // clear/home
                        for (l = 0; l < 2; l++)
                        {
                           for (p = 0; p < 16 && (space ? : mydev[id].k.text[l][p]) == dev[id].k.text[l][p]; p++);      // First character changed
                           if (p == 16)
                              continue; // Line not different
                           if (p < 2)
                           {    // Start line
                              if (l || !mydev[id].send07)
                                 cmd[++q] = l + 1;
                              p = 0;
                           } else
                           {    // Move cursor
                              cmd[++q] = 0x03;
                              cmd[++q] = (l ? 0x40 : 0) + p;
                           }
                           int p2 = p;
                           for (p2 = 15; p2 > p && (space ? : mydev[id].k.text[l][p2]) == dev[id].k.text[l][p2]; p2--); // Last character changed
                           while (p <= p2)
                           {
                              unsigned char c = (dummy ? dev[id].k.text[l][p] : (mydev[id].k.text[l][p] = dev[id].k.text[l][p]));
                              if (c < ' ')
                                 cmd[++q] = ' ';
                              else if (c == '0' && !dev[id].k.cross)
                                 cmd[++q] = 'O';
                              else
                                 cmd[++q] = c;
                              p++;
                           }
                           if (!dummy)
                           {
                              // Record where we left cursor
                              if (p == 16)
                                 mydev[id].k.cursor = (mydev[id].k.cursor & ~0x1F) | (l ? 0 : 0x10);    // wrapped
                              else
                                 mydev[id].k.cursor = (mydev[id].k.cursor & ~0x1F) | (l ? 0x10 : 0) | p;
                           }
                        }
                        return q;
                     }
                     if (mydev[id].send07 || mydev[id].k.cursor)
                     {
                        cmd[++cmdlen] = 0x07;   // Cursor off
                        mydev[id].k.cursor = 0x1F;      // Off
                     }
                     if (mydev[id].send07)
                     {
                        cmd[++cmdlen] = 0x17;   // clear/home
                        for (l = 0; l < 2; l++)
                           for (p = 0; p < 16; p++)
                              mydev[id].k.text[l][p] = ' ';
                        mydev[id].k.cursor &= ~0x1F;    // Home
                        cmdlen = maketext(0, ' ');
                     } else
                     {          // Work out if clear/home would be worthwhile
                        int c1 = maketext(1, ' ');
                        int c2 = maketext(0, 0);
                        if (c1 < c2)
                        {       // Work from blank
                           for (l = 0; l < 2; l++)
                              for (p = 0; p < 16; p++)
                                 mydev[id].k.text[l][p] = ' ';
                           cmdlen = maketext(0, ' ');
                        } else
                           cmdlen = c2;
                     }

                     unsigned char cursor = dev[id].k.cursor;
                     if (cursor)
                     {
                        if ((cursor & 0x1F) != (mydev[id].k.cursor & 0x1F))
                        {       // Move to right place
                           cmd[++cmdlen] = 0x03;
                           cmd[++cmdlen] = ((cursor & 0x10) ? 0x40 : 0) + (cursor & 0x0F);
                        }
                        if (cursor & 0x80)
                           cmd[++cmdlen] = 0x06;        // Solid block
                        else if (cursor & 0x40)
                           cmd[++cmdlen] = 0x10;        // Underline
                     }
                     mydev[id].k.cursor = cursor;
                     if (!mydev[id].toggle07)
                        mydev[id].send07 = 0;   // Sends twice initially
                     mydev[id].toggle07 ^= 1;
                  }
               }
               if (mydev[id].send19 || mydev[id].k.quiet != dev[id].k.quiet || mydev[id].k.silent != dev[id].k.silent)
               {                // Settings changed
                  if (cmdlen)
                     more++;
                  else
                  {
                     mydev[id].k.quiet = dev[id].k.quiet;
                     mydev[id].k.silent = dev[id].k.silent;
                     cmd[++cmdlen] = 0x19;
                     if (mydev[id].k.silent)
                        cmd[++cmdlen] = 0x03;
                     else if (dev[id].k.quiet)
                        cmd[++cmdlen] = 0x05;
                     else
                        cmd[++cmdlen] = 0x01;
                     mydev[id].send19 = 0;
                  }
               }
               if (mydev[id].send0C || mydev[id].k.beep[0] != dev[id].k.beep[0] || mydev[id].k.beep[1] != dev[id].k.beep[1])
               {                // Beep change
                  if (cmdlen)
                     more++;
                  else
                  {
                     mydev[id].k.beep[0] = dev[id].k.beep[0];
                     mydev[id].k.beep[1] = dev[id].k.beep[1];
                     cmd[++cmdlen] = 0x0C;
                     cmd[++cmdlen] = ((mydev[id].k.beep[0] || mydev[id].k.beep[1]) ? mydev[id].k.beep[1] ? 3 : 1 : 0);
                     cmd[++cmdlen] = mydev[id].k.beep[0];
                     cmd[++cmdlen] = mydev[id].k.beep[1];
                     mydev[id].send0C = 0;
                  }
               }
               if (mydev[id].send0D || mydev[id].k.backlight != dev[id].k.backlight)
               {
                  if (cmdlen)
                     more++;
                  else
                  {
                     cmd[++cmdlen] = 0x0D;
                     mydev[id].k.backlight = cmd[++cmdlen] = dev[id].k.backlight;
                     mydev[id].send0D = 0;
                  }
               }
               break;
            }
         case TYPE_RIO:        // Output to RIO
            {
               if (mydev[id].restart)
               {                // RIO specific resets
                  mydev[id].restart = 0;
                  mydev[id].send00 = (1 << MAX_INPUT) - 1;
                  mydev[id].send01 = 1;
                  mydev[id].send02 = 1;
                  mydev[id].toggle00 = 1;
                  int n;
                  for (n = 0; n < MAX_INPUT; n++)
                     mydev[id].threshold[n][0] = 0xFF;  // Seriously, we have to force sending twice even when normal defaults as you cannot set back
               }
               int n;
               for (n = 0; n < MAX_INPUT && mydev[id].response[n] == dev[id].ri[n].response; n++);
               if (mydev[id].send01 || n < MAX_INPUT)
               {
                  if (cmdlen)
                     more++;
                  else
                  {
                     cmd[++cmdlen] = 1;
                     for (n = 0; n < MAX_INPUT; n++)
                     {
                        mydev[id].response[n] = dev[id].ri[n].response;
                        cmd[++cmdlen] = (mydev[id].response[n] ? : RIO_DEFAULT_RESPONSE / 10);
                     }
                     mydev[id].send01 = 0;
                  }
               }
               for (n = 0; n < MAX_INPUT; n++)
                  if ((mydev[id].send00 & (1 << n)) || memcmp(mydev[id].threshold[n], (unsigned char *) dev[id].ri[n].threshold, sizeof(mydev[id].threshold[n])))
                  {             // Send thresholds
                     if (cmdlen)
                        more++;
                     else
                     {

// Has to be sent twice!
                        if (memcmp(mydev[id].threshold[n], (unsigned char *) dev[id].ri[n].threshold, sizeof(mydev[id].threshold[n])))
                           mydev[id].send00 |= (1 << n);        // Send again
                        else
                           mydev[id].send00 &= ~(1 << n);
                        mydev[id].toggle00 ^= 1;
                        cmd[++cmdlen] = 0x00;
                        cmd[++cmdlen] = 0x0E;
                        cmd[++cmdlen] = 0x00;
                        cmd[++cmdlen] = n;
                        memcpy(mydev[id].threshold[n], (unsigned char *) dev[id].ri[n].threshold, sizeof(mydev[id].threshold[n]));
                        if (mydev[id].threshold[n][sizeof(mydev[id].threshold[n]) - 1])
                        {
                           memcpy(cmd + cmdlen + 1, (unsigned char *) mydev[id].threshold[n], sizeof(mydev[id].threshold[n]));
                           cmdlen += sizeof(mydev[id].threshold[n]);
                        } else
                        {
                           cmd[++cmdlen] = rio_thresholds[0].tampersc;
                           cmd[++cmdlen] = rio_thresholds[0].lowres;
                           cmd[++cmdlen] = rio_thresholds[0].normal;
                           cmd[++cmdlen] = rio_thresholds[0].highres;
                           cmd[++cmdlen] = rio_thresholds[0].open;
                        }
                     }
                  }
               if (mydev[id].send02 || mydev[id].output != dev[id].output || mydev[id].invert != dev[id].invert)
               {                // Settings changed - send new output
                  if (cmdlen)
                     more++;
                  else
                  {
                     mydev[id].output = dev[id].output;
                     mydev[id].invert = dev[id].invert;
                     cmd[++cmdlen] = 0x02;
                     cmd[++cmdlen] = 0x00;
                     for (n = 0; n < MAX_OUTPUT; n++)
                     {
                        if (mydev[id].invert & (1 << n))
                        {       // Invert
                           if (dev[id].output & (1 << n))
                              cmd[++cmdlen] = 0x0C;
                           else
                              cmd[++cmdlen] = 0x09;
                        } else
                        {
                           if (mydev[id].output & (1 << n))
                              cmd[++cmdlen] = 0x05;
                           else
                              cmd[++cmdlen] = 0x00;
                        }
                        cmd[++cmdlen] = 0x00;
                        cmd[++cmdlen] = 0x00;
                     }
                     mydev[id].send02 = 0;
                  }
               }
               break;
            }
         case TYPE_RFR:        // Output to RF RIO
            {
               // TODO - is anything sent?
               break;
            }
         }

      if (!cmdlen)
      {                         // Poll or scan
         if (type && mydev[id].polling)
         {
            if (type == TYPE_RIO)
               cmd[++cmdlen] = 0x01;    // Simple poll
            else
               cmd[++cmdlen] = 0x06;    // Simple poll
         } else
         {                      // Check for device
            cmd[++cmdlen] = 0;
            cmd[++cmdlen] = 0x0E;
            if (!dev[id].type || dev[id].missing)
               retry = MAX_RETRY;       // try once
         }
      }
      {                         // Send next command
         unsigned int c = 0xAA, // Work out checksum
             n;
         cmd[0] = id;
         cmdlen++;
         for (n = 0; n < cmdlen; n++)
            c += cmd[n];
         while (c > 0xFF)
            c = (c >> 8) + (c & 0xFF);
         cmd[cmdlen++] = c;
         sendcmd();
      }
      reslen = 0;               // ready for next message
      if (nextka <= now.tv_sec)
      {                         // Keep alive stats
         nextka = now.tv_sec + 60;
         event_t *e = newevent(EVENT_KEEPALIVE, 0, 0);
         e->rx = rx;
         e->tx = tx;
         e->errors = errors;
         e->stalled = stalled;
         e->retries = retries;
         postevent(e);
         retries = 0;
         stalled = 0;
         errors = 0;
         tx = 0;
         rx = 0;
      }
      void newevents(int etype, unsigned short status, unsigned short changed) {        // Post each change separately
         int i;
         for (i = 0; changed && i < MAX_TAMPER; i++)
            if (changed & (1 << i))
            {
               changed &= ~(1 << i);
               event_t *e = newevent(etype, 1, i + 1);
               e->state = ((status & (1 << i)) ? 1 : 0);
               postevent(e);
            }
      }
      // Status change event?
      if (mydev[id].input != dev[id].input)
      {
         unsigned short changed = ((mydev[id].input ^ dev[id].input) & ~dev[id].inhibit);
         if (changed)
         {
            dev[id].input ^= changed;
            newevents(EVENT_INPUT, dev[id].input, changed);
         }
      }
      // Tamper change event?
      if (mydev[id].tamper != dev[id].tamper)
      {
         unsigned short changed = (mydev[id].tamper ^ dev[id].tamper);
         dev[id].tamper ^= changed;
         newevents(EVENT_TAMPER, dev[id].tamper, changed);
      }
      // Fault change event?
      if (mydev[id].fault != dev[id].fault)
      {
         unsigned short changed = (mydev[id].fault ^ dev[id].fault);
         dev[id].fault ^= changed;
         newevents(EVENT_FAULT, dev[id].fault, changed);
      }
   }

   return d;
}

// Functions

void bus_init(void)
{                               // General init - call before anything else
// Event queue
   pthread_mutex_init(&qmutex, 0);
   pthread_mutex_init(&outputmutex, 0);
   pipe2(qpipe, O_NONBLOCK);    // We check queue anyway an we don't want to risk stalling if app is stalled for some reason and a lot of events
// Thread
}

void bus_start(int n)
{
   if (n < 0 || n >= MAX_BUS)
      errx(1, "Bad bus ID to start %d", n);
   pthread_attr_t a;
   struct sched_param s = {
      0
   };
   s.sched_priority = 10;
   pthread_attr_init(&a);
   pthread_attr_setschedparam(&a, &s);
   pthread_attr_setschedpolicy(&a, SCHED_RR);
   if (pthread_create(&busthreads[n], &a, poller, (void *) (long) n))
      warn("Bus start failed");
   pthread_attr_destroy(&a);
}

void bus_stop(int n)
{
   if (n < 0 || n >= MAX_BUS)
      errx(1, "Bad bus ID to stop %d", n);
   if (pthread_cancel(busthreads[n]))
      warn("Bad thread cancel");
}

event_t *bus_event(long long usec)
{                               // Get next event
   char x;
// Check for event waiting
   pthread_mutex_lock(&qmutex);
   event_t *e = (event_t *) event;
   if (e)
      event = event->next;
   pthread_mutex_unlock(&qmutex);
   if (e)
   {                            // an event was waiting
      if (read(qpipe[0], &x, 1) < 0)
         perror("queue recv");
      return e;
   }
   if (usec < 0)
      return NULL;
// No event waiting - wait timeout specified
   fd_set readfds;
   FD_ZERO(&readfds);
   FD_SET(qpipe[0], &readfds);
   struct timeval timeout = { 0
   };
   timeout.tv_sec = usec / 1000000ULL;
   timeout.tv_usec = usec % 1000000ULL;
   int s = select(qpipe[0] + 1, &readfds, NULL, NULL, &timeout);
   if (s <= 0)
      return NULL;              // Nothing waiting in the time
   if (read(qpipe[0], &x, 1) < 0)
      perror("queue recv");
// Get the waiting event
   pthread_mutex_lock(&qmutex);
   e = (event_t *) event;
   if (e)
      event = event->next;
   pthread_mutex_unlock(&qmutex);
   return e;
}

#ifndef	LIB
int main(int argc, const char *argv[])
{
   {
      int c;
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         POPT_AUTOHELP {
                         }
      };
      optCon = poptGetContext(NULL, argc, argv, optionsTable, 0);
//poptSetOtherOptionHelp (optCon, "]");
      if ((c = poptGetNextOpt(optCon)) < -1)
         errx(1, "%s: %s\n", poptBadOption(optCon, POPT_BADOPTION_NOALIAS), poptStrerror(c));
      if (poptPeekArg(optCon))
      {
         poptPrintUsage(optCon, stderr, 0);
         return -1;
      }
      poptFreeContext(optCon);
   }


   return 0;
}
#endif
