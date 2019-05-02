// An alarm panel using galaxybus library

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
#include <stdarg.h>
#include <unistd.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>
#include <err.h>
#include <signal.h>
#include <pthread.h>
#include <curl/curl.h>
#include <galaxybus.h>
#include <axl.h>
#include <dataformat.h>
#ifdef	LIBEMAIL
#include <libemail.h>
#endif
#ifdef	LIBWS
#include <websocket.h>
#endif
#ifdef	LIBMQTT
#include <mosquitto.h>
#endif

xml_t config = NULL;
const char *maxfrom = NULL,
   *maxto = NULL;
volatile int configchanged = 0;
pthread_mutex_t eventmutex;;
#ifdef LIBMQTT
struct mosquitto *iot = NULL;
struct mosquitto *mqtt = NULL;
#endif

#define	MAX_GROUP	10      // Mainly to work with keypad...
typedef unsigned long long group_t;     // Set of groups
#define	ALL_GROUPS	((1<<MAX_GROUP)-1)

#define	MAX_FOB		5       // Fobs per user

// Key states - these are the states (one bit per group) used for output, and derived from inputs or internally
// Input states (triggers) are t(), with latching triggers l()
// Other internally derived states s(), as are OPEN and INTRUDER (based on ZONE and SET)
#define	STATE	\
	l(INTRUDER)	\
	l(TAMPER)	\
	l(FIRE)		\
	l(FAULT)	\
	t(WARNING)	\
	t(OPEN)	\
	t(ZONE)	\
	t(NONEXIT)	\
	t(ENTRY)	\
	t(USER1)	\
	t(USER2)	\
	t(USER3)	\
	t(USER4)	\
	s(ENGINEERING)	\
	s(UNSET)	\
	s(SET)		\
	s(ARM)	\
	s(PREALARM)	\
	s(ALARM)	\
	s(BELL)		\
	s(STROBE)	\

// First, count the latching states
#define l(x) DUMMY1_##x,
#define t(x)
#define s(x)
enum
{ STATE STATE_LATCHED
};
#undef l
#undef t
#undef s

// Now, count the trigger and latching states
#define l(x) DUMMY2_##x,
#define t(x) DUMMY2_##x,
#define s(x)
enum
{ STATE STATE_TRIGGERS
};
#undef l
#undef t
#undef s

// Now, make the actual state list, latching and triggers first, then latching _LATCH, then others
enum
{
#define l(x) STATE_##x,
#define t(x)
#define s(x)
   STATE
#undef l
#undef t
#undef s
#define l(x)
#define t(x) STATE_##x,
#define s(x)
      STATE
#undef l
#undef t
#undef s
#define l(x) STATE_##x##_LATCH,
#define t(x)
#define s(x)
      STATE
#undef l
#undef t
#undef s
#define l(x)
#define t(x)
#define s(x) STATE_##x,
      STATE
#undef l
#undef t
#undef s
   STATES
};

// Now the same as above but to define the state names matching the above enum
const char *state_name[STATES] = {
#define l(x) #x,
#define t(x)
#define s(x)
   STATE
#undef l
#undef t
#undef s
#define l(x)
#define t(x) #x,
#define s(x)
      STATE
#undef l
#undef t
#undef s
#define l(x) #x"_LATCH",
#define t(x)
#define s(x)
      STATE
#undef l
#undef t
#undef s
#define l(x)
#define t(x)
#define s(x) #x,
      STATE
#undef l
#undef t
#undef s
};

typedef struct keypad_s keypad_t;
typedef unsigned char state_t;

struct port_app_s
{
   xml_t config;                // The XML config
   keypad_t *keypad;            // If a keypad
   int a,
     x,
     y;                         // Location on floor plan
   const char *t;               // Icon on floor plan
   unsigned char onplan:1;      // Is on floor plan
   unsigned char missing:1;     // Device missing
   unsigned char invert:1;      // Invert logic
   union
   {
      struct
      {                         // Inputs
         unsigned char inuse:1; // Mentioned in config at all
         unsigned char isexit:1;        // Door related input
         unsigned char isbell:1;        // Door related input
         unsigned char tamper:1;        // Known tamper state
         unsigned char fault:1; // Known fault state
         unsigned char input:1; // Known input state
         group_t trigger[STATE_TRIGGERS];       // If this input applies to a state
      };
      struct
      {                         // Outputs
         state_t type;          // Which state we are outputting
         group_t group;         // Which groups it applies to
      };
   };
};

#ifdef	LIBWS
char *wsport = NULL;
const char *wsorigin = NULL;
const char *wshost = NULL;
const char *wscertfile = NULL;
const char *wskeyfile = NULL;
char *wsfloorplan = NULL;
#endif

unsigned int commfailcount = 0;
unsigned int commfailreported = 0;
group_t walkthrough = 0;


// The actual state bits...
const char *setfile = "/var/local/alarmset";
static group_t state[STATES] = { };     // the states, for each group
static group_t previous_state[STATES] = { };    // the states, for each group

typedef struct statelist_s statelist_t;
struct statelist_s
{
   statelist_t *next;
   statelist_t **prev;
   time_t when;
   group_t groups;
   state_t type;
   char *port;
   char *name;
};
static statelist_t *statelist = NULL;

// My internal structures for panel use...
struct
{
   char *name;
   time_t when_set;             // When set complete
   time_t when_fail;            // When set fail
   time_t when_alarm;           // When entry/alarm started
   int time_set;                // How long to set
   int time_fail;               // How long for failing to set
   int entry_time;              // Entry time
   int bell_delay;              // Delay before ringing (from alarm set)
   int bell_time;               // Max time to ring bell (from alarm set)
   int bell_rest;               // Rest time (from alarm set)
   int count[STATE_TRIGGERS];   // Input counts
   const char *armed_by;        // Who armed the alarm
   group_t setifany;            // Auto set, if any of these set
   group_t setifall;            // Auto set, if all of these set
} group[MAX_GROUP];

// Doors not a linked list as mapped to door structure in library
typedef struct mydoor_s mydoor_t;
struct mydoor_s
{                               // Per door settings
   port_p i_fob[2];             // Max readers for FOB to exit and set alarm
   port_p i_exit[3];            // Exit buttons, max readers, etc, or reception desk
   port_p i_bell[2];            // Bell button(s)
   port_p o_bell[2];            // Bell output(s)
   group_t group_fire;          // Which groups, any of which force the door open for fire
   group_t group_lock;          // Which groups, any of which mean the door is "locked" (if not fire)
   group_t group_arm;           // Which groups can arm alarm
   group_t group_disarm;        // Which groups can unset alarm
   int time_set;                // If set, the hold fob does timed set, else does not
   char *name;
   int airlock;                 // door that must be closed before this one is opened
   state_t lockdown;            // Outputs triggering lockdown
   unsigned char entry:1;       // Reported states from door
   unsigned char intruder:1;
   unsigned char tamper:1;
   unsigned char fault:1;
   unsigned char warning:1;
   unsigned char lock_arm:1;
   unsigned char lock_set:1;
   unsigned char opening:1;
   int a,
     x,
     y;                         // Position on floor plan
   char *t;                     // icon
} mydoor[MAX_DOOR] =
{
};

typedef struct user_s user_t;
struct user_s
{
   user_t *next;
   char *name;
   char *fullname;
   char *hash;
   unsigned long pin;
   fob_t fob[MAX_FOB];
   group_t group_arm;
   group_t group_disarm;
   group_t group_reset;
   group_t group_open;
   group_t group_prop;
};
static user_t *users = NULL;

typedef struct keypad_s keypad_t;
struct keypad_s
{
   char *name;
   keypad_t *next;              // Linked list
   keypad_data_t k;             // Display, etc
   keypad_data_t kwas;
   port_p port;                 // The port it is on
   port_p prox;                 // Prox reader
   char *message;               // Display message
   time_t when_logout;          // Auto user logout
   int time_logout;             // Config logout time
   time_t when;                 // When to next update keypad
   group_t groups;              // What groups the keypad applies to
   group_t group_arm;           // What groups can be armed with no login
   group_t group_reset;         // What groups can be reset with no login
   unsigned char pininput;      // Inputting PIN
   unsigned long pin;           // PIN so far
   user_t *user;                // User if logged in
   const char *msg;             // Flash up message
   int posn;                    // List position
   time_t when_posn;            // List time
   const char *alert;           // Last alert state
   unsigned char ack:1;         // Alert acknowledged so do not beep
   unsigned char block:1;	// Block input
};
static keypad_t *keypad = NULL;

static struct timeval now;
static struct tm lnow;
static unsigned int buses = 0;
static group_t groups = 0;

struct
{
   time_t watchdog;
   unsigned char fault:1;
} mybus[MAX_BUS] =
{
};

// Functions
static void *dolog (group_t g, const char *type, const char *user, const char *port, const char *fmt, ...);
static keypad_t *keypad_new (port_p p);
static void keypads_message (group_t g, const char *msg);
static void state_change (group_t g);
#define	port_name(p) real_port_name(alloca(30),p)
static const char *real_port_name (char *v, port_p p);

void
mqtt_output (port_p p, int v)
{                               // Send output via MQTT
   if (!p || !p->mqtt)
      return;
   int port = port_port (p);
   if (!port)
      return;
   if (p->app && p->app->invert)
      v = 1 - v;
   char *topic;
   asprintf (&topic, "command/SS/%s/output%d", p->mqtt, port);
   char msg = v + '0';
   mosquitto_publish (mqtt, NULL, topic, 1, &msg, 1, 0);
   free (topic);
}

static int
parse_time (const char *v, int min, int max)
{
   int n = 0;
   if (v)
   {
      const char *p = v;
      while (isdigit (*p))
         n = n * 10 + *p++ - '0';
      n *= 10;
      if (*p == '.' && isdigit (p[1]))
      {
         n += p[1] - '0';
         p += 2;
      }
      if (*p)
         dolog (groups, "CONFIG", NULL, NULL, "Cannot parse time %s", v);
   }
   if (n < min)
   {
      dolog (groups, "CONFIG", NULL, NULL, "Time too short %s<%d.%d", v, min / 10, min % 10);
      n = min;
   }
   if (max && n > max)
   {
      dolog (groups, "CONFIG", NULL, NULL, "Time too long %s>%d.%d", v, max / 10, max % 10);
      n = max;
   }
   return n;
}

static state_t
state_parse (const char *v)
{                               // Get output type
   if (!v)
      return -1;
   int n = 0;
   if (v)
      for (n = 0; n < STATES && strcasecmp (state_name[n], v); n++);
   if (n < STATES)
      return n;
   return 0;
}

#define	port_parse_i(v,e) port_parse(v,e,1)
#define	port_parse_o(v,e) port_parse(v,e,0)
static port_p
port_parse (const char *v, const char **ep, int i)
{                               // Parse a port name/id, return 0 if invalid, port part 0 if no port
   if (!v || !*v)
      return NULL;
   int l = 0;
   while (v[l] && v[l] != ',' && !isspace (v[l]))
      l++;
   char *n = alloca (l + 1);
   memcpy (n, v, l);
   n[l] = 0;
   if (v[l] == ',')
      l++;
   while (isspace (v[l]))
      l++;
   if (ep)
      *ep = (v[l] ? v + l : NULL);
   v = n;
   int port = 0;
   if (l > 2 && v[l - 2] == '#' && isdigit (v[l - 1]))
   {                            // Port
      if (i < 0)
         return NULL;           // After device only
      port = v[l - 1] - '0';
      l -= 2;
      ((char *) v)[l] = 0;
   }
   if (!port && i == 0)
   {                            // Check output port names
      port_p p;
      for (p = ports; p && (!p->name || !p->port || p->isinput || strcmp (p->name, v)); p = p->next);
      if (p)
         return p;
   } else if (!port && i == 1)
   {                            // Check input port names
      port_p p;
      for (p = ports; p && (!p->name || !p->port || !p->isinput || strcmp (p->name, v)); p = p->next);
      if (p)
         return p;
   } else if (port || i == -1)
   {                            // Reference device
      port_p p;
      for (p = ports; p && (!p->name || p->port || strcmp (p->name, v)); p = p->next);
      if (p)
      {
         if (port)
            return port_new_base (p, i, port);
         return p;
      }
   }
   // Parse the port
   unsigned int id = 0,
      type = 0;
   while (type < MAX_TYPE && strncmp (type_name[type], v, strlen (type_name[type])))
      type++;
   if (type < MAX_TYPE)
   {                            // Bus based port
      v += strlen (type_name[type]);
      if (*v > '0' && *v <= '0' + MAX_BUS && isxdigit (v[1]) && isxdigit (v[2]))
      {                         //  Device
         id = ((*v - '1') << 8) + (((isalpha (v[1]) ? 9 : 0) + (v[1] & 0xF)) << 4) + ((isalpha (v[2]) ? 9 : 0) + (v[2] & 0xF));
         v += 3;
         if (*v > '0' && *v <= '9')
         {
            if (port)
               return NULL;     // silly as # as well
            port = *v - '0';
         }
      }
      while (*v && !isspace (*v) && *v != ',')
         v++;
      if (type && id && id < MAX_DEVICE)
      {
         if (!device[id].type)
         {
            device[id].type = type;
            if (type == TYPE_MAX)
               device[id].fob_hold = 30;        // 3 second default
            buses |= (1 << (id >> 8));
         } else if (device[id].type != type)
         {
            dolog (groups, "CONFIG", NULL, NULL, "Device type clash port %s %s/%s, device disabled", v, type_name[device[id].type],
                   type_name[type]);
            device[id].disabled = 1;
         }
      }
      if (!port && i >= 0)
         return NULL;           // Expecting a port
      if (port && i < 0)
         return NULL;           // Expecting base device
      return port_new_bus (id >> 8, id & 0xFF, i, port);
   }
   if (strlen (v) < 6 || !isxdigit (v[0]) || !isxdigit (v[1]) || !isxdigit (v[2]) || !isxdigit (v[3]) || !isxdigit (v[4])
       || !isxdigit (v[5]))
      return NULL;
   if (v[6] == 'I')
   {
      if (port)
         return NULL;           // # and I
      if (i != 1 || !isdigit (v[7]) || v[8])
         return NULL;
      port = v[7] - '0';
   } else if (v[6] == 'O')
   {
      if (port)
         return NULL;           // # and O
      if (i != 0 || !isdigit (v[7]) || v[8])
         return NULL;
      port = v[7] - '0';
   }
   ((char *) v)[6] = 0;
   if (!port && i >= 0)
      return NULL;              // Expecting a port
   if (port && i < 0)
      return NULL;              // Expecting base device
   return port_new (v, i, port);
}

static group_t
group_parse (const char *v)
{
   if (!v)
      return 0;
   group_t g = 0;
   while (*v)
   {
      if (*v >= '0' && *v < '0' + MAX_GROUP)
         g |= (1 << (*v - '0'));
      else if (*v == '*')
         return -1;
      v++;
   }
   groups |= g;
   return g;
}

#define	group_list(g) real_group_list(alloca(MAX_GROUP+1),g)
static char *
real_group_list (char *v, group_t g)
{
   char *o = v;
   int n;
   for (n = 0; n < MAX_GROUP; n++)
      if (g & (1 << n))
         *o++ = '0' + n;
      else
         *o++ = '-';
   *o = 0;
   return v;
}

static const char *
real_port_name (char *v, port_p p)
{                               // Port name
   if (!p)
      return "?";
   if (p->id == US)
      return "PANEL";
   unsigned int id = port_device (p);
   int port = port_port (p);
   char *o = v;
   if (p->mqtt)
   {
      o += sprintf (o, "%.6s", p->mqtt);
      if (port)
         o += sprintf (o, "%d", port);
      return v;
   }
   if (id >= MAX_DEVICE)
      o += sprintf (o, "???");
   else
      o += sprintf (o, type_name[device[id].type]);
   o += sprintf (o, "%X%02X", port_bus (p) + 1, port_id (p));
   if (port_port (p))
      o += sprintf (o, "%d", port_port (p));
   return v;
}

// Functions
static port_app_t *
port_app (port_p p)
{                               // ensure port has app
   if (!p)
      return NULL;
   if (!p->app)
   {
      p->app = malloc (sizeof (*p->app));
      if (!p->app)
         errx (1, "malloc");
      memset (p->app, 0, sizeof (*p->app));
   }
   return p->app;
}

#define	port_exit_set(w,v,p,door) port_exit_set_n(port_set_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p),v,p,1,door,"Exit"),sizeof(w)/sizeof(port_p))
#define	port_bell_set(w,v,p,door) port_bell_set_n(port_set_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p),v,p,1,door,"Bell"),sizeof(w)/sizeof(port_p))
#define	port_i_set(w,v,p,door,name) port_set_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p),v,p,1,door,name)
#define	port_o_set(w,v,p,door,name) port_set_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p),v,p,0,door,name)
#define	port_set(w,v,p,door,name) port_set_n((volatile port_p*)&(w),sizeof(w)/sizeof(port_p),v,p,-1,door,name)
static volatile port_p *
port_set_n (volatile port_p * w, int n, const char *v, unsigned char p, int i, char *door, const char *name)
{                               // Set up port
   int q = 0;
   while (v && *v && q < n)
   {
      const char *tag = v;
      port_p port = port_parse (tag, &v, i);
      if (p && !port)
         port = port_parse (tag, &v, -1);
      if (p && port && !port_port (port))
         port = port_new_base (port, i, p);
      if (!port)
         continue;
      w[q++] = port;
      int pd = port_device (port);
      p = port_port (port);
      if (p && pd < MAX_DEVICE)
      {                         // port
         if (i == 1 && device[pd].type == TYPE_RIO)
            device[pd].ri[p].response = 1;
         if (i == 0 && port_app (port)->type >= STATES)
            port_app (port)->type = STATES;
      }
      if (p && name && !port->name)
         asprintf ((char **) &port->name, "%s-%s", door ? : tag, name);
   }
   if (v)
      dolog (groups, "CONFIG", NULL, NULL, "Too many ports in list %s", v);
   return w;
}

static volatile port_p *
port_exit_set_n (volatile port_p * w, int n)
{
   while (n--)
      if (w[n])
         port_app (w[n])->isexit = 1;
   return w;
}

