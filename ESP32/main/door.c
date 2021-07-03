// Solar System
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "door";
#include "SS.h"
const char *door_fault = NULL;
const char *door_tamper = NULL;

#include "desfireaes.h"

// Autonomous door control
// Door mode set by door setting
// 0 - no control
// 1 - tracking and reporting state, managing lock when closed
// 2 - also handling exit button (if not deadlocked)
// 3 - also handling entry for secure card use
// 4 - stand alone control using secure card
// This uses pre-set input and output numbers
#define IEXIT1 1
#define IEXIT2 8
#define IOPEN 2
#define IUNLOCK 3
#define OUNLOCK 1
#define OBEEP 3
#define OERROR 4

#define FILE3			// Check for old file 3 access file
#define MINAFILE  32		// Minimum we read from afile before checking length

#include "nfc.h"
#include "input.h"
#include "output.h"
#include "door.h"

uint8_t afile[256];		// Access file saved

#define settings  \
  u8(doorauto,0);   \
  u32(doorunlock,1000); \
  u32(doorlock,3000); \
  u32(dooropen,5000); \
  u32(doorclose,500); \
  u32(doorprop,60000); \
  u32(doorexit,60000); \
  u32(doorpoll,100); \
  u32(doordebounce,100); \
  u1(doordebug); \
  u1(doorsilent); \
  ta(fallback,10); \
  ta(blacklist,10); \
  s(iotopen)	\
  s(iotclose)	\
  s(iotarm)	\
  s(iotdisarm)	\

#define u32(n,d) uint32_t n;
#define u16(n,d) uint16_t n;
#define u8(n,d) uint8_t n;
#define u1(n) uint8_t n;
#define ta(n,c) const char*n[c]={};
#define area(n) area_t n;
#define s(n) char *n;
settings
#undef ta
#undef u32
#undef u16
#undef u8
#undef u1
#undef area
#undef s
#define lock_states \
  l(LOCKING) \
  l(LOCKED) \
  l(UNLOCKING) \
  l(UNLOCKED) \
  l(LOCKFAIL) \
  l(UNLOCKFAIL) \
  l(FORCED) \
  l(FAULT) \

#define door_states \
  d(DEADLOCKED,R) \
  d(LOCKED,5RR+A) \
  d(UNLOCKING,-R+A) \
  d(UNLOCKED,-G) \
  d(OPEN,G) \
  d(CLOSED,-A) \
  d(LOCKING,-G+A) \
  d(NOTCLOSED,RAGA) \
  d(PROPPED,G+A4G) \
  d(AJAR,R+A-G+A) \

#define l(n) LOCK_##n,
  enum
{
  lock_states
};
#undef l

#define l(n) #n,
const char *lockstates[] = {
  lock_states
};

#undef l

#define d(n,l) DOOR_##n,
enum
{
  door_states
};
#undef d

#define d(n,l) #n,
const char *doorstates[] = {
  door_states
};

#undef d

#define d(n,l) #l,
char *doorled[] = {
  door_states
};

#undef d

struct
{
  uint8_t o, i;
  uint8_t state;
  int64_t timeout;
} lock[2] = {
  0
};

uint8_t doorstate = -1;
uint8_t doordeadlock = true;
const char *doorwhy = NULL;

const char *
door_access (const uint8_t * a)
{				// Confirm access
  if (!a)
    return "";			// No action
  if (*a == *afile && !memcmp (a + 1, afile + 1, *afile))
    return "";			// Same
  if (!df.keylen)
    return "";			// Not logged in
  xSemaphoreTake (nfc_mutex, portMAX_DELAY);
  memcpy (afile, a, *a + 1);
  const char *e = df_write_data (&df, 0x0A, 'B', DF_MODE_CMAC, 0, *afile + 1, afile);
  if (!e)
    e = df_commit (&df);
  xSemaphoreGive (nfc_mutex);
  if (!e)
    {
      nfc_retry ();
      return "";
    }
  return e;
}

const char *
door_unlock (const uint8_t * a, const char *why)
{				// Unlock the door - i.e. exit button, entry allowed, etc.
  if (why && !doorwhy)
    doorwhy = why;
  ESP_LOGI (TAG, "Unlock %s", why ? : "?");
  doordeadlock = false;
  output_set (OUNLOCK + 0, 1);
  output_set (OUNLOCK + 1, 1);
  return door_access (a);
}

