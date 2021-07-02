// Door control logic

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <malloc.h>
#include <err.h>
#include <string.h>
#include "door.h"
#include "port.h"
#include "galaxybus.h"

door_t door[MAX_DOOR] = { };

#define d(x,l) #x,
const char *door_name[DOOR_COUNT] = { DOOR };

#undef d
#define d(x,l) #l,
const char *door_led_seq[] = { DOOR };

#undef d

pthread_t doorthread;
pthread_mutex_t lockmutex;
port_output_callback_t *port_output_callback = NULL;
port_led_callback_t *port_led_callback = NULL;

static void
lock_open (volatile lock_t * l)
{				// Open a lock
  if (!l->locked)
    return;
  pthread_mutex_lock (&lockmutex);
  if (l->locked)
    {				// Is locked - start unlock
      l->locked = 0;
      l->timer = l->time_unlock;
      port_output (l->o_unlock, 1);
      port_urgent (l->o_unlock);
    }
  pthread_mutex_unlock (&lockmutex);
}

static void
lock_lock (volatile lock_t * l)
{				// Lock a lock
  if (l->locked)
    return;
  pthread_mutex_lock (&lockmutex);
  if (!l->locked)
    {				// Is unlocked - start lock
      l->locked = 1;
      l->timer = l->time_lock;
      port_output (l->o_unlock, 0);
    }
  pthread_mutex_unlock (&lockmutex);
}

static void
lock_tick (volatile lock_t * l, int open)
{
  pthread_mutex_lock (&lockmutex);
  if (l->locked)
    {				// Locked or locking
      if (open)
	l->timer = l->time_lock;	// Waiting for door to close before we start
      else if (port_defined (l->i_unlock) && !port_input (l->i_unlock))
	l->timer = 0;		// Locking completed
      else if (l->timer > 0 && !--l->timer && port_defined (l->i_unlock))
	l->timer--;		// -1 error if defined and we got to zero
    }
  else
    {				// Unlocked or unlocking
      if (port_defined (l->i_unlock) && port_input (l->i_unlock))
	l->timer = 0;		// Unlocking completed
      else if (l->timer > 0 && !--l->timer && port_defined (l->i_unlock))
	l->timer--;		// -1 error if defined and we got to zero
    }
  pthread_mutex_unlock (&lockmutex);
}

static void
door_led (int d, unsigned char state)
{
  unsigned char led = 0;
  if (state == DOOR_TAMPER)
    led = -1;
  else
    {				// Max LED pattern
      if (!door[d].mainlock.locked)
	led |= (1 << 0);
      if (door[d].mainlock.locked)
	led |= (1 << 1);
      if ((state != DOOR_OPEN && (door[d].mainlock.timer > 0 || door[d].deadlock.timer > 0)) || state == DOOR_NOTCLOSED)
	led |= (1 << 2);
      if (state == DOOR_FORCED || state == DOOR_FAULT)
	led |= (1 << 3);
      if (state == DOOR_AJAR || state == DOOR_FAULT)
	led |= (1 << 4);
      if (door[d].deadlock.locked)
	led |= (1 << 5);
    }
  unsigned int n;
  for (n = 0; n < sizeof (door[d].o_led) / sizeof (*door[d].o_led); n++)
    {
      if (port_device (door[d].o_led[n]))
	{
	  device_led (port_device (door[d].o_led[n]), led);
	  if (port_led_callback)
	    port_led_callback (door[d].o_led[n], led);
	}
      extern void mqtt_led (port_p, const char *);
      mqtt_led (door[d].o_led[n], door_led_seq[state]);
    }
}

void
door_error (int d, const unsigned char *afile)
{				// Indicate error
  afile = afile;
  door[d].blip = 5;
}

void
door_confirm (int d, const unsigned char *afile)
{				// Indicated confirmation
  afile = afile;
  door[d].blip = 10;
}

extern void mqtt_door (int, const char *, const unsigned char *afile);

