// RS485 polling library for Honeywell Galaxy stuff
//

    /*
       A complete alarm panel using devices compatible with Honeywell/Galaxy RS485 buses
       Copyright (C) 2017  RevK

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

// Constants

#define	US 	0x11		// Our bus ID

#define	MAX_BUS	10		// Maximum buses
#define	MAX_DEVICE (MAX_BUS*256)	// 256 addresses per bus

#define	MAX_DOOR	30	// Maximum doors

#undef	MAX_INPUT
#define	MAX_INPUT	8	// Max inputs per device
#define	MAX_OUTPUT	4	// Max outputs per device
#define	MAX_TAMPER	9	// Max tamper inputs
#define	MAX_FAULT	11	// Max fault reports

typedef unsigned char input_t;
typedef unsigned char output_t;
typedef unsigned short tamper_t;
typedef unsigned short fault_t;
typedef unsigned short voltage_t;

typedef unsigned int fob_t;	// Fob number

#define MAX_RETRY	3	// Message retry
#define	MAX_STALL	20	// Messages to same device in a row

#define	TYPE	\
	t(XXX)	\
	t(PAD)	\
	t(MAX)	\
	t(RIO)	\
	t(RFR)	\

#define t(x)	TYPE_##x,
enum
{				// Type of device, in events and commands...
  TYPE MAX_TYPE			// How many device types we handle
};
#undef t
const char *type_name[MAX_TYPE];	// Device type name

#define	EVENT	\
	e(KEEPALIVE)	\
	e(FOUND)	\
	e(MISSING)	\
	e(DISABLED)	\
	e(INPUT)	\
	e(TAMPER)	\
	e(FAULT)	\
	e(KEY)		\
	e(FOB)		\
	e(FOB_HELD)	\
	e(DOOR)		\
	e(RF)		\

#define e(x) EVENT_##x,
enum
{				// Events
  EVENT EVENT_COUNT
};
#undef e

const char *event_name[EVENT_COUNT];	// Event name

#define	DOOR	\
	d(UNUSED)	\
	d(NEW)	\
	d(DEADLOCKED) \
	d(LOCKED) \
	d(UNLOCKING) \
	d(CLOSED) \
	d(OPEN) \
	d(LOCKING) \
	d(PROPPED) \
	d(FORCED) \
	d(AJAR)	\
	d(FAULT)\
	d(TAMPER) \

#define d(x) DOOR_##x,
enum
{
  DOOR DOOR_COUNT
};
#undef d
const char *door_name[DOOR_COUNT];

enum
{				// Max input mappings
  INPUT_MAX_OPEN,		// Door open sense
  INPUT_MAX_EXIT,		// Exit button
  INPUT_MAX_FOB,		// Keyfob present
  INPUT_MAX_FOB_HELD,		// Keyfob held
};

enum
{				// Settings for Max
  OUTPUT_MAX_BEEP,		// Beep output
  OUTPUT_MAX_OPEN,		// Operate door lock to open
};

enum
{
  FAULT_RIO_NO_PWR = MAX_INPUT,	// Mains missing
  FAULT_RIO_NO_BAT,		// Battery missing
  FAULT_RIO_BAD_BAT,		// Battery fault
};

// RIO presets
typedef struct rio_threshold_s rio_threshold_t;
struct rio_threshold_s
{
	const char *name;
	// Resistance, upper for each bank, in 100 ohm multiples.
	unsigned char tampersc;
	unsigned char lowres;
	unsigned char normal;
	unsigned char highres;
	unsigned char open;
};
const rio_threshold_t rio_thresholds[3];

// A port ID is used for several purposes
// A zero is invalid.
// Reference to a device on a bus is (busid<<16)+(deviceid<<8)
// Reference to a port on a device on a bus is (busid<<16)+(deviceid<<8)+(1<<port)
// Reference to an RF device is 0x10000000+(serialno<<8) optionally +(1<<port)
typedef unsigned int port_t;

// Structures

typedef volatile struct device_s device_t;	// Device
typedef volatile struct event_s event_t;
typedef volatile struct door_s door_t;

struct device_s
{				// Device on the bus...
  input_t input;		// Bit map of inputs
  input_t inhibit;		// Inhibited inputs
  output_t output;		// Bit map of outputs
  output_t invert;		// Invert outputs
  fault_t fault;		// Bit map of faults
  tamper_t tamper;		// Bit map of tampers (device tamper is 1<<8)
  unsigned char type;		// Type of device
  unsigned char disabled:1;	// Device disabled, not polled
  unsigned char missing:1;	// Device is missing
  unsigned char found:1;	// Device has been found
  unsigned char urgent:1;	// Device needs urgent update as time critical
  union
  {				// type specific
    struct
    {				// Keypad
      unsigned char backlight:1;	// Backlight
      unsigned char blink:1;	// Blink led
      unsigned char quiet:1;	// Quiet key beeps
      unsigned char silent:1;	// Silent keybeeps
      unsigned char cross:1;	// Crossed zeros
      unsigned char beep[2];	// Beeping
      unsigned char text[2][17];	// Display (display is 16, we have 17 to allow printf and the like)
      unsigned char cursor;	// 0x80 for solid, 0x40 for underline, 0x10 for second line, 0x0P for position
    };
    struct
    {				// Max
      unsigned char disable:1;	// Disable reader
      unsigned char pad:1;	// Is keypad attached proxy
      unsigned char config:1;	// Set to program EEPROM to change ID (self clearing)
      unsigned char fob_hold;	// Fob hold time in 10th of a second
      unsigned char led;	// LED state
      unsigned char newid;	// New ID for Max to be configured
    };
    struct
    {				// RIO
      input_t low;		// Low res fault or short circuit tamper, set by library
      voltage_t voltage[8];	// Reported voltages, seems 5 is battery and 6 is mains on the RIO PSU, 0 is main power on normal RIO
      struct
      {
	unsigned char response;	// Response time in 10ms units
	unsigned char threshold[5];	// Resistance thresholds (Tamper/Low/Close/High/Open/Tamper) in 100 ohm units
	unsigned short resistance;	// Resistance in ohms (0xFFFF for infinity / open circuit) set by library
      } ri[MAX_INPUT];
      struct
      {
	unsigned char invert:1;
      } ro[MAX_OUTPUT];
    };
    struct			// RF
    {
      // TODO
    };
  };
};

struct event_s
{
  event_t *next;		// Next event in queue
  struct timeval when;
  unsigned char event;
  port_t port;			// The device/port
  union
  {
    struct
    {				// EVENT_INPUT or EVENT_TAMPER or EVENT_FAULT
      unsigned short status;
      unsigned short changed;
    };
    struct
    {				// EVENT_KEEPALIVE
      int tx, rx, errors, stalled, retries;
    };
    struct
    {				// EVENT_KEY
      char key;			// ASCII key
    };
    struct
    {				// EVENT DOOR
      unsigned char door;	// Which door for DOOR event
      unsigned char state;	// The state of the door
    };
    struct
    {				// EVENT_FOB / EVENT_FOB_HELD
      fob_t fob;		// Decimal fob ID for FOB related door events
    };
    struct
    {				// EVENT_RF
      unsigned int rfserial;
      unsigned int rfstatus;
      unsigned char rftype;     // Type
      unsigned char rfsignal;	// Out of 10
    };
  };
};

typedef struct lock_s lock_t;
struct lock_s
{
  // Config
  port_t o_lock;		// Output active to lock
  port_t i_unlock;		// Input, active if unlocked
  unsigned char time_lock;	// How long to lock
  unsigned char time_unlock;	// How long to unlock
  // Status
  unsigned char locked:1;	// We want locked
  // Times in 10th of a second
  signed char timer;		// Timer, +ve for waiting to lock/unlock, 0 for lock/unlock done, -1 for lock/unlock fault
};

struct door_s
{				// general door object
  lock_t mainlock;		// Main lock
  lock_t deadlock;		// Deadlock for when alarm set
  port_t o_led[2];		// Max readers on which to show LED status. port is mask of LEDs we se
  port_t o_beep[2];		// Outputs for beep
  port_t i_open;		// Input for open
  unsigned char open_quiet:1;	// Don't beep on opening, just use LEDs
  // Times in 10th of a second
  unsigned int time_open;	// Time to allow for door to be opened once unlocked
  unsigned int time_force;	// Time for door to be unexpectedly open before counts as forced
  unsigned int time_prop;	// Time for door to be expectedly open before considered propped
  // Status - managed by library
  unsigned int timer;		// Ticks in current state
  unsigned int force_timer;	// Ticks delayed force state
  unsigned char state;		// Door state as last reported
  unsigned char blip;		// Blip counter
  unsigned char beep:1;		// Beep required
};

// Data
extern int debug;
extern int dump;
extern int scan;
extern device_t device[MAX_DEVICE];	// Device table
extern door_t door[MAX_DOOR];	// Door table
extern event_t *event, **eventp;	// Event queue

// Functions
#define port_device(w) ((w)>>8)
#define port_output(w,v) port_output_n((volatile port_t *)&(w),sizeof(w)/sizeof(port_t),v)
void port_output_n (volatile port_t * w, int n, int v);
#define port_urgent(w) port_urgent_n((volatile port_t *)&(w),sizeof(w)/sizeof(port_t))
void port_urgent_n (volatile port_t * w, int n);
void bus_init (void);
void bus_start (int bus);
void bus_stop (int bus);
void door_error (int d);	// Indicate error
void door_confirm (int d);	// Indicate confirmation
void door_open (int d);		// Open the door
void door_quiet (int d);	// Stop beep for now
void door_lock (int d);		// Lock the door
void door_deadlock (int d);	// Deadlock the door
void door_unlock (int d);	// Un deadlock the door
event_t *bus_event (long long usec);	// Get next event, wait up to usec if none ready