static volatile port_p *
port_bell_set_n (volatile port_p * w, int n)
{
   while (n--)
      if (w[n])
         port_app (w[n])->isbell = 1;
   return w;
}

static char *
xml_copy (xml_t x, char *n)
{                               // get attribute/value and copy
   char *v = xml_get (x, n);
   if (v)
      v = strdup (v);
   return v;
}

void
keypad_send (keypad_t * k, int force)
{
   if (!k->port)
      return;                   // WTF
   int id = port_device (k->port);
   if (id)
   {                            // Bus based
      memcpy ((void *) &device[id].k, (void *) &k->k, sizeof (k->k));
      return;
   }
   if (!k->port->mqtt)
      return;                   // WTF
   if (port_app (k->port)->missing)
      return;                   // No point
   char topic[50];
   unsigned char message[50];
   if (force || memcmp ((void *) &k->k.text, (void *) &k->kwas.text, sizeof (k->k.text)))
   {
      snprintf (topic, sizeof (topic), "command/SS/%s/display", k->port->mqtt);
      memcpy (message, (void *) &k->k.text[0], 16);
      memcpy (message + 16, (void *) &k->k.text[1], 16);
      if (!k->k.cross)
      {                         // Not crossed zeros
         int n;
         for (n = 0; n < 32; n++)
            if (message[n] == '0')
               message[n] = 'O';
            else if (message[n] < ' ')
               message[n] = ' ';
      }
      mosquitto_publish (mqtt, NULL, topic, 32, message, 1, 0);
   }
   if (force || k->k.backlight != k->kwas.backlight)
   {
      snprintf (topic, sizeof (topic), "command/SS/%s/backlight", k->port->mqtt);
      *message = '0' + k->k.backlight;
      mosquitto_publish (mqtt, NULL, topic, 1, message, 1, 0);
   }
   if (force || k->k.blink != k->kwas.blink)
   {
      snprintf (topic, sizeof (topic), "command/SS/%s/blink", k->port->mqtt);
      *message = '0' + k->k.blink;
      mosquitto_publish (mqtt, NULL, topic, 1, message, 1, 0);
   }
   if (force || k->k.quiet != k->kwas.quiet || k->k.silent != k->kwas.silent)
   {
      snprintf (topic, sizeof (topic), "command/SS/%s/keyclick", k->port->mqtt);
      *message = '0' + (k->k.silent ? 3 : k->k.quiet ? 5 : 1);
      mosquitto_publish (mqtt, NULL, topic, 1, message, 1, 0);
   }
   if (force || k->k.cursor != k->kwas.cursor)
   {
      snprintf (topic, sizeof (topic), "command/SS/%s/blink", k->port->mqtt);
      *message = 0x20 + k->k.cursor;
      mosquitto_publish (mqtt, NULL, topic, 1, message, 1, 0);
   }
   if (force || memcmp ((void *) &k->k.beep, (void *) &k->kwas.beep, sizeof (k->k.beep)))
   {
      snprintf (topic, sizeof (topic), "command/SS/%s/sounder", k->port->mqtt);
      memcpy (message, (void *) &k->k.beep, sizeof (k->k.beep));
      mosquitto_publish (mqtt, NULL, topic, k->k.beep[0] ? sizeof (k->k.beep) : 0, message, 1, 0);
   }
   memcpy ((void *) &k->kwas, (void *) &k->k, sizeof (k->k));
}

#ifdef	LIBWS
xml_t
device_ws (xml_t root, port_p p)
{
   port_app_t *app = p->app;
   if (!app)
      return NULL;
   xml_t x = xml_element_add (root, "device");
   xml_add (x, "@id", p->mqtt);
   xml_add (x, "@dev", p->mqtt);
   xml_add (x, "@name", p->name ? : p->mqtt);
   return x;
}

xml_t
keypad_ws (xml_t root, keypad_t * k)
{                               // Add keypad status to XML
   xml_t x = xml_element_add (root, "keypad");
   xml_add (x, "@id", port_name (k->port));
   if (xml_get (root, "@full-data"))
   {
      if (k->name)
         xml_add (x, "@name", k->name);
   }
   xml_add (x, "+line", (char *) k->k.text[0]);
   xml_add (x, "+line", (char *) k->k.text[1]);
   if(k->k.beep[0])
   {
   xml_addf (x, "+-beep", "%d", k->k.beep[0]);
   xml_addf (x, "+-beep", "%d", k->k.beep[1]);
   }
   if (k->k.cursor)
      xml_addf (x, "+@-cursor", "%d", k->k.cursor);
   if (k->k.silent)
      xml_add (x, "+@-silent", "true");
   if (k->k.blink)
      xml_add (x, "+@-blink", "true");
   if (k->k.backlight)
      xml_add (x, "+@-backlight", "true");
   return x;
}

xml_t
door_ws (xml_t root, int d)
{                               // Add door status to XML
   if (!door[d].state)
      return NULL;              // Not active
   xml_t x = xml_element_add (root, "door");
   xml_addf (x, "@id", "DOOR%02d", d);
   if (xml_get (root, "@full-data"))
   {
      if (mydoor[d].a >= 0)
         xml_addf (x, "@-a", "%d", mydoor[d].a);
      if (mydoor[d].x >= 0)
         xml_addf (x, "@-x", "%d", mydoor[d].x);
      if (mydoor[d].y >= 0)
         xml_addf (x, "@-y", "%d", mydoor[d].y);
      if (mydoor[d].t)
         xml_add (x, "@t", mydoor[d].t);
      if (mydoor[d].name)
         xml_add (x, "@name", mydoor[d].name);
   }
   xml_add (x, "@state", door_name[door[d].state]);
   return x;
}

xml_t
state_ws (xml_t root, char *tag, int s, int c)
{
   if (!c)
      return NULL;
   xml_t x = xml_add (root, tag, NULL);
   const char *i = state_name[s];
   char *name = alloca (strlen (i) + 3),
      *p = name;
   *p++ = '+';
   *p++ = '-';
   while (*i)
      *p++ = tolower (*i++);
   *p = 0;
   int g;
   for (g = 0; g < MAX_GROUP; g++)
      if (c & (1 << g))
         xml_addf (x, name, "%d", g);
   return x;
}

xml_t
input_ws (xml_t root, port_p port)
{
   if (!port || !port->isinput || !port->port)
      return NULL;
   int id = port_device (port);
   int n = port_port (port);
   if (!n)
      return NULL;
   port_app_t *app = port_app (port);
   n--;
   xml_t x = xml_element_add (root, "input");
   xml_add (x, "@id", port_name (port));
   if (xml_get (root, "@full-data"))
   {
      if (app->onplan)
      {
         xml_addf (x, "@-a", "%d", app->a);
         xml_addf (x, "@-x", "%d", app->x);
         xml_addf (x, "@-y", "%d", app->y);
         xml_add (x, "@t", app->t);
      }
      if (port_name (port))
         xml_add (x, "@name", port->name ? : port_name (port));
      if (port->mqtt)
         xml_add (x, "@dev", port->mqtt);
      else if (id)
         xml_addf (x, "@dev", "%s%d%02X", type_name[device[port_device (port)].type], port_bus (port) + 1, port_id (port));
      xml_addf (x, "@port", "%d", port_port (port));
      if (port->mqtt)
         xml_add (x, "@type", "esp");
      else if (id && device[id].type == TYPE_RIO)
         xml_add (x, "@type", "rio");
      else if (id && device[id].type == TYPE_MAX)
         xml_add (x, "@type", "max");
   }
   if (app->input)
      xml_add (x, "@-active", "true");
   if (app->tamper)
      xml_add (x, "@-tamper", "true");
   if (app->fault)
      xml_add (x, "@-fault", "true");
   return NULL;
}

xml_t
output_ws (xml_t root, port_p port)
{
   if (!port)
      return NULL;
   int id = port_device (port);
   int n = port_port (port);
   port_app_t *app = port_app (port);
   n--;
   if (app->type == (state_t) - 1)
      return NULL;              // Not in use
   xml_t x = xml_element_add (root, "output");
   xml_add (x, "@id", port_name (port));
   if (xml_get (root, "@full-data"))
   {
      if (app->onplan)
      {
         xml_addf (x, "@-a", "%d", app->a);
         xml_addf (x, "@-x", "%d", app->x);
         xml_addf (x, "@-y", "%d", app->y);
         xml_add (x, "@t", app->t);
      }
      if (port_name (port))
         xml_add (x, "@name", port->name ? : port_name (port));
      if (port->mqtt)
         xml_add (x, "@dev", port->mqtt);
      else if (id)
         xml_addf (x, "@dev", "%s%d%02X", type_name[device[port_device (port)].type], port_bus (port) + 1, port_id (port));
      xml_addf (x, "@port", "%d", port_port (port));
      if (port->mqtt)
         xml_add (x, "@type", "esp");
      else if (id && device[id].type == TYPE_RIO)
         xml_add (x, "@type", "rio");
      else if (id && device[id].type == TYPE_MAX)
         xml_add (x, "@type", "max");
   }
   if (port->state)
      xml_add (x, "@-active", "true");
   return NULL;
}
#endif

static void
save_config (const char *configfile)
{
   if (!config || !configchanged)
      return;
   pthread_mutex_lock (&eventmutex);    // Avoid things changing
   char *temp = alloca (strlen (configfile) + 5);
   sprintf (temp, "%s.tmp", configfile);
   FILE *f = fopen (temp, "w");
   if (f)
   {
      xml_write (f, config);
      fclose (f);
      if (rename (temp, configfile))
         dolog (groups, "CONFIG", NULL, NULL, "Cannot save %s", configfile);
      else
      {
         dolog (groups, "CONFIG", NULL, NULL, "Saved %s", configfile);
         configchanged = 0;
      }
   }
   pthread_mutex_unlock (&eventmutex);  // Avoid things changing
   sync ();
}

