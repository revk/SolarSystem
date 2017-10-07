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
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <ctype.h>
#include <err.h>
#include <signal.h>
#include <execinfo.h>
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

xml_t config = NULL;

#define	MAX_GROUP	10	// Mainly to work with keypad...
typedef unsigned short group_t;	// Set of groups
#define	ALL_GROUPS	((1<<MAX_GROUP)-1)

#define	MAX_FOB		5	// Fobs per user

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

#ifdef	LIBWS
char *wsport = NULL;
char *wsauth = NULL;
const char *wsorigin = NULL;
const char *wshost = NULL;
const char *wspath = NULL;
const char *wscertfile = NULL;
const char *wskeyfile = NULL;
#endif

unsigned int commfailcount = 0;
unsigned int commfailreported = 0;
group_t walkthrough = 0;

typedef unsigned char state_t;

// The actual state bits...
const char *setfile = "/var/local/alarmset";
static group_t state[STATES] = { };	// the states, for each group
static group_t previous_state[STATES] = { };	// the states, for each group

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
  time_t when_set;		// When set complete
  time_t when_fail;		// When set fail
  time_t when_alarm;		// When entry/alarm started
  int set_time;			// How long to set
  int set_fail;			// How long for failing to set
  int entry_time;		// Entry time
  int bell_delay;		// Delay before ringing (from alarm set)
  int bell_time;		// Max time to ring bell (from alarm set)
  int bell_rest;		// Rest time (from alarm set)
  int count[STATE_TRIGGERS];	// Input counts
  const char *armed_by;		// Who armed the alarm
} group[MAX_GROUP];

// Doors not a linked list as mapper to door structure in library
typedef struct mydoor_s mydoor_t;
struct mydoor_s
{				// Per door settings
  port_t i_fob[2];		// Max readers for FOB to exit and set alarm
  port_t i_exit[3];		// Exit buttons, max readers, etc, or reception desk
  port_t i_bell[2];		// Bell button(s)
  port_t o_bell[2];		// Bell output(s)
  group_t groups;		// Groups that apply
  group_t group_set;		// Which groups can set alarm
  group_t group_unset;		// Which groups can set alarm
  group_t fire;			// Open for fire in specified zones
  int time_set;			// If set, the hold fob does timed set, else does not
  char *name;
  int airlock;			// door that must be closed before this one is opened
  state_t lockdown;		// Outputs triggering lockdown
  unsigned char entry:1;	// Reported states from door
  unsigned char intruder:1;
  unsigned char tamper:1;
  unsigned char fault:1;
  unsigned char warning:1;
  unsigned char lock_arm:1;
  unsigned char lock_set:1;
  unsigned char opening:1;
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
  group_t group_set;
  group_t group_unset;
  group_t group_reset;
  group_t group_open;
  group_t group_prop;
};
static user_t *users = NULL;

typedef struct keypad_s keypad_t;
struct keypad_s
{
  char *name;
  keypad_t *next;		// Linked list
  port_t port;			// The port it is on
  port_t prox;			// Prox reader
  char *message;		// Display message
  time_t when_logout;		// Auto user logout
  int time_logout;		// Config logout time
  time_t when;			// When to next update keypad
  group_t groups;		// What groups the keypad applies to
  unsigned char pininput;	// Inputting PIN
  unsigned long pin;		// PIN so far
  user_t *user;			// User if logged in
  const char *msg;		// Flash up message
  int posn;			// List position
  time_t when_posn;		// List time
};
static keypad_t *keypad = NULL;

// Device settings at alarm panel level
static struct
{				// Per device settings
  char *name;
  unsigned char missing:1;	// Device is AWOL
  struct
  {				// Inputs
    char *name;
    unsigned char isexit:1;	// Door related input
    unsigned char isbell:1;	// Door related input
    group_t trigger[STATE_TRIGGERS];	// If this input applies to a state
  } input[MAX_INPUT];
  struct
  {				// Outputs
    char *name;
    state_t type;		// Which state we are outputting
    group_t group;		// Which groups it applies to
  } output[MAX_OUTPUT];
} mydevice[MAX_DEVICE] =
{
};

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
static keypad_t *keypad_new (port_t p);
static void keypads_message (group_t g, const char *msg);
static void state_change (group_t g);
#define device_name(n) port_name((n)<<8)
#define	port_name(p) real_port_name(alloca(20),p)
static char *real_port_name (char *v, port_t p);

