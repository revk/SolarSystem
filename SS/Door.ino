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
#define IEXIT 1
#define IOPEN 2
#define IUNLOCK 3
#define OUNLOCK 1
#define OBEEP 3

const char* Door_fault = NULL;
const char* Door_tamper = NULL;

#define app_settings  \
  s(door,0);   \
  s(doorunlock,1000); \
  s(doorlock,1000); \
  s(dooropen,5000); \
  s(doorclose,0); \
  s(doorprop,30000); \
  s(doorexit,10000); \
  s(doorpoll,100); \
  s(doordebug,0); \

#define s(n,d) unsigned int n=d;
  app_settings
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
  d(DEADLOCKED) \
  d(LOCKED) \
  d(UNLOCKING) \
  d(UNLOCKED) \
  d(OPEN) \
  d(LOCKING) \
  d(PROPPED) \
  d(AJAR) \

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

#define d(n) DOOR_##n,
  enum {
    door_states
  };
#undef d

#define d(n) #n,
  const char *doorstates[] = {
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
  boolean doorprod = false;

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
    doorprop = true;
  }

  const char* Door_setting(const char *tag, const byte *value, size_t len)
  { // Called for commands retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
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
      {
        // Check locks
        int l;
        for (l = 0; l < 2; l++)
        {
          byte last = lock[l].state;
          boolean o = false, i = false;
          if (Input_get(IOPEN) || (Output_active(OUNLOCK + l) && Output_get(OUNLOCK + l)))o = true;
          if (Input_get(IUNLOCK + l))i = true;
          if (lock[l].o && !o)
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
          if (doordebug && (force || last != lock[l].state))
            revk.state(l ? F("deadlock") : F("lock"), lock[l].timeout ? F("%s %dms") : F("%s"), lockstates[lock[l].state], (int)(lock[l].timeout - now));
        }
      }
      // Check state changes
      if (lock[0].state == LOCK_LOCKING || lock[1].state == LOCK_LOCKING)doorstate = DOOR_LOCKING;
      else if (lock[0].state == LOCK_UNLOCKING || lock[1].state == LOCK_UNLOCKING)doorstate = DOOR_UNLOCKING;
      else if (lock[1].state == LOCK_LOCKED)doorstate = DOOR_DEADLOCKED;
      else if (lock[0].state == LOCK_LOCKED)doorstate = DOOR_LOCKED;
      else if (!Input_get(IOPEN) && lock[0].state == LOCK_LOCKFAIL && (lock[1].state == LOCK_NOLOCK || lock[1].state == LOCK_UNLOCKED))doorstate = DOOR_AJAR;
      else if (!Input_get(IOPEN))doorstate = DOOR_UNLOCKED;
      else doorstate = DOOR_OPEN;
      // TODO PROPPED state
      // TODO engage lock when closed
      // Check faults
      // TODO
      // Check tampers
      // TODO
      // Check exit button
      // TODO
      if (force || doorstate != lastdoorstate)
      {
        lastdoorstate = doorstate;
        revk.state(F("door"), F("%s"), doorstates[doorstate]);
      }
    }
    return true;
  }
