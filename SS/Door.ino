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

  static byte bcdtime(time_t now, byte datetime[7])
  {
    struct tm *t;
    t = localtime (&now);
    int v = t->tm_year + 1900;
    datetime[0] = (v / 1000) * 16 + (v / 100 % 10);
    datetime[1] = (v / 10 % 10) * 16 + (v % 10);
    v = t->tm_mon + 1;
    datetime[2] = (v / 10) * 16 + (v % 10);
    v = t->tm_mday;
    datetime[3] = (v / 10) * 16 + (v % 10);
    v = t->tm_hour;
    datetime[4] = (v / 10) * 16 + (v % 10);
    v = t->tm_min;
    datetime[5] = (v / 10) * 16 + (v % 10);
    v = t->tm_sec;
    datetime[6] = (v / 10) * 16 + (v % 10);
    return t->tm_wday;
  }

  const char * Door_fob(char *id, String &err)
  { // Consider fob, and return PSTR() if not acceptable, NULL if OK
    if (!door)return PSTR(""); // just not allowed
    if (blacklist && checkfob(blacklist, id)) return PSTR("Blacklisted fob");
    if (doorstate == DOOR_OPEN)return PSTR("Door is open"); // Door is open!
    if (door >= 4)
    { // Autonomous door control logic - check access file and times, etc
      if (!NFC.secure)return PSTR("Not a secure fob");
      // Check access file (expected to exist)
      int32_t filesize = NFC.desfire_filesize(3, err);
      byte buf[100];
      time_t now;
      time (&now);
      byte datetime[7]; // BCD date time
      int xoff = 0, xlen = 0, xdays = 0; // Expiry data
      if (filesize < 0)return PSTR("No access file on fob");
      if (filesize > sizeof(buf) - 10)return PSTR("Access file too big");
      if (filesize)
      {
        if (NFC.desfire_fileread (3, 0, filesize, sizeof(buf), buf, err) < 0)return PSTR("Cannot read access file");
        // Check access
        byte *p = buf + 1, *e = buf + 1 + filesize;
        boolean ax = false, aok = false;
        byte *fok = NULL, *tok = NULL;
        unsigned int cid = ESP.getChipId ();
        byte dow = bcdtime(now, datetime);
        while (p < e)
        {
          byte l = (*p & 0xF);
          byte c = (*p++ >> 4);
          if (p + l > e)return PSTR("Invalid access file");
          if (c == 0xA)
          { // Allow
            ax = true;
            if (l % 3)return PSTR("Invalid allow list");
            byte n = l;
            while (n && !aok)
            {
              n -= 3;
              if ((p[n] << 16) + (p[n + 1] << 8) + p[n + 2] == cid)aok = 1;
            }
          } else if (c == 0xB)
          { // Barred
            if (l % 3)return PSTR("Invalid barred list");
            byte n = l;
            while (n)
            {
              n -= 3;
              if ((p[n] << 16) + (p[n + 1] << 8) + p[n + 2] == cid)return PSTR("Barred door");
            }
          } else if (c == 0xF)
          { // From
            if (fok)return PSTR("Duplicate from time");
            if (l == 2)fok = p;
            else if (l == 4)fok = p + ((dow && dow < 6) ? 2 : 0);
            else if (l == 14)fok = p + dow * 2;
            else return PSTR("Bad from time"); // Bad time
          } else if (c == 0x2)
          { // To
            if (tok)return PSTR("Duplicate to time");
            if (l == 2)tok = p;
            else if (l == 4)tok = p + ((dow && dow < 6) ? 2 : 0);
            else if (l == 14)tok = p + dow * 2;
            else return PSTR("Bad to time");
          } else if (c == 0xE)
          { // Expiry
            if (l == 1)xdays = *p;
            else
            {
              xoff = p - buf - 1;
              xlen = l;
              if (memcmp(datetime, p, l) > 0)return PSTR("Expired"); // expired
            }
          } else
            return PSTR("Unknown access code"); // Unknown access code
          p += l;
        }
        if (ax && !aok)return PSTR("Not allowed door"); // Not on allow list
        if (fok || tok)
        { // Time check
          if (fok && tok && memcmp(fok, tok, 2) > 0)
          { // reverse
            if (memcmp(datetime + 4, fok, 2) < 0 && memcmp(datetime + 4, tok, 2) >= 0)return PSTR("Outside time");
          } else
          {
            if (fok && memcmp(datetime + 4, fok, 2) < 0)return PSTR("Too soon");
            if (tok && memcmp(datetime + 4, tok, 2) >= 0)return PSTR("Too late");
          }
        }
      }
      if (doordeadlock && door < 5)return PSTR("Door deadlocked");
      if (xdays && xoff && xlen <= 7)
      { // Update expiry
        now += 86400 * xdays;
        bcdtime(now, datetime);
        if (memcmp(datetime, buf + 1 + xoff, xlen) > 0)
        { // Changed expiry
          buf[1] = 3;
          buf[2] = xoff;
          buf[3] = 0;
          buf[4] = 0;
          buf[5] = xlen;
          buf[6] = 0;
          buf[7] = 0;
          memcpy(buf + 8, datetime, xlen);
          if (NFC.desfire (0x3D, 7 + xlen, buf, sizeof(buf), err, 0) < 0)return PSTR("Expiry update failed");
        }
      }
      return NULL;
    }
    if (door == 3)
    {
      if (!NFC.secure)return PSTR("Not a secure fob");
      if (doordeadlock)return PSTR("Door deadlocked");
      return NULL;
    }
    if (offlinemode)
    {
      if (!fallback)return PSTR("Offline, and no fallback");
      if (!checkfob(fallback, id))return PSTR("Offline, and fallback not matched");
      return NULL;
    }
    return PSTR(""); // Just not allowed
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
              if ((Input_get(IOPEN) || lock[l].o) && !o && last != LOCK_FORCED)
              { // Change to lock (an open door is seen as still trying to lock if !o)
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
