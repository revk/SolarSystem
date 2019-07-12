// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Autonomous door control
// Door mode set by door setting
// 0 - no control
// 1 - tracking and reporting state, managing lock when closed
// 2 - also handling exit button (if not deadlocked)
// 3 - also handling entry for secure card use
// 4 - TODO stand alone control using secure card
// This uses pre-set input and output numbers
#define IEXIT1 1
#define IEXIT2 8
#define IOPEN 2
#define IUNLOCK 3
#define OUNLOCK 1
#define OBEEP 3

#include "PN532RevK.h"
extern PN532RevK NFC;

const char* Door_fault = NULL;
const char* Door_tamper = NULL;

#define app_settings  \
  s(door,0);   \
  s(doorunlock,1000); \
  s(doorlock,3000); \
  s(dooropen,5000); \
  s(doorclose,500); \
  s(doorprop,60000); \
  s(doorexit,60000); \
  s(doorpoll,100); \
  s(doordebug,0); \
  s(doorbeep,1); \
  t(fallback); \
  t(blacklist); \

#define s(n,d) unsigned int n=d;
#define t(n) const char*n=NULL;
  app_settings
#undef t
#undef s

#define lock_states \
  l(NOLOCK) \
  l(LOCKING) \
  l(LOCKED) \
  l(UNLOCKING) \
  l(UNLOCKED) \
  l(LOCKFAIL) \
  l(UNLOCKFAIL) \
  l(FORCED) \

#define door_states \
  d(DEADLOCKED,) \
  d(LOCKED,R) \
  d(UNLOCKING,--R) \
  d(UNLOCKED,--G) \
  d(OPEN,G) \
  d(CLOSED,--G) \
  d(LOCKING,RR-) \
  d(PROPPED,RG-) \
  d(AJAR,RG-) \

#define l(n) LOCK_##n,
  enum {
    lock_states
  };
#undef l

#define l(n) #n,
  const char *lockstates[] = {
    lock_states
  };
#undef l

#define d(n,l) DOOR_##n,
  enum {
    door_states
  };
#undef d

#define d(n,l) #n,
  const char *doorstates[] = {
    door_states
  };
#undef d

#define d(n,l) #l,
  const char *doorled[] = {
    door_states
  };