static void *
load_config (const char *configfile)
{
   buses = 0;
   groups = 0;
   const char *v,
    *pl;
   if (config)
      xml_tree_delete (config);
   if (debug)
      warnx ("Config read");
   config = NULL;
   if (configfile)
      if (!(config = xml_tree_read_file (configfile)) && !(config = xml_tree_read_file_json (configfile)))
      {
         if (debug)
            warnx ("Config read failed");
         return dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Cannot read %s", configfile);
      }
   if (!config)
      return dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "No config");
   if (debug)
      warnx ("Config parsed");
   xml_t x = NULL;
   {
      int g = 0;
      for (g = 0; g < MAX_GROUP; g++)
      {                         // Defaults in cased not named, etc.
         group[g].time_set = 10;
         group[g].time_fail = 120;
         group[g].bell_time = 300;
         group[g].bell_rest = 3600;
      }
      g = 0;
      while ((x = xml_element_next_by_name (config, x, "group")))
      {
         if (xml_get (x, "@id"))
            g = atoi (xml_get (x, "@id"));
         if (g < 0 || g >= MAX_GROUP)
            continue;
         groups |= (1 << g);
         group[g].name = xml_copy (x, "@name");
         if ((v = xml_get (x, "@time-set")))
            group[g].time_set = atoi (v);
         if ((v = xml_get (x, "@time-fail")))
            group[g].time_fail = atoi (v);
         if ((v = xml_get (x, "@entry-time")))
            group[g].entry_time = atoi (v);
         if ((v = xml_get (x, "@bell-delay")))
            group[g].bell_delay = atoi (v);
         if ((v = xml_get (x, "@bell-time")))
            group[g].bell_time = atoi (v);
         if ((v = xml_get (x, "@bell-rest")))
            group[g].bell_rest = atoi (v);
         group[g].setifany = group_parse (xml_get (x, "@set-if-any"));
         group[g].setifall = group_parse (xml_get (x, "@set-if-all"));
         g++;
      }
      for (g = 0; g < MAX_GROUP; g++)
      {                         // Compound times
         group[g].bell_delay += group[g].entry_time;
         group[g].bell_time += group[g].bell_delay;
         group[g].bell_rest += group[g].bell_time;
      }
   }
   if (debug)
      warnx ("Config check devices");
   while ((x = xml_element_next_by_name (config, x, "device")))
   {
      if (!(pl = xml_get (x, "@id")) || !*pl)
         dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Device with no id");
      else
         while (pl)
         {
            port_p p = port_parse (pl, &pl, -1);
            if (!p)
               continue;
            port_app_t *app = port_app (p);
            if (app->config)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Device duplicate %s", p->name);
               continue;
            }
            app->config = x;
         }
   }
   if (debug)
      warnx ("Config check inputs");
   while ((x = xml_element_next_by_name (config, x, "input")))
   {                            // Scan inputs, get names
      if (!(pl = xml_get (x, "@id")) || !*pl)
         dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Input with no id");
      else
         while (pl)
         {
            port_p p = port_parse (pl, &pl, 1);
            if (!p)
               continue;
            port_app_t *app = port_app (p);
            if (app->config)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Input duplicate %s", p->name);
               continue;
            }
            app->config = x;
            unsigned int id = port_device (p);
            int n = port_port (p);
            if (!n)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Input with bad port");
               continue;
            }
            n--;
            port_app (p)->a = atoi (xml_get (x, "@a") ? : "");
            port_app (p)->x = atoi (xml_get (x, "@x") ? : "");
            port_app (p)->y = atoi (xml_get (x, "@y") ? : "");
            {
               char *t = xml_get (x, "@t");
               if (t)
                  port_app (p)->t = strdup (t);
            }
            if (port_app (p)->a || port_app (p)->x || port_app (p)->y || port_app (p)->t)
               port_app (p)->onplan = 1;
            p->name = xml_copy (x, "@name");
            // triggers
            int t;
            for (t = 0; t < STATE_TRIGGERS; t++)
               if (t != STATE_INTRUDER)
               {
                  char at[50];
                  {
                     char *p;
                     snprintf (at, sizeof (at), "@%s", state_name[t]);
                     for (p = at; *p; p++)
                        if (*p == '_')
                           *p = '-';
                        else
                           *p = tolower (*p);
                  }
                  port_app (p)->trigger[t] = group_parse (xml_get (x, at));
               }
            port_app (p)->trigger[STATE_ZONE] |= group_parse (xml_get (x, "@intruder"));
            // Special case for NONEXIT which are always intruder anyway
            group_t exit = group_parse (xml_get (x, "@exit"));
            port_app (p)->trigger[STATE_NONEXIT] = port_app (p)->trigger[STATE_ZONE] & ~exit;
            port_app (p)->trigger[STATE_ZONE] |= exit;
            // Special case as entry is treated not as intruder directly
            port_app (p)->trigger[STATE_ZONE] &= ~port_app (p)->trigger[STATE_ENTRY];
            if (device[id].type == TYPE_RIO)
            {                   // Resistance and response
               if ((v = xml_get (x, "@response")))
                  device[id].ri[n].response = atoi (v) / 10;
               if ((v = xml_get (x, "@thresholds")) || (v = xml_get (x, "@preset")))
               {                // resistance thresholds
                  unsigned int q = 0;
                  for (q = 0; q < sizeof (rio_thresholds) / sizeof (*rio_thresholds) && strcasecmp (rio_thresholds[q].name, v);
                       q++);
                  if (q < sizeof (rio_thresholds) / sizeof (*rio_thresholds))
                  {
                     device[id].ri[n].threshold[0] = rio_thresholds[q].tampersc;
                     device[id].ri[n].threshold[1] = rio_thresholds[q].lowres;
                     device[id].ri[n].threshold[2] = rio_thresholds[q].normal;
                     device[id].ri[n].threshold[3] = rio_thresholds[q].highres;
                     device[id].ri[n].threshold[4] = rio_thresholds[q].open;
                  } else
                  {
                     while (isdigit (*v) && q < 5)
                     {
                        int o = 0;
                        while (isdigit (*v))
                           o = o * 10 + *v++ - '0';
                        if (*v == ',')
                           v++;
                        while (isspace (*v))
                           v++;
                        device[id].ri[n].threshold[q++] = o / 100;
                     }
                     if (*v)
                        dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Input with bad threshold value %s", v);
                  }
               }
            }
            if ((v = xml_get (x, "@polarity")) && toupper (*v) == 'N')
            {
               device[id].invert |= (1 << n);
               app->invert = 1;
            }
         }
   }
   if (debug)
      warnx ("Config check RF");
   x = NULL;
   while ((x = xml_element_next_by_name (config, x, "rf-rio")))
   {                            // Scan inputs, get names
      if (!(pl = xml_get (x, "@id")) || !*pl)
         dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "RF RIO with no id");
      else
         while (pl)
         {
            port_p p = port_parse (pl, &pl, 0);
            if (!p)
               continue;
            unsigned int id = port_device (p);
            if (!id)
               dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Bad address for RF RIO");
         }
   }
   if (debug)
      warnx ("Config check outputs");
   x = NULL;
   while ((x = xml_element_next_by_name (config, x, "output")))
   {                            // Scan inputs, get names
      if (!(pl = xml_get (x, "@id")) || !*pl)
         dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Output with no id");
      else
         while (pl)
         {
            port_p p = port_parse (pl, &pl, 0);
            if (!p)
               continue;
            port_app_t *app = port_app (p);
            if (app->config)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Output duplicate %s", p->name);
               continue;
            }
            app->config = x;
            unsigned int id = port_device (p);
            int n = port_port (p);
            if (!n)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Output with bad port");
               continue;
            }
            n--;
            port_app (p)->a = atoi (xml_get (x, "@a") ? : "");
            port_app (p)->x = atoi (xml_get (x, "@x") ? : "");
            port_app (p)->y = atoi (xml_get (x, "@y") ? : "");
            {
               char *t = xml_get (x, "@t");
               if (t)
                  port_app (p)->t = strdup (t);
            }
            if (port_app (p)->a || port_app (p)->x || port_app (p)->y || port_app (p)->t)
               port_app (p)->onplan = 1;
            port_app (p)->type = state_parse (xml_get (x, "@type"));
            p->name = xml_copy (x, "@name");
            port_app (p)->group = group_parse (xml_get (x, "@groups") ? : "*");
            if ((v = xml_get (x, "@polarity")) && toupper (*v) == 'N')
            {
               device[id].invert |= (1 << n);
               app->invert = 1;
            }
         }
   }
   if (debug)
      warnx ("Config check max");
   x = NULL;
   while ((x = xml_element_next_by_name (config, x, "max")))
   {                            // Scan max readers
      if (!(pl = xml_get (x, "@id")) || !*pl)
         dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Max with no id");
      else
      {
         char *from = xml_get (x, "@from");     // Renumber max, e.g. a new one, assumes id is simple max port
         if (from)
         {
            if (maxfrom)
               dolog (ALL_GROUPS, "CONFIG", NULL, pl, "Can only renumber one max at a time");
            maxfrom = from;
            maxto = pl;
         }
         while (pl)
         {
            port_p p = port_parse (pl, &pl, -1);
            if (!p)
               continue;
            port_app_t *app = port_app (p);
            if (app->config)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Max duplicate %s", p->name);
               continue;
            }
            app->config = x;
            if ((v = xml_get (x, "@fob-held")))
               device[port_device (p)].fob_hold = atoi (v) * 10;
         }
      }
   }
   if (debug)
      warnx ("Config check keypad");
   x = NULL;
   while ((x = xml_element_next_by_name (config, x, "keypad")))
   {                            // Scan keypads
      if (!(pl = xml_get (x, "@id")) || !*pl)
         dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Keypad with no id");
      else
         while (pl)
         {
            port_p p = port_parse (pl, &pl, -1);
            if (!p)
               continue;
            port_app_t *app = port_app (p);
            if (app->keypad)
            {
               dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Keypad duplicate %s", p->name);
               continue;
            }
            keypad_t *k = keypad_new (p);
            k->port = p;
            k->name = xml_copy (x, "@name");
            k->groups = group_parse (xml_get (x, "@groups") ? : "*");
            k->group_arm = (group_parse (xml_get (x, "@arm") ? : "*") & k->groups);     // default is all groups covered
            k->group_reset = (group_parse (xml_get (x, "@reset") ? : "") & k->groups);  // default is no groups, i.e. needs login
            k->prox = port_parse (xml_get (x, "@prox"), NULL, 0);
            k->time_logout = atoi (xml_get (x, "@logout") ? : "60");
            k->message = xml_copy (x, "@message");
            if ((v = xml_get (x, "@crossed-zeros")) && !strcasecmp (v, "true"))
               k->k.cross = 1;
            if ((v = xml_get (x, "@silent")) && !strcasecmp (v, "true"))
               k->k.silent = 1;
            if ((v = xml_get (x, "@quiet")) && !strcasecmp (v, "true"))
               k->k.quiet = 1;
            if (!(v = xml_get (x, "@dark")) || strcasecmp (v, "true"))
               k->k.backlight = 1;
         }
   }
   if (debug)
      warnx ("Config check user");
   x = NULL;
   while ((x = xml_element_next_by_name (config, x, "user")))
   {                            // Scan users
      user_t *u = malloc (sizeof (*u));
      memset (u, 0, sizeof (*u));
      u->name = xml_copy (x, "@name");
      u->fullname = xml_copy (x, "@full-name");
      if ((v = xml_get (x, "@pin")))
      {
         u->pin = strtoul (v, NULL, 10);
         if (!u->pin)
            dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with PIN zero");
         else
         {
            user_t *f;
            for (f = users; f && (!f->pin || f->pin != u->pin); f = f->next);
            if (f)
               dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with duplicate PIN", f->name ? : "?");
         }
      }
      if ((v = xml_get (x, "@fob")))
      {                         // FOBs
         unsigned int q = 0;
         while (*v && q < sizeof (u->fob) / sizeof (*u->fob))
         {
            while (*v == '0')
               v++;
            fob_t f = { };
            char *o = f;
            while (isxdigit (*v) && o < f + sizeof (f) - 1)
               *o++ = *v++;
            *o = 0;
            if (!*f)
               dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with zero fob");
            else
            {
               user_t *o;
               for (o = users; o; o = o->next)
               {
                  unsigned int z;
                  for (z = 0; z < sizeof (o->fob) / sizeof (*o->fob) && strcmp (o->fob[z], f); z++);
                  if (z < sizeof (o->fob) / sizeof (*o->fob))
                     break;
               }
               if (o)
                  dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with duplicate fob", f);
               else
                  strcpy (u->fob[q++], f);
            }
            if (*v == ',')
               v++;
            while (*v && isspace (*v))
               v++;
            if (*v && !isdigit (*v))
               break;
         }
         if (*v)
         {
            dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with invalid fob [%s]", v);
            break;
         }
      }
      group_t mask = group_parse (xml_get (x, "@mask") ? : "*");        // Default is all zones
      u->group_open = mask & group_parse (xml_get (x, "@open") ? : "*");        // Default is all zones so all doors
      u->group_arm = mask & group_parse (xml_get (x, "@arm"));  // Default is no zones
      u->group_disarm = mask & group_parse (xml_get (x, "@disarm"));    // Default is no zones
      u->group_reset = mask & group_parse (xml_get (x, "@reset"));
      u->group_prop = mask & group_parse (xml_get (x, "@prop"));
      u->next = users;
      users = u;
   }
   if (debug)
      warnx ("Config check door");
   x = NULL;
   {
      int d = 0;
      for (d = 0; d < MAX_DOOR; d++)
         mydoor[d].airlock = -1;
      d = 0;
      while ((x = xml_element_next_by_name (config, x, "door")))
      {
         if (d >= MAX_DOOR)
         {
            dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Too many doors");
            continue;
         }
         char doorno[8];
         snprintf (doorno, sizeof (doorno), "DOOR%02u", d);
         //if (!xml_get (x, "@id"))
         xml_add (x, "@id", doorno);
         mydoor[d].a = atoi (xml_get (x, "@a") ? : "");
         mydoor[d].x = atoi (xml_get (x, "@x") ? : "");
         mydoor[d].y = atoi (xml_get (x, "@y") ? : "");
         char *t = xml_get (x, "@t");
         if (t)
            mydoor[d].t = strdup (t);
         mydoor[d].group_lock = group_parse (xml_get (x, "@lock") ? : xml_get (x, "@lock") ? : "*");
         mydoor[d].group_fire = group_parse (xml_get (x, "@fire") ? : xml_get (x, "@lock") ? : "*");
         mydoor[d].group_arm = group_parse (xml_get (x, "@arm") ? : xml_get (x, "@lock") ? : "*");
         mydoor[d].group_disarm = group_parse (xml_get (x, "@disarm") ? : xml_get (x, "@lock") ? : "*");
         mydoor[d].name = xml_copy (x, "@name");
         char *doorname = mydoor[d].name ? : doorno;
         const char *max = xml_get (x, "@max");
         if (max)
         {                      // short cut to set based on max reader
            port_p maxport = port_parse (max, NULL, -2);
            if (maxport && !maxport->name)
               maxport->name = mydoor[d].name;
            if (port_device (maxport))
            {
               port_set (mydoor[d].i_fob, max, 0, doorname, "Max");
               port_o_set (door[d].o_led, max, 0, doorname, "Max");
               port_o_set (door[d].mainlock.o_unlock, max, 2, doorname, "Unlock");
               port_i_set (door[d].i_open, max, 1, doorname, "Open");
               port_o_set (door[d].o_beep, max, 1, doorname, "Beep");
               port_exit_set (mydoor[d].i_exit, max, 2, doorname);
            } else
            {                   // WiFi device - differect default port IDs
               port_set (mydoor[d].i_fob, max, 0, doorname, "Reader");
               port_o_set (door[d].mainlock.o_unlock, max, 1, doorname, "Unlock");
               port_exit_set (mydoor[d].i_exit, max, 1, doorname);      // TODO invert?
               port_exit_set (mydoor[d].i_exit, max, 8, doorname);
               port_i_set (door[d].i_open, max, 2, doorname, "Open");
               port_i_set (door[d].mainlock.i_unlock, max, 3, doorname, "Unlock");      // Expect people to wire to read switch
            }
         }
         port_set (mydoor[d].i_fob, xml_get (x, "@fob"), 0, doorname, "Max");
         port_o_set (door[d].o_led, xml_get (x, "@o-led"), 0, doorname, "Max");
         port_o_set (door[d].mainlock.o_unlock, xml_get (x, "@o-unlock"), 0, doorname, "Unlock");
         port_i_set (door[d].mainlock.i_unlock, xml_get (x, "@i-unlock"), 0, doorname, "Unlock");
         port_o_set (door[d].o_beep, xml_get (x, "@o-beep"), 0, doorname, "Beep");
         port_i_set (door[d].i_open, xml_get (x, "@i-open"), 0, doorname, "Open");
         port_o_set (door[d].deadlock.o_unlock, xml_get (x, "@o-undeadlock"), 0, doorname, "UnDeadlock");
         port_i_set (door[d].deadlock.i_unlock, xml_get (x, "@i-undeadlock"), 0, doorname, "UnDeadlock");
         port_exit_set (mydoor[d].i_exit, xml_get (x, "@i-exit"), 0, doorname);
         port_bell_set (mydoor[d].i_bell, xml_get (x, "@i-bell"), 0, doorname);
         port_o_set (mydoor[d].o_bell, xml_get (x, "@o-bell"), 0, doorname, "Bell");
         mydoor[d].time_set = parse_time (xml_get (x, "@time-set") ? : "3", 10, 0) / 10;        // Time set is in whole seconds
         door[d].time_open = parse_time (xml_get (x, "@time-open") ? : "10", 0, 0);
         door[d].time_force = parse_time (xml_get (x, "@time-force") ? : "0", 0, 100);
         door[d].time_prop = parse_time (xml_get (x, "@time-prop") ? : "0", 0, 0);
         door[d].mainlock.time_lock = parse_time (xml_get (x, "@time-lock") ? : "1", 5, 127);
         door[d].mainlock.time_unlock = parse_time (xml_get (x, "@time-unlock") ? : "1", 5, 127);
         door[d].deadlock.time_lock = parse_time (xml_get (x, "@time-deadlock") ? : "3", 5, 127);
         door[d].deadlock.time_unlock = parse_time (xml_get (x, "@time-undeadlock") ? : "1", 5, 127);
         if ((v = xml_get (x, "@open-quiet")) && strcasecmp (v, "false"))
            door[d].open_quiet = 1;
         if (!(v = xml_get (x, "@deadlock")) || !strcasecmp (v, "arm"))
            mydoor[d].lock_set = mydoor[d].lock_arm = 1;
         else if (!strcasecmp (v, "set"))
            mydoor[d].lock_set = 1;
         mydoor[d].lockdown = state_parse (xml_get (x, "@lock-down"));
         door_lock (d);
         d++;
      }
      d = 0;
      while ((x = xml_element_next_by_name (config, x, "door")))
      {
         char doorno[8];
         snprintf (doorno, sizeof (doorno), "DOOR%02u", d);
         if ((v = xml_get (x, "@airlock")))
         {
            int d2;
            for (d2 = 0; d2 < MAX_DOOR && (!mydoor[d2].name || strcasecmp (mydoor[d2].name, v)); d2++);
            if (d2 < MAX_DOOR && isdigit (*v))
            {
               const char *n = v;
               d2 = 0;
               while (isdigit (*n))
                  d2 = d2 * 10 + *n++ - '0';
               if (*n)
                  d2 = MAX_DOOR;
            }
            if (d2 < MAX_DOOR)
               mydoor[d].airlock = d2;
            else
               dolog (mydoor[d].group_lock ? : ALL_GROUPS, "CONFIG", NULL, doorno, "Airlock not found %s", v);
         }
         if (++d == MAX_DOOR)
            break;
      }
   }
   if (debug)
      warnx ("Config check system");
   // System
   if ((x = xml_element_next_by_name (config, NULL, "system")))
   {
      state[STATE_ENGINEERING] = group_parse (xml_get (x, "@engineering"));
      walkthrough = xml_get (x, "@walk-through") ? 1 : 0;
      char *wd = xml_get (x, "@watchdog");
      if (wd && strcasecmp (wd, "false"))
      {
         if (*wd == '/')
            WATCHDOG = strdup (wd);
         else
            WATCHDOG = "/dev/watchdog";
      }
#ifdef	LIBWS
      char *ws = xml_get (x, "@ws-host");
      if (ws)
         wshost = strdup (ws);
      ws = xml_get (x, "@ws-port");
      if (ws)
         wsport = strdup (ws);
      ws = xml_get (x, "@ws-cert-file");
      if (ws)
         wscertfile = strdup (ws);
      ws = xml_get (x, "@ws-key-file");
      if (ws)
         wskeyfile = strdup (ws);
      ws = xml_get (x, "@ws-floorplan");
      if (ws)
         asprintf (&wsfloorplan, "@%s", ws);
#endif
   }
   if (debug)
      warnx ("Config check groups");
   // All groups...
   {
      if (!groups)
         groups = 1;            // Group 0 only
      keypad_t *k;
      for (k = keypad; k; k = k->next)
         k->groups &= groups;
      int d;
      for (d = 0; d < MAX_DOOR; d++)
      {
         mydoor[d].group_lock &= groups;
         mydoor[d].group_fire &= groups;
         mydoor[d].group_arm &= groups;
         mydoor[d].group_disarm &= groups;
      }
      user_t *u;
      for (u = users; u; u = u->next)
      {
         u->group_open &= groups;
         u->group_arm &= groups;
         u->group_disarm &= groups;
         u->group_reset &= groups;
         u->group_prop &= groups;
      }
      int s;
      port_p p;
      for (p = ports; p; p = p->next)
         if (p->port)
         {
            port_p parent = port_new_base (p, 0, 0);
            if (!p->isinput)
            {
               port_app (p)->group &= groups;
               port_app (parent)->group |= port_app (p)->group;
            } else
               for (s = 0; s < STATE_TRIGGERS; s++)
               {
                  port_app (p)->trigger[s] &= groups;
                  port_app (parent)->group |= port_app (p)->trigger[s];
               }
         }
      for (s = 0; s < STATES; s++)
         state[s] &= groups;
   }
   if (debug)
      warnx ("Config check done");
   return NULL;
}

static int
door_locked (int d)
{                               // Should door be locked
   group_t mask = 0;
   if (mydoor[d].lock_arm)
      mask |= state[STATE_ARM];
   if (mydoor[d].lock_set)
      mask |= state[STATE_SET];
   if (mydoor[d].lockdown < STATES)
      mask |= state[mydoor[d].lockdown];
   if (mydoor[d].group_lock & mask)
      return 1;
   return 0;
}

static void
door_state (group_t g)
{                               // Update door locking state after state change
   int d;
   for (d = 0; d < MAX_DOOR; d++)
      if ((mydoor[d].group_lock | mydoor[d].group_fire) & g)
      {
         if (door_locked (d))
            door_deadlock (d);  // No point if deadlocked as will set of intruder alarm
         else if (state[STATE_FIRE] & mydoor[d].group_fire)
            door_open (d);      // Fire alarm
         else
            door_unlock (d);
      }
}

static void
keypad_state (group_t g)
{
   keypad_t *k;
   for (k = keypad; k; k = k->next)
      if (k->groups & g)
         k->when = 0;           // Force display update
   postevent (NULL);
}

#ifdef	LIBWS
static void
ws_port_output_callback (port_p id)
{
   xml_t root = xml_tree_new (NULL);
   output_ws (root, id);
   websocket_send_all (root);
   xml_tree_delete (root);
}
#endif

static void
output_state (group_t g)
{                               // Set outputs after state change
   if (!g)
      return;
   port_p port;
   for (port = ports; port; port = port->next)
      if (port_isoutput (port))
      {
         if ((port_app (port)->group & g) && port_app (port)->type < STATES)
         {
            if (state[port_app (port)->type] & g)
            {
               if (!port->state)
                  port_output (port, 1);
            } else
            {
               if (port->state)
                  port_output (port, 0);
            }
         }
      }
}

// Main state change events
static void
del_state (group_t g, statelist_t * s)
{                               // Delete
   s->groups &= ~g;
   if (s->groups)
      return;                   // Still active
   if (s->next)
      s->next->prev = s->prev;
   *s->prev = s->next;
   if (s->port)
      free (s->port);
   if (s->name)
      free (s->name);
   free (s);
}

#define add_entry(g,p,n)	add_state(g,p,n,STATE_ENTRY)
#define rem_entry(g,p,n)	rem_state(g,p,n,STATE_ENTRY)
#define add_intruder(g,p,n)	add_state(g,p,n,STATE_ZONE)
#define rem_intruder(g,p,n)	rem_state(g,p,n,STATE_ZONE)
#define add_tamper(g,p,n)	add_state(g,p,n,STATE_TAMPER)
#define rem_tamper(g,p,n)	rem_state(g,p,n,STATE_TAMPER)
#define add_fault(g,p,n)	add_state(g,p,n,STATE_FAULT)
#define rem_fault(g,p,n)	rem_state(g,p,n,STATE_FAULT)
#define add_warning(g,p,n)	add_state(g,p,n,STATE_WARNING)
#define rem_warning(g,p,n)	rem_state(g,p,n,STATE_WARNING)
static void
add_state (group_t g, const char *port, const char *name, state_t which)
{                               // alarms=1 means just latches, alarms=2 means alarms
   if (!g)
      return;
   group_t changed = 0,
      logging = 0;
   void addtolist (group_t g, int which)
   {
      statelist_t *s = NULL;
      for (s = statelist; s && (s->type != which || strcmp (s->port ? : "", port ? : "") || strcmp (s->name ? : "", name ? : ""));
           s = s->next);
      if (!s)
      {                         // New
         s = malloc (sizeof (*s));
         memset (s, 0, sizeof (*s));
         s->next = statelist;
         s->prev = &statelist;
         if (statelist)
            statelist->prev = &s->next;
         statelist = s;
         s->port = (port ? strdup (port) : NULL);
         s->name = (name ? strdup (name) : NULL);
         s->type = which;
         s->when = time (0);
      }
      s->groups |= g;
   }
   addtolist (g, which);
   if (which < STATE_LATCHED)
      addtolist (g, which + STATE_TRIGGERS);
   if (which == STATE_ZONE && (state[STATE_SET] & g))
      addtolist (state[STATE_SET] & g, STATE_INTRUDER_LATCH);   // how do we log the ENTRY before alarm?
   if (which == STATE_ENTRY && (state[STATE_ALARM] & g))
      addtolist (state[STATE_ALARM] & g, STATE_INTRUDER_LATCH);
   int n;
   for (n = 0; n < MAX_GROUP && g; n++)
      if (g & (1 << n))
      {
         g &= ~(1 << n);
         group[n].count[which]++;
         if (!(state[which] & (1 << n)))
         {
            state[which] |= (1 << n);
            changed |= (1 << n);
         }
         // Latch
         if (which < STATE_LATCHED)
            state[STATE_TRIGGERS + which] |= (1 << n);
         // Do we log it
         if ((which != STATE_ZONE && which != STATE_ENTRY && which != STATE_NONEXIT) || (state[STATE_SET] & (1 << n)))
            logging |= (1 << n);
         //if (debug) printf ("Group %d %s=%d %s\n", n, state_name[which], group[n].count[which], port ? : "");
      }
   if (logging)
      dolog (logging, state_name[which], NULL, port, "%s", name ? : "");
   state_change (changed);
}