const char *
door_deadlock (const uint8_t * a)
{				// Deadlock the door - i.e. move to alarm armed, no exit/entry
  doordeadlock = true;
  output_set (OUNLOCK + 0, 0);
  output_set (OUNLOCK + 1, 0);
  return door_access (a);
}

const char *
door_lock (const uint8_t * a)
{				// Lock the door - i.e. move to normal locked operation
  doordeadlock = false;
  output_set (OUNLOCK + 0, 0);
  output_set (OUNLOCK + 1, 1);
  return door_access (a);
}

const char *
door_prop (const uint8_t * a)
{				// Allow door propping
  if (doorstate != DOOR_OPEN && doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED)
    return false;
  doorstate = DOOR_PROPPED;
  return door_access (a);
}

const char *
door_fob (fob_t * fob)
{				// Consider fob - sets details of action in the fob object
  if (!fob || doorauto < 3 || fob->fail || fob->deny)
    return NULL;		// Do nothing
  // Check the card security
  time_t now = time (0);
  uint8_t datetime[7];		// BCD date time
  int xoff = 0, xlen = 0, xdays = 0;	// Expiry data
  const char *afilecheck (void)
  {				// Read Afile
    fob->checked = 1;
    const char *e = df_read_data (&df, 0x0A, DF_MODE_CMAC, 0, MINAFILE, afile);	// Initial
    if (!e && *afile + 1 > MINAFILE)
      e = df_read_data (&df, 0x0A, DF_MODE_CMAC, MINAFILE, *afile + 1 - MINAFILE, afile + MINAFILE);	// More data
    if (e)
      {
	if (!strstr (e, "TIMEOUT"))
	  fob->fail = e;
	return "Cannot access Afile";
      }
    fob->afile = 1;
    fob->crc = df_crc (*afile, afile + 1);
    // Check access file (expected to exist)
    if (*afile)
      {				// Check access
	uint8_t *p = afile + 1, *e = afile + 1 + *afile;
	uint8_t *fok = NULL, *tok = NULL;
	uint8_t dow = bcdlocaltime (now, datetime);
	while (p < e)
	  {			// Scan the afile
	    uint8_t l = (*p & 0xF);
	    uint8_t c = (*p++ >> 4);
	    if (c != 0xF && p + l > e)
	      return "Invalid access file";
	    if (c == 0xF)
	      {
		switch (l)
		  {
		  case 0x0:
		    fob->commit = 1;
		    break;
		  case 0x1:
		    fob->log = 1;
		    break;
		  case 0x2:
		    fob->count = 1;
		    break;
		  case 0xA:
		    fob->armlate = 1;
		    break;
		  case 0xB:
		    fob->block = 1;
		    break;
		  case 0xC:
		    fob->clock = 1;
		    break;
		  case 0xF:
		    fob->override = 1;
		    break;
		  default:
		    return "Unknown flag";
		  }
		l = 0;		// Flag, so no length
	      }
	    else if (c == 0x0)
	      {			// Padding
		if (!l)
		  break;	// end of file
	      }
	    else if (c == 0xA)
	      {			// Arm
		fob->arm = 0;
		for (int q = 0; q < l; q++)
		  fob->arm |= (p[q] << (24 - q * 8));
		fob->armset = 1;
	      }
	    else if (c == 0xD)
	      {			// Disarm
		fob->disarm = 0;
		for (int q = 0; q < l; q++)
		  fob->disarm |= (p[q] << (24 - q * 8));
		fob->disarmset = 1;
	      }
	    else if (c == 0xE)
	      {			// Enter
		fob->enter = 0;
		for (int q = 0; q < l; q++)
		  fob->enter |= (p[q] << (24 - q * 8));
		fob->enterset = 1;
	      }
	    else if (c == 0x1)
	      {			// From
		if (fok)
		  return "Duplicate from time";
		if (l == 2)
		  fok = p;
		else if (l == 4)
		  fok = p + ((dow && dow < 6) ? 2 : 0);
		else if (l == 6)
		  fok = p + (!dow ? 0 : dow < 6 ? 2 : 4);
		else if (l == 8)
		  fok = p + (!dow ? 0 : dow < 5 ? 2 : dow < 6 ? 4 : 6);
		else if (l == 14)
		  fok = p + dow * 2;
		else
		  return "Bad from time";	// Bad time
	      }
	    else if (c == 0x2)
	      {			// To
		if (tok)
		  return "Duplicate to time";
		if (l == 2)
		  tok = p;
		else if (l == 4)
		  tok = p + ((dow && dow < 6) ? 2 : 0);
		else if (l == 6)
		  tok = p + (!dow ? 0 : dow < 6 ? 2 : 4);
		else if (l == 8)
		  tok = p + (!dow ? 0 : dow < 5 ? 2 : dow < 6 ? 4 : 6);
		else if (l == 14)
		  tok = p + dow * 2;
		else
		  return "Bad to time";
	      }
	    else if (c == 0x3)
	      {			// Expiry
		if (l == 1)
		  xdays = *p;
		else
		  {
		    xoff = p - afile;
		    xlen = l;
		    if (memcmp (datetime, p, l) > 0)
		      return "Expired";	// expired
		  }
	      }
	    else
	      return "Unknown access code";	// Unknown access code
	    p += l;
	  }
	if (xoff)
	  {
	    if (*datetime < 0x20)
	      {			// Clock not set
		if (!fob->clock)
		  return "Date not set";
	      }
	    else if (memcmp (datetime, afile + xoff, xlen) > 0)
	      return "Expired";	// expired
	  }
	if (fok || tok)
	  {			// Time check
	    const char *e = NULL;
	    if (*datetime < 0x20)
	      {			// Clock not set
		if (!fob->clock)
		  e = "*Time not set";
	      }
	    else if (fok && tok && memcmp (fok, tok, 2) > 0)
	      {			// reverse
		if (memcmp (datetime + 4, fok, 2) < 0 && memcmp (datetime + 4, tok, 2) >= 0)
		  e = "*Outside time";
	      }
	    else
	      {
		if (fok && memcmp (datetime + 4, fok, 2) < 0)
		  e = "*Too soon";
		if (tok && memcmp (datetime + 4, tok, 2) >= 0)
		  e = "*Too late";
	      }
	    if (e)
	      {
		if (fob->armlate && fob->held && (!fob->armset || !(area & ~fob->arm)))
		  {
		    fob->armok = 1;
		    return e;
		  }
		return e + 1;	// Without the *
	      }
	  }
      }
    if (fob->block)
      return "Card blocked";
    if (fob->held && fob->disarmset && !(area & ~fob->disarm))
      fob->disarmok = 1;
    else if (!fob->held && (!fob->armset || !(area & ~fob->arm)))
      fob->armok = 1;
    if (doordeadlock && (doorauto < 5 || !fob->disarmok))
      return "Deadlocked";
    if (!fob->enterset || !(area & ~fob->enter))
      fob->unlockok = 1;
    return NULL;
  }
  if (fob->secure && df.keylen)
    fob->deny = afilecheck ();
  // Check fallback
  for (int i = 0; i < sizeof (fallback) / sizeof (*fallback); i++)
    if (!strcmp (fallback[i], fob->id))
      {
	fob->fallback = 1;
	fob->unlockok = 1;
	break;
      }
  // Check blacklist
  if (*fob->id)
    for (int i = 0; i < sizeof (blacklist) / sizeof (*blacklist); i++)
      if (!strcmp (blacklist[i], fob->id))
	{
	  fob->blacklist = 1;
	  fob->unlockok = 0;
	  fob->disarmok = 0;
	  fob->armok = 0;
	  if (!fob->deny)
	    fob->deny = "Blacklist";
	  if (fob->secure && df.keylen)
	    {
	      if (*afile != 1 || afile[1] != 0xFB)
		{
		  *afile = 1;
		  afile[1] = 0xFB;	// Blocked
		  fob->crc = df_crc (*afile, afile + 1);
		  const char *e = df_write_data (&df, 0x0A, 'B', DF_MODE_CMAC, 0, *afile + 1, afile);
		  if (!e)
		    fob->updated = 1;
		}
	    }
	  break;
	}
  if (doorauto >= 4)
    {				// Actually do the doors (the open is done by the caller)
      fob->unlocked = fob->unlockok;
      if (doorauto >= 5)
	{
	  fob->disarmed = fob->disarmok;
	  fob->armed = fob->armok;
	}
    }
  if (fob->held)
    return NULL;
  if (!fob->deny && fob->secure && df.keylen && *datetime >= 0x20 && xdays && xoff && xlen <= 7)
    {				// Update expiry
      now += 86400LL * (int64_t) xdays;
      bcdutctime (now, datetime);
      if (memcmp (datetime, afile + xoff, xlen) > 0)
	{			// Changed expiry
	  memcpy (afile + xoff, datetime, xlen);
	  fob->crc = df_crc (*afile, afile + 1);
	  if (!df_write_data (&df, 0x0A, 'B', DF_MODE_CMAC, xoff, xlen, datetime))
	    fob->updated = 1;
	  // We don't really care if this fails, as we get a chance later, this means the access is allowed so will log and commit later
	}
    }

  return NULL;
}

