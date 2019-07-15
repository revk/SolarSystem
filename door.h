// Door control

#ifndef DOOR_H
#define DOOR_H

#include "port.h"

#define MAX_DOOR        30      // Maximum doors

#define DOOR    \
        d(UNUSED,)       \
        d(NEW,)  \
	d(OFFLINE,) \
        d(DEADLOCKED,) \
        d(LOCKED,R) \
        d(UNLOCKING,--R) \
        d(UNLOCKED,--G) \
        d(OPEN,G) \
        d(CLOSED,--G) \
        d(LOCKING,RR-) \
        d(PROPPED,GGG-) \
        d(NOTCLOSED,R-) \
        d(FORCED,RG) \
        d(AJAR,RG-) \
        d(FAULT,RGR-)\
        d(TAMPER,RGRG-) \
	d(BLIP,--------R) \

#define d(x,l) DOOR_##x,
enum
{
   DOOR DOOR_COUNT
};
#undef d
const char *door_name[DOOR_COUNT];

typedef volatile struct door_s door_t;

typedef struct lock_s lock_t;
struct lock_s
{
   // Config
   port_p o_unlock;             // Output active to unlock
   port_p i_unlock;             // Input, active if unlocked
   unsigned char time_lock;     // How long to lock
   unsigned char time_unlock;   // How long to unlock
   // Status
   unsigned char locked:1;      // We want locked
   // Times in 10th of a second
   signed char timer;           // Timer, +ve for waiting to lock/unlock, 0 for lock/unlock done, -1 for lock/unlock fault
};

struct door_s
{                               // general door object
   lock_t mainlock;             // Main lock
   lock_t deadlock;             // Deadlock for when alarm set
   port_p o_led[2];             // Max readers on which to show LED status. port is mask of LEDs we se
   port_p o_beep[2];            // Outputs for beep
   port_p i_open;               // Input for open
   unsigned char open_quiet:1;  // Don't beep on opening, just use LEDs
   // Times in 10th of a second
   unsigned int time_open;      // Time to allow for door to be opened once unlocked
   unsigned int time_force;     // Time for door to be unexpectedly open before counts as forced
   unsigned int time_prop;      // Time for door to be expectedly open before considered propped
   // Status - managed by library
   unsigned int timer;          // Ticks in current state
   unsigned int force_timer;    // Ticks delayed force state
   unsigned char autonomous;	// Autonomous operation setting
   unsigned char state;         // Door state as last reported
   unsigned char blip;          // Blip counter
   unsigned char beep:2;        // Beep required (1=continuous, 2=intermittent)
   unsigned char auth:1;        // Auth propped
};

// A port ID is used for several purposes
// A zero is invalid.
// Reference to a device on a bus is (busid<<16)+(deviceid<<8)
// Reference to a port on a device on a bus is (busid<<16)+(deviceid<<8)+(1<<port)
// Reference to an RF device is 0x10000000+(serialno<<8) optionally +(1<<port)
typedef void port_output_callback_t (port_p);
extern port_output_callback_t *port_output_callback;

typedef void port_led_callback_t (port_p,unsigned char led);
extern port_led_callback_t *port_led_callback;

extern door_t door[MAX_DOOR];   // Door table

void door_start (void);


#endif