static void
rem_state (group_t g, const char *port, const char *name, int which)
{
   if (!g)
      return;
   statelist_t *s;
   for (s = statelist; s && (s->type != which || strcmp (s->port ? : "", port ? : "") || strcmp (s->name ? : "", name ? : ""));
        s = s->next);
   if (s)
      del_state (g, s);
   group_t changed = 0;
   int n;
   for (n = 0; n < MAX_GROUP && g; n++)
      if (g & (1 << n))
      {
         g &= ~(1 << n);
         if (!--group[n].count[which] && (state[which] & (1 << n)))
         {
            state[which] &= ~(1 << n);
            changed |= (1 << n);
         }
         //if (debug) printf ("Group %d %s=%d %s\n", n, state_name[which], group[n].count[which], port ? : "");
      }
   state_change (changed);
}

static void
state_change (group_t g)
{                               // Post state change update
   int n,
     s;
   if (!g)
      return;
   // Trigger pre alarm for entry intruder input
   group_t alarm = (g & state[STATE_SET] & state[STATE_ENTRY] & ~state[STATE_PREALARM] & ~state[STATE_ALARM]);
   if (alarm)
      for (n = 0; n < MAX_GROUP && alarm; n++)
         if (alarm & (1 << n))
         {
            alarm &= ~(1 << n);
            if (group[n].entry_time)
            {
               state[STATE_PREALARM] |= (1 << n);       // Start entry timer
               group[n].when_alarm = now.tv_sec;
            }
         }
   // Derive OPEN and INTRUDER states
   state[STATE_OPEN] = (state[STATE_OPEN] & ~g) | (g & ~state[STATE_SET] & (state[STATE_ENTRY] | state[STATE_ZONE]));
   state[STATE_INTRUDER] =
      (state[STATE_INTRUDER] & ~g) | (g & state[STATE_SET] & (state[STATE_ZONE] | (~state[STATE_PREALARM] & state[STATE_ENTRY])));
   state[STATE_INTRUDER_LATCH] |= state[STATE_INTRUDER];
   // Trigger full alarm for any trigger apart from entry (handled about) and fault and nonexit (as covered by intruder or entry)
   group_t trigger = 0;
   for (s = 0; s < STATE_TRIGGERS; s++)
      if (s != STATE_FAULT && s != STATE_ENTRY && s != STATE_NONEXIT && s != STATE_WARNING && s < STATE_USER1)
         trigger |= state[s];
   alarm = (g & state[STATE_SET] & trigger & ~state[STATE_ALARM]);
   if (alarm)
      for (n = 0; n < MAX_GROUP && alarm; n++)
         if (alarm & (1 << n))
         {
            alarm &= ~(1 << n);
            state[STATE_PREALARM] &= ~(1 << n);
            state[STATE_ALARM] |= (1 << n);
            state[STATE_STROBE] |= (1 << n);
            group[n].when_alarm = now.tv_sec - group[n].entry_time;
         }
   // Restart bell on new alarm if rested
   alarm = (g & state[STATE_ALARM] & trigger & ~state[STATE_BELL]);
   if (alarm)
      for (n = 0; n < MAX_GROUP && alarm; n++)
         if (alarm & (1 << n))
         {
            alarm &= ~(1 << n);
            if (group[n].when_alarm + group[n].bell_rest < now.tv_sec)
               group[n].when_alarm = now.tv_sec - group[n].entry_time;  // restart to allow bell to ring again
         }
#ifdef	LIBWS
   xml_t root = xml_tree_new (NULL);
#endif
   // Log state changes
   for (s = 0; s < STATES; s++)
      if (s != STATE_ZONE && s != STATE_ENTRY && s != STATE_NONEXIT && s != STATE_OPEN)
         if (state[s] != previous_state[s])
         {
            char type[20];
            group_t c = (state[s] & ~previous_state[s]);
            if (c)
            {
               snprintf (type, sizeof (type), "+%s", state_name[s]);
               dolog (c, type, NULL, NULL, NULL);
#ifdef	LIBWS
               state_ws (root, "*set", s, c);
#endif
            }
            c = (~state[s] & previous_state[s]);
            if (c)
            {
               snprintf (type, sizeof (type), "-%s", state_name[s]);
               dolog (c, type, NULL, NULL, NULL);
#ifdef	LIBWS
               state_ws (root, "*clr", s, c);
#endif
            }
            previous_state[s] = state[s];
            if (s == STATE_SET)
            {
               FILE *f = fopen (setfile, "w");
               if (!f)
                  dolog (groups, "CONFIG", NULL, NULL, "Cannot open %s", setfile);
               else
               {
                  fprintf (f, group_list (state[s]));
                  fclose (f);
               }
            }
         }
#ifdef	LIBWS
   if (xml_element_next (root, NULL))
      websocket_send_all (root);
   xml_tree_delete (root);
#endif
   // Do outputs and stuff resulting from state change
   output_state (g);
   door_state (g);
   keypad_state (g);
}

static void
alarm_timed (group_t g, int t)
{                               // Restart timers
   int n;
   for (n = 0; n < MAX_GROUP; n++)
      if (g & (1 << n))
         group[n].when_set = now.tv_sec + (t ? : group[n].time_set);
}

static group_t
alarm_arm (const char *who, const char *where, group_t mask, int t)
{                               // Arm alarm - return which groups set
   group_t allow = (state[STATE_ARM] | state[STATE_SET]);
   int n;
   while (1)
   {
      group_t was = mask;
      for (n = 0; n < MAX_GROUP; n++)
         if ((group[n].setifany & (allow | mask))
             || (group[n].setifall && group[n].setifall == (group[n].setifall & (allow | mask))))
            mask |= (1 << n);
      if (mask == was)
         break;
   }
   mask &= ~allow;              // Ignore already setting / set
   if (!mask)
      return mask;              // nothing to do
   dolog (mask, "ARM", who, where, "Alarm armed (%d seconds)", t);
   state[STATE_ARM] |= mask;
   for (n = 0; n < MAX_GROUP; n++)
      if (mask & (1 << n))
      {
         group[n].when_fail = now.tv_sec + group[n].time_fail;
         group[n].armed_by = who;
      }
   alarm_timed (mask, t);
   state_change (mask);
   return mask;
}

static void
logarmed (group_t mask, const char *who, const char *where, const char *type, const char *msg)
{
   if (who)
      dolog (mask, type, who, where, "%s", msg);        // log who finally set the alarm
   else
      while (mask)
      {
         int a,
           b;
         for (a = 0; a < MAX_GROUP && !(mask & (1 << a)); a++);
         group_t found = 0;
         for (b = a; b < MAX_GROUP; b++)
            if ((mask & (1 << b)) && group[a].armed_by == group[b].armed_by)
               found |= (1 << b);
         mask &= ~found;
         dolog (found, type, group[a].armed_by, where, "%s", msg);
      }
}

static group_t
alarm_set (const char *who, const char *where, group_t mask)
{                               // Set alarm proper - return which groups set
   mask &= ~state[STATE_SET];   // Already set
   if (!mask)
      return mask;              // nothing to do
   logarmed (mask, who, where, "SET", "Alarm set");
   state[STATE_SET] |= mask;
   state[STATE_UNSET] &= ~mask;
   state[STATE_ARM] &= ~mask;
   state[STATE_INTRUDER_LATCH] &= ~mask;        // reset the intruder state as alarm set again
   state_change (mask);
   keypads_message (mask, "\a-- ALARM SET --");
   return mask;
}

static group_t
alarm_unset (const char *who, const char *where, group_t mask)
{                               // Unset alarm - return which groups unset
   group_t allow = state[STATE_SET] | state[STATE_ARM] | state[STATE_PREALARM] | state[STATE_BELL];
   int n;
   while (1)
   {
      group_t was = mask;
      for (n = 0; n < MAX_GROUP; n++)
         if ((group[n].setifany || group[n].setifall)
             && !((group[n].setifany & allow & ~mask)
                  || (group[n].setifall && group[n].setifall == (group[n].setifall & allow & ~mask))))
            mask |= (1 << n);
      if (mask == was)
         break;
   }
   mask &= allow;
   if (!mask)
      return mask;              // Nothing to do
   group_t unset = (state[STATE_SET] & mask);
   if (unset)
      logarmed (unset, who, where, "UNSET", "Alarm unset");
   if (mask & ~unset)
      logarmed (mask & ~unset, who, where, "CANCEL", "Alarm cancelled");
   state[STATE_UNSET] |= mask;
   state[STATE_ALARM] &= ~mask;
   state[STATE_SET] &= ~mask;
   state[STATE_ARM] &= ~mask;
   state[STATE_PREALARM] &= ~mask;
   state[STATE_BELL] &= ~mask;
   state_change (mask);
   keypads_message (unset, "\a- ALARM UNSET -");
   return mask;
}

static group_t
alarm_failset (const char *who, const char *where, group_t mask)
{                               // Failed set alarm - return which groups unset
   mask &= state[STATE_ARM];
   if (!mask)
      return mask;              // Nothing to do
   logarmed (mask, who, where, "FAILSET", "Alarm set failed");
   state[STATE_UNSET] |= mask;
   state[STATE_ARM] &= ~mask;
   state_change (mask);
   keypads_message (mask, "\a- ALARM SET FAILED -");
   return mask;
}

static group_t
alarm_reset (const char *who, const char *where, group_t mask)
{                               // Reset alarm - return which groups reset
   int n;
   group_t set = state[STATE_STROBE];   // What can be cleared
   for (n = STATE_TRIGGERS; n < STATE_TRIGGERS + STATE_LATCHED; n++)
      set |= state[n];
   mask &= set;
   if (!mask)
      return mask;              // Nothing to do
   dolog (mask, "RESET", who, where, "Reset");
   state[STATE_STROBE] &= ~mask;
   // Clear latched
   for (n = STATE_TRIGGERS; n < STATE_TRIGGERS + STATE_LATCHED; n++)
      state[n] &= ~mask;
   statelist_t *s = statelist;
   while (s)
   {
      statelist_t *n = s->next;
      if ((s->groups & mask) && s->type >= STATE_TRIGGERS && s->type < STATE_TRIGGERS + STATE_LATCHED)
         del_state (mask, s);   // These do not use counters
      s = n;
   }
   state_change (mask);
   keypads_message (mask, "\a- SYSTEM RESET -");
   return mask;
}

static keypad_t *
keypad_new (port_p p)
{
   keypad_t *k = port_app (p)->keypad;
   if (!k)
      for (k = keypad; k && k->port != p; k = k->next);
   if (!k)
   {
      k = malloc (sizeof (*k));
      memset (k, 0, sizeof (*k));
      k->next = keypad;
      //device[port_device (p)].output = 0;
      keypad = k;
   }
   k->port = p;
   port_app (p)->keypad = k;
   keypad_send (k, 1);
   return k;
}

// Logging

typedef struct log_s log_t;
struct log_s
{
   log_t *next;
   time_t when;
   group_t groups;
   char *type;
   char *user;
   char *port;
   char *msg;
};
volatile log_t *logs = NULL,
   **logp = NULL;
pthread_mutex_t logmutex;
int logpipe[2];
static log_t *
next_log (long long usec)
{                               // Get next log
   char x;
   // Check for log waiting
   pthread_mutex_lock (&logmutex);
   log_t *l = (log_t *) logs;
   if (l)
      logs = logs->next;
   pthread_mutex_unlock (&logmutex);
   if (l)
   {                            // an log was waiting
      if (read (logpipe[0], &x, 1) < 0)
         perror ("queue recv");
      return l;
   }
   if (usec < 0)
      return NULL;
   // No log waiting - wait timeout specified
   fd_set readfds;
   FD_ZERO (&readfds);
   FD_SET (logpipe[0], &readfds);
   struct timeval timeout = {
      0
   };
   timeout.tv_sec = usec / 1000000ULL;
   timeout.tv_usec = usec % 1000000ULL;
   int s = select (logpipe[0] + 1, &readfds, NULL, NULL, &timeout);
   if (s <= 0)
      return NULL;              // Nothing waiting in the time
   if (read (logpipe[0], &x, 1) < 0)
      perror ("queue recv");
   // Get the waiting log
   pthread_mutex_lock (&logmutex);
   l = (log_t *) logs;
   if (l)
      logs = logs->next;
   pthread_mutex_unlock (&logmutex);
   return l;
}

static int
checklist (char *l, const char *t)
{
   if (!t && !*l)
      return 0;
   int n = 0;
   while (*l)
   {
      l = strdupa (l);
      char *e = l;
      while (*e && !isspace (*e) && *e != ',')
         e++;
      if (*e)
         *e++ = 0;
      while (*e && isspace (*e))
         e++;
      if (!strcasecmp (l, t))
         return n;
      n++;
      l = e;
   }
   return -1;
}

static void
dologger (CURL * curl, log_t * l)
{
   xml_t system = xml_element_next_by_name (config, NULL, "system");
   char when[20];
   strftime (when, sizeof (when), "%FT%T", localtime (&l->when));
   char groups[MAX_GROUP + 1];
   {
      int n;
      char *o = groups;
      for (n = 0; n < MAX_GROUP; n++)
         if (l->groups & (1 << n))
            *o++ = '0' + n;
         else
            *o++ = '-';
      *o = 0;
   }
   const char *name = NULL;
   if (l->port)
   {
      port_p p;
      if (!strncmp (l->port, "DOOR", 4))
         name = mydoor[atoi (l->port + 4)].name;
      else if ((p = port_parse (l->port, NULL, -2)))
         name = p->name;
   }
   // Syslog (except boring keepalives)
   if (!l->type || strcasecmp (l->type, "KEEPALIVE"))
      syslog (LOG_INFO, "%*s %s %s %s %s %s", MAX_GROUP, groups, l->type ? : "?", l->port ? : "", name ? : "", l->user ? : "",
              l->msg ? : "");
   if (debug)
      printf ("%*s\t%s\t%s\t%s\t%s\t%s\n", MAX_GROUP, groups, l->type ? : "?", l->port ? : "", name ? : "", l->user ? : "",
              l->msg ? : "");
   // Other logging
   xml_t c = NULL;
   while ((c = xml_element_next_by_name (config, c, "log")))
   {                            // What other logging to do
      struct curl_httppost *fi = NULL,
         *li = NULL;
      char *v;
      if ((v = xml_get (c, "@groups")))
      {
         if (!(group_parse (v) & l->groups))
            continue;           // Not in group
      }
      if ((v = xml_get (c, "@type")))
      {                         // Check matching type
         if (checklist (v, l->type) < 0)
            continue;
      }
      if ((v = xml_get (c, "@port")))
      {                         // Check matching ports
         if (checklist (v, l->port) < 0)
            continue;
      }
      if ((v = xml_get (c, "@user")))
      {                         // Check matching users
         if (checklist (v, l->user) < 0)
            continue;
      }
      // What action to take
      v = xml_get (c, "@action");
      if (!v)
         continue;
      // Special cases
      if (!strcasecmp (v, "sms") || !strncasecmp (v, "sms:", 4))
      {                         // sms for user
         char *name = l->user;  // default for just "sms"
         if (v[3])
            name = v + 4;       // use name for "sms:name"
         if (!name || !*name)
            continue;           // No action
         xml_t u = NULL;
         while ((u = xml_element_next_by_name (config, u, "user")))
            if ((v = xml_get (u, "@name")) && !strcasecmp (v, name))
               break;
         if (!u)
            continue;           // No user match
         v = xml_get (u, "@sms");
         if (!v || !*v)
            continue;           // No SMS
      } else if (!strcasecmp (v, "email") || !strncasecmp (v, "email:", 6))
      {                         // email for user
         char *name = l->user;  // default for just "email"
         if (v[5])
            name = v + 6;       // use name for "email:name"
         if (!name || !*name)
            continue;           // No action
         xml_t u = NULL;
         while ((u = xml_element_next_by_name (config, u, "user")))
            if ((v = xml_get (u, "@name")) && !strcasecmp (v, name))
               break;
         if (!u)
            continue;           // No user match
         v = xml_get (u, "@email");
         if (!v || !*v)
            continue;           // No SMS
      }
      // Handle standard actions (web/email/sms)
      if (!strncasecmp (v, "http:", 5) || !strncasecmp (v, "https:", 6))
      {                         // CURL
         FILE *o = fopen ("/dev/null", "w");
         curl_easy_setopt (curl, CURLOPT_WRITEDATA, o);
         char *url = NULL;
         char *q = strrchr (v, '?');
         if (q)
         {                      // GET, add to 
            curl_easy_setopt (curl, CURLOPT_HTTPGET, 1L);
            char *url = NULL;
            size_t urllen = 0;
            FILE *u = open_memstream (&url, &urllen);
            fprintf (u, "%s", v);
            char and = 0;
            if (q[1])
               and = 1;
            void add (const char *tag, const char *value)
            {
               if (!value)
                  return;
               if (and)
                  fprintf (u, "&");
               and = 1;
               char *e = curl_easy_escape (curl, value, strlen (value));
               fprintf (u, "%s=%s", tag, e);
               curl_free (e);
            }
            add ("when", when);
            add ("groups", groups);
            add ("type", l->type);
            add ("userid", l->user);
            add ("port", l->port);
            add ("name", name);
            add ("msg", l->msg);
            fclose (u);
            curl_easy_setopt (curl, CURLOPT_URL, url);
         } else
         {                      // POST
            void add (const char *tag, const char *value)
            {
               if (!value)
                  return;
               curl_formadd (&fi, &li, CURLFORM_PTRNAME, tag, CURLFORM_PTRCONTENTS, value, CURLFORM_END);
            }
            add ("when", when);
            add ("groups", groups);
            add ("type", l->type);
            add ("userid", l->user);
            add ("port", l->port);
            add ("name", name);
            add ("msg", l->msg);
            curl_easy_setopt (curl, CURLOPT_URL, v);
            curl_easy_setopt (curl, CURLOPT_HTTPPOST, fi);
         }
         CURLcode result = curl_easy_perform (curl);
         if (q)
            free (url);
         else
            curl_formfree (fi); // Free post data
         if (result)
         {
            syslog (LOG_INFO, "Log to %s failed\n", v);
            commfailcount++;
         }
         fclose (o);
      }
#ifdef	LIBEMAIL
      else if (strchr (v, '@'))
      {                         // Email
         char email[200];
         char *e = dataformat_email_n (email, sizeof (email), v);
         if (!e)
            syslog (LOG_INFO, "Bad log email %s", v);
         else
         {
            FILE *o = NULL;
            email_t m = email_new (&o);
            email_subject (m, "%s", (l->msg && *l->msg) ? l->msg : l->type);
            email_address (m, "From", xml_get (system, "@email"), xml_get (system, "@name"));
            email_address (m, "To", e, NULL);
            fprintf (o, "Email from alarm system\n\n");
            fprintf (o, "Groups:\t%s\n", groups);
            fprintf (o, "When:\t%s\n", when);
            fprintf (o, "Event:\t%s\n", l->type);
            if (l->user)
               fprintf (o, "User:\t%s\n", l->user);
            if (l->port)
               fprintf (o, "Port:\t%s %s\n", l->port, name ? : "");
            if (l->msg)
               fprintf (o, "Message:\n\n%s\n", l->msg);
            const char *err = email_send (m, 0);
            if (err)
            {
               syslog (LOG_INFO, "Email failed to %s: %s", e, err);
               commfailcount++;
            }
         }
      }
#endif
#ifdef	LIBMQTT
      else if (strchr (v, '/'))
      {
         if (iot)
         {
            v = strdupa (v);
            char *msg = strchr (v, ' ');
            if (msg)
               *msg++ = 0;
            int e = mosquitto_publish (iot, NULL, v, strlen (msg ? : ""), msg, 1, 0);
            if (e)
            {
               syslog (LOG_INFO, "IoT publish to %s failed (%s)", v, mosquitto_strerror (e));
               commfailcount++;
            } else
               syslog (LOG_INFO, "IoT %s %s", v, msg);
         }
      }
#endif
      else
      {
         char tel[40];
         char *n = dataformat_telephone_n (tel, sizeof (tel), v, 0, 0);
         if (n)
         {                      // SMS
            char *u,
             *p;
            if (!system || !(u = xml_get (system, "@sms-user")) || !(p = xml_get (system, "@sms-pass")))
            {
               syslog (LOG_INFO, "No system details for SMS to %s", v);
               commfailcount++;
            } else if (l->groups & ~state[STATE_ENGINEERING])
            {
               // response file
               char *reply = NULL;
               size_t replylen = 0;
               FILE *o = open_memstream (&reply, &replylen);
               curl_easy_setopt (curl, CURLOPT_WRITEDATA, o);
               void add (char *tag, char *value)
               {
                  if (!value)
                     return;
                  curl_formadd (&fi, &li, CURLFORM_PTRNAME, tag, CURLFORM_PTRCONTENTS, value, CURLFORM_END);
               }
               add ("username", u);
               add ("password", p);
               add ("da", n);
               add ("oa", xml_get (system, "@name"));
               char *ud;
               asprintf (&ud, "%s\t%s\n%.*s\n%s\t%s\n%s\n%s", l->type ? : "?", l->msg ? : "", MAX_GROUP, groups, l->port ? : "",
                         name ? : "", l->user ? : "", when);
               add ("ud", ud);
               const char *server = xml_get (system, "@sms-host") ? : "https://sms.aa.net.uk/";
               curl_easy_setopt (curl, CURLOPT_URL, server);
               curl_easy_setopt (curl, CURLOPT_HTTPPOST, fi);
               CURLcode result = curl_easy_perform (curl);
               curl_formfree (fi);      // Free post data
               if (result)
               {
                  syslog (LOG_INFO, "SMS to %s failed to connect to server %s", v, server);
                  commfailcount++;
               }
               free (ud);
               fclose (o);
               if (!strstr (reply, "OK"))
               {
                  syslog (LOG_INFO, "SMS to %s failed to send (%s)", v, reply);
                  commfailcount++;
               }
               if (reply)
                  free (reply);
            }
         }
      }
   }
   if (l->type)
      free (l->type);
   if (l->user)
      free (l->user);
   if (l->port)
      free (l->port);
   if (l->msg)
      free (l->msg);
   free (l);
}