static uint8_t resend;

const char *
door_command (const char *tag, jo_t j)
{				// Called for incoming MQTT messages
  if (!doorauto)
    return false;		// No door control in operation
  const char *e = NULL;
  char temp[256];
  const uint8_t *afile = NULL;
  if (j)
    {
      if (jo_here (j) == JO_STRING)
	{
	  int len = jo_strncpy16 (j, temp, sizeof (temp));
	  if (len < 0)
	    e = "Bad hex";
	  else if (len > 0 && *temp == len - 1)
	    afile = (uint8_t *) temp;
	  else
	    e = "Bad afile";
	}
      else
	e = "Expecting JSON string";
      if (!e)
	e = jo_error (j, NULL);
    }
  if (!strcasecmp (tag, "deadlock"))
    return e ? : door_deadlock (afile);
  if (!strcasecmp (tag, "lock"))
    return e ? : door_lock (afile);
  if (!strcasecmp (tag, "unlock"))
    return e ? : door_unlock (afile, "remote");
  if (!strcasecmp (tag, "prop"))
    return e ? : door_prop (afile);
  if (!strcasecmp (tag, "access"))
    return e ? : door_access (afile);
  if (!strcasecmp (tag, "connect"))
    resend = 1;
  return NULL;
}

static void
task (void *pvParameters)
{				// Main RevK task
  sleep (1);
  esp_task_wdt_add (NULL);
  pvParameters = pvParameters;
  if (input_get (IOPEN))
    {
      doorstate = DOOR_OPEN;
      output_set (OUNLOCK + 0, 1);	// Start with unlocked doors
      output_set (OUNLOCK + 1, 1);
    }
  else
    {
      int64_t now = esp_timer_get_time ();
      doorstate = DOOR_LOCKING;
      output_set (OUNLOCK + 0, 0);	// Start with locked doors
      output_set (OUNLOCK + 1, 0);
      lock[0].timeout = now + (int64_t) doorlock *1000LL;
      lock[1].timeout = now + (int64_t) doorlock *1000LL;
    }
  while (1)
    {
      esp_task_wdt_reset ();
      usleep (1000);		// ms
      int64_t now = esp_timer_get_time ();
      static uint64_t doornext = 0;
      static uint8_t lastdoorstate = -1;
      uint8_t iopen = input_get (IOPEN);
      if (doornext < now)
	{
	  uint8_t force = resend;
	  resend = 0;
	  doornext = now + (int64_t) doorpoll *1000LL;
	  {			// Check locks
	    int l;
	    for (l = 0; l < 2; l++)
	      {
		uint8_t last = lock[l].state;
		uint8_t o = output_get (OUNLOCK + l), i = input_get (IUNLOCK + l);
		if (!output_active (OUNLOCK + l))
		  {
		    if (!input_active (IUNLOCK + l))
		      lock[l].state = (o ? LOCK_UNLOCKED : LOCK_LOCKED);	// No input or output, just track output
		    else
		      lock[l].state = (i ? LOCK_UNLOCKED : LOCK_LOCKED);	// No output, just track input
		  }
		else
		  {		// Lock state tracking
		    if (((iopen && last == LOCK_LOCKING) || lock[l].o) && !o)
		      {		// Change to lock - timer constantly restarted if door is open as it will not actually engage
			lock[l].timeout = now + (int64_t) doorlock *1000LL;
			lock[l].state = LOCK_LOCKING;
		      }
		    else if (o && !lock[l].o)
		      {		// Change to unlock
			lock[l].timeout = now + (int64_t) doorunlock *1000LL;
			lock[l].state = LOCK_UNLOCKING;
		      }
		    if (lock[l].timeout)
		      {		// Timeout running
			if (lock[l].i != i)
			  lock[l].timeout = now + (int64_t) doordebounce *1000LL;	// Allow some debounce before ending timeout
			if (lock[l].timeout <= now)
			  {	// End of timeout
			    lock[l].timeout = 0;
			    lock[l].state = ((i == o || !input_active (IUNLOCK + l)) ? o ? LOCK_UNLOCKED : LOCK_LOCKED : o ? LOCK_UNLOCKFAIL : LOCK_LOCKFAIL);
			  }
		      }
		    else if (lock[l].i != i)	// Input state change
		      lock[l].state = ((i == o) ? i ? LOCK_UNLOCKED : LOCK_LOCKED : i ? LOCK_FORCED : LOCK_FAULT);
		  }
		lock[l].o = o;
		lock[l].i = i;
		if (doordebug && (force || last != lock[l].state))
		  {
		    jo_t j = jo_object_alloc ();
		    jo_string (j, "state", lockstates[lock[l].state]);
		    if (lock[l].timeout > now)
		      jo_int (j, "timeout", (lock[l].timeout - now) / 1000);
		    revk_statej (l ? "deadlock" : "lock", &j, NULL);
		  }
	      }
	  }
	  static int64_t doortimeout = 0;
	  // Door states
	  if (iopen)
	    {			// Open
	      if (doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED && doorstate != DOOR_OPEN)
		{		// We have moved to open state, this can cancel the locking operation
		  jo_t j = jo_object_alloc ();
		  if (!doorwhy)
		    doorwhy = ((lock[0].state == LOCK_LOCKED) ? "forced" : "manual");
		  if (doorwhy)
		    jo_string (j, "trigger", doorwhy);
		  revk_eventj ("open", &j, NULL);
		  doorwhy = NULL;
		  doorstate = DOOR_OPEN;
		  if (lock[0].state == LOCK_LOCKING || lock[0].state == LOCK_LOCKFAIL)
		    output_set (OUNLOCK + 0, 1);	// Cancel lock
		  if (lock[1].state == LOCK_LOCKING || lock[1].state == LOCK_LOCKFAIL)
		    output_set (OUNLOCK + 1, 1);	// Cancel deadlock
		}
	    }
	  else
	    {			// Closed
	      if (lock[1].state == LOCK_LOCKED && lock[0].state == LOCK_LOCKED)
		doorstate = DOOR_DEADLOCKED;
	      else if (lock[0].state == LOCK_LOCKED && lock[1].state == LOCK_UNLOCKED)
		doorstate = DOOR_LOCKED;
	      else if (lock[0].state == LOCK_UNLOCKING || lock[1].state == LOCK_UNLOCKING)
		doorstate = DOOR_UNLOCKING;
	      else if (lock[0].state == LOCK_LOCKING || lock[1].state == LOCK_LOCKING)
		doorstate = DOOR_LOCKING;
	      else if (lock[0].state == LOCK_LOCKFAIL || lock[1].state == LOCK_LOCKFAIL)
		doorstate = DOOR_AJAR;
	      else
		doorstate = ((doorstate == DOOR_OPEN || doorstate == DOOR_NOTCLOSED || doorstate == DOOR_PROPPED || doorstate == DOOR_CLOSED) ? DOOR_CLOSED : DOOR_UNLOCKED);
	    }
	  if (doorstate != lastdoorstate)
	    {			// State change - iot and set timeout
	      if (doorstate == DOOR_OPEN && *iotopen)
		lwmqtt_send_str (iot, iotopen);
	      if (lastdoorstate == DOOR_OPEN && *iotclose)
		lwmqtt_send_str (iot, iotclose);
	      if (doorstate == DOOR_DEADLOCKED && *iotarm)
		lwmqtt_send_str (iot, iotarm);
	      if (lastdoorstate == DOOR_DEADLOCKED && *iotdisarm)
		lwmqtt_send_str (iot, iotdisarm);
	      if (doorstate == DOOR_OPEN)
		doortimeout = now + (int64_t) doorprop *1000LL;
	      else if (doorstate == DOOR_CLOSED)
		doortimeout = now + (int64_t) doorclose *1000LL;
	      else if (doorstate == DOOR_UNLOCKED)
		doortimeout = now + (int64_t) dooropen *1000LL;
	      else
		doortimeout = 0;
	      output_set (OBEEP, doorstate == DOOR_UNLOCKED && !doorsilent ? 1 : 0);
	    }
	  else if (doortimeout && doortimeout < now)
	    {			// timeout
	      output_set (OBEEP, 0);
	      if (doorstate == DOOR_OPEN)
		{
		  doorstate = DOOR_NOTCLOSED;
		  doortimeout = 0;
		}
	      else if (doorstate == DOOR_UNLOCKED || doorstate == DOOR_CLOSED)
		{		// Time to lock the door
		  if (doorstate == DOOR_UNLOCKED)
		    {
		      jo_t j = jo_object_alloc ();
		      if (doorwhy)
			jo_string (j, "trigger", doorwhy);
		      revk_eventj ("notopen", &j, NULL);
		    }
		  if (doordeadlock)
		    door_deadlock (NULL);
		  else
		    door_lock (NULL);
		  doorwhy = NULL;
		}
	    }
	  static int64_t exit1 = 0;	// Main exit button
	  if (input_get (IEXIT1))
	    {
	      if (!exit1)
		{
		  exit1 = now + (int64_t) doorexit *1000LL;
		  if (doorauto >= 2)
		    {
		      if (!doordeadlock)
			door_unlock (NULL, "button");
		      else
			{	// Not opening door
			  jo_t j = jo_object_alloc ();
			  jo_string (j, "trigger", "exit");
			  revk_eventj ("notopen", &j, NULL);
			}
		    }
		}
	    }
	  else
	    exit1 = 0;
	  static int64_t exit2 = 0;	// Secondary exit button
	  if (input_get (IEXIT2))
	    {
	      if (!exit2)
		{
		  exit2 = now + (int64_t) doorexit *1000LL;
		  if (doorauto >= 2)
		    {
		      if (!doordeadlock)
			door_unlock (NULL, "ranger");
		      else
			{	// Not opening door
			  jo_t j = jo_object_alloc ();
			  jo_string (j, "trigger", "ranger");
			  revk_eventj ("notopen", &j, NULL);
			}
		    }
		}
	    }
	  else
	    exit2 = 0;
	  // Check faults
	  if (lock[0].state == LOCK_UNLOCKFAIL)
	    status (door_fault = "Lock stuck");
	  else if (lock[1].state == LOCK_UNLOCKFAIL)
	    status (door_fault = "Deadlock stuck");
	  else if (lock[0].state == LOCK_FAULT)
	    status (door_fault = "Lock fault");
	  else if (lock[1].state == LOCK_FAULT)
	    status (door_fault = "Deadlock fault");
	  else if (exit1 && exit1 < now)
	    status (door_fault = "Exit stuck");
	  else if (exit2 && exit2 < now)
	    status (door_fault = "Ranger stuck");
	  else
	    status (door_fault = NULL);
	  // Check tampers
	  if (lock[0].state == LOCK_FORCED)
	    status (door_tamper = "Lock forced");
	  else if (lock[1].state == LOCK_FORCED)
	    status (door_tamper = "Deadlock forced");
	  else if (iopen && (lock[0].state == LOCK_LOCKED || lock[1].state == LOCK_LOCKED))
	    status (door_tamper = "Door forced");
	  else
	    status (door_tamper = NULL);
	  // Beep
	  if (door_tamper || door_fault || doorstate == DOOR_AJAR || doorstate == DOOR_NOTCLOSED)
	    output_set (OBEEP, ((now - doortimeout) & (512 * 1024)) ? 1 : 0);
	  if (force || doorstate != lastdoorstate)
	    {
	      nfc_led (strlen (doorled[doorstate]), doorled[doorstate]);
	      jo_t j = jo_object_alloc ();
	      jo_string (j, "state", doorstates[doorstate]);
	      if (doorwhy && doorstate == DOOR_UNLOCKING)
		jo_string (j, "trigger", doorwhy);
	      if (doortimeout > now)
		jo_int (j, "timeout", (doortimeout - now) / 1000);
	      revk_statej ("door", &j, iotstatedoor ? iot : NULL);
	      lastdoorstate = doorstate;
	    }
	  output_set (OERROR, door_tamper || door_fault);
	}
    }
}

void
door_init (void)
{
  extern char *ledIDLE;
  revk_register ("door", 0, sizeof (doorauto), &doorauto, NULL, SETTING_SECRET);	// Parent
  revk_register ("led", 0, 0, &ledIDLE, NULL, SETTING_SECRET);	// Parent
#define u32(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define ta(n,c) revk_register(#n,c,0,&n,NULL,SETTING_LIVE);
#define d(n,l) revk_register("led"#n,0,0,&doorled[DOOR_##n],#l,0);
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD);
#define s(n) revk_register(#n,0,0,&n,NULL,0);
  settings door_states
#undef ta
#undef u32
#undef u16
#undef u8
#undef u1
#undef d
#undef area
#undef s
  if (!doorauto)
      return;			// No door control in operation
  revk_task (TAG, task, NULL);
}