void
door_access (int d, const unsigned char *afile)
{
  if (door[d].autonomous)
    {				// Send door lock control
      mqtt_door (d, "access", afile);
      return;
    }
}

void
door_open (int d, const unsigned char *afile)
{				// Unlock deadlock and lock, unless unlocked or actually open!
  if (door[d].autonomous)
    {				// Send door lock control
      mqtt_door (d, "unlock", afile);	// Send unlock even in cases that seem not locked to clear lock state if needed
      return;
    }
  if (door[d].mainlock.locked && !port_input (door[d].i_open))
    {
      if (!door[d].open_quiet)
	port_output (door[d].o_beep, door[d].beep = 1);
      if (door[d].mainlock.o_unlock)
	lock_open (&door[d].mainlock);
      lock_open (&door[d].deadlock);
    }
}

void
door_auth (int d, const unsigned char *afile)
{				// Stop beep (authorised door prop)
  if (door[d].autonomous)
    {
      mqtt_door (d, "prop", afile);
      return;
    }
  door[d].auth = 1;
}

void
door_lock (int d, const unsigned char *afile)
{				// Lock
  if (door[d].autonomous)
    {
      mqtt_door (d, "lock", afile);
      return;
    }
  if (!door[d].state)
    door[d].state = DOOR_NEW;
  lock_lock (&door[d].mainlock);
}

void
door_deadlock (int d, const unsigned char *afile)
{				// Lock and deadlock
  if (door[d].autonomous)
    {
      mqtt_door (d, "deadlock", afile);
      return;
    }
  lock_lock (&door[d].mainlock);
  lock_lock (&door[d].deadlock);
}

void
door_undeadlock (int d, const unsigned char *afile)
{				// Un deadlock
  if (door[d].autonomous)
    {
      if (door[d].state == DOOR_DEADLOCKED)
	mqtt_door (d, "lock", afile);
      return;
    }
  lock_open (&door[d].deadlock);
}

void
door_unlock (int d, const unsigned char *afile)
{				// Unlock
  if (door[d].autonomous)
    {
      mqtt_door (d, "lock", afile);
      return;
    }
  lock_open (&door[d].deadlock);
}