static void *
logger (void *d)
{                               // Processing logs in separate thread
   d = d;                       // Unused
   openlog ("alarm", LOG_CONS | LOG_PID, LOG_USER);
   CURL *curl = curl_easy_init ();
   if (debug)
      curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
   curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);
   curl_easy_setopt (curl, CURLOPT_TIMEOUT, 10L);
   while (1)
   {
      log_t *l = next_log (1000000);
      if (!l)
         continue;
      dologger (curl, l);
   }
   return NULL;
}

static void *
dolog (group_t g, const char *type, const char *user, const char *port, const char *fmt, ...)
{                               // Log a message
   log_t *l = malloc (sizeof (*l));
   if (!l)
   {
      warn ("malloc");
      return NULL;
   }
   memset (l, 0, sizeof (*l));
   char *msg = NULL;
   if (fmt)
   {
      va_list ap;
      va_start (ap, fmt);
      vasprintf (&msg, fmt, ap);
      va_end (ap);
      // If malloc for msg leaves NULL, then fine, we do log with no message
   }
   l->groups = g;
   l->when = time (0);
   if (type)
      l->type = strdup (type);
   if (port)
      l->port = strdup (port);
   if (user)
      l->user = strdup (user);
   if (msg)
      l->msg = msg;
   pthread_mutex_lock (&logmutex);
   if (logs)
      *logp = l;
   else
      logs = l;
   logp = (void *) &l->next;
   pthread_mutex_unlock (&logmutex);
   // Use pipe to signal that event is waiting, but it will get events anyway so non blocking
   char x = 0;
   if (write (logpipe[1], &x, 1) < 0)
      perror ("queue send");
   return NULL;
}

static void
keypads_message (group_t g, const char *msg)
{
   keypad_t *k;
   for (k = keypad; k; k = k->next)
      if (k->groups & g)
      {
         k->msg = msg;
         k->when = 0;
      }
}

static void *
keypad_message (keypad_t * k, char *fmt, ...)
{                               // Simple keypad message display
   if (!k)
      return NULL;
   char *l1 = (char *) k->k.text[0];
   char *l2 = (char *) k->k.text[1];
   k->k.cursor = 0;
   // Format
   char *msg = NULL;
   va_list ap;
   va_start (ap, fmt);
   vasprintf (&msg, fmt, ap);
   va_end (ap);
   char *v = msg;
   if (*v == '\a')
   {
      v++;
      if (*v == '\a')
      {
         v++;
         if (*v == '\a')
         {
            v++;
            k->k.beep[0] = 1;
            k->k.beep[1] = 1;
         } else
         {
            k->k.beep[0] = 10;
            k->k.beep[1] = 10;
         }
      } else
      {
         k->k.beep[0] = 10;
         k->k.beep[1] = 0;
      }
   } else
   {
      k->k.beep[0] = 0;
      k->k.beep[1] = 0;
   }
   char *nl = strchr (v, '\n');
   if (nl)
      *nl++ = 0;
   snprintf (l1, 17, "%-16s", v);
   snprintf (l2, 17, "%-16s", nl ? : "");
   k->when = now.tv_sec + (k->block ? 10 : 3);
   free (msg);
   keypad_send (k, 0);
#ifdef  LIBWS
   xml_t root = xml_tree_new (NULL);
   keypad_ws (root, k);
   websocket_send_all (root);
   xml_tree_delete (root);
#endif
   return NULL;
}

static void *
keypad_login (keypad_t * k, user_t * u, const char *where)
{                               // PIN or fob login
   if (u && k->user != u)
   {
      k->ack = 1;               // Acknowledged
      k->user = u;
      if (k->time_logout && !(state[STATE_ENGINEERING] & u->group_reset))
         k->when_logout = now.tv_sec + k->time_logout;  // No logout in engineering if we can reset
      dolog (k->groups, "LOGIN", u->name, where, "Keypad login");
      if (!alarm_unset (u->name, where, k->groups & u->group_disarm))
         return keypad_message (k, "LOGGED IN\n%s", u->fullname ? : u->name ? : "");
   } else                       // Second login, same as using keyfob twice, arm alarm
      alarm_arm (u->name, where, k->groups & u->group_arm, 0);
   return NULL;
}

static void *
do_keypad_update (keypad_t * k, char key)
{                               // Update keypad display / beep (key non 0 for key press).
   // Called either for a key, or when k->when passed.
   int p;
   char *l1 = (char *) k->k.text[0];
   char *l2 = (char *) k->k.text[1];
   k->when = (now.tv_sec + 60) / 60 * 60;       // Next update default if not set below
   if (k->user && k->when_logout)
   {                            // Auto logout
      if (k->when_logout <= now.tv_sec)
      {
         k->user = 0;
         k->when_logout = 0;
      } else if (key)
         k->when_logout = now.tv_sec + k->time_logout;
   } else if (!k->user && k->when_logout && k->when_logout <= now.tv_sec)
   {                            // PIN timeout
      k->when_logout = 0;
      k->pininput = 0;
   }
   if (k->when_logout && k->when_logout < k->when)
      k->when = k->when_logout;
   if (k->msg)
   {
      const char *msg = k->msg;
      k->msg = NULL;
      return keypad_message (k, "%s", msg);
   }
   if (!k->groups)
   {                            // Not in use at all!
      snprintf (l1, 17, "%-16s", k->message ? : "-- NOT IN USE --");
      snprintf (l2, 17, "%02d:%02d %10s", lnow.tm_hour, lnow.tm_min, port_name (k->port));
      k->block = 1;          // No keys
      return NULL;
   }
   if (key && k->block)
      return keypad_message (k, "Wait");
   int s;
   group_t trigger = 0;
   for (s = 0; s < STATE_LATCHED; s++)
      if (s != STATE_FAULT)
         trigger |= state[s];
   // Status
   if (k->groups & state[STATE_ARM])
   {                            // Arming
      if (key >= '0' && key < '0' + MAX_GROUP)
      {                         // Change groups?
         group_t g = (((int) 1 << (int) (key - '0')) & k->groups);
         if (g)
         {
            if (state[STATE_ARM] & g)
               alarm_unset (k->user ? k->user->name : k->name, port_name (k->port), g);
            else if (!(state[STATE_SET] & g))
               alarm_arm (k->user ? k->user->name : k->name, port_name (k->port), g, 0);
            alarm_timed (state[STATE_ARM] & g, 0);
         }
      } else if (key == '\e')   // Cancel
         alarm_unset (k->user ? k->user->name : k->name, port_name (k->port), k->groups & state[STATE_ARM]);
      else if (key == 'B')      // Part set
      {
         alarm_unset (k->user ? k->user->name : k->name, port_name (k->port), state[STATE_ARM] & ~(k->groups & trigger));
      } else if (key == '\n')
      {                         // Set
         alarm_set (k->user ? k->user->name : k->name, port_name (k->port), k->groups & state[STATE_ARM]);
         return NULL;
      }
      if (!(k->groups & state[STATE_ARM]))
         return keypad_message (k, "CANCELLED SET");    // Nothing left to set
      k->k.cursor = 0;
      if (k->groups & state[STATE_ARM] & trigger)
      {                         // Not setting
         k->k.beep[0] = 0;
         k->k.beep[1] = 1;
      } else
      {                         // Setting
         k->k.beep[0] = 1;
         k->k.beep[1] = 9;
      }
      int n,
        left = 99;
      for (n = 0; n < MAX_GROUP; n++)
         if ((k->groups & state[STATE_ARM] & (1 << n)) && group[n].when_set - now.tv_sec < left)
            left = group[n].when_set - now.tv_sec;
      if (left < 0)
         left = 0;
      statelist_t *l = NULL;
      int alt = (now.tv_sec & 1);
      for (n = 0; n < MAX_GROUP; n++)
         if (!(k->groups & (1 << n)))
            l2[n] = ' ';
         else if (state[STATE_SET] & (1 << n))
            l2[n] = '*';
         else if (state[STATE_ARM] & (1 << n))
         {
            for (s = 0;
                 s < STATE_TRIGGERS && (s == STATE_FAULT || s == STATE_WARNING || s == STATE_ZONE || s >= STATE_USER1
                                        || !(state[s] & (1 << n))); s++);
            if (alt && s < STATE_TRIGGERS)
            {
               if (!l)
                  for (l = statelist; l && ((s != STATE_OPEN && l->type != s) || !(l->groups & (1 << n))); l = l->next);
               l2[n] = (s == STATE_OPEN ? 'X' : *state_name[s]);
            } else
               l2[n] = n + '0';
         } else
            l2[n] = '-';
      for (; n < 16; n++)
         l2[n] = ' ';
      if (l)
         snprintf (l1, 17, "%s %s", l->port, l->name);
      else
         snprintf (l1, 17, "SETTING ALARM %2d", left);
      k->when = now.tv_sec + 1;
      return NULL;
   }
   k->block = 0;
   // PIN entry?
   if (k->pininput || isdigit (key))
   {
      if (!isdigit (key))
      {                         // End of PIN entry, do we login?
         k->pininput = 0;
         user_t *u = NULL;
         if (k->pin)
            for (u = users; u && u->pin != k->pin; u = u->next);
         if (!u)
         {                      // PIN 0 or user not valid
            k->block = 1;
            return keypad_message (k, "INVALID CODE");
         }
         keypad_login (k, u, port_name (k->port));
         if (key == '\n')
            return NULL;
      } else
      {                         // Digit
         if (k->pininput < 9)
         {
            if (!k->pininput++)
               k->pin = 0;      // Start
            k->pin = k->pin * 10 + key - '0';
         }
         for (p = 0; p < k->pininput; p++)
            l1[p] = '*';
         k->k.cursor = 0x40 + p;
         for (; p < 16; p++)
            l1[p] = ' ';
         for (p = 0; p < 16; p++)
            l2[p] = ' ';
         k->when = k->when_logout = now.tv_sec + 10;    // Timeout
         return NULL;
      }
   }
   // Other keys
   if (key == '\e')
   {                            // ESC - logout
      k->ack = 1;               // Acknowledged - stop beeping
      if (k->user)
      {                         // Logout
         k->user = NULL;
         return keypad_message (k, "LOGGED OUT");
      }
      // No action for not logged in
   }
   if (key == '\n')
   {                            // ENT - reset
      k->ack = 1;               // Acknowledged - stop beeping
      if (k->user)
      {
         if (!alarm_reset (k->user->name, port_name (k->port), k->user->group_reset))
            return keypad_message (k, "CANNOT RESET!"); // Allow wider reset than keypad
      } else
      {
         if (!alarm_reset (k->name, port_name (k->port), k->group_reset))
            return keypad_message (k, "CANNOT RESET!");
      }
   }
   if (key == 'A')
   {                            // A - arm timed
      if (k->user)
      {
         if (!alarm_arm (k->user->name, port_name (k->port), k->groups & k->user->group_arm, 0))
            return keypad_message (k, "CANNOT SET");    // Only alarm what keypad allows
      } else
      {
         if (!alarm_arm (k->name, port_name (k->port), k->group_arm, 0))
            return keypad_message (k, "CANNOT SET");
      }
   }
   if (key == 'B')
   {                            // B - arm instant
      if (k->user)
      {
         if (!alarm_arm (k->user->name, port_name (k->port), k->groups & k->user->group_arm, 1))
            return keypad_message (k, "CANNOT SET");    // Only alarm what keypad allows
      } else
      {
         if (!alarm_arm (k->name, port_name (k->port), k->group_arm, 1))
            return keypad_message (k, "CANNOT SET");
      }
   }
   // Status display
   const char *alert = NULL;
   if (k->groups & state[STATE_FIRE])
      alert = "\a\aFIRE ALARM!";
   else if (k->groups & state[STATE_PREALARM])
      alert = "\a\a\aENTRY ALARM!";
   else if (k->groups & state[STATE_ALARM])
      alert = "\a\aINTRUDER!";
   else if (k->groups & state[STATE_TAMPER] & ~state[STATE_ENGINEERING])
      alert = "\a\aTAMPER!";
   else if (k->groups & state[STATE_BELL])
      alert = "\a\aALARM!";
   else if (k->groups & state[STATE_FAULT] & ~state[STATE_ENGINEERING])
      alert = "\aFAULT!";
   else if (k->groups & state[STATE_WARNING])
      alert = "\aWARNING!";
   else if (k->user && k->groups & state[STATE_ENGINEERING])
      alert = "ENGINEERING MODE";
   if (alert && *alert == '\a')
      k->k.blink = 1;           // Blink anyway, even if this is acked
   else
      k->k.blink = 0;
   k->k.cursor = 0;
   if (k->alert != alert)
   {                            // New alert or end of alert
      k->alert = alert;
      k->ack = 0;               // Allow beeping again
   }
   if (k->ack && alert)
      while (*alert == '\a')
         alert++;               // Silence
   if (k->user)
   {                            // User logged in
      k->k.beep[0] = 0;
      k->k.beep[1] = 0;
      if (k->when_posn != now.tv_sec || k->posn < 0)
      {
         k->when_posn = now.tv_sec;
         k->posn++;
      }
      int n = k->posn;
      int t;
      for (t = 0; t <= STATE_WARNING; t++)
         if (k->groups & state[t])
         {                      // Active
            k->when = now.tv_sec + 1;
            statelist_t *s;
            for (s = statelist; s && (!(s->groups & k->groups) || s->type != t || n--); s = s->next);
            if (!s)
               continue;
            snprintf (l1, 17, "%-7s %-8s", s->port, s->name ? : "");
            if (t == STATE_FAULT || t == STATE_TAMPER)
            {
               port_p p = port_parse (s->port, NULL, -2);
               if (port_bits (p) && device[port_device (p)].type == TYPE_RIO)
               {
                  unsigned int v = device[port_device (p)].ri[port_port (p) - 1].resistance;
                  if (v)
                  {
                     if (v == 65535)
                        snprintf (l2, 17, "%-6s %-9s", "OPEN", state_name[t]);
                     else
                        snprintf (l2, 17, "%5u\xF4 %-9s", v, state_name[t]);
                     return NULL;
                  }
               }
            }
            snprintf (l2, 17, "ACTIVE %-9s", state_name[t]);
            return NULL;
         }
      for (t = 0; t < STATE_LATCHED; t++)
         if (k->groups & state[STATE_TRIGGERS + t] & ~state[t])
         {                      // Latched
            k->when = now.tv_sec + 1;
            statelist_t *s;
            for (s = statelist; s && (!(s->groups & k->groups) || s->type != STATE_TRIGGERS + t || n--); s = s->next);
            if (!s)
               continue;
            struct tm *l = localtime (&s->when);
            snprintf (l1, 17, "%-7s %-8s", s->port, s->name ? : "");
            snprintf (l2, 17, "%02d:%02d %-10s", l->tm_hour, l->tm_min, state_name[t]);
            return NULL;
         }
      k->posn = -1;
      snprintf (l1, 17, "%-16s", k->user->fullname ? : k->user->name ? : "Logged in");
      if (alert)
         snprintf (l2, 17, "%-16s", alert);
      else
         snprintf (l2, 17, "%02d:%02d %10s", lnow.tm_hour, lnow.tm_min, port_name (k->port));
      return NULL;
   }
   if (alert)
      return keypad_message (k, "%s", alert);
   // Not logged in
   snprintf (l1, 17, "%-16s", k->message ? : "SolarSystem");
   {                            // Reset required??
      int s;
      for (s = 0; s < STATE_LATCHED && !state[STATE_TRIGGERS + s]; s++);
      if (s < STATE_LATCHED)
      {                         // Something needs resetting
         snprintf (l2, 17, "RESET %-10s", state_name[s]);
         if (k->ack || (state[STATE_ENGINEERING] & state[STATE_TRIGGERS + s]))
         {                      // No beep, just blink
            k->k.beep[0] = 0;
            k->k.beep[1] = 0;
         } else
         {                      // Beep
            k->k.beep[0] = 1;
            k->k.beep[1] = 49;
         }
         return NULL;
      }
   }
   snprintf (l2, 17, "%04d-%02d-%02d %02d:%02d", lnow.tm_year + 1900, lnow.tm_mon + 1, lnow.tm_mday, lnow.tm_hour, lnow.tm_min);
   k->k.beep[0] = 0;
   k->k.beep[1] = 0;
   return NULL;
}

