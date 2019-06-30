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
  s(doorprop,30000); \
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

#define door_states \
  d(DEADLOCKED) \
  d(LOCKED) \
  d(UNLOCKING) \
  d(CLOSED) \
  d(OPEN) \
  d(LOCKING) \
  d(PROPPED) \
  d(FORCED) \
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
    boolean olast;
    boolean ilast;
    unsigned char state;
    unsigned long timeout;
  } lock[2];
  unsigned char doorstate = -1;

  void Door_open()
  { // Unlock the door - i.e. exit button, entry allowed, etc.
    // TODO
  }

  void Door_deadlock()
  { // Deadlock the door - i.e. move to alarm armed, no exit/entry
    // TODO
  }

  void Door_lock()
  { // Lock the door - i.e. move to normal locked operation
    // TODO
  }

  void Door_prop()
  { // Allow door propping
    // TODO
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
    if (!strcasecmp_P(tag, PSTR("open")))
    {
      Door_open();
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
    static unsigned char lastdoor = -1;
    if ((int)(doornext - now) < 0)
    {
      doornext = now + doorpoll;
      {
        // Check locks
        int l;
        for (l = 0; l < 2; l++)
        {
          unsigned char last = lock[l].state;
          // TODO
          if (doordebug && last != lock[l].state)
            revk.state(l ? F("deadlock") : F("lock"), F("%s"), lockstates[lock[l].state]);
        }
      }
      // Check state changes
      // TODO
      if (doorstate != lastdoor)
      {
        lastdoor = doorstate;
        revk.state(F("door"), F("%s"), doorstates[doorstate]);
      }
    }
    return true;
  }
