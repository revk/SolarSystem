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

#include <axl.h>
#include "port.h"
#include "door.h"

// Constants

#define	US 	0x11            // Our bus ID

#undef	MAX_INPUT
#define	MAX_INPUT	8       // Max inputs per device
#define	MAX_OUTPUT	4       // Max outputs per device
#define	MAX_TAMPER	9       // Max tamper inputs
#define	MAX_FAULT	11      // Max fault reports

typedef unsigned char input_t;
typedef unsigned char output_t;
typedef unsigned short tamper_t;
typedef unsigned short fault_t;
typedef unsigned short voltage_t;

typedef char fob_t[15];         // Fob ID (text, leading zeros having been removed)

extern char *WATCHDOG;          // Watchdog device if needed (default NULL)

#define MAX_RETRY	3       // Message retry
#define	MAX_STALL	20      // Messages to same device in a row

#define	TYPE	\
	t(XXX)	\
	t(PAD)	\
	t(MAX)	\
	t(RIO)	\
	t(RFR)	\

#define t(x)	TYPE_##x,
enum
{                               // Type of device, in events and commands...
   TYPE MAX_TYPE                // How many device types we handle
};
#undef t
const char *type_name[MAX_TYPE];        // Device type name

#define	EVENT	\
	e(KEEPALIVE)	\
	e(FOUND)	\
	e(CONFIG)	\
	e(MISSING)	\
	e(DISABLED)	\
	e(INPUT)	\
	e(TAMPER)	\
	e(FAULT)	\
	e(OPEN)		\
	e(NOTOPEN)	\
	e(KEY)		\
	e(KEY_HELD)	\
	e(FOB)		\
	e(FOB_HELD)	\
	e(FOB_GONE)	\
	e(FOB_ACCESS)	\
	e(FOB_NOACCESS)	\
	e(FOB_FAIL)	\
	e(DOOR)		\
	e(RF)		\

#define e(x) EVENT_##x,
enum
{                               // Events
   EVENT EVENT_COUNT
};
#undef e

const char *event_name[EVENT_COUNT];    // Event name

enum
{                               // Max input mappings
   INPUT_MAX_OPEN,              // Door open sense
   INPUT_MAX_EXIT,              // Exit button
   INPUT_MAX_FOB,               // Keyfob present
   INPUT_MAX_FOB_HELD,          // Keyfob held
};

enum
{                               // Settings for Max
   OUTPUT_MAX_BEEP,             // Beep output
   OUTPUT_MAX_OPEN,             // Operate door lock to open
};

enum
{
   FAULT_RIO_NO_PWR = MAX_INPUT,        // Mains missing
   FAULT_RIO_NO_BAT,            // Battery missing
   FAULT_RIO_BAD_BAT,           // Battery fault
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

// Structures

typedef volatile struct device_s device_t;      // Device
typedef volatile struct event_s event_t;
typedef volatile struct keypad_data_s keypad_data_t;

struct keypad_data_s
{                               // Keypad
   unsigned char backlight:1;   // Backlight
   unsigned char blink:1;       // Blink led
   unsigned char quiet:1;       // Quiet key beeps
   unsigned char silent:1;      // Silent keybeeps
   unsigned char cross:1;       // Crossed zeros
   unsigned char beep[2];       // Beeping
   unsigned char text[2][17];   // Display (display is 16, we have 17 to allow printf and the like)
   unsigned char cursor;        // 0x80 for solid, 0x40 for underline, 0x10 for second line, 0x0P for position
};

struct device_s
{                               // Device on the bus...
   port_p port[MAX_TAMPER + 1]; // Device ports in use for event reporting (base device ID, and then inputs and extra tampers)
   input_t input;               // Bit map of inputs
   input_t inhibit;             // Inhibited inputs
   output_t output;             // Bit map of outputs
   output_t invert;             // Invert outputs
   fault_t fault;               // Bit map of faults
   tamper_t tamper;             // Bit map of tampers (device tamper is 1<<8)
   unsigned char type;          // Type of device
   unsigned char disabled:1;    // Device disabled, not polled
   unsigned char missing:1;     // Device is missing
   unsigned char found:1;       // Device has been found
   unsigned char urgent:1;      // Device needs urgent update as time critical
   union
   {                            // type specific
      struct keypad_data_s k;
      struct
      {                         // Max
         unsigned char disable:1;       // Disable reader
         unsigned char pad:1;   // Is keypad attached proxy
         unsigned char config:1;        // Set to program EEPROM to change ID (self clearing)
         unsigned char fob_hold;        // Fob hold time in 10th of a second
         unsigned char led;     // LED state
         unsigned char newid;   // New ID for Max to be configured
      };
      struct
      {                         // RIO
         input_t low;           // Low res fault or short circuit tamper, set by library
         voltage_t voltage[8];  // Reported voltages, seems 5 is battery and 6 is mains on the RIO PSU, 0 is main power on normal RIO
         struct
         {
            unsigned char response;     // Response time in 10ms units
            unsigned char threshold[5]; // Resistance thresholds (Tamper/Low/Close/High/Open/Tamper) in 100 ohm units
            unsigned short resistance;  // Resistance in ohms (0xFFFF for infinity / open circuit) set by library
         } ri[MAX_INPUT];
         struct
         {
            unsigned char invert:1;
         } ro[MAX_OUTPUT];
      };
      struct                    // RF
      {
         // TODO
      };
   };
};

struct event_s
{
   event_t *next;               // Next event in queue
   struct timeval when;
   unsigned char event;
   port_p port;                 // The device/port
   char *message;
   union
   {
      struct
      {                         // EVENT_KEEPALIVE
         int tx,
           rx,
           errors,
           stalled,
           retries;
      };
      struct
      {                         // EVENT_KEY
         char key;              // ASCII key
      };
      struct
      {                         // EVENT DOOR
         unsigned char door;    // Which door for DOOR event
         unsigned char state;   // The state of the door (or state for input/tamper/fault)
      };
      struct
      {                         // EVENT_FOB / EVENT_FOB_HELD
         fob_t fob;             // Decimal fob ID for FOB related door events
      };
      struct
      {                         // EVENT_RF
         unsigned int rfserial;
         unsigned int rfstatus;
         unsigned char rftype;  // Type
         unsigned char rfsignal;        // Out of 10
      };
   };
};

// Data
extern int debug;
extern int dump;
extern int scan;
extern device_t device[MAX_DEVICE];     // Device table
extern event_t *event,
**eventp;                       // Event queue

// Functions
void bus_init (void);
void bus_start (int bus);
void bus_stop (int bus);
event_t *bus_event (long long usec);    // Get next event, wait up to usec if none ready
void postevent (event_t * e);   // Post an event (updates input/tamper/fault on port from state)

// Access to devices for door control, etc
int device_found (int id);      // Check device exists
void device_urgent (int id);    // Mark device urgent
void device_output (int id, int port, int value);       // Set output
void device_led (int id, int led);
int device_input (int id, int port);    // Read input