static void *
keypad_update (keypad_t * k, char key)
{                               // Do keypad update, possibly with a key pressed
   void *ret = do_keypad_update (k, key);
   keypad_send (k, 0);
#ifdef	LIBWS
   xml_t root = xml_tree_new (NULL);
   xml_t x = keypad_ws (root, k);
   if (key == '\e')
      xml_add (x, "@key", "esc");
   else if (key == '\n')
      xml_add (x, "@key", "ent");
   else if (key)
      xml_addf (x, "@key", "%c", key);
   websocket_send_all (root);
   xml_tree_delete (root);
#endif
   return ret;
}

void
doevent (event_t * e)
{                               // Handle an event
   gettimeofday (&now, NULL);
   unsigned int id = port_device (e->port);
   unsigned char type = 0;
   if (id > 0 && id < MAX_DEVICE)
      type = device[id].type;
   if (debug)
   {                            // Debug logging
      if (e->event == EVENT_DOOR)
         printf ("DOOR%02d %s", e->door, door_name[e->state]);
      else if (e->event == EVENT_KEEPALIVE)
         printf ("BUS%d %s ", (port_device (e->port) >> 8) + 1, event_name[e->event]);
      else
         printf ("%s %s ", port_name (e->port), event_name[e->event]);
      if (e->event == EVENT_KEEPALIVE)
         printf ("%5d %5d %5d %5d %5d", e->tx, e->rx, e->errors, e->stalled, e->retries);
      if (e->event == EVENT_INPUT || e->event == EVENT_TAMPER || e->event == EVENT_FAULT)
         printf ("%d", e->state);
      if (e->event == EVENT_KEY)
         printf ("%02X", e->key);
      if (e->event == EVENT_FOB || e->event == EVENT_FOB_HELD)
         printf (" %s", e->fob);
      if (e->event == EVENT_RF)
         printf ("%08X %08X %02X %2d/10", e->rfserial, e->rfstatus, e->rftype, e->rfsignal);
      printf ("\n");
   }
   // Simple sanity checks
   if (e->event == EVENT_DOOR && e->door >= MAX_DOOR)
   {
      if (debug)
         printf ("Bad door %d\n", e->door);
      return;
   }
   port_p port = e->port;
   port_app_t *app = port_app (port);
   // Handle event
   switch (e->event)
   {
   case EVENT_KEEPALIVE:
      {
         int n = (id >> 8) + 1;
         char busno[10];
         snprintf (busno, sizeof (busno), "BUS%d", n);
         dolog (groups, "KEEPALIVE", NULL, busno, "Keepalive");
         if (e->tx)
            mybus[n - 1].watchdog = now.tv_sec + 150;
         if (e->errors > 5 || e->stalled > 1 || !e->rx || !e->tx)
         {
            if (!mybus[n].fault)
            {
               mybus[n].fault = 1;
               add_fault (groups, busno, NULL);
            }
            dolog (groups, "BUSERROR", NULL, busno, "Bus reports errors:%d stalled:%d retries:%d tx:%d rx:%d", e->errors,
                   e->stalled, e->retries, e->tx, e->rx);
         } else
         {
            if (mybus[n].fault)
            {
               mybus[n].fault = 0;
               rem_fault (groups, busno, NULL);
            }
            if (e->retries > 5)
               syslog (LOG_INFO, "%s retries: %d", busno, e->retries);
         }
      }
      break;
   case EVENT_CONFIG:
      dolog (groups, "BUSCONFIG", NULL, port_name (port), "Device config started");
      break;
   case EVENT_FOUND:
      {
         dolog (groups, "BUSFOUND", NULL, port_name (port), "Device found on bus");
         if (app && app->missing)
         {
            app->missing = 0;
            rem_tamper (groups, port_name (port), port->name);
         }
         if (type == TYPE_PAD || app->keypad)
            keypad_new (port);
      }
      break;
   case EVENT_MISSING:
      if (app && !app->missing)
      {
         app->missing = 1;
         dolog (groups, "BUSMISSING", NULL, port_name (port), "Device missing from bus");
         add_tamper (groups, port_name (port), port->name);
      }
      break;
   case EVENT_DISABLED:
      {
         dolog (groups, "BUSDISABLED", NULL, port_name (port), "Device disabled on bus");
      }
      break;
   case EVENT_DOOR:
      {
         mydoor_t *d = &mydoor[e->door];
         char doorno[8];
         snprintf (doorno, sizeof (doorno), "DOOR%02u", e->door);
         char *doorname = d->name;
         if (d->group_fire & state[STATE_FIRE])
            door_open (e->door);        // fire alarm override
         // Log some states
         if (e->state == DOOR_CLOSED)
            d->opening = 1;
         else if (e->state == DOOR_OPEN)
            d->opening = 0;
         else if (e->state == DOOR_LOCKING && d->opening)
            dolog (d->group_lock, "DOORNOTOPEN", NULL, doorno, "Door was not opened");
         else if (e->state == DOOR_AJAR)
            dolog (d->group_lock, "DOORAJAR", NULL, doorno, "Door ajar (lock not engaged)");
         else if (e->state == DOOR_FORCED)
            dolog (d->group_lock, "DOORFORCED", NULL, doorno, "Door forced");
         else if (e->state == DOOR_TAMPER)
            dolog (d->group_lock, "DOORTAMPER", NULL, doorno, "Door tamper");
         else if (e->state == DOOR_FAULT)
            dolog (d->group_lock, "DOORFAULT", NULL, doorno, "Door fault");
         else if (e->state == DOOR_PROPPED)
            dolog (d->group_lock, "DOORPROPPED", NULL, doorno, "Door propped");
         else if (e->state == DOOR_PROPPEDOK)
            dolog (d->group_lock, "DOORPROPPEDOK", NULL, doorno, "Door prop authorised");
         // Update alarm state linked to doors
         // Entry
         if (e->state == DOOR_OPEN)
         {
            if (!d->entry)
            {
               d->entry = 1;
               add_entry (d->group_lock, doorno, doorname);
            }
         } else
         {
            if (d->entry)
            {
               d->entry = 0;
               rem_entry (d->group_lock, doorno, doorname);
            }
         }
         // Intruder
         if (e->state == DOOR_FORCED || e->state == DOOR_PROPPED)
         {
            if (!d->intruder)
            {
               d->intruder = 1;
               add_intruder (d->group_lock, doorno, doorname);
            }
         } else
         {
            if (d->intruder)
            {
               d->intruder = 0;
               rem_intruder (d->group_lock, doorno, doorname);
            }
         }
         // Tamper
         if (e->state == DOOR_TAMPER)
         {
            if (!d->tamper)
            {
               d->tamper = 1;
               add_tamper (d->group_lock, doorno, doorname);
            }
         } else
         {
            if (d->tamper)
            {
               d->tamper = 0;
               rem_tamper (d->group_lock, doorno, doorname);
            }
         }
         // Warning
         if (e->state == DOOR_AJAR || e->state == DOOR_PROPPED)
         {
            if (!d->warning)
            {
               d->warning = 1;
               add_warning (d->group_lock, doorno, doorname);
            }
         } else
         {
            if (d->warning)
            {
               d->warning = 0;
               rem_warning (d->group_lock, doorno, doorname);
            }
         }
         // Fault
         if (e->state == DOOR_FORCED)
         {
            if (!d->fault)
            {
               d->fault = 1;
               add_fault (d->group_lock, doorno, doorname);
            }
         } else
         {
            if (d->fault)
            {
               d->fault = 0;
               rem_fault (d->group_lock, doorno, doorname);
            }
         }
#ifdef	LIBWS
         xml_t root = xml_tree_new (NULL);
         if (door_ws (root, e->door))
            websocket_send_all (root);
         xml_tree_delete (root);
#endif
      }
      break;
   case EVENT_INPUT:
      {
         if (!app || !port_isinput (port))
            break;
         int s;
         const char *tag = port_name (port);
         const char *name = port->name ? : tag;
         if (e->state)
         {                      // on
            app->input = 1;
            if (walkthrough)
               syslog (LOG_INFO, "+%s(%s)", tag, name ? : "");
            for (s = 0; s < STATE_TRIGGERS; s++)
               add_state (app->trigger[s], tag, name, s);
         } else
         {                      // off
            app->input = 0;
            if (walkthrough)
               syslog (LOG_INFO, "-%s(%s)", tag, name ? : "");
            for (s = 0; s < STATE_TRIGGERS; s++)
               rem_state (app->trigger[s], tag, name, s);
         }
         if (app->isexit && e->state)
         {
            unsigned int d,
              n;
            for (d = 0; d < MAX_DOOR; d++)
               for (n = 0; n < sizeof (mydoor[d].i_exit) / sizeof (*mydoor[d].i_exit); n++)
                  if (port_device (mydoor[d].i_exit[n]) == id)
                  {
                     char doorno[8];
                     snprintf (doorno, sizeof (doorno), "DOOR%02u", d);
                     if (!door_locked (d))
                     {
                        if (mydoor[d].airlock >= 0 && door[mydoor[d].airlock].state != DOOR_LOCKED
                            && door[mydoor[d].airlock].state != DOOR_DEADLOCKED)
                        {
                           dolog (mydoor[d].group_lock, "DOORAIRLOCK", NULL, doorno,
                                  "Airlock violation with DOOR%02d, exit rejected", mydoor[d].airlock);
                           door_error (d);
                        } else if (mydoor[d].lockdown && (state[mydoor[d].lockdown] & mydoor[d].group_lock))
                        {       // Door in lockdown
                           dolog (mydoor[d].group_lock, "DOORLOCKDOWN", NULL, doorno, "Lockdown violation, exit rejected");
                           door_error (d);
                        } else
                           door_open (d);
                     } else
                     {
                        dolog (mydoor[d].group_lock, "DOORREJECT", NULL, doorno, "Door is deadlocked, exit rejected");
                        door_error (d);
                     }
                  }
         }
#ifdef	LIBWS
         xml_t root = xml_tree_new (NULL);
         input_ws (root, port);
         websocket_send_all (root);
         xml_tree_delete (root);
#endif
      }
      break;
   case EVENT_TAMPER:
      {
         if (!app)
            break;
         const char *tag = port_name (port);
         const char *name = port->name ? : tag;
         group_t g = 0;
         int s;
         for (s = 0; s < STATE_TRIGGERS; s++)
            g |= app->trigger[s];
         if (e->state)
         {
            app->tamper = 1;
            if (walkthrough)
               syslog (LOG_INFO, "+%s(%s) Tamper", tag, name ? : "");
            add_tamper (g, tag, name);
         } else
         {
            app->tamper = 0;
            if (walkthrough)
               syslog (LOG_INFO, "-%s(%s) Tamper", tag, name ? : "");
            rem_tamper (g, tag, name);
         }
#ifdef	LIBWS
         xml_t root = xml_tree_new (NULL);
         input_ws (root, port);
         websocket_send_all (root);
         xml_tree_delete (root);
#endif
      }
      break;
   case EVENT_FAULT:
      {
         if (!app)
            break;
         const char *tag = port_name (port);
         const char *name = port->name ? : tag;
         group_t g = 0;
         int s;
         for (s = 0; s < STATE_TRIGGERS; s++)
            g |= app->trigger[s];
         if (e->state)
         {
            app->fault = 1;
            if (walkthrough)
               syslog (LOG_INFO, "+%s(%s) Fault", tag, name ? : "");
            add_fault (g, tag, name);
         } else
         {
            app->fault = 0;
            if (walkthrough)
               syslog (LOG_INFO, "-%s(%s) Fault", tag, name ? : "");
            rem_fault (g, tag, name);
         }
         if (id && device[id].type == TYPE_RIO)
         {
            int i = port_port (port) - 1;
            if (i == FAULT_RIO_NO_PWR)
            {
               char tag[20];
               snprintf (tag, sizeof (tag), "%sNOPWR", port_name (port));
               if (e->state)
                  add_warning (g, tag, name);
               else
                  rem_warning (g, tag, name);
            }
            if (i == FAULT_RIO_NO_BAT)
            {
               char tag[20];
               snprintf (tag, sizeof (tag), "%sNOBAT", port_name (port));
               if (e->state)
                  add_warning (g, tag, name);
               else
                  rem_warning (g, tag, name);
            }
            if (i == FAULT_RIO_BAD_BAT)
            {
               char tag[20];
               snprintf (tag, sizeof (tag), "%sBADBAT", port_name (port));
               if (e->state)
                  add_warning (g, tag, name);
               else
                  rem_warning (g, tag, name);
            }
         }
#ifdef	LIBWS
         xml_t root = xml_tree_new (NULL);
         input_ws (root, port);
         websocket_send_all (root);
         xml_tree_delete (root);
#endif
      }
      break;
   case EVENT_FOB:
   case EVENT_FOB_HELD:
      {                         // Check users, doors?
         unsigned int d,
           n;
         user_t *u = NULL;
         if (!e->fob || !*e->fob)
            break;
         for (u = users; u; u = u->next)
         {
            for (n = 0; n < sizeof (u->fob) / sizeof (*u->fob) && strcmp (u->fob[n], (char *) e->fob); n++);
            if (n < sizeof (u->fob) / sizeof (*u->fob))
               break;
         }
         if (id && device[id].pad)
         {                      // Prox for keypad, so somewhat different
            if (!u)
               dolog (groups, "FOBBAD", NULL, port_name (port), "Unrecognised fob %s", e->fob);
            else
            {
               keypad_t *k;
               for (k = keypad; k && k->prox != port; k = k->next);
               if (k)
                  keypad_login (k, u, port_name (port));
               else
                  dolog (groups, "FOBBAD", NULL, port_name (port), "Prox not linked to keypad, fob %s", e->fob);
            }
            return;
         }
         if (app->keypad)
            keypad_login (app->keypad, u, port_name (port));
         int found = 0;
         // We only do stuff for Max readers on doors - maybe we need some logic for stand alone max readers - or make a dummy door.
         for (d = 0; d < MAX_DOOR; d++)
            for (n = 0; n < sizeof (mydoor[0].i_fob) / sizeof (*mydoor[0].i_fob); n++)
               if (mydoor[d].i_fob[n] == port)
               {
                  found++;
                  char doorno[8];
                  snprintf (doorno, sizeof (doorno), "DOOR%02u", d);
                  if (!u)
                  {
                     door_error (d);
                     door_lock (d);     // Cancel open
                     dolog (mydoor[d].group_lock, "FOBBAD", NULL, doorno, "Unrecognised fob %s", e->fob);
                  } else if (e->event == EVENT_FOB)
                  {
                     // disarm is the groups that can be disarmed by this user on this door.
                     group_t disarm = ((u->group_arm & mydoor[d].group_arm & state[STATE_ARM]) | (port_name (port),
                                                                                                  u->group_disarm &
                                                                                                  mydoor[d].group_disarm &
                                                                                                  state[STATE_SET]));
                     if (door[d].state == DOOR_PROPPED || door[d].state == DOOR_OPEN || door[d].state == DOOR_PROPPEDOK)
                     {
                        if (disarm && alarm_unset (u->name, port_name (port), disarm))
                           door_confirm (d);
                        if (u->group_prop & mydoor[d].group_lock)
                        {
                           door_auth (d);
                           if (door[d].state != DOOR_PROPPEDOK)
                           {
                              dolog (mydoor[d].group_lock, "DOORHELD", u->name, doorno, "Door prop authorised by fob %s", e->fob);
                              door_confirm (d);
                           }
                        } else
                        {
                           dolog (mydoor[d].group_lock, "DOORSTILLPROPPED", u->name, doorno,
                                  "Door prop not authorised by fob %s as not allowed", e->fob);
                           door_error (d);
                        }
                     } else if (door[d].state == DOOR_CLOSED)
                     {
                        if (disarm && alarm_unset (u->name, port_name (port), disarm))
                           door_confirm (d);
                        door_lock (d);  // Cancel open
                        dolog (mydoor[d].group_lock, "DOORCANCEL", u->name, doorno, "Door open cancelled by fob %s", e->fob);
                        mydoor[d].opening = 0;  // Don't report not opened
                     } else
                     {
                        if (u->group_open & mydoor[d].group_lock)
                        {
                           if (mydoor[d].airlock >= 0 && door[mydoor[d].airlock].state != DOOR_LOCKED
                               && door[mydoor[d].airlock].state != DOOR_DEADLOCKED)
                           {
                              dolog (mydoor[d].group_lock, "DOORAIRLOCK", u->name, doorno,
                                     "Airlock violation with DOOR%02d using fob %s", mydoor[d].airlock, e->fob);
                              door_error (d);
                           } else if (mydoor[d].lockdown && (state[mydoor[d].lockdown] & mydoor[d].group_lock))
                           {    // Door in lockdown
                              dolog (mydoor[d].group_lock, "DOORLOCKDOWN", u->name, doorno,
                                     "Lockdown violation with DOOR%02d using fob %s", mydoor[d].airlock, e->fob);
                              door_error (d);
                           } else if (mydoor[d].group_lock & ((state[STATE_SET] | state[STATE_ARM]) & ~disarm))
                           {
                              dolog (mydoor[d].group_lock, "DOORALARMED", u->name, doorno,
                                     "Door is alarmed, not opening DOOR%02d using fob %s", d, e->fob);
                              door_error (d);
                           } else
                           {    // Allowed to be opened
                              if (disarm && alarm_unset (u->name, port_name (port), disarm))
                                 door_confirm (d);
                              if (door[d].state != DOOR_OPEN && door[d].state != DOOR_UNLOCKING)
                              { // Open it
                                 dolog (mydoor[d].group_lock, "DOOROPEN", u->name, doorno, "Door open by fob %s", e->fob);
                                 door_open (d); // Open the door
                              } else if (door[d].state == DOOR_OPEN)
                                 dolog (mydoor[d].group_lock, "FOBIGNORED", u->name, doorno, "Ignored fob %s as door open", e->fob);
                           }
                           // Other cases (unlocking) are transient and max will sometimes multiple read
                        } else
                        {
                           door_error (d);
                           dolog (mydoor[d].group_lock, "FOBBAD", u->name, doorno, "Not allowed fob %s", e->fob);
                        }
                     }
                  } else if (mydoor[d].time_set)
                  {             // Held and we are allowed to set
                     group_t set = (mydoor[d].group_arm & u->group_arm & ~state[STATE_SET] & ~state[STATE_ARM]);
                     if (set)
                     {
                        door_confirm (d);
                        door_lock (d);
                        alarm_arm (u->name, port_name (port), set, mydoor[d].time_set);
                     } else
                     {
                        dolog (mydoor[d].group_lock, "FOBHELDIGNORED", u->name, doorno,
                               "Ignored held fob %s as no setting options", e->fob);
                        door_error (d);
                     }
                  } else
                  {
                     dolog (mydoor[d].group_lock, "FOBHELDIGNORED", u->name, doorno,
                            "Ignored held fob %s as door cannot set alarm", e->fob);
                     door_error (d);
                  }
               }
         if (!found)
         {                      // Unassociated max reader
            dolog (groups, e->event == EVENT_FOB_HELD ? "FOBHELDIGNORE" : "FOBIGNORED", u ? u->name : NULL, port_name (port),
                   "Ignored fob %s as reader not linked to a door", e->fob);
         }
      }
      break;
   case EVENT_KEY:
      {                         // Key
         keypad_t *k;
         for (k = keypad; k && k->port != port; k = k->next);
         if (k)
            keypad_update (k, e->key);
      }
      break;
   case EVENT_RF:
      {
         // Meh, one day
      }
      break;
   }
   free ((void *) e);
}