static int
port_id (unsigned int port)
{				// Port ID from mask
  port &= 0xFF;
  if (port)
    {
      unsigned int n;
      for (n = 0; n < 8 && port != (1U << n); n++);
      if (n < 8)
	return n;
    }
  return -1;
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
{				// Get output type
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
static port_t
port_parse (const char *v, const char **ep, int i)
{				// Parse a port name/id, return 0 if invalid, port part 0 if no port
  if (!v || !*v)
    return 0;
  int l = 0;
  while (v[l] && v[l] != ',' && !isspace (v[l]))
    l++;
  if (v[l])
    {				// One word at a time
      char *n = alloca (l + 1);
      memcpy (n, v, l);
      n[l] = 0;
      if (v[l] == ',')
	l++;
      while (isspace (v[l]))
	l++;
      if (ep)
	*ep = v + l;
      v = n;
    }
  else if (ep)
    *ep = NULL;			// Last word
  if (i == 0)
    {				// Check output port names
      int d, p;
      for (d = 0; d < MAX_DEVICE; d++)
	for (p = 0; p < MAX_OUTPUT; p++)
	  if (mydevice[d].output[i].name && !strcmp (mydevice[d].output[i].name, v))
	    return (i << 8) + (1 << p);
    }
  else if (i == 1)
    {				// Check input port names
      int d, p;
      for (d = 0; d < MAX_DEVICE; d++)
	for (p = 0; p < MAX_INPUT; p++)
	  if (mydevice[d].input[i].name && !strcmp (mydevice[d].input[i].name, v))
	    return (i << 8) + (1 << p);
    }
  else if (i == -1)
    {				// Reference device
      int d;
      for (d = 0; d < MAX_DEVICE; d++)
	if (mydevice[d].name && !strcmp (mydevice[d].name, v))
	  return (i << 8);
    }
  // Parse the port
  unsigned int id = 0, type = 0, port = 0;
  while (type < MAX_TYPE && strncmp (type_name[type], v, strlen (type_name[type])))
    type++;
  if (type < MAX_TYPE)
    v += strlen (type_name[type]);
  if (*v > '0' && *v <= '0' + MAX_BUS && isxdigit (v[1]) && isxdigit (v[2]))
    {				//  Device
      id = ((*v - '1') << 8) + (((isalpha (v[1]) ? 9 : 0) + (v[1] & 0xF)) << 4) + ((isalpha (v[2]) ? 9 : 0) + (v[2] & 0xF));
      v += 3;
      if (*v > '0' && *v <= '0' + MAX_INPUT)
	port |= (1 << (*v++ - '1'));
    }
  while (*v && !isspace (*v) && *v != ',')
    v++;
  if (type && id && id < MAX_DEVICE)
    {
      if (!device[id].type)
	{
	  device[id].type = type;
	  if (type == TYPE_MAX)
	    device[id].fob_hold = 30;	// 3 second default
	  buses |= (1 << (id >> 8));
	}
      else if (device[id].type != type)
	{
	  dolog (groups, "CONFIG", NULL, NULL, "Device type clash port %s %s/%s, device disabled", port_name (id), type_name[device[id].type], type_name[type]);
	  device[id].disabled = 1;
	}
    }
  return (id << 8) | port;
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

static char *
real_port_name (char *v, port_t p)
{				// Port name
  if (((p >> 8) & 0xFF) == US)
    return "PANEL";
  char *o = v;
  if ((p >> 8) >= MAX_DEVICE)
    o += sprintf (o, "?");
  else if (device[p >> 8].type)
    o += sprintf (o, type_name[device[p >> 8].type]);
  o += sprintf (o, "%X%02X", ((p >> 16) + 1) & 0xF, (p >> 8) & 0xFF);
  int n;
  for (n = 0; n < 8; n++)
    if (p & (1 << n))
      o += sprintf (o, "%d", n + 1);
  return v;
}

// Functions
#define	port_exit_set(w,v,p) port_exit_set_n(port_set_n((volatile port_t*)&(w),sizeof(w)/sizeof(port_t),v,p,1),sizeof(w)/sizeof(port_t))
#define	port_bell_set(w,v,p) port_bell_set_n(port_set_n((volatile port_t*)&(w),sizeof(w)/sizeof(port_t),v,p,1),sizeof(w)/sizeof(port_t))
#define	port_i_set(w,v,p) port_set_n((volatile port_t*)&(w),sizeof(w)/sizeof(port_t),v,p,1)
#define	port_o_set(w,v,p) port_set_n((volatile port_t*)&(w),sizeof(w)/sizeof(port_t),v,p,0)
#define	port_set(w,v,p) port_set_n((volatile port_t*)&(w),sizeof(w)/sizeof(port_t),v,p,-1)
static volatile port_t *
port_set_n (volatile port_t * w, int n, const char *v, unsigned char p, int i)
{				// Set up port
  int q = 0;
  while (v && *v && q < n)
    {
      port_t id = port_parse (v, &v, i);
      if (!(id & 0xFF))
	id |= p;		// default port
      w[q++] = id;
      if (i == 1 && port_id (id) && device[port_device (id)].type == TYPE_RIO)
	device[port_device (id)].ri[port_id (id)].response = 1;
    }
  if (v)
    dolog (groups, "CONFIG", NULL, NULL, "Too many ports in list %s", v);
  return w;
}

static volatile port_t *
port_exit_set_n (volatile port_t * w, int n)
{
  while (n--)
    {
      if (w[n] && port_device (w[n]) < MAX_DEVICE && port_id (w[n]) < MAX_INPUT)
	mydevice[port_device (w[n])].input[port_id (w[n])].isexit = 1;
    }
  return w;
}

static volatile port_t *
port_bell_set_n (volatile port_t * w, int n)
{
  while (n--)
    {
      if (w[n] && port_device (w[n]) < MAX_DEVICE && port_id (w[n]) < MAX_INPUT)
	mydevice[port_device (w[n])].input[port_id (w[n])].isbell = 1;
    }
  return w;
}

static char *
xml_copy (xml_t x, char *n)
{				// get attribute/value and copy
  char *v = xml_get (x, n);
  if (v)
    v = strdup (v);
  return v;
}

static void *
load_config (const char *configfile)
{
  buses = 0;
  groups = 0;
  const char *v, *pl;
  if (config)
    xml_tree_delete (config);
  config = NULL;
  if (configfile)
    if (!(config = xml_tree_read_file (configfile)) && !(config = xml_tree_read_file_json (configfile)))
      return dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Cannot read %s", configfile);
  if (!config)
    return dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "No config");
  xml_t x = NULL;
  {
    int g = 0;
    for (g = 0; g < MAX_GROUP; g++)
      {				// Defaults in cased not named, etc.
	group[g].set_time = 10;
	group[g].set_fail = 120;
	group[g].bell_time = 300;
	group[g].bell_rest = 3600;
      }
    g = 0;
    while ((x = xml_element_next_by_name (config, x, "group")))
      {
	group[g].name = xml_copy (x, "@name");
	if ((v = xml_get (x, "@set-time")))
	  group[g].set_time = atoi (v);
	if ((v = xml_get (x, "@set-fail")))
	  group[g].set_fail = atoi (v);
	if ((v = xml_get (x, "@entry-time")))
	  group[g].entry_time = atoi (v);
	if ((v = xml_get (x, "@bell-delay")))
	  group[g].bell_delay = atoi (v);
	if ((v = xml_get (x, "@bell-time")))
	  group[g].bell_time = atoi (v);
	if ((v = xml_get (x, "@bell-rest")))
	  group[g].bell_rest = atoi (v);
	g++;
      }
    for (g = 0; g < MAX_GROUP; g++)
      {				// Compound times
	group[g].bell_delay += group[g].entry_time;
	group[g].bell_time += group[g].bell_delay;
	group[g].bell_rest += group[g].bell_time;
      }
    int p, o;
    for (p = 0; p < MAX_DEVICE; p++)
      for (o = 0; o < MAX_OUTPUT; o++)
	mydevice[p].output[o].type = -1;
  }
  while ((x = xml_element_next_by_name (config, x, "input")))
    {				// Scan inputs, get names
      if (!(pl = xml_get (x, "@id")) || !*pl)
	dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Input with no id");
      else
	while (pl)
	  {
	    port_t p = port_parse (pl, &pl, 1);
	    unsigned int id = port_device (p);
	    int n = port_id (p);
	    if (n < 0 || n >= MAX_INPUT)
	      {
		dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Input with bad port");
		continue;
	      }
	    mydevice[id].input[n].name = xml_copy (x, "@name");
	    // triggers
	    int t;
	    for (t = 0; t < STATE_TRIGGERS; t++)
	      if (t != STATE_INTRUDER)
		{
		  char at[50], *p;
		  snprintf (at, sizeof (at), "@%s", state_name[t]);
		  for (p = at; *p; p++)
		    if (*p == '_')
		      *p = '-';
		    else
		      *p = tolower (*p);
		  mydevice[id].input[n].trigger[t] = group_parse (xml_get (x, at));
		}
	    mydevice[id].input[n].trigger[STATE_ZONE] |= group_parse (xml_get (x, "@intruder"));
	    // Special case for NONEXIT which are always intruder anyway
	    group_t exit = group_parse (xml_get (x, "@exit"));
	    mydevice[id].input[n].trigger[STATE_NONEXIT] = mydevice[id].input[n].trigger[STATE_ZONE] & ~exit;
	    mydevice[id].input[n].trigger[STATE_ZONE] |= exit;
	    // Special case as entry is treated not as intruder directly
	    mydevice[id].input[n].trigger[STATE_ZONE] &= ~mydevice[id].input[n].trigger[STATE_ENTRY];
	    if (device[id].type == TYPE_RIO)
	      {			// Resistance and response
		if ((v = xml_get (x, "@response")))
		  device[id].ri[n].response = atoi (v) / 10;
		if ((v = xml_get (x, "@thresholds")) || (v = xml_get (x, "@preset")))
		  {		// resistance thresholds
		    unsigned int q = 0;
		    for (q = 0; q < sizeof (rio_thresholds) / sizeof (*rio_thresholds) && strcasecmp (rio_thresholds[q].name, v); q++);
		    if (q < sizeof (rio_thresholds) / sizeof (*rio_thresholds))
		      {
			device[id].ri[n].threshold[0] = rio_thresholds[q].tampersc;
			device[id].ri[n].threshold[1] = rio_thresholds[q].lowres;
			device[id].ri[n].threshold[2] = rio_thresholds[q].normal;
			device[id].ri[n].threshold[3] = rio_thresholds[q].highres;
			device[id].ri[n].threshold[4] = rio_thresholds[q].open;
		      }
		    else
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
	  }
    }
  x = NULL;
  while ((x = xml_element_next_by_name (config, x, "rf-rio")))
    {				// Scan inputs, get names
      if (!(pl = xml_get (x, "@id")) || !*pl)
	dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "RF RIO with no id");
      else
	while (pl)
	  {
	    port_t p = port_parse (pl, &pl, 0);
	    unsigned int id = port_device (p);
	    if (!id)
	      dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Bad address for RF RIO");
	  }
    }
  x = NULL;
  while ((x = xml_element_next_by_name (config, x, "output")))
    {				// Scan inputs, get names
      if (!(pl = xml_get (x, "@id")) || !*pl)
	dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Output with no id");
      else
	while (pl)
	  {
	    port_t p = port_parse (pl, &pl, 0);
	    unsigned int id = port_device (p);
	    int n = port_id (p);
	    if (n < 0 || n >= MAX_OUTPUT)
	      {
		dolog (ALL_GROUPS, "CONFIG", NULL, port_name (p), "Output with bad port");
		continue;
	      }
	    mydevice[id].output[n].type = state_parse (xml_get (x, "@type"));
	    mydevice[id].output[n].name = xml_copy (x, "@name");
	    mydevice[id].output[n].group = group_parse (xml_get (x, "@groups") ? : "*");
	    if ((v = xml_get (x, "@polarity")) && toupper (*v) == 'N')
	      device[id].invert |= (1 << n);
	  }
    }
  x = NULL;
  while ((x = xml_element_next_by_name (config, x, "max")))
    {				// Scan keypads
      if (!(pl = xml_get (x, "@id")) || !*pl)
	dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Max with no id");
      else
	while (pl)
	  {
	    port_t p = port_parse (pl, &pl, -1);
	    if ((v = xml_get (x, "@fob-held")))
	      device[port_device (p)].fob_hold = atoi (v) * 10;
	  }
    }
  x = NULL;
  while ((x = xml_element_next_by_name (config, x, "keypad")))
    {				// Scan keypads
      if (!(pl = xml_get (x, "@id")) || !*pl)
	dolog (ALL_GROUPS, "CONFIG", NULL, NULL, "Keypad with no id");
      else
	while (pl)
	  {
	    port_t p = port_parse (pl, &pl, -1);
	    keypad_t *k = keypad_new (p);
	    k->port = p;
	    k->name = xml_copy (x, "@name");
	    k->groups = group_parse (xml_get (x, "@groups") ? : "*");
	    k->prox = port_parse (xml_get (x, "@prox"), NULL, 0);
	    k->time_logout = atoi (xml_get (x, "@logout") ? : "60");
	    k->message = xml_copy (x, "@message");
	    if ((v = xml_get (x, "@crossed-zeros")) && !strcasecmp (v, "true"))
	      device[port_device (p)].cross = 1;
	  }
    }
  x = NULL;
  while ((x = xml_element_next_by_name (config, x, "user")))
    {				// Scan users
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
	{			// FOBs
	  unsigned int q = 0;
	  while (*v && q < sizeof (u->fob) / sizeof (*u->fob))
	    {
	      fob_t n = 0;
	      while (isdigit (*v))
		n = n * 10 + *v++ - '0';
	      if (*v == ',')
		v++;
	      while (*v && isspace (*v))
		v++;
	      if (!n)
		dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with zero fob");
	      else
		{		// Check duplicates

		  user_t *f;
		  for (f = users; f; f = f->next)
		    {
		      unsigned int z;
		      for (z = 0; z < sizeof (u->fob) / sizeof (*u->fob) && f->fob[z] != n; z++);
		      if (z < sizeof (u->fob) / sizeof (*u->fob))
			break;
		    }
		  if (f)
		    dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with duplicate fob", f->name ? : "?");
		}
	      if (*v && !isdigit (*v))
		break;
	      u->fob[q++] = n;
	    }
	  if (*v)
	    {
	      dolog (ALL_GROUPS, "CONFIG", u->name, NULL, "User with invalid fob [%s]", v);
	      break;
	    }
	}
      group_t mask = group_parse (xml_get (x, "@mask") ? : "*");	// Default is all zones
      u->group_open = mask & group_parse (xml_get (x, "@open") ? : "*");	// Default is all zones so all doors
      u->group_set = mask & group_parse (xml_get (x, "@set"));	// Default is no zones
      u->group_unset = mask & group_parse (xml_get (x, "@unset"));	// Default is no zones
      u->group_reset = mask & group_parse (xml_get (x, "@reset"));
      u->group_prop = mask & group_parse (xml_get (x, "@prop"));
      u->next = users;
      users = u;
    }
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
	mydoor[d].groups = group_parse (xml_get (x, "@groups") ? : "*");
	mydoor[d].group_set = group_parse (xml_get (x, "@set") ? : xml_get (x, "@groups") ? : "*");
	mydoor[d].group_unset = group_parse (xml_get (x, "@unset") ? : xml_get (x, "@groups") ? : "*");
	mydoor[d].name = xml_copy (x, "@name");
	const char *max = xml_get (x, "@max");
	mydoor[d].fire = group_parse (xml_get (x, "@fire") ? : "*");
	if (max)
	  {			// short cut to set based on max reader
	    port_t maxport = port_parse (max, NULL, -2);
	    if (maxport && !mydevice[port_device (maxport)].name)
	      mydevice[port_device (maxport)].name = mydoor[d].name;
	    port_o_set (door[d].o_led, max, 0xFF);
	    port_o_set (door[d].mainlock.o_lock, max, 1 << 1);
	    port_i_set (door[d].i_open, max, 1 << 0);
	    port_o_set (door[d].o_beep, max, 1 << 0);
	    port_exit_set (mydoor[d].i_exit, max, 1 << 1);
	    port_set (mydoor[d].i_fob, max, -2);
	  }
	port_set (mydoor[d].i_fob, xml_get (x, "@fob"), 0);
	port_o_set (door[d].o_led, xml_get (x, "@o-led"), 0xFF);
	port_o_set (door[d].mainlock.o_lock, xml_get (x, "@o-lock"), 0);
	port_i_set (door[d].mainlock.i_unlock, xml_get (x, "@i-unlock"), 0);
	port_o_set (door[d].o_beep, xml_get (x, "@o-beep"), 0);
	port_i_set (door[d].i_open, xml_get (x, "@i-open"), 0);
	port_o_set (door[d].deadlock.o_lock, xml_get (x, "@o-deadlock"), 0);
	port_i_set (door[d].deadlock.i_unlock, xml_get (x, "@i-undeadlock"), 0);
	port_exit_set (mydoor[d].i_exit, xml_get (x, "@i-exit"), 0);
	port_bell_set (mydoor[d].i_bell, xml_get (x, "@i-bell"), 0);
	port_o_set (mydoor[d].o_bell, xml_get (x, "@o-bell"), 0);
	mydoor[d].time_set = parse_time (xml_get (x, "time-set") ? : "3", 10, 0) / 10;	// Time set is in whole seconds
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
	      dolog (mydoor[d].groups, "CONFIG", NULL, doorno, "Airlock not found %s", v);
	  }
	if (++d == MAX_DOOR)
	  break;
      }
  }
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
      char *ws = xml_get (x, "@ws-auth");
      if (ws)
	wsauth = strdup (ws);
      ws = xml_get (x, "@ws-host");
      if (ws)
	wshost = strdup (ws);
      ws = xml_get (x, "@ws-port");
      if (ws)
	wsport = strdup (ws);
      ws = xml_get (x, "@ws-path");
      if (ws)
	wspath = strdup (ws);
      ws = xml_get (x, "@ws-cert-file");
      if (ws)
	wscertfile = strdup (ws);
      ws = xml_get (x, "@ws-key-file");
      if (ws)
	wskeyfile = strdup (ws);
#endif
    }
  // All groups...
  {
    if (!groups)
      groups = 1;		// Group 0 only
    keypad_t *k;
    for (k = keypad; k; k = k->next)
      k->groups &= groups;
    int d;
    for (d = 0; d < MAX_DOOR; d++)
      {
	mydoor[d].groups &= groups;
	mydoor[d].fire &= groups;
	mydoor[d].group_set &= groups;
      }
    user_t *u;
    for (u = users; u; u = u->next)
      {
	u->group_open &= groups;
	u->group_set &= groups;
	u->group_unset &= groups;
	u->group_reset &= groups;
	u->group_prop &= groups;
      }
    for (d = 0; d < MAX_DEVICE; d++)
      {
	int p;
	for (p = 0; p < MAX_OUTPUT; p++)
	  mydevice[d].output[p].group &= groups;
	int s;
	for (p = 0; p < MAX_INPUT; p++)
	  for (s = 0; s < STATE_TRIGGERS; s++)
	    mydevice[d].input[p].trigger[s] &= groups;
      }
    int s;
    for (s = 0; s < STATES; s++)
      state[s] &= groups;
  }
  return NULL;
}