static void *
doorman (void *d)
{
  d = d;			// Unused
  while (1)
    {
      struct timeval tv;
      gettimeofday (&tv, NULL);
      usleep (100000 - (tv.tv_usec % 100000));
      // Scan doors
      int d;
      for (d = 0; d < MAX_DOOR; d++)
	if (door[d].state && !door[d].autonomous)
	  {
	    unsigned char state = door[d].state;	// State
	    int open = port_input (door[d].i_open);
	    pthread_mutex_lock (&lockmutex);
	    if (!open)
	      door[d].force_timer = 0;
	    else if (!door[d].deadlock.locked && door[d].mainlock.locked && !door[d].mainlock.timer && open && door[d].force_timer < door[d].time_force && ++door[d].force_timer < door[d].time_force)
	      open = 0;		// Pretend closed
	    if (state != DOOR_NEW)
	      {
		if (port_tamper (door[d].i_open) || port_tamper (door[d].mainlock.i_unlock) || port_tamper (door[d].deadlock.i_unlock))
		  state = DOOR_TAMPER;
		else if (open)
		  {		// Door is open
		    if (door[d].deadlock.timer < 0)
		      state = DOOR_FAULT;
		    else if (door[d].deadlock.locked && !door[d].deadlock.timer)
		      state = DOOR_FORCED;
		    else if (door[d].mainlock.locked && !door[d].mainlock.timer && door[d].mainlock.o_unlock)
		      state = DOOR_FORCED;
		    else if (state != DOOR_NOTCLOSED && state != DOOR_FORCED && state != DOOR_PROPPED)
		      state = DOOR_OPEN;
		    if ((state == DOOR_OPEN || state == DOOR_NOTCLOSED) && door[d].auth)
		      state = DOOR_PROPPED;
		  }
		else
		  {		// Door is closed
		    door[d].auth = 0;
		    if (door[d].deadlock.timer < 0)
		      state = DOOR_FAULT;
		    else if (door[d].deadlock.locked && !door[d].deadlock.timer)
		      state = DOOR_DEADLOCKED;
		    else if (door[d].mainlock.locked && !door[d].mainlock.timer)
		      state = DOOR_LOCKED;
		    else if (door[d].mainlock.locked && door[d].mainlock.timer < 0)
		      state = DOOR_AJAR;
		    else if (door[d].mainlock.locked && door[d].mainlock.timer > 0)
		      state = DOOR_LOCKING;
		    else if (!door[d].mainlock.locked && door[d].mainlock.timer > 0)
		      state = DOOR_UNLOCKING;
		    else
		      state = DOOR_UNLOCKED;
		  }
	      }
	    pthread_mutex_unlock (&lockmutex);
	    if (open)
	      door_lock (d, NULL);
	    lock_tick (&door[d].mainlock, open);
	    lock_tick (&door[d].deadlock, open);
	    if (state == door[d].state)
	      {			// State unchanged - timer for unstable states
		if (!++door[d].timer)
		  door[d].timer--;
		if (state == DOOR_NEW && (door[d].timer > 20 || (!port_defined (door[d].i_open) || port_found (door[d].i_open))))
		  {
		    door[d].mainlock.timer = 10;
		    door[d].deadlock.timer = 10;
		    state = (open ? DOOR_UNLOCKING : DOOR_LOCKING);
		  }
		else if (state == DOOR_OPEN && door[d].time_prop && door[d].timer >= door[d].time_prop && door[d].mainlock.o_unlock)
		  state = DOOR_NOTCLOSED;
		else if (state == DOOR_UNLOCKED && door[d].time_open && door[d].timer >= door[d].time_open)
		  {
		    door_lock (d, NULL);
		    state = DOOR_LOCKING;
		  }
	      }
	    if (state != door[d].state)
	      {			// State change
		door[d].state = state;
		door[d].timer = 0;
		if (!door[d].mainlock.locked && !door[d].open_quiet)
		  door[d].beep = 1;
		else if (state == DOOR_TAMPER || state == DOOR_FORCED || state == DOOR_NOTCLOSED || state == DOOR_AJAR)
		  door[d].beep = 2;
		else
		  door[d].beep = 0;
		event_t *e = malloc (sizeof (event_t));
		if (!e)
		  errx (1, "malloc");
		memset ((void *) e, 0, sizeof (*e));
		gettimeofday ((struct timeval *) &e->when, NULL);
		e->door = d;
		e->state = state;
		e->event = EVENT_DOOR;
		postevent (e);
	      }
	    if (!door[d].blip)
	      {
		port_output (door[d].o_beep, (door[d].beep == 1 || (door[d].beep && tv.tv_usec >= 500000)) ? 1 : 0);
		door_led (d, state);	// LED
	      }
	    else
	      {			// Override led/beep
		door[d].blip--;
		if (door[d].blip == 3 || door[d].blip == 9)
		  {		// Start of blip
		    door_led (d, DOOR_BLIP);
		    port_output (door[d].o_beep, 1);
		    port_urgent (door[d].o_beep);
		  }
		else if (!door[d].blip || door[d].blip == 6)
		  {		// End of blip - put back to normal
		    door_led (d, state);
		    port_output (door[d].o_beep, door[d].blip ? 0 : (door[d].beep == 1 || (door[d].beep && tv.tv_usec >= 500000)) ? 1 : 0);
		    port_urgent (door[d].o_beep);
		  }
	      }
	  }
    }
}

void
door_start (void)
{
  pthread_mutex_init (&lockmutex, 0);
  pthread_attr_t a;
  struct sched_param s = {
    0
  };
  s.sched_priority = 10;
  pthread_attr_init (&a);
  pthread_attr_setschedparam (&a, &s);
  pthread_attr_setschedpolicy (&a, SCHED_RR);
  if (pthread_create (&doorthread, &a, doorman, NULL))
    warn ("Bus start failed");
  pthread_attr_destroy (&a);
}