static void
profile_check (void)
{                               // Update profiles.
   int changed = 0;
   xml_t p = NULL;
   while ((p = xml_element_next_by_name (config, p, "profile")))
   {                            // Scan profiles
      // TODO check dates, times, days of weeks, etc
   }
   if (changed)
   {                            // Apply profiles where needed
      // TODO allow inputs to have profiles, and other things like auto setting alarms, or reporting alarm is not set when it should be
   }
}

#ifdef	LIBWS
static char *
do_wscallback (websocket_t * w, xml_t head, xml_t data)
{
   // TODO a better way than Basic http auth would be good some time
   xml_t http = xml_find (head, "http");
   char apath[SHA_DIGEST_LENGTH * 2 + 1];
   char *authpath (char *user, char *pass)
   {                            // return an authorisation path - this is used once logged in, and allows websocket to use the same path to then authenticate as no Authorization gets to wwebsocket it seems
      char *ip;
      if (!head || !(ip = xml_get (head, "@IP")))
         return NULL;
      unsigned char hash[SHA_DIGEST_LENGTH];
      time_t t = time (0);
      char today[11];
      strftime (today, sizeof (today), "%F", localtime (&t));
      SHA_CTX c;
      SHA1_Init (&c);
      SHA1_Update (&c, user, strlen (user));
      SHA1_Update (&c, "/", 1);
      SHA1_Update (&c, pass, strlen (pass));
      SHA1_Update (&c, "/", 1);
      SHA1_Update (&c, ip, strlen (ip));
      SHA1_Final (hash, &c);
      int n;
      for (n = 0; n < SHA_DIGEST_LENGTH; n++)
         sprintf (apath + n * 2, "%02X", hash[n]);
      return apath;
   }
   if (!w && head && !data)
   {                            // Non websocket get
      char *expect = NULL;
      char *auth = xml_get (http, "@authorization");
      if (auth)
      {
         char *pass = auth;
         while (*pass && *pass != ':')
            pass++;
         if (*pass)
            *pass++ = 0;
         if (*auth && *pass)
         {
            xml_t e = NULL;
            char *name = NULL;
            while ((e = xml_element_next_by_name (config, e, "user"))
                   && (!(name = xml_get (e, "@name")) || strcasecmp (name, auth)));
            if (e)
            {                   // found a user
               char *check = xml_get (e, "@pass");
               if (check && !strcmp (check, pass))
                  asprintf (&expect, ">/%s/%s/", name, authpath (name, pass));  // Good password
            }
         }
      }
      if (!expect)
      {                         // Not valid
         if (auth)
         {
            syslog (LOG_INFO, "Failed login %s from %s", auth, xml_get (head, "@IP"));
            sleep (10);
         }
         return "401 SolarSystem";
      }
      char *path = xml_element_content (http);
      if (!path || !*path)
         return "404 WTF";
      if (strncmp (path, expect + 1, strlen (expect) - 1))
         return expect;
      path += strlen (expect) - 2;
      free (expect);
      if (!path[1])
         path = "/index.html";
      if (!isalnum (path[1]))
         return "404 WTF";
      char *p;
      for (p = path + 1; isalnum (*p) || *p == '_' || *p == '-'; p++);
      if (*p != '.' || (strcmp (p, ".html") && strcmp (p, ".js") && strcmp (p, ".css") && strcmp (p, ".png") && strcmp (p, ".svg") && strcmp (p, ".ttf")))      // Very limited options of files to serve
         return "404 Not found";
      if (wsfloorplan && !strcmp (path, "/floorplan.png"))
         path = strdup (wsfloorplan);   // already has @ prefix
      else
      {
         path = strdup (path);
         *path = '@';
      }
      return path;
   }
   if (w && head && !data)
   {                            // New connection, authenticate
      char *path = xml_element_content (http);
      if (!path || *path != '/')
         return "404 WTF";
      char *user = ++path;
      while (*path && *path != '/')
         path++;
      if (!*path)
         return "404 WTF";
      *path++ = 0;
      xml_t e = NULL;
      char *name = NULL;
      char *pass = NULL;
      while ((e = xml_element_next_by_name (config, e, "user")) && (!(name = xml_get (e, "@name")) || strcasecmp (name, user)));
      if (!e || !(pass = xml_get (e, "@pass")))
         return "404 WTF";
      user_t *u;
      for (u = users; u && strcmp (u->name, user); u = u->next);
      if (!u)
         return "404 WTF";
      websocket_set_data (w, u);
      char *a = authpath (user, pass);
      if (!a || strncmp (path, a, strlen (a)) || path[strlen (a)] != '/')
         return "403 sorry";
      // We want to send current state data to this connection
      pthread_mutex_lock (&eventmutex); // Avoid things changing
      xml_t root = xml_tree_new (NULL);
      xml_add (root, "@full-data", "true");     // Send name, etc.
      xml_add (root, "*user", user);
      int g;
      for (g = 0; g < MAX_GROUP; g++)
         if (groups & (1 << g))
         {
            xml_t x = xml_addf (root, "+group@-id", "%d", g);
            if (group[g].name)
            {
               xml_add (x, "@name", group[g].name);
               if (u->group_open & (1 << g))
                  xml_add (x, "@-user-open", "true");
               if (u->group_arm & (1 << g))
                  xml_add (x, "@-user-arm", "true");
               if (u->group_disarm & (1 << g))
                  xml_add (x, "@-user-disarm", "true");
               if (u->group_reset & (1 << g))
                  xml_add (x, "@-user-reset", "true");
               if (u->group_prop & (1 << g))
                  xml_add (x, "@-prop", "true");
            }
         }
      int s;
      for (s = 0; s < STATES; s++)
         if (s != STATE_ZONE && s != STATE_ENTRY && s != STATE_NONEXIT && s != STATE_OPEN)
         {
            state_ws (root, "*set", s, state[s] & groups);
            state_ws (root, "*clr", s, (~state[s]) & groups);
         }
      keypad_t *k;
      for (k = keypad; k; k = k->next)
         keypad_ws (root, k);
      int d;
      for (d = 0; d < MAX_DOOR; d++)
         door_ws (root, d);
      port_p p;
      for (p = ports; p; p = p->next)
         if (p->port)
         {
            if (p->isinput)
               input_ws (root, p);
            else
               output_ws (root, p);
         } else if (p->mqtt)
            device_ws (root, p);
      websocket_send (1, &w, root);
      pthread_mutex_unlock (&eventmutex);
      xml_tree_delete (root);
      return NULL;
   }
   if (w && !head && data)
   {                            // Existing connection
      user_t *user = websocket_data (w);
      if (!user)
         return "No user";
      // Process valid requests
      pthread_mutex_lock (&eventmutex); // Stop simultaneous event processingx
      xml_t root = xml_tree_new (NULL);
      xml_add (root, "@full-data", "true");     // Send name, etc.
      xml_t e;
      for (e = NULL; (e = xml_element_next_by_name (data, e, "keypad"));)
      {                         // Key presses
         port_p port = port_parse (xml_get (e, "@id"), NULL, -1);
         if (!port)
            continue;
         keypad_t *k = NULL;
         for (k = keypad; k && k->port != port; k = k->next);
         if (!k)
            continue;
         char *key = xml_get (e, "@key");
         if (!key || !*key)
            continue;
         if (!strcasecmp (key, "ESC"))
            key = "\e";
         else if (!strcasecmp (key, "ENT"))
            key = "\n";
         keypad_update (k, *key);
      }
      for (e = NULL; (e = xml_element_next_by_name (data, e, "position"));)
      {                         // Position update
         char *type = xml_get (e, "@type");
         if (!type)
            continue;
         char *id = xml_get (e, "@id");
         if (!id || !*id)
            continue;
         int a = atoi (xml_get (e, "@a") ? : "-1");
         int x = atoi (xml_get (e, "@x") ? : "-1");
         int y = atoi (xml_get (e, "@y") ? : "-1");
         char *t = xml_get (e, "@t");
         void patch (void)
         {                      // Update config
            if (!config)
               return;
            xml_t e = NULL;
            while ((e = xml_element_next_by_name (config, e, type)))
            {
               char *xid = xml_get (e, "@id");
               if (!xid)
                  continue;
               if (!strcmp (xid, id))
                  break;        // found
            }
            if (!e)
            {
               e = xml_element_add (config, type);
               xml_add (e, "@id", id);
            }
            if (a >= 0)
               xml_addf (e, "@a", "%d", a);
            if (x >= 0)
               xml_addf (e, "@x", "%d", x);
            if (y >= 0)
               xml_addf (e, "@y", "%d", y);
            if (t)
               xml_addf (e, "@t", "%s", t);
            configchanged = 1;
         }
         if (!strcasecmp (type, "door") && !strncasecmp (id, "DOOR", 4))
         {
            int d = atoi (id + 4);
            if (d < 0 || d >= MAX_DOOR)
               continue;
            if (a >= 0)
               mydoor[d].a = a;
            if (a >= 0)
               mydoor[d].x = x;
            if (y >= 0)
               mydoor[d].y = y;
            if (t)
            {
               if (mydoor[d].t)
                  free (mydoor[d].t);
               mydoor[d].t = strdup (t);
            }
            patch ();
            door_ws (root, d);
            continue;
         }
         if (!strcasecmp (type, "input"))
         {
            port_p p = port_parse_i (id, NULL);
            if (!p)
               continue;
            int id = port_device (p);
            if (id >= MAX_DEVICE)
               continue;
            int port = port_port (p);
            if (!port)
               continue;
            port--;
            if (a >= 0)
               port_app (p)->a = a;
            if (x >= 0)
               port_app (p)->x = x;
            if (y >= 0)
               port_app (p)->y = y;
            if (t)
            {
               if (port_app (p)->t)
                  free ((void *) port_app (p)->t);
               port_app (p)->t = strdup (t);
            }
            if (a >= 0 || x >= 0 || y >= 0 || t)
               port_app (p)->onplan = 1;
            patch ();
            input_ws (root, p);
            continue;
         }
         if (!strcasecmp (type, "output"))
         {
            port_p p = port_parse_o (id, NULL);
            if (!p)
               continue;
            int id = port_device (p);
            if (id >= MAX_DEVICE)
               continue;
            int port = port_port (p);
            if (!port)
               continue;
            port--;
            if (a >= 0)
               port_app (p)->a = a;
            if (x >= 0)
               port_app (p)->x = x;
            if (y >= 0)
               port_app (p)->y = y;
            if (t)
            {
               if (port_app (p)->t)
                  free ((void *) port_app (p)->t);
               port_app (p)->t = strdup (t);
            }
            if (a >= 0 || x >= 0 || y >= 0 || t)
               port_app (p)->onplan = 1;
            patch ();
            output_ws (root, p);
            continue;
         }
      }
      for (e = NULL; (e = xml_element_next_by_name (data, e, "door"));)
      {                         // Door action
         char *id = xml_get (e, "@id");
         if (!id || strncasecmp (id, "door", 4))
            continue;
         int d = atoi (id + 4);
         if (d < 0 || d >= MAX_DOOR || !door[d].state)
            continue;
         if (!(mydoor[d].group_lock & user->group_open))
            continue;           // Not allowed
         if (door[d].state == DOOR_CLOSED)
            door_lock (d);
         else if (door[d].state != DOOR_DEADLOCKED)
            door_open (d);
      }
      for (e = NULL; (e = xml_element_next_by_name (data, e, "arm"));)
      {                         // Group ARM
         int g = atoi (xml_element_content (e) ? : "-1");
         if (g >= 0 && g < MAX_GROUP && (user->group_arm & (1 << g)))
            alarm_arm (user->name ? : "web", NULL, 1 << g, 1);
      }
      for (e = NULL; (e = xml_element_next_by_name (data, e, "disarm"));)
      {                         // Group DISARM
         int g = atoi (xml_element_content (e) ? : "-1");
         if (g >= 0 && g < MAX_GROUP && (user->group_disarm & (1 << g)))
            alarm_unset (user->name ? : "web", NULL, 1 << g);
      }
      for (e = NULL; (e = xml_element_next_by_name (data, e, "reset"));)
      {                         // Group RESET
         int g = atoi (xml_element_content (e) ? : "-1");
         if (g >= 0 && g < MAX_GROUP && (user->group_reset & (1 << g)))
            alarm_reset (user->name ? : "web", NULL, 1 << g);
      }
      if (xml_element_next (root, NULL))
         websocket_send_all (root);
      xml_tree_delete (root);
      pthread_mutex_unlock (&eventmutex);
      return NULL;
   }
   return NULL;
}

static char *
wscallback (websocket_t * w, xml_t head, xml_t data)
{                               // Do callback and tidy up afterwards
   char *e = do_wscallback (w, head, data);
   if (head)
      xml_tree_delete (head);
   if (data)
      xml_tree_delete (data);
   return e;
}
#endif

// Main
int
main (int argc, const char *argv[])
{
   port_start ();
#ifdef	LIBWS
   char *d = strrchr (argv[0], '/');
   if (d)
   {
      char *dir = strdupa (argv[0]);
      d = strrchr (dir, '/');
      *d = 0;
      if (chdir (dir))
         errx (1, "Count not chdir to %s", dir);
   }
   port_output_callback = ws_port_output_callback;
#endif
   const char *configfile = NULL;
   {
      int c;
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         {
          "config", 'c', POPT_ARG_STRING, &configfile, 0, "Config", "filename"},
         {
          "set-file", 's', POPT_ARG_STRING | POPT_ARGFLAG_SHOW_DEFAULT, &setfile, 0, "File holding set state", "filename"},
         {
          "max-from", 0, POPT_ARG_STRING, &maxfrom, 0, "Max from port", "ID"},
         {
          "max-to", 0, POPT_ARG_STRING, &maxto, 0, "Max to port", "ID"},
         {
          "dump", 'V', POPT_ARG_NONE, &dump, 0, "Dump", NULL},
         {
          "debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug", NULL}, POPT_AUTOHELP {
                                                                                 NULL}
      };
      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));
      if (!configfile && poptPeekArg (optCon))
         configfile = poptGetArg (optCon);
      if (poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
      poptFreeContext (optCon);
   }
   if (debug)
      warnx ("Alarm panel in debug mode");
   gettimeofday (&now, NULL);
   bus_init ();
   pthread_mutex_init (&eventmutex, 0);
   pthread_mutex_init (&logmutex, 0);
   pipe2 (logpipe, O_NONBLOCK); // We check queue anyway an we don't want to risk stalling if app is stalled for some reason and a lot of events
   if (debug)
      warnx ("Create log thread");
   pthread_t logthread;
   if (pthread_create (&logthread, NULL, logger, NULL))
      warn ("Log thread failed");
   if (debug)
      warnx ("Load config");
   load_config (configfile);
   if (debug)
      warnx ("Config loaded");
   if (buses)
   {
      int n;
      for (n = 0; n < MAX_BUS; n++)
         if (buses & (1 << n))
         {
            bus_start (n);
            if (debug)
               printf ("Starting bus %d\n", n + 1);
            mybus[n].watchdog = now.tv_sec + 120;
         }
   } else
   {
      // TODO alternative watchdog
   }
   door_start ();
   if (debug)
      printf ("%s Groups found\n", group_list (groups));