static int
door_locked (int d)
{				// Should door be locked
  group_t mask = 0;
  if (mydoor[d].lock_arm)
    mask |= state[STATE_ARM];
  if (mydoor[d].lock_set)
    mask |= state[STATE_SET];
  if (mydoor[d].lockdown < STATES)
    mask |= state[mydoor[d].lockdown];
  if (mydoor[d].groups & mask)
    return 1;
  return 0;
}

static void
door_state (group_t g)
{				// Update door locking state after state change
  int d;
  for (d = 0; d < MAX_DOOR; d++)
    if (mydoor[d].groups & g)
      {
	if (state[STATE_FIRE] & mydoor[d].fire)
	  door_open (d);	// Fire alarm
	else if (door_locked (d))
	  door_deadlock (d);
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
      k->when = 0;		// Force display update
}

static void
output_state (group_t g)
{				// Set outputs after state change
  if (!g)
    return;
  port_t p;
  for (p = 0; p < MAX_DEVICE; p++)
    if (device[p].type && device[p].type < STATES)
      {
	int o;
	for (o = 0; o < MAX_OUTPUT; o++)
	  if ((mydevice[p].output[o].group & g) && mydevice[p].output[o].type < STATES)
	    {
	      if (state[mydevice[p].output[o].type] & g)
		{
		  if (!(device[p].output & (1 << o)))
		    {
		      port_t id = (p << 8) | (1 << o);
		      port_output (id, 1);
		    }
		}
	      else
		{
		  if (device[p].output & (1 << o))
		    {
		      port_t id = (p << 8) | (1 << o);
		      port_output (id, 0);
		    }
		}
	    }
      }
}

// Main state change events
static void
del_state (group_t g, statelist_t * s)
{				// Delete
  s->groups &= ~g;
  if (s->groups)
    return;			// Still active
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
{				// alarms=1 means just latches, alarms=2 means alarms
  if (!g)
    return;
  group_t changed = 0, logging = 0;
  void addtolist (group_t g, int which)
  {
    statelist_t *s = NULL;
    for (s = statelist; s && (s->type != which || strcmp (s->port ? : "", port ? : "") || strcmp (s->name ? : "", name ? : "")); s = s->next);
    if (!s)
      {				// New
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
    addtolist (state[STATE_SET] & g, STATE_INTRUDER_LATCH);	// how do we log the ENTRY before alarm?
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
  for (s = statelist; s && (s->type != which || strcmp (s->port ? : "", port ? : "") || strcmp (s->name ? : "", name ? : "")); s = s->next);
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
{				// Post state change update
  int n, s;
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
	      state[STATE_PREALARM] |= (1 << n);	// Start entry timer
	      group[n].when_alarm = now.tv_sec;
	    }
	}
  // Derive OPEN and INTRUDER states
  state[STATE_OPEN] = (state[STATE_OPEN] & ~g) | (g & ~state[STATE_SET] & (state[STATE_ENTRY] | state[STATE_ZONE]));
  state[STATE_INTRUDER] = (state[STATE_INTRUDER] & ~g) | (g & state[STATE_SET] & (state[STATE_ZONE] | (~state[STATE_PREALARM] & state[STATE_ENTRY])));
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
	    group[n].when_alarm = now.tv_sec - group[n].entry_time;	// restart to allow bell to ring again
	}
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
	    }
	  c = (~state[s] & previous_state[s]);
	  if (c)
	    {
	      snprintf (type, sizeof (type), "-%s", state_name[s]);
	      dolog (c, type, NULL, NULL, NULL);
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
  // Do outputs and stuff resulting from state change
  output_state (g);
  door_state (g);
  keypad_state (g);
}

static void
alarm_timed (group_t g, int t)
{				// Restart timers
  int n;
  for (n = 0; n < MAX_GROUP; n++)
    if (g & (1 << n))
      group[n].when_set = now.tv_sec + (t ? : group[n].set_time);
}

static group_t
alarm_arm (const char *who, const char *where, group_t mask, int t)
{				// Arm alarm - return which groups set
  mask &= ~state[STATE_ARM];	// Already setting
  mask &= ~state[STATE_SET];	// Already set
  if (!mask)
    return mask;		// nothing to do
  dolog (mask, "ARM", who, where, "Alarm armed");
  state[STATE_ARM] |= mask;
  int n;
  for (n = 0; n < MAX_GROUP; n++)
    if (mask & (1 << n))
      {
	group[n].when_fail = now.tv_sec + group[n].set_fail;
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
    dolog (mask, type, who, where, "%s", msg);	// log who finally set the alarm
  else
    while (mask)
      {
	int a, b;
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
{				// Set alarm proper - return which groups set
  mask &= ~state[STATE_SET];	// Already set
  if (!mask)
    return mask;		// nothing to do
  logarmed (mask, who, where, "SET", "Alarm set");
  state[STATE_SET] |= mask;
  state[STATE_UNSET] &= ~mask;
  state[STATE_ARM] &= ~mask;
  state[STATE_INTRUDER_LATCH] &= ~mask;	// reset the intruder state as alarm set again
  state_change (mask);
  keypads_message (mask, "\a-- ALARM SET --");
  return mask;
}

static group_t
alarm_unset (const char *who, const char *where, group_t mask)
{				// Unset alarm - return which groups unset
  mask &= (state[STATE_SET] | state[STATE_ARM] | state[STATE_PREALARM] | state[STATE_BELL]);
  if (!mask)
    return mask;		// Nothing to do
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
{				// Failed set alarm - return which groups unset
  mask &= state[STATE_ARM];
  if (!mask)
    return mask;		// Nothing to do
  logarmed (mask, who, where, "FAILSET", "Alarm set failed");
  state[STATE_UNSET] |= mask;
  state[STATE_ARM] &= ~mask;
  state_change (mask);
  keypads_message (mask, "\a- ALARM SET FAILED -");
  return mask;
}

static group_t
alarm_reset (const char *who, const char *where, group_t mask)
{				// Reset alarm - return which groups reset
  int n;
  group_t set = state[STATE_STROBE];	// What can be cleared
  for (n = STATE_TRIGGERS; n < STATE_TRIGGERS + STATE_LATCHED; n++)
    set |= state[n];
  mask &= set;
  if (!mask)
    return mask;		// Nothing to do
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
	del_state (mask, s);	// These do not use counters
      s = n;
    }
  state_change (mask);
  keypads_message (mask, "\a- SYSTEM RESET -");
  return mask;
}

static keypad_t *
keypad_new (port_t p)
{
  keypad_t *k;
  for (k = keypad; k && k->port != p; k = k->next);
  if (!k)
    {
      k = malloc (sizeof (*k));
      memset (k, 0, sizeof (*k));
      k->next = keypad;
      device[port_device (p)].backlight = 1;
      device[port_device (p)].output = 0;
      keypad = k;
    }
  k->port = p;
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
volatile log_t *logs = NULL, **logp = NULL;
pthread_mutex_t logmutex;
pthread_mutex_t eventmutex;;
int logpipe[2];
static log_t *
next_log (long long usec)
{				// Get next log
  char x;
  // Check for log waiting
  pthread_mutex_lock (&logmutex);
  log_t *l = (log_t *) logs;
  if (l)
    logs = logs->next;
  pthread_mutex_unlock (&logmutex);
  if (l)
    {				// an log was waiting
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
  struct timeval timeout = { 0
  };
  timeout.tv_sec = usec / 1000000ULL;
  timeout.tv_usec = usec % 1000000ULL;
  int s = select (logpipe[0] + 1, &readfds, NULL, NULL, &timeout);
  if (s <= 0)
    return NULL;		// Nothing waiting in the time
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
dologger (CURL * curl, xml_t system, log_t * l)
{
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
  char *name = NULL;
  if (l->port)
    {
      port_t p;
      if (!strncmp (l->port, "DOOR", 4))
	name = mydoor[atoi (l->port + 4)].name;
      else if ((p = port_parse (l->port, NULL, -2)))
	name = mydevice[port_device (p)].name;
    }
  // Syslog (except boring keepalives)
  if (!l->type || strcasecmp (l->type, "KEEPALIVE"))
    syslog (LOG_INFO, "%*s %s %s %s %s %s", MAX_GROUP, groups, l->type ? : "?", l->port ? : "", name ? : "", l->user ? : "", l->msg ? : "");
  if (debug)
    printf ("%*s\t%s\t%s\t%s\t%s\t%s\n", MAX_GROUP, groups, l->type ? : "?", l->port ? : "", name ? : "", l->user ? : "", l->msg ? : "");
  // Other logging
  xml_t c = NULL;
  while ((c = xml_element_next_by_name (config, c, "log")))
    {				// What other logging to do
      struct curl_httppost *fi = NULL, *li = NULL;
      char *v;
      if ((v = xml_get (c, "@groups")))
	{
	  if (!(group_parse (v) & l->groups))
	    continue;		// Not in group
	}
      if ((v = xml_get (c, "@type")))
	{			// Check matching type
	  if (checklist (v, l->type) < 0)
	    continue;
	}
      if ((v = xml_get (c, "@port")))
	{			// Check matching ports
	  if (checklist (v, l->port) < 0)
	    continue;
	}
      if ((v = xml_get (c, "@user")))
	{			// Check matching users
	  if (checklist (v, l->user) < 0)
	    continue;
	}
      // What action to take
      v = xml_get (c, "@action");
      if (!v)
	continue;
      if (!strncasecmp (v, "http:", 5) || !strncasecmp (v, "https:", 6))
	{			// CURL
	  FILE *o = fopen ("/dev/null", "w");
	  curl_easy_setopt (curl, CURLOPT_WRITEDATA, o);
	  char *url = NULL;
	  char *q = strrchr (v, '?');
	  if (q)
	    {			// GET, add to 
	      curl_easy_setopt (curl, CURLOPT_HTTPGET, 1L);
	      char *url = NULL;
	      size_t urllen = 0;
	      FILE *u = open_memstream (&url, &urllen);
	      fprintf (u, "%s", v);
	      char and = 0;
	      if (q[1])
		and = 1;
	      void add (char *tag, char *value)
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
	      add ("user", l->user);
	      add ("port", l->port);
	      add ("name", name);
	      add ("msg", l->msg);
	      fclose (u);
	      curl_easy_setopt (curl, CURLOPT_URL, url);
	    }
	  else
	    {			// POST
	      void add (char *tag, char *value)
	      {
		if (!value)
		  return;
		curl_formadd (&fi, &li, CURLFORM_PTRNAME, tag, CURLFORM_PTRCONTENTS, value, CURLFORM_END);
	      }
	      add ("when", when);
	      add ("groups", groups);
	      add ("type", l->type);
	      add ("user", l->user);
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
	    curl_formfree (fi);	// Free post data
	  if (result)
	    {
	      syslog (LOG_INFO, "Log to %s failed\n", v);
	      commfailcount++;
	    }
	  fclose (o);
	}
#ifdef	LIBEMAIL
      else if (strchr (v, '@'))
	{			// Email
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
      else
	{
	  char tel[40];
	  char *n = dataformat_telephone_n (tel, sizeof (tel), v, 0, 0);
	  if (n)
	    {			// SMS
	      char *u, *p;
	      if (!system || !(u = xml_get (system, "@sms-user")) || !(p = xml_get (system, "@sms-pass")))
		{
		  syslog (LOG_INFO, "No system details for SMS to %s", v);
		  commfailcount++;
		}
	      else if (l->groups & ~state[STATE_ENGINEERING])
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
		  asprintf (&ud, "%s\t%s\n%.*s\n%s\t%s\n%s", l->type ? : "?", l->msg ? : "", MAX_GROUP, groups, l->port ? : "", name ? : "", l->user ? : "");
		  add ("ud", ud);
		  const char *server = xml_get (system, "@sms-host") ? : "https://sms.aa.net.uk/";
		  curl_easy_setopt (curl, CURLOPT_URL, server);
		  curl_easy_setopt (curl, CURLOPT_HTTPPOST, fi);
		  CURLcode result = curl_easy_perform (curl);
		  curl_formfree (fi);	// Free post data
		  if (result)
		    {
		      syslog (LOG_INFO, "SMS to %s failed to connect to server %s\n", v, server);
		      commfailcount++;
		    }
		  free (ud);
		  fclose (o);
		  if (!strstr (reply, "OK"))
		    {
		      syslog (LOG_INFO, "SMS to %s failed to send\n", v);
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
{				// Processing logs in separate thread
  d = d;			// Unused
  openlog ("alarm", LOG_CONS | LOG_PID, LOG_USER);
  CURL *curl = curl_easy_init ();
  if (debug)
    curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt (curl, CURLOPT_TIMEOUT, 10L);
  xml_t system = xml_element_next_by_name (config, NULL, "system");
  while (1)
    {
      log_t *l = next_log (1000000);
      if (!l)
	continue;
      dologger (curl, system, l);
    }
  return NULL;
}

static void *
dolog (group_t g, const char *type, const char *user, const char *port, const char *fmt, ...)
{				// Log a message
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
{				// Simple keypad message display
  if (!k)
    return NULL;
  unsigned int n = port_device (k->port);
  char *l1 = (char *) device[n].text[0];
  char *l2 = (char *) device[n].text[1];
  device[n].cursor = 0;
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
	      device[n].beep[0] = 1;
	      device[n].beep[1] = 1;
	    }
	  else
	    {
	      device[n].beep[0] = 10;
	      device[n].beep[1] = 10;
	    }
	}
      else
	{
	  device[n].beep[0] = 10;
	  device[n].beep[1] = 0;
	}
    }
  else
    {
      device[n].beep[0] = 0;
      device[n].beep[1] = 0;
    }
  char *nl = strchr (v, '\n');
  if (nl)
    *nl++ = 0;
  snprintf (l1, 17, "%-16s", v);
  snprintf (l2, 17, "%-16s", nl ? : "");
  k->when = now.tv_sec + (device[n].silent ? 10 : 3);
  free (msg);
  return NULL;
}

static void *
keypad_login (keypad_t * k, user_t * u, const char *where)
{				// PIN or fob login
  if (u && k->user != u)
    {
      k->user = u;
      if (k->time_logout && !(state[STATE_ENGINEERING] & u->group_reset))
	k->when_logout = now.tv_sec + k->time_logout;	// No logout in engineering if we can reset
      dolog (k->groups, "LOGIN", u->name, where, "Keypad login");
      if (!alarm_unset (u->name, where, k->groups & u->group_unset))
	return keypad_message (k, "LOGGED IN\n%s", u->fullname ? : u->name ? : "");
    }
  else
    {
      if (!alarm_reset (u->name, where, k->groups & u->group_reset))
	return keypad_message (k, "CANNOT RESET!");
    }
  return NULL;
}

static void *
do_keypad_update (keypad_t * k, char key)
{				// Update keypad display / beep (key non 0 for key press).
  // Called either for a key, or when k->when passed.
  int p;
  unsigned int n = port_device (k->port);
  char *l1 = (char *) device[n].text[0];
  char *l2 = (char *) device[n].text[1];
  k->when = (now.tv_sec + 60) / 60 * 60;	// Next update default if not set below
  if (k->user && k->when_logout)
    {				// Auto logout
      if (k->when_logout <= now.tv_sec)
	{
	  k->user = 0;
	  k->when_logout = 0;
	}
      else if (key)
	k->when_logout = now.tv_sec + k->time_logout;
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
    {				// Not in use at all!
      snprintf (l1, 17, "%-16s", k->message ? : "-- NOT IN USE --");
      snprintf (l2, 17, "%02d:%02d %10s", lnow.tm_hour, lnow.tm_min, port_name (k->port));
      device[n].silent = 1;	// No keys
      return NULL;
    }
  if (key && device[n].silent)
    return keypad_message (k, "Wait");
  int s;
  group_t trigger = 0;
  for (s = 0; s < STATE_LATCHED; s++)
    if (s != STATE_FAULT)
      trigger |= state[s];
  // Status
  if (k->groups & state[STATE_ARM])
    {
      if (key >= '0' && key < '0' + MAX_GROUP)
	{			// Change groups?
	  group_t g = (((int) 1 << (int) (key - '0')) & k->groups);
	  if (g)
	    {
	      if (state[STATE_ARM] & g)
		alarm_unset (k->user ? k->user->name : k->name, port_name (k->port), g);
	      else if (!(state[STATE_SET] & g))
		alarm_arm (k->user ? k->user->name : k->name, port_name (k->port), g, 0);
	      alarm_timed (state[STATE_ARM] & g, 0);
	    }
	}
      else if (key == '\e')	// Immediate set
	alarm_unset (k->user ? k->user->name : k->name, port_name (k->port), k->groups & state[STATE_ARM]);
      else if (key == 'B')	// Part set
	{
	  alarm_unset (k->user ? k->user->name : k->name, port_name (k->port), state[STATE_ARM] & ~(k->groups & trigger));
	}
      else if (key == '\n')
	{
	  alarm_set (k->user ? k->user->name : k->name, port_name (k->port), k->groups & state[STATE_ARM]);
	  return NULL;
	}
      if (!(k->groups & state[STATE_ARM]))
	return keypad_message (k, "CANCELLED SET");	// Nothing left to set
      device[n].cursor = 0;
      if (k->groups & state[STATE_ARM] & trigger)
	{			// Not setting
	  device[n].beep[0] = 0;
	  device[n].beep[1] = 1;
	}
      else
	{			// Setting
	  device[n].beep[0] = 1;
	  device[n].beep[1] = 9;
	}
      int n, left = 99;
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
	    for (s = 0; s < STATE_TRIGGERS && (s == STATE_FAULT || s == STATE_WARNING || s == STATE_ZONE || s >= STATE_USER1 || !(state[s] & (1 << n))); s++);
	    if (alt && s < STATE_TRIGGERS)
	      {
		if (!l)
		  for (l = statelist; l && ((s != STATE_OPEN && l->type != s) || !(l->groups & (1 << n))); l = l->next);
		l2[n] = (s == STATE_OPEN ? 'X' : *state_name[s]);
	      }
	    else
	      l2[n] = n + '0';
	  }
	else
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
  device[n].silent = 0;
  // PIN entry?
  if (k->pininput || isdigit (key))
    {
      if (!key)
	k->pininput = 0;	// timed out
      else if (key == '\n' || key == '\e')
	{
	  k->pininput = 0;
	  if (key == '\n' || key == 'A')
	    {			// Login?
	      user_t *u = NULL;
	      if (k->pin)
		for (u = users; u && u->pin != k->pin; u = u->next);
	      if (!u)
		{		// PIN 0 or user not valid
		  device[n].silent = 1;
		  return keypad_message (k, "INVALID CODE");
		}
	      if (key == 'A')
		{
		  if (!alarm_arm (u->name ? : k->name, port_name (k->port), k->groups & u->group_set, 0))
		    return keypad_message (k, "CANNOT SET!");
		}
	      else
		keypad_login (k, u, port_name (k->port));
	    }
	}
      else if (isdigit (key))
	{
	  if (k->pininput < 9)
	    {
	      if (!k->pininput++)
		k->pin = 0;	// Start
	      k->pin = k->pin * 10 + key - '0';
	    }
	  for (p = 0; p < k->pininput; p++)
	    l1[p] = '*';
	  device[n].cursor = 0x40 + p;
	  for (; p < 16; p++)
	    l1[p] = ' ';
	  for (p = 0; p < 16; p++)
	    l2[p] = ' ';
	  k->when = now.tv_sec + 10;	// Timeout
	}
      if (k->pininput)
	return NULL;
    }
  if (k->user && key == '\e')
    {
      k->user = NULL;
      return keypad_message (k, "LOGGED OUT");
    }
  if (!k->user && key == 'A' && !alarm_arm (k->name, port_name (k->port), k->groups, 0))
    return keypad_message (k, "CANNOT SET");
  if (!k->user && key == 'B' && ((k->groups & trigger) || !alarm_arm (k->name, port_name (k->port), k->groups, 1)))
    return keypad_message (k, "CANNOT SET");
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
    device[n].blink = 1;
  else
    device[n].blink = 0;
  device[n].cursor = 0;
  if (k->user)
    {				// User logged in
      if (alert)
	while (*alert == '\a')
	  alert++;
      device[n].beep[0] = 0;
      device[n].beep[1] = 0;
      if (k->when_posn != now.tv_sec || k->posn < 0)
	{
	  k->when_posn = now.tv_sec;
	  k->posn++;
	}
      int n = k->posn;
      int t;
      for (t = 0; t <= STATE_WARNING; t++)
	if (k->groups & state[t])
	  {			// Active
	    k->when = now.tv_sec + 1;
	    statelist_t *s;
	    for (s = statelist; s && (!(s->groups & k->groups) || s->type != t || n--); s = s->next);
	    if (!s)
	      continue;
	    snprintf (l1, 17, "%-7s %-8s", s->port, s->name ? : "");
	    if (t == STATE_FAULT || t == STATE_TAMPER)
	      {
		port_t p = port_parse (s->port, NULL, -2);
		if ((p & 0xFF) && device[port_device (p)].type == TYPE_RIO)
		  {
		    unsigned int v = device[port_device (p)].ri[port_id (p)].resistance;
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
	  {			// Latched
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
  {				// Reset required??
    int s;
    for (s = 0; s < STATE_LATCHED && !state[STATE_TRIGGERS + s]; s++);
    if (s < STATE_LATCHED)
      {
	snprintf (l2, 17, "RESET %-10s", state_name[s]);
	if ((s != STATE_TAMPER && s != STATE_FAULT) || !(state[STATE_ENGINEERING] & state[STATE_TRIGGERS + s]))
	  {			// Beep if not engineering
	    device[n].beep[0] = 1;
	    device[n].beep[1] = 49;
	  }
	return NULL;
      }
    else			// Idle
      snprintf (l2, 17, "%04d-%02d-%02d %02d:%02d", lnow.tm_year + 1900, lnow.tm_mon + 1, lnow.tm_mday, lnow.tm_hour, lnow.tm_min);
  }
  device[n].beep[0] = 0;
  device[n].beep[1] = 0;
  return NULL;
}

#ifdef	LIBWS
xml_t
keypad_ws (xml_t root, keypad_t * k)
{				// Add keypad status to XML
  xml_t x = xml_element_add (root, "keypad");
  xml_add (x, "@id", port_name (k->port));
  unsigned int n = port_device (k->port);
  xml_add (x, "+line", (char *) device[n].text[0]);
  xml_add (x, "+line", (char *) device[n].text[1]);
  xml_addf (x, "+-beep", "%d", device[n].beep[0]);
  xml_addf (x, "+-beep", "%d", device[n].beep[1]);
  xml_addf (x, "+@-cursor", "%d", device[n].cursor);
  xml_addf (x, "+@-silent", "%s", device[n].silent ? "true" : "false");
  xml_addf (x, "+@-blink", "%s", device[n].blink ? "true" : "false");
  return x;
}
#endif

static void *
keypad_update (keypad_t * k, char key)
{				// Do keypad update, possibly with a key pressed
  void *ret = do_keypad_update (k, key);
#ifdef	LIBWS
  xml_t root = xml_tree_new (NULL);
  xml_t x = keypad_ws (root, k);
  if (key)
    xml_addf (x, "@key", "%c", key);
  websocket_send_all (root);
  xml_tree_delete (root);
#endif
  return ret;
}

void
doevent (event_t * e)
{				// Handle an event
  gettimeofday (&now, NULL);
  unsigned int id = port_device (e->port);
  unsigned char type = device[id].type;
  if (debug)
    {				// Debug logging
      if (e->event == EVENT_DOOR)
	printf ("DOOR%02d %s", e->door, door_name[e->state]);
      else if (e->event == EVENT_KEEPALIVE)
	printf ("BUS%d %s ", e->port >> 16, event_name[e->event]);
      else
	printf ("%s %s ", port_name (e->port), event_name[e->event]);
      if (e->event == EVENT_KEEPALIVE)
	printf ("%5d %5d %5d %5d %5d", e->tx, e->rx, e->errors, e->stalled, e->retries);
      if (e->event == EVENT_INPUT || e->event == EVENT_TAMPER || e->event == EVENT_FAULT)
	printf ("%02X %02X", e->changed, e->status);
      if (e->event == EVENT_KEY)
	printf ("%02X", e->key);
      if (e->event == EVENT_FOB || e->event == EVENT_FOB_HELD)
	printf (" %09u", e->fob);
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
  if (id >= MAX_DEVICE)
    {
      if (debug)
	printf ("Bad id %d\n", id);
      return;
    }
  // Handle event
  switch (e->event)
    {
    case EVENT_KEEPALIVE:
      {
	int n = (id >> 8);
	char busno[10];
	snprintf (busno, sizeof (busno), "BUS%d", n);
	dolog (groups, "KEEPALIVE", NULL, busno, "Keepalive");
	if (e->tx)
	  mybus[n].watchdog = now.tv_sec + 120;
	if (e->errors || e->stalled || !e->rx || !e->tx)
	  {
	    if (!mybus[n].fault)
	      {
		mybus[n].fault = 1;
		add_fault (groups, busno, NULL);
	      }
	    dolog (groups, "BUSERROR", NULL, busno, "Bus reports errors:%d stalled:%d retries:%d tx:%d rx:%d", e->errors, e->stalled, e->retries, e->tx, e->rx);
	  }
	else
	  {
	    if (mybus[n].fault)
	      {
		mybus[n].fault = 0;
		rem_fault (groups, busno, NULL);
	      }
	    if (e->retries)
	      syslog (LOG_INFO, "Bus %d retries: %d", n, e->retries);
	  }
      }
      break;
    case EVENT_FOUND:
      {
	dolog (groups, "BUSFOUND", NULL, port_name (e->port), "Device found on bus");
	if (mydevice[id].missing)
	  {
	    mydevice[id].missing = 0;
	    rem_tamper (groups, port_name (e->port), mydevice[id].name);
	  }
	if (type == TYPE_PAD)
	  keypad_new (e->port);
      }
      break;
    case EVENT_MISSING:
      if (!mydevice[id].missing)
	{
	  mydevice[id].missing = 1;
	  dolog (groups, "BUSMISSING", NULL, port_name (e->port), "Device missing from bus");
	  add_tamper (groups, port_name (e->port), mydevice[id].name);
	}
      break;
    case EVENT_DISABLED:
      {
	dolog (groups, "BUSDISABLED", NULL, port_name (e->port), "Device disabled on bus");
      }
      break;
    case EVENT_DOOR:
      {
	mydoor_t *d = &mydoor[e->door];
	char doorno[8];
	snprintf (doorno, sizeof (doorno), "DOOR%02u", e->door);
	char *doorname = d->name;
	if (d->fire & state[STATE_FIRE])
	  door_open (e->door);	// fire alarm override
	// Log some states
	if (e->state == DOOR_CLOSED)
	  d->opening = 1;
	else if (e->state == DOOR_OPEN)
	  d->opening = 0;
	else if (e->state == DOOR_LOCKING && d->opening)
	  dolog (d->groups, "DOORNOTOPEN", NULL, doorno, "Door was not opened");
	else if (e->state == DOOR_AJAR)
	  dolog (d->groups, "DOORAJAR", NULL, doorno, "Door ajar (lock not engaged)");
	else if (e->state == DOOR_FORCED)
	  dolog (d->groups, "DOORFORCED", NULL, doorno, "Door forced");
	else if (e->state == DOOR_TAMPER)
	  dolog (d->groups, "DOORTAMPER", NULL, doorno, "Door tamper");
	else if (e->state == DOOR_FAULT)
	  dolog (d->groups, "DOORFAULT", NULL, doorno, "Door fault");
	else if (e->state == DOOR_PROPPED)
	  dolog (d->groups, "DOORPROPPED", NULL, doorno, "Door propped");
	// Update alarm state linked to doors
	// Entry
	if (e->state == DOOR_OPEN)
	  {
	    if (!d->entry)
	      {
		d->entry = 1;
		add_entry (d->groups, doorno, doorname);
	      }
	  }
	else
	  {
	    if (d->entry)
	      {
		d->entry = 0;
		rem_entry (d->groups, doorno, doorname);
	      }
	  }
	// Intruder
	if (e->state == DOOR_FORCED || e->state == DOOR_PROPPED)
	  {
	    if (!d->intruder)
	      {
		d->intruder = 1;
		add_intruder (d->groups, doorno, doorname);
	      }
	  }
	else
	  {
	    if (d->intruder)
	      {
		d->intruder = 0;
		rem_intruder (d->groups, doorno, doorname);
	      }
	  }
	// Tamper
	if (e->state == DOOR_TAMPER)
	  {
	    if (!d->tamper)
	      {
		d->tamper = 1;
		add_tamper (d->groups, doorno, doorname);
	      }
	  }
	else
	  {
	    if (d->tamper)
	      {
		d->tamper = 0;
		rem_tamper (d->groups, doorno, doorname);
	      }
	  }
	// Warning
	if (e->state == DOOR_AJAR || e->state == DOOR_PROPPED)
	  {
	    if (!d->warning)
	      {
		d->warning = 1;
		add_warning (d->groups, doorno, doorname);
	      }
	  }
	else
	  {
	    if (d->warning)
	      {
		d->warning = 0;
		rem_warning (d->groups, doorno, doorname);
	      }
	  }
	// Fault
	if (e->state == DOOR_FORCED)
	  {
	    if (!d->fault)
	      {
		d->fault = 1;
		add_fault (d->groups, doorno, doorname);
	      }
	  }
	else
	  {
	    if (d->fault)
	      {
		d->fault = 0;
		rem_fault (d->groups, doorno, doorname);
	      }
	  }
      }
      break;
    case EVENT_INPUT:
      {
	int i;
	for (i = 0; i < MAX_INPUT && e->changed; i++)
	  if (e->changed & (1 << i))
	    {
	      int s;
	      e->changed &= ~(1 << i);
	      char *port = port_name (e->port + (1 << i));
	      char *name = mydevice[id].input[i].name ? : mydevice[id].name;
	      if ((e->status & (1 << i)))
		{		// on
		  if (walkthrough)
		    syslog (LOG_INFO, "+%s(%s)", port, name ? : "");
		  for (s = 0; s < STATE_TRIGGERS; s++)
		    add_state (mydevice[id].input[i].trigger[s], port, name, s);
		}
	      else
		{		// off
		  if (walkthrough)
		    syslog (LOG_INFO, "-%s(%s)", port, name ? : "");
		  for (s = 0; s < STATE_TRIGGERS; s++)
		    rem_state (mydevice[id].input[i].trigger[s], port, name, s);
		}
	      if (mydevice[id].input[i].isexit && (e->status & (1 << i)))
		{
		  unsigned int d, n;
		  for (d = 0; d < MAX_DOOR; d++)
		    for (n = 0; n < sizeof (mydoor[d].i_exit) / sizeof (*mydoor[d].i_exit); n++)
		      if (port_device (mydoor[d].i_exit[n]) == id)
			{
			  char doorno[8];
			  snprintf (doorno, sizeof (doorno), "DOOR%02u", d);
			  if (!door_locked (d))
			    {
			      if (mydoor[d].airlock >= 0 && door[mydoor[d].airlock].state != DOOR_LOCKED && door[mydoor[d].airlock].state != DOOR_DEADLOCKED)
				{
				  dolog (mydoor[d].groups, "DOORAIRLOCK", NULL, doorno, "Airlock violation with DOOR%02d, exit rejected", mydoor[d].airlock);
				  door_error (d);
				}
			      else if (mydoor[d].lockdown && (state[mydoor[d].lockdown] & mydoor[d].groups))
				{	// Door in lockdown
				  dolog (mydoor[d].groups, "DOORLOCKDOWN", NULL, doorno, "Lockdown violation, exit rejected");
				  door_error (d);
				}
			      else
				door_open (d);
			    }
			  else
			    {
			      dolog (mydoor[d].groups, "DOORREJECT", NULL, doorno, "Door is deadlocked, exit rejected");
			      door_error (d);
			    }
			}
		}
	    }
      }
      break;
    case EVENT_TAMPER:
      {
	int i;
	for (i = 0; i < MAX_INPUT && e->changed; i++)
	  if (e->changed & (1 << i))
	    {
	      e->changed &= ~(1 << i);
	      char *port = port_name (e->port + (1 << i));
	      char *name = mydevice[id].input[i].name ? : mydevice[id].name;
	      group_t g = 0;
	      int s;
	      for (s = 0; s < STATE_TRIGGERS; s++)
		g |= mydevice[id].input[i].trigger[s];
	      if ((e->status & (1 << i)))
		{
		  if (walkthrough)
		    syslog (LOG_INFO, "+%s(%s) Tamper", port, name ? : "");
		  add_tamper (g, port, name);
		}
	      else
		{
		  if (walkthrough)
		    syslog (LOG_INFO, "-%s(%s) Tamper", port, name ? : "");
		  rem_tamper (g, port, name);
		}
	    }
	for (; i < MAX_TAMPER && e->changed; i++)
	  if (e->changed & (1 << i))
	    {			// Device level tamper - apply if any inputs in use, etc
	      e->changed &= ~(1 << i);
	      char *port = port_name (e->port);
	      char *name = mydevice[id].name;
	      group_t g = 0;
	      int q, s;
	      if (device[id].type == TYPE_RIO)
		{
		  for (q = 0; q < MAX_INPUT; q++)
		    for (s = 0; s < STATE_TRIGGERS; s++)
		      g |= mydevice[id].input[q].trigger[s];
		  for (q = 0; q < MAX_OUTPUT; q++)
		    g |= mydevice[id].output[q].group;
		}
	      else if (device[id].type == TYPE_PAD)
		{
		  keypad_t *k;
		  for (k = keypad; k && port_device (k->port) != port_device (e->port); k = k->next);
		  if (k)
		    g = k->groups;
		}
	      else
		g = groups;
	      if ((e->status & (1 << i)))
		add_tamper (g, port, name);
	      else
		rem_tamper (g, port, name);
	    }
      }
      break;
    case EVENT_FAULT:
      {
	int i;
	for (i = 0; i < MAX_INPUT && e->changed; i++)
	  if (e->changed & (1 << i))
	    {
	      e->changed &= ~(1 << i);
	      char *port = port_name (e->port + (1 << i));
	      char *name = mydevice[id].name;
	      group_t g = 0;
	      int s;
	      for (s = 0; s < STATE_TRIGGERS; s++)
		g |= mydevice[id].input[i].trigger[s];
	      if ((e->status & (1 << i)))
		{
		  if (walkthrough)
		    syslog (LOG_INFO, "+%s(%s) Fault", port, name ? : "");
		  add_fault (g, port, name);
		}
	      else
		{
		  if (walkthrough)
		    syslog (LOG_INFO, "-%s(%s) Fault", port, name ? : "");
		  rem_fault (g, port, name);
		}
	    }
	if (device[id].type == TYPE_RIO && e->changed)
	  {
	    group_t g = 0;
	    int q, s;
	    for (q = 0; q < MAX_INPUT; q++)
	      for (s = 0; s < STATE_TRIGGERS; s++)
		g |= mydevice[id].input[q].trigger[s];
	    for (q = 0; q < MAX_OUTPUT; q++)
	      g |= mydevice[id].output[q].group;
	    if (g)
	      {
		if (e->changed & (1 << FAULT_RIO_NO_PWR))
		  {
		    char port[20];
		    snprintf (port, sizeof (port), "%sNOPWR", port_name (e->port));
		    if ((e->status & (1 << FAULT_RIO_NO_PWR)))
		      add_fault (g, port, mydevice[id].name);
		    else
		      rem_fault (g, port, mydevice[id].name);
		  }
		if (e->changed & (1 << FAULT_RIO_NO_BAT))
		  {
		    char port[20];
		    snprintf (port, sizeof (port), "%sNOBAT", port_name (e->port));
		    if ((e->status & (1 << FAULT_RIO_NO_BAT)))
		      add_fault (g, port, mydevice[id].name);
		    else
		      rem_fault (g, port, mydevice[id].name);
		  }
		if (e->changed & (1 << FAULT_RIO_BAD_BAT))
		  {
		    char port[20];
		    snprintf (port, sizeof (port), "%sBADBAT", port_name (e->port));
		    if ((e->status & (1 << FAULT_RIO_BAD_BAT)))
		      add_fault (g, port, mydevice[id].name);
		    else
		      rem_fault (g, port, mydevice[id].name);
		  }
	      }
	  }
      }
      break;
    case EVENT_FOB:
    case EVENT_FOB_HELD:
      {				// Check users, doors?
	unsigned int d, n;
	user_t *u = NULL;
	if (e->fob)
	  for (u = users; u; u = u->next)
	    {
	      for (n = 0; n < sizeof (u->fob) / sizeof (*u->fob) && u->fob[n] != e->fob; n++);
	      if (n < sizeof (u->fob) / sizeof (*u->fob))
		break;
	    }
	if (device[id].pad)
	  {			// Prox for keypad, so somewhat different
	    if (!u)
	      dolog (groups, "FOBBAD", NULL, port_name (e->port), "Unrecognised fob %lu", e->fob);
	    else
	      {
		keypad_t *k;
		for (k = keypad; k && k->prox != e->port; k = k->next);
		if (k)
		  keypad_login (k, u, port_name (e->port));
		else
		  dolog (groups, "FOBBAD", NULL, port_name (e->port), "Prox not linked to keypad, fob %lu", e->fob);
	      }
	    return;
	  }
	int found = 0;
	// We only do stuff for Max readers on doors - maybe we need some logic for stand alone max readers - or make a dummy door.
	for (d = 0; d < MAX_DOOR; d++)
	  for (n = 0; n < sizeof (mydoor[0].i_fob) / sizeof (*mydoor[0].i_fob); n++)
	    if (port_device (mydoor[d].i_fob[n]) == id)
	      {
		found++;
		char doorno[8];
		snprintf (doorno, sizeof (doorno), "DOOR%02u", d);
		if (!u)
		  {
		    door_error (d);
		    door_lock (d);	// Cancel open
		    dolog (mydoor[d].groups, "FOBBAD", NULL, doorno, "Unrecognised fob %lu", e->fob);
		  }
		else if (e->event == EVENT_FOB)
		  {
		    if (alarm_unset (u->name, port_name (e->port), u->group_set & mydoor[d].group_set & state[STATE_ARM]))	// cancel any setting
		      door_confirm (d);
		    if (alarm_unset (u->name, port_name (e->port), u->group_unset & mydoor[d].group_unset & state[STATE_SET]))	// unset
		      door_confirm (d);
		    if (door[d].state == DOOR_PROPPED)
		      {
			if (u->group_prop & mydoor[d].groups)
			  {
			    //door_confirm (d); // no as this undoes the quieting and beeping stopping should be obvious
			    door_quiet (d);
			    dolog (mydoor[d].groups, "DOORHELD", u->name, doorno, "Door prop cancelled by fob %lu", e->fob);
			  }
			else
			  {
			    dolog (mydoor[d].groups, "DOORSTILLPROPPED", u->name, doorno, "Door prop not cancelled by fob %lu as not allowed", e->fob);
			    door_error (d);
			  }
		      }
		    else if (door[d].state == DOOR_CLOSED)
		      {
			//door_confirm (d); // Beeping annoying and clear from LEDs
			door_lock (d);	// Cancel open
			dolog (mydoor[d].groups, "DOORCANCEL", u->name, doorno, "Door open cancelled by fob %lu", e->fob);
			mydoor[d].opening = 0;	// Don't report not opened
		      }
		    else
		      {
			if (u->group_open & mydoor[d].groups)
			  {
			    if (mydoor[d].groups & (state[STATE_SET] | state[STATE_ARM]))
			      {
				dolog (mydoor[d].groups, "DOORALARMED", u->name, doorno, "Door is alarmed, not opening DOOR%02d using fob %lu", d, e->fob);
				door_error (d);
			      }
			    else if (mydoor[d].airlock >= 0 && door[mydoor[d].airlock].state != DOOR_LOCKED && door[mydoor[d].airlock].state != DOOR_DEADLOCKED)
			      {
				dolog (mydoor[d].groups, "DOORAIRLOCK", u->name, doorno, "Airlock violation with DOOR%02d using fob %lu", mydoor[d].airlock, e->fob);
				door_error (d);
			      }
			    else if (mydoor[d].lockdown && (state[mydoor[d].lockdown] & mydoor[d].groups))
			      {	// Door in lockdown
				dolog (mydoor[d].groups, "DOORLOCKDOWN", u->name, doorno, "Lockdown violation with DOOR%02d using fob %lu", mydoor[d].airlock, e->fob);
				door_error (d);
			      }
			    else if (door[d].state != DOOR_OPEN && door[d].state != DOOR_UNLOCKING)
			      {	// Open
				dolog (mydoor[d].groups, "DOOROPEN", u->name, doorno, "Door open by fob %lu", e->fob);
				door_open (d);	// Open the door
			      }
			    else if (door[d].state == DOOR_OPEN)
			      dolog (mydoor[d].groups, "FOBIGNORED", u->name, doorno, "Ignored fob %lu as door open", e->fob);
			    // Other cases (unlocking) are transient and max will sometimes multiple read
			  }
			else
			  {
			    door_error (d);
			    dolog (mydoor[d].groups, "FOBBAD", u->name, doorno, "Not allowed fob %lu", e->fob);
			  }
		      }
		  }
		else if (mydoor[d].time_set)
		  {		// Held and we are allowed to set
		    group_t set = (mydoor[d].group_set & u->group_set & ~state[STATE_SET] & ~state[STATE_ARM]);
		    if (set)
		      {
			door_confirm (d);
			door_lock (d);
			alarm_arm (u->name, port_name (e->port), set, mydoor[d].time_set);
		      }
		    else
		      {
			dolog (mydoor[d].groups, "FOBHELDIGNORED", u->name, doorno, "Ignored held fob %lu as no setting options", e->fob);
			door_error (d);
		      }
		  }
		else
		  {
		    dolog (mydoor[d].groups, "FOBHELDIGNORED", u->name, doorno, "Ignored held fob %lu as door cannot set alarm", e->fob);
		    door_error (d);
		  }
	      }
	if (!found)
	  {			// Unassociated max reader
	    dolog (groups, e->event == EVENT_FOB_HELD ? "FOBHELDIGNORE" : "FOBIGNORED", u ? u->name : NULL, port_name (e->port), "Ignored fob %lu as reader not linked to a door", e->fob);
	    door_error (d);
	  }
      }
      break;
    case EVENT_KEY:
      {				// Key
	keypad_t *k;
	for (k = keypad; k && k->port != e->port; k = k->next);
	if (k)
	  keypad_update (k, e->key);
      }
      break;
    case EVENT_RF:
      {
	// TODO
      }
      break;
    }
  free ((void *) e);
}

static void
profile_check (void)
{				// Update profiles.
  int changed = 0;
  xml_t p = NULL;
  while ((p = xml_element_next_by_name (config, p, "profile")))
    {				// Scan profiles
      // TODO check dates, times, days of weeks, etc
    }
  if (changed)
    {				// Apply profiles where needed
      // TODO allow inputs to have profiles, and other things like auto setting alarms, or reporting alarm is not set when it should be
    }
}

#ifdef	LIBWS
char *
wscallback (websocket_t * w, xml_t head, xml_t data)
{
  if (!w || (head && data))
    {
      if (head)
	xml_tree_delete (head);
      if (data)
	xml_tree_delete (data);
      return "Websocket only";	// Would be nice to serve necessary for letsencrypt maybe
    }
  if (head)
    {				// New connection, authenticate
      char *er = NULL;
      if (wsauth)
	{
	  char *auth = xml_get (head, "@authorization");
	  if (!auth)
	    er = "401 Need auth";
	  else if (!wsauth || strcmp (wsauth, auth))
	    er = "403 Bad auth";
	}
      syslog (LOG_INFO, "%s Websocket %s", xml_get (head, "@IP"), er ? : "OK");
      xml_tree_delete (head);
      if (!er)
	{			// We want to send current state data to this connection
	  pthread_mutex_lock (&eventmutex);	// Avoid things changing
	  xml_t root = xml_tree_new (NULL);
	  keypad_t *k;
	  for (k = keypad; k; k = k->next)
	    keypad_ws (root, k);
	  websocket_send (1, &w, root);
	  pthread_mutex_unlock (&eventmutex);
	  xml_tree_delete (root);
	}
      return er;
    }
  if (data)
    {				// Existing connection
      // Process valid requests
      syslog (LOG_INFO, "Websocket data");
      pthread_mutex_lock (&eventmutex);	// Stop simultaneous event processing
      // TODO
      pthread_mutex_unlock (&eventmutex);
      xml_tree_delete (data);
      return NULL;
    }
  return NULL;			// Closed connection, we don't care really
}
#endif

// Main
int
main (int argc, const char *argv[])
{
  const char *configfile = NULL;
  const char *maxfrom = NULL, *maxto = NULL;
#include <trace.h>
  {
    int c;
    poptContext optCon;		// context for parsing command-line options
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
       "debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug", NULL}, POPT_AUTOHELP {NULL}
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
  gettimeofday (&now, NULL);
  bus_init ();
  pthread_mutex_init (&eventmutex, 0);
  pthread_mutex_init (&logmutex, 0);
  pipe2 (logpipe, O_NONBLOCK);	// We check queue anyway an we don't want to risk stalling if app is stalled for some reason and a lot of events
  pthread_t logthread;
  if (pthread_create (&logthread, NULL, logger, NULL))
    warn ("Bus start failed");
  load_config (configfile);
  if (!buses)
    buses = 1;			// Poll one anyway
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
  }
  if (debug)
    printf ("%s Groups found\n", group_list (groups));
#ifdef	LIBWS
  if (wsport)
    {
      const char *e = websocket_bind (wsport, wsorigin, wshost, wspath, wscertfile, wskeyfile, wscallback);
      if (e)
	errx (1, "Websocket fail: %s", e);
      syslog (LOG_INFO, "Websocket bind %s", wsport);
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
    {				// Move a max
      port_t f = port_parse (maxfrom, NULL, -1);
      port_t t = port_parse (maxto, NULL, -1);
      if (!f || device[port_device (f)].type != TYPE_MAX)
	dolog (groups, "CONFIG", NULL, NULL, "max-from invalid");
      else if (!t || device[port_device (t)].type != TYPE_MAX)
	dolog (groups, "CONFIG", NULL, NULL, "max-to invalid");
      else if ((f >> 16) != (t >> 16))
	dolog (groups, "CONFIG", NULL, NULL, "max-from and max-to on different buses");
      else
	{
	  device[port_device (f)].newid = (t >> 8);
	  device[port_device (f)].config = 1;
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
	{			// Every minute
	  lastmin = nextpoll;
	  if (commfailcount)
	    {
	      commfailcount = 0;
	      if (!commfailreported)
		{
		  commfailreported = 1;
		  add_warning (groups, "COMMS", NULL);
		}
	    }
	  else if (commfailreported)
	    {
	      commfailreported = 0;
	      rem_warning (groups, "COMMS", NULL);
	    }
	  profile_check ();
	}
      if (state[STATE_ARM])
	{			// Top level timed settings
	  int s;
	  group_t stalled = 0;	// Which have reason to restart, not intruder as covered by NONEXIT
	  for (s = 0; s < STATE_TRIGGERS; s++)
	    if (s != STATE_ZONE && s != STATE_FAULT && s != STATE_WARNING && s != STATE_OPEN && s < STATE_USER1)
	      stalled |= state[s];
	  stalled &= state[STATE_ARM];
	  if (stalled)
	    alarm_timed (stalled, 0);	// Restart timers
	  group_t set = 0, failed = 0;
	  int n;
	  for (n = 0; n < MAX_GROUP; n++)
	    if (state[STATE_ARM] & (1 << n))
	      {
		if (group[n].when_fail <= now.tv_sec)
		  failed |= (1 << n);
		else if (group[n].when_set <= now.tv_sec)
		  set |= (1 << n);
	      }
	    else
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
	{			// Entry timer
	  int n;
	  for (n = 0; n < MAX_GROUP; n++)
	    if (state[STATE_PREALARM] & (1 << n))
	      {
		if (group[n].when_alarm + group[n].entry_time < now.tv_sec)
		  {		// Entry time expired, fill alarm
		    state[STATE_PREALARM] &= ~(1 << n);
		    state[STATE_ALARM] |= (1 << n);
		    state[STATE_STROBE] |= (1 << n);
		    changed |= (1 << n);
		  }
		else if (group[n].when_alarm + group[n].entry_time < nextpoll)
		  nextpoll = group[n].when_alarm + group[n].entry_time;
	      }
	}
      if (state[STATE_ALARM])
	{			// Bell timer
	  int n;
	  for (n = 0; n < MAX_GROUP; n++)
	    if (state[STATE_ALARM] & (1 << n))
	      {
		if (group[n].when_alarm + group[n].bell_time < now.tv_sec)
		  {		// Should be off
		    if (state[STATE_BELL] & (1 << n))
		      {
			state[STATE_BELL] &= ~(1 << n);	// Bell off
			changed |= (1 << n);
		      }
		  }
		else if (group[n].when_alarm + group[n].bell_delay < now.tv_sec)
		  {		// Should be on
		    if (!(state[STATE_BELL] & (1 << n)))
		      {
			state[STATE_BELL] |= (1 << n);	// Bell on
			changed |= (1 << n);
		      }
		    if (nextpoll > group[n].when_alarm + group[n].bell_time)
		      nextpoll = group[n].when_alarm + group[n].bell_time;
		  }
		else if (nextpoll > group[n].when_alarm + group[n].bell_delay)
		  nextpoll = group[n].when_alarm + group[n].bell_delay;
	      }
	}
      {				// keypad
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
      {				// KA check
	int n;
	for (n = 0; n < MAX_BUS; n++)
	  if ((buses & (1 << n)) && mybus[n].watchdog < now.tv_sec)
	    {
	      syslog (LOG_INFO, "KA timeout bus %d (%lu)", n, now.tv_sec - mybus[n].watchdog);
	      errx (1, "KA timeout bus %d (%lu)", n, now.tv_sec - mybus[n].watchdog);
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