#undef d

  struct
  {
    boolean o, i;
    byte state;
    unsigned long timeout;
  } lock[2] = {0};
  byte doorstate = -1;
  boolean doordeadlock = true;
  boolean doorpropable = false;
  boolean doorforced = false;

  void Door_unlock()
  { // Unlock the door - i.e. exit button, entry allowed, etc.
    doordeadlock = false;
    Output_set(OUNLOCK + 0, 1);
    Output_set(OUNLOCK + 1, 1);
  }

  void Door_deadlock()
  { // Deadlock the door - i.e. move to alarm armed, no exit/entry
    doordeadlock = true;
    Output_set(OUNLOCK + 0, 0);
    Output_set(OUNLOCK + 1, 0);
  }

  void Door_lock()
  { // Lock the door - i.e. move to normal locked operation
    doordeadlock = false;
    Output_set(OUNLOCK + 0, 0);
    Output_set(OUNLOCK + 1, 1);
  }

  void Door_prop()
  { // Allow door propping
    doorpropable = true;
  }

  static int checkfob(const char *fobs, const char *id)
  { // is fob in list
    if (!fobs || !id)return 0;
    int l = strlen(id), n = 0;
    while (*fobs)
    {
      n++;
      const char *p = fobs;
      while (*p && *p != ' ')p++;
      if (p - fobs == l && !memcmp(fobs, id, l))return n;
      while (*p && *p == ' ')p++;
      fobs = p;
    }
    return 0;
  }

  boolean Door_fob(char *id)
  { // Consider fob, and return true if we are opening door
    if (!door)return false;
    boolean ret = false;
    if (blacklist && checkfob(blacklist, id)) return false;
    if (doorstate == DOOR_OPEN)return false; // Door is open!
    if (door >= 4 && NFC.secure)
    { // Autonomous door control logic - check access file and times, etc
      // TODO
    }
    else if (door == 3 && NFC.secure && !doordeadlock) ret = true;
    else if (offlinemode && fallback && checkfob(fallback, id)) ret = true;
    if (ret)Door_unlock(); // Open the door
    return ret;
  }

  const char* Door_setting(const char *tag, const byte *value, size_t len)
  { // Called for commands retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
#define t(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char*)value;return t;}}while(0)
    app_settings
#undef t
#undef s
    return NULL; // Done
  }

  boolean Door_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!door)return false; // No door control in operation
    if (!strcasecmp_P(tag, PSTR("deadlock")))
    {
      Door_deadlock();
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("lock")))
    {
      Door_lock();
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("unlock")))
    {
      Door_unlock();
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("prop")))
    {
      Door_prop();
      return true;
    }
    return false;
  }

  boolean Door_setup(ESPRevK&revk)
  {
    if (!door)return false; // No door control in operation
    if (Input_active(IOPEN) && Input_get(IOPEN)) doorstate = DOOR_OPEN;
    else doorstate = DOOR_LOCKING;
    lock[0].timeout = 1000;
    lock[1].timeout = 1000;
    return true;
  }

  boolean Door_loop(ESPRevK&revk, boolean force)
  {
    if (!door)return false; // No door control in operation
    unsigned long now = millis();
    static unsigned long doornext = 0;
    static byte lastdoorstate = -1;
    if ((int)(doornext - now) < 0)
    {
      doornext = now + doorpoll;
      { // Check locks
        int l;
        for (l = 0; l < 2; l++)
          if (Output_active(OUNLOCK + l) || Input_active(IUNLOCK + l))
          { // Lock exists
            byte last = lock[l].state;
            {
              boolean o = false, i = false;
              if (Output_active(OUNLOCK + l) && Output_get(OUNLOCK + l))o = true;
              if (Input_get(IUNLOCK + l))i = true;
              if (lock[l].o && !o && last != LOCK_FORCED)
              { // Change to lock
                lock[l].timeout = ((now + doorlock) ? : 1);
                lock[l].state = LOCK_LOCKING;
              } else if (o && !lock[l].o)
              { // Change to unlock
                lock[l].timeout = ((now + doorunlock) ? : 1);
                lock[l].state = LOCK_UNLOCKING;
              }
              if (!lock[l].i && i && lock[l].state == LOCK_LOCKED)lock[l].state = LOCK_FORCED;
              if (lock[l].timeout && ((Input_active(IUNLOCK + l) && i == o) || (int)(lock[l].timeout - now) <= 0))
                lock[l].timeout = 0;
              if (!lock[l].timeout && (!i || lock[l].state != LOCK_FORCED))
              {
                if (Input_active(IUNLOCK + l) && i != o)lock[l].state = (o ? LOCK_UNLOCKFAIL : LOCK_LOCKFAIL);
                else lock[l].state = (o ? LOCK_UNLOCKED : LOCK_LOCKED);
              }
              lock[l].o = o;
              lock[l].i = i;
            }
            if (doordebug && (force || last != lock[l].state))
              revk.state(l ? F("deadlock") : F("lock"), lock[l].timeout ? F("%s %dms") : F("%s"), lockstates[lock[l].state], (int)(lock[l].timeout - now));
          }
      }
      static long doortimeout = 0;
      // Check force check
      if (Input_get(IOPEN) && (lock[0].state == LOCK_LOCKED || lock[1].state == LOCK_LOCKED))doorforced = true;
      else if (!Input_get(IOPEN))
      {
        doorforced = false;
        doorpropable = false;
      }
      // Door states
      if (Input_get(IOPEN))
      { // Open, so door is propped or open state only
        if (doorstate != DOOR_PROPPED || doorpropable)doorstate = DOOR_OPEN;
        Output_set(OUNLOCK + 0, 1); // No point trying to lock when door is open
        Output_set(OUNLOCK + 1, 1);
      } else if (doorstate != DOOR_AJAR && (lock[0].state == LOCK_LOCKING || lock[1].state == LOCK_LOCKING))doorstate = DOOR_LOCKING;
      else if (doorstate != DOOR_AJAR && (lock[0].state == LOCK_UNLOCKING || lock[1].state == LOCK_UNLOCKING))doorstate = DOOR_UNLOCKING;
      else if ((lock[0].state == LOCK_LOCKED || lock[0].state == LOCK_UNLOCKFAIL) &&
               ((doordeadlock && lock[1].state == LOCK_NOLOCK) || lock[1].state == LOCK_LOCKED || lock[1].state == LOCK_UNLOCKFAIL))doorstate = DOOR_DEADLOCKED;
      else if (lock[0].state == LOCK_LOCKED || lock[0].state == LOCK_UNLOCKFAIL)doorstate = DOOR_LOCKED;
      else if (lock[0].state == LOCK_LOCKFAIL && (lock[1].state == LOCK_NOLOCK || lock[1].state == LOCK_UNLOCKED))doorstate = DOOR_AJAR;
      else if (doorstate == DOOR_OPEN)doorstate = DOOR_CLOSED;
      else if (doorstate != DOOR_AJAR && doorstate != DOOR_CLOSED)doorstate = DOOR_UNLOCKED;
      if (doorstate != lastdoorstate)
      { // State change
        NFC_led(strlen(doorled[doorstate]), doorled[doorstate]);
        if (doorstate == DOOR_OPEN) doortimeout = (now + doorprop ? : 1);
        else if (doorstate == DOOR_CLOSED) doortimeout = (now + doorclose ? : 1);
        else if (doorstate == DOOR_UNLOCKED)doortimeout = (now + dooropen ? : 1);
        else doortimeout = 0;
        Output_set(OBEEP, doorstate == DOOR_UNLOCKED && doorbeep ? 1 : 0);
      } else if (doortimeout && (int)(doortimeout - now) < 0)
      { // timeout
        Output_set(OBEEP, 0);
        doortimeout = 0;
        if (doorstate == DOOR_OPEN && !doorpropable)doorstate = DOOR_PROPPED;
        else if (doorstate == DOOR_UNLOCKED || doorstate == DOOR_CLOSED)
        {
          if (doordeadlock)Door_deadlock();
          else Door_lock();
        }
      }
      static long exit1 = 0; // Main exit button
      if (Input_get(IEXIT1))
      {
        if (!exit1)
        {
          exit1 = (now + doorexit ? : 1);
          if (door >= 2 && !doordeadlock)Door_unlock();
        }
      } else
        exit1 = 0;
      static long exit2 = 0; // Secondary exit button
      if (Input_get(IEXIT2))
      {
        if (!exit2)
        {
          exit2 = (now + doorexit ? : 1);
          if (door >= 2 && !doordeadlock)Door_unlock();
        }
      } else
        exit2 = 0;
      // Check faults
      if (lock[0].state == LOCK_UNLOCKFAIL)Door_fault = PSTR("Lock stuck");
      else if (lock[1].state == LOCK_UNLOCKFAIL)Door_fault = PSTR("Deadlock stuck");
      else if (Input_get(IOPEN) && Input_active(IUNLOCK + 0) && !Input_get(IUNLOCK + 0))Door_fault = PSTR("Lock invalid");
      else if (Input_get(IOPEN) && Input_active(IUNLOCK + 1) && !Input_get(IUNLOCK + 1))Door_fault = PSTR("Deadlock invalid");
      else if (exit1 && (int)(exit1 - now) < 0)Door_fault = PSTR("Exit stuck");
      else if (exit2 && (int)(exit2 - now) < 0)Door_fault = PSTR("Ranger stuck");
      else Door_fault = NULL;
      // Check tampers
      if (doorforced)Door_tamper = PSTR("Door forced");
      else if (lock[0].state == LOCK_FORCED)Door_tamper = PSTR("Lock forced");
      else if (lock[1].state == LOCK_FORCED)Door_tamper = PSTR("Deadlock forced");
      else Door_tamper = NULL;
      // Beep
      if (Door_tamper || Door_fault || doorstate == DOOR_AJAR || doorstate == DOOR_PROPPED || doorforced)
        Output_set(OBEEP, ((now - doortimeout) & 512) ? 1 : 0);
      if (force || doorstate != lastdoorstate)
      {
        lastdoorstate = doorstate;
        revk.state(F("door"), doortimeout && doordebug ? F("%s %dms") : F("%s"), doorstates[doorstate], (int)(doortimeout - now));
      }
    }
    return true;
  }