#ifdef	LIBMQTT
   mosquitto_lib_init ();
   iot = mosquitto_new (xml_get (config, "system@name"), true, NULL);
   if (!iot)
      warnx ("IoT init failed");
   else
   {
      group_t iot_arm = group_parse (xml_get (config, "system@iot-arm"));
      group_t iot_unset = group_parse (xml_get (config, "system@iot-unset"));
      char *iot_topic = NULL;
      void iot_connected (struct mosquitto *iot, void *obj, int rc)
      {
         obj = obj;
         dolog (groups, "IOT", NULL, NULL, "Server connected %d", rc);
         if (iot_arm || iot_unset)
         {                      // Subscribe
            if (asprintf (&iot_topic, "cmnd/%s/+", xml_get (config, "system@name") ? : "SolarSystem") < 0)
               errx (1, "malloc");
            if (mosquitto_subscribe (iot, NULL, iot_topic, 0))
               dolog (groups, "IOT", NULL, NULL, "Subscribe failed %s", iot_topic);
         }
      }
      void iot_message (struct mosquitto *iot, void *obj, const struct mosquitto_message *msg)
      {
         iot = iot;
         obj = obj;
         if (iot_topic)
         {
            int l = strlen (iot_topic) - 1;
            if (!strncmp (msg->topic, iot_topic, l))
            {
               char *m = malloc (msg->payloadlen + 1);
               if (!m)
                  errx (1, "malloc");
               memcpy (m, msg->payload, msg->payloadlen);
               m[msg->payloadlen] = 0;
               group_t g = group_parse (m);
               free (m);
               if (!strcmp (msg->topic + l, "arm"))
               {
                  if (g & ~iot_arm)
                     dolog (g & ~iot_arm, "IOT", NULL, NULL, "Attempting arm of invalid groups");
                  g &= iot_arm;
                  pthread_mutex_lock (&eventmutex);
                  alarm_arm ("IOT", NULL, g, 0);
                  pthread_mutex_unlock (&eventmutex);
                  return;
               }
               if (!strcmp (msg->topic + l, "cancel"))
               {
                  if (g & ~iot_arm)
                     dolog (g & ~iot_arm, "IOT", NULL, NULL, "Attempting cancel of invalid groups");
                  g &= state[STATE_ARM];        // Must be arming
                  g &= iot_arm;
                  pthread_mutex_lock (&eventmutex);
                  alarm_unset ("IOT", NULL, g);
                  pthread_mutex_unlock (&eventmutex);
                  return;
               }
               if (!strcmp (msg->topic + l, "unset"))
               {
                  if (g & ~iot_unset)
                     dolog (g & ~iot_unset, "IOT", NULL, NULL, "Attempting unset of invalid groups");
                  g &= iot_unset;
                  pthread_mutex_lock (&eventmutex);
                  alarm_unset ("IOT", NULL, g);
                  pthread_mutex_unlock (&eventmutex);
                  return;
               }
            }
         }
         dolog (groups, "IOT", NULL, NULL, "Unexpected message %s", msg->topic);
      }
      void iot_disconnected (struct mosquitto *iot, void *obj, int rc)
      {
         obj = obj;
         mosquitto_reconnect_async (iot);
         dolog (groups, "IOT", NULL, NULL, "Server disconnected %s", mosquitto_strerror (rc));
         commfailcount++;
      }
      mosquitto_connect_callback_set (iot, iot_connected);
      if (iot_arm || iot_unset)
         mosquitto_message_callback_set (iot, iot_message);
      mosquitto_disconnect_callback_set (iot, iot_disconnected);
      mosquitto_username_pw_set (iot, xml_get (config, "system@iot-user"), xml_get (config, "system@iot-pass"));
      char *host = xml_get (config, "system@iot-host") ? : "localhost";
      char *ca = xml_get (config, "system@iot-ca");
      if (ca && mosquitto_tls_set (iot, ca, NULL, NULL, NULL, NULL))
         warnx ("IoT cert failed %s", ca);
      int port = atoi (xml_get (config, "system@iot-port") ? : ca ? "8883" : "1883");
      if (mosquitto_connect_async (iot, host, port, 60))
         warnx ("IoT connect failed %s:%d", host, port);
      mosquitto_loop_start (iot);
   }
   if (xml_get (config, "system@mqtt-ca"))
   {
      mqtt = mosquitto_new (xml_get (config, "system@name"), true, NULL);
      if (!mqtt)
         warnx ("MQTT init failed");
      else
      {
         void mqtt_connected (struct mosquitto *mqtt, void *obj, int rc)
         {
            obj = obj;
            if (mosquitto_subscribe (mqtt, NULL, "state/SS/#", 0))
               dolog (groups, "MQTT", NULL, NULL, "Subscribe failed");
            if (mosquitto_subscribe (mqtt, NULL, "event/SS/#", 0))
               dolog (groups, "MQTT", NULL, NULL, "Subscribe failed");
            dolog (groups, "MQTT", NULL, NULL, "Server connected %d", rc);
         }
         void mqtt_message (struct mosquitto *mqtt, void *obj, const struct mosquitto_message *msg)
         {
            mqtt = mqtt;
            obj = obj;
            char *t = msg->topic;
            if (msg && (!strncmp (t, "state/SS/", 9) || !strncmp (t, "event/SS/", 9)) && strlen (t + 9) >= 6)
            {
               char *id = t + 9;
               char *tag = id + 6;
               if (*tag == '/')
                  tag++;
               else
                  tag = NULL;
               id[6] = 0;
               port_p port = NULL;
               if (tag)
               {
                  char *e = tag + strlen (tag);
                  if (e > tag && isdigit (e[-1]) && e[-1] > '0')
                     port = port_new (id, 1, e[-1] - '0');      // Input port
                  else
                     port = port_new (id, 0, 0);        // Device
               } else
                  port = port_new (id, 0, 0);   // Device
               port_app_t *app = port_app (port);
               if (port && !strncmp (t, "state", 5) && msg->payloadlen >= 1)
               {
                  int state = (((char *) msg->payload)[0] > '0' ? 1 : 0);
                  if (app->invert)
                     state = 1 - state;
                  if (!tag && !app->config)
                  {             // New device
                     app->config = xml_element_add (config, "device");
                     xml_add (app->config, "@id", id);
                     configchanged = 1;
                  }
                  if (!tag && !port->state && state)
                  {             // Load settings
                     xml_attribute_t a = NULL;
                     while ((a = xml_attribute_next (app->config, a)))
                     {
                        char *n = xml_attribute_name (a);
                        char *v = xml_attribute_content (a);
                        if (!strcmp (n, "id"))
                           continue;
                        if (!strcmp (n, "name"))
                           continue;
                        char *topic;
                        asprintf (&topic, "setting/SS/%s/%s", port->mqtt, n);
                        mosquitto_publish (mqtt, NULL, topic, strlen (v ? : ""), v, 1, 0);
                     }
                     port_p o;
                     for (o = ports; o; o = o->next)
                        if (o->mqtt && port_isoutput (o) && !strcmp (o->mqtt, id))
                           mqtt_output (o, o->state);
                  }
                  int etype = 0;
                  if (!tag && (state || port->state != state))
                     etype = (state ? EVENT_FOUND : EVENT_MISSING);
                  else if (tag && !strncmp (tag, "input", 5) && port->state != state)
                     etype = EVENT_INPUT;
                  else if (tag && !strncmp (tag, "fault", 5) && port->fault != state)
                     etype = EVENT_FAULT;
                  else if (tag && !strncmp (tag, "tamper", 6) && port->tamper != state)
                     etype = EVENT_TAMPER;
                  if (etype)
                  {             // Send event
                     event_t *e = malloc (sizeof (*e));
                     if (!e)
                        errx (1, "malloc");
                     memset ((void *) e, 0, sizeof (*e));
                     e->event = etype;
                     e->port = port;
                     e->state = state;
                     struct timezone tz;
                     gettimeofday ((void *) &e->when, &tz);
                     postevent (e);
                  }
                  return;
               }
               if (port && !strncmp (t, "event", 5) && msg->payloadlen >= 1)
               {
                  if (!strcmp (tag, "id") || !strcmp (tag, "held"))
                  {             // Fob
                     char *m = alloca (msg->payloadlen + 1);
                     memcpy (m, msg->payload, msg->payloadlen);
                     m[msg->payloadlen] = 0;
                     while (*m == '0')
                        m++;
                     event_t *e = malloc (sizeof (*e));
                     if (!e)
                        errx (1, "malloc");
                     memset ((void *) e, 0, sizeof (*e));
                     e->event = (!strcmp (tag, "id") ? EVENT_FOB : EVENT_FOB_HELD);
                     e->port = port;
                     strncpy ((char *) e->fob, m, sizeof (e->fob));
                     struct timezone tz;
                     gettimeofday ((void *) &e->when, &tz);
                     postevent (e);
                     return;
                  }
                  if (!strcmp (tag, "key") || !strcmp (tag, "held"))
                  {
                     event_t *e = malloc (sizeof (*e));
                     if (!e)
                        errx (1, "malloc");
                     memset ((void *) e, 0, sizeof (*e));
                     e->event = (!strcmp (tag, "key") ? EVENT_KEY : EVENT_KEY_HELD);
                     e->port = port;
                     e->key = *(char *) msg->payload;
                     struct timezone tz;
                     gettimeofday ((void *) &e->when, &tz);
                     postevent (e);
                     return;
                  }
               }
            }
            dolog (groups, "MQTT", NULL, NULL, "Unexpected message %s", msg->topic);
         }
         void mqtt_disconnected (struct mosquitto *mqtt, void *obj, int rc)
         {
            obj = obj;
            mosquitto_reconnect_async (mqtt);
            dolog (groups, "MQTT", NULL, NULL, "Server disconnected %s", mosquitto_strerror (rc));
            commfailcount++;
         }
         mosquitto_connect_callback_set (mqtt, mqtt_connected);
         mosquitto_message_callback_set (mqtt, mqtt_message);
         mosquitto_disconnect_callback_set (mqtt, mqtt_disconnected);
         mosquitto_username_pw_set (mqtt, xml_get (config, "system@mqtt-user"), xml_get (config, "system@mqtt-pass"));
         char *host = xml_get (config, "system@mqtt-host") ? : "localhost";
         char *ca = xml_get (config, "system@mqtt-ca");
         int port = atoi (xml_get (config, "system@mqtt-port") ? : ca ? "8883" : "1883");
         if (ca && mosquitto_tls_set (mqtt, ca, NULL, NULL, NULL, NULL))
            warnx ("MQTT cert failed %s", ca);
         if (mosquitto_connect_async (mqtt, host, port, 60))
            warnx ("MQTT connect failed %s:%d", host, port);
         mosquitto_loop_start (mqtt);
      }
   }
#endif
#ifdef	LIBWS
   //if (wsport || wskeyfile)
   {
      const char *e = websocket_bind (wsport, wsorigin, wshost, NULL, wscertfile, wskeyfile,
                                      wscallback);
      if (e)
         errx (1, "Websocket fail: %s", e);
   }
#endif
   state[STATE_UNSET] = groups;
   if (setfile)
   {
      FILE *f = fopen (setfile, "r");
      if (!f)
         dolog (groups, "CONFIG", NULL, NULL, "Cannot open %s", setfile);
      else
      {
         char line[100] = "";
         if (fgets (line, sizeof (line), f))
         {
            group_t set = group_parse (line);
            state[STATE_SET] |= set;
            state[STATE_UNSET] &= ~set;
         }
         fclose (f);
      }
   }
   dolog (groups, "STARTUP", NULL, NULL, "System started");
   if (maxfrom && maxto)
   {                            // Move a max
      port_p f = port_parse (maxfrom, NULL, -1);
      port_p t = port_parse (maxto, NULL, -1);
      if (!f || device[port_device (f)].type != TYPE_MAX)
         dolog (groups, "CONFIG", NULL, NULL, "max-from invalid");
      else if (!t || device[port_device (t)].type != TYPE_MAX)
         dolog (groups, "CONFIG", NULL, NULL, "max-to invalid");
      else if ((port_device (f) >> 8) != (port_device (t) >> 8))
         dolog (groups, "CONFIG", NULL, NULL, "max-from and max-to on different buses");
      else
      {
         device[port_device (f)].newid = port_device (t);
         device[port_device (f)].config = 1;
         dolog (groups, "CONFIG", NULL, NULL, "Max renumber planned");
      }
   }
   state_change (groups);
   time_t lastmin = time (0);
   while (1)
   {
      gettimeofday (&now, NULL);
      localtime_r (&now.tv_sec, &lnow);
      time_t nextpoll = (now.tv_sec + 60) / 60 * 60;
      if (nextpoll > lastmin)
      {                         // Every minute
         save_config (configfile);
         lastmin = nextpoll;
         if (commfailcount)
         {
            commfailcount = 0;
            if (!commfailreported)
            {
               commfailreported = 1;
               add_warning (groups, "COMMS", NULL);
            }
         } else if (commfailreported)
         {
            commfailreported = 0;
            rem_warning (groups, "COMMS", NULL);
         }
         profile_check ();
      }
      if (state[STATE_ARM])
      {                         // Top level timed settings
         int s;
         group_t stalled = 0;   // Which have reason to restart, not intruder as covered by NONEXIT
         for (s = 0; s < STATE_TRIGGERS; s++)
            if (s != STATE_ZONE && s != STATE_FAULT && s != STATE_WARNING && s != STATE_OPEN && s < STATE_USER1)
               stalled |= state[s];
         stalled &= state[STATE_ARM];
         if (stalled)
            alarm_timed (stalled, 0);   // Restart timers
         group_t set = 0,
            failed = 0;
         int n;
         for (n = 0; n < MAX_GROUP; n++)
            if (state[STATE_ARM] & (1 << n))
            {
               if (group[n].when_fail <= now.tv_sec)
                  failed |= (1 << n);
               else if (group[n].when_set <= now.tv_sec)
                  set |= (1 << n);
            } else
            {
               if (nextpoll < group[n].when_fail)
                  nextpoll = group[n].when_fail;
               if (nextpoll < group[n].when_set)
                  nextpoll = group[n].when_set;
            }
         if (set)
         {
            pthread_mutex_lock (&eventmutex);
            alarm_set (NULL, NULL, set);
            pthread_mutex_unlock (&eventmutex);
         }
         if (failed)
         {
            pthread_mutex_lock (&eventmutex);
            alarm_failset (NULL, NULL, failed);
            keypads_message (failed, "SET FAILED");
            pthread_mutex_unlock (&eventmutex);
         }
      }
      group_t changed = 0;
      if (state[STATE_PREALARM])
      {                         // Entry timer
         int n;
         for (n = 0; n < MAX_GROUP; n++)
            if (state[STATE_PREALARM] & (1 << n))
            {
               if (group[n].when_alarm + group[n].entry_time < now.tv_sec)
               {                // Entry time expired, fill alarm
                  state[STATE_PREALARM] &= ~(1 << n);
                  state[STATE_ALARM] |= (1 << n);
                  state[STATE_STROBE] |= (1 << n);
                  changed |= (1 << n);
               } else if (group[n].when_alarm + group[n].entry_time < nextpoll)
                  nextpoll = group[n].when_alarm + group[n].entry_time;
            }
      }
      if (state[STATE_ALARM])
      {                         // Bell timer
         int n;
         for (n = 0; n < MAX_GROUP; n++)
            if (state[STATE_ALARM] & (1 << n))
            {
               if (group[n].when_alarm + group[n].bell_time < now.tv_sec)
               {                // Should be off
                  if (state[STATE_BELL] & (1 << n))
                  {
                     state[STATE_BELL] &= ~(1 << n);    // Bell off
                     changed |= (1 << n);
                  }
               } else if (group[n].when_alarm + group[n].bell_delay < now.tv_sec)
               {                // Should be on
                  if (!(state[STATE_BELL] & (1 << n)))
                  {
                     state[STATE_BELL] |= (1 << n);     // Bell on
                     changed |= (1 << n);
                  }
                  if (nextpoll > group[n].when_alarm + group[n].bell_time)
                     nextpoll = group[n].when_alarm + group[n].bell_time;
               } else if (nextpoll > group[n].when_alarm + group[n].bell_delay)
                  nextpoll = group[n].when_alarm + group[n].bell_delay;
            }
      }
      {                         // keypad
         keypad_t *k;
         for (k = keypad; k; k = k->next)
         {
            if (k->when <= now.tv_sec)
               keypad_update (k, 0);
            if (k->when > now.tv_sec && k->when < nextpoll)
               nextpoll = k->when;
         }
      }
      if (changed)
      {
         pthread_mutex_lock (&eventmutex);
         state_change (changed);
         pthread_mutex_unlock (&eventmutex);
      }
      {                         // KA check
         int n;
         for (n = 0; n < MAX_BUS; n++)
            if ((buses & (1 << n)) && mybus[n].watchdog < now.tv_sec)
            {
               syslog (LOG_INFO, "KA timeout bus %d (%lu)", n + 1, now.tv_sec - mybus[n].watchdog);
               errx (1, "KA timeout bus %d (%lu)", n + 1, now.tv_sec - mybus[n].watchdog);
            }
      }
      event_t *e;
      if ((e = bus_event ((nextpoll - now.tv_sec) * 1000000ULL - now.tv_usec)))
      {
         pthread_mutex_lock (&eventmutex);
         doevent (e);
         pthread_mutex_unlock (&eventmutex);
      }
   }
   return 0;
}
