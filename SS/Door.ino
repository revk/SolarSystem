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
#define OERROR 4

#define FILE3                   // Check for old file 3 access file
#define MINAFILE  32            // Minimum we read from afile before checking length

byte afile[256 + 10 + 7];       // Access file saved - starts at byte 8

#include "PN532RevK.h"
extern PN532RevK NFC;

const char *Door_fault = NULL;
const char *Door_tamper = NULL;

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
  s(lockdebounce,100); \
  t(fallback); \
  t(blacklist); \

#define s(n,d) unsigned int n=d;
#define t(n) const char*n=NULL;
  app_settings
#undef t
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
  d(DEADLOCKED,) \
  d(LOCKED,R) \
  d(UNLOCKING,--R) \
  d(UNLOCKED,--G) \
  d(OPEN,G) \
  d(CLOSED,--G) \
  d(LOCKING,R-R) \
  d(NOTCLOSED,R-G) \
  d(PROPPED,G-GGGG) \
  d(AJAR,R-G) \

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
  const char *doorled[] = {
    door_states
  };

#undef d

  struct
  {
    boolean o,
            i;
    byte state;
    unsigned long timeout;
  } lock[2] =
  {
    0
  };

  byte doorstate = -1;
  boolean doordeadlock = true;

  boolean
  Door_access (const byte * a)
  { // Confirm access, and profile new access file to write if changed.
    if (!a)
      return true;              // No action
    if (*a == afile[8] && !memcmp (a + 1, afile + 9, afile[8]))
      return true;              // Same
    if (!NFC.authenticated)
      return false;
    afile[1] = 0x0A;
    afile[2] = 0;
    afile[3] = 0;
    afile[4] = 0;
    afile[5] = afile[8] + 1;
    afile[6] = 0;
    afile[7] = 0;
    memcpy (afile + 8, a, *a + 1);
    String err;
    boolean ret = (NFC.desfire_dx (0x3D, sizeof (afile), afile, 8 + 1 + afile[8], 0xFF) >= 0 &&
                   NFC.desfire (0xC7, 0, afile, sizeof (afile), err, 0) >= 0);
    memcpy (afile + 8, a, *a + 1);       // Restore
    return ret;
  }

  boolean
  Door_unlock (const byte * a)
  { // Unlock the door - i.e. exit button, entry allowed, etc.
    doordeadlock = false;
    Output_set (OUNLOCK + 0, 1);
    Output_set (OUNLOCK + 1, 1);
    return Door_access (a);
  }

  boolean
  Door_deadlock (const byte * a)
  { // Deadlock the door - i.e. move to alarm armed, no exit/entry
    doordeadlock = true;
    Output_set (OUNLOCK + 0, 0);
    Output_set (OUNLOCK + 1, 0);
    return Door_access (a);
  }

  boolean
  Door_lock (const byte * a)
  { // Lock the door - i.e. move to normal locked operation
    doordeadlock = false;
    Output_set (OUNLOCK + 0, 0);
    Output_set (OUNLOCK + 1, 1);
    return Door_access (a);
  }

  boolean
  Door_prop (const byte * a)
  { // Allow door propping
    if (doorstate != DOOR_OPEN && doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED)
      return false;
    doorstate = DOOR_PROPPED;
    return Door_access (a);
  }

  int
  checkfob (const char *fobs, const char *id)
  { // is fob in list
    if (!fobs || !id)
      return 0;
    int l = strlen (id),
        n = 0;
    if (l && id[l - 1] == '+')
      l--;                      // Don't check secure tag suffix
    while (*fobs)
    {
      n++;
      const char *p = fobs;
      while (*p && *p != ' ')
        p++;
      if (p - fobs == l && !memcmp (fobs, id, l))
        return n;
      while (*p && *p == ' ')
        p++;
      fobs = p;
    }
    return 0;
  }

  byte
  bcdtime (time_t now, byte datetime[7])
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

  const char *
  Door_fob (char *id, String & err)
  { // Consider fob, and return PSTR() if not acceptable, NULL if OK
    if (!door)
      return PSTR ("");         // just not allowed
    if (blacklist && checkfob (blacklist, id))
    {
      if (door >= 4 && NFC.authenticated)
      { // Zap the access file
        byte buf[20];
        buf[1] = 0x0A;         // File
        buf[2] = 0;            // Offset
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 2;            // Len
        buf[6] = 0;
        buf[7] = 0;
        buf[8] = 0x01;         // Len 1
        buf[9] = 0xA0;         // Blacklist
        if (NFC.desfire_dx (0x3D, sizeof (buf), buf, 10, 0xFF) < 0)
          return PSTR ("Blacklist update failed");
        return PSTR ("Blacklist (zapped)");
      }
      return PSTR ("Blacklisted fob");
    }
    if (door >= 4)
    { // Autonomous door control logic - check access file and times, etc
      if (!NFC.secure)
        return PSTR ("");      // Don't make a fuss, control system may allow this, and it is obvious
      // Just read file A
      if (NFC.desfire_fileread (0xA, 0, 16, sizeof (afile) - 7, afile + 7, err) < 0)
        return PSTR ("Cannot read access file 0A");
      if (afile[8] + 1 > 16 && NFC.desfire_fileread (0xA, 0, afile[8] + 1, sizeof (afile) - 7, afile + 7, err) < 0)
        return PSTR ("Cannot read access full file 0A");
      // Check access file (expected to exist)
      time_t now;
      time (&now);
      byte datetime[7];         // BCD date time
      int xoff = 0,
          xlen = 0,
          xdays = 0;             // Expiry data
      byte clockoverride = 0;
      byte deadlockoverride = 0;
      if (afile[8])
      { // Check access
        byte *p = afile + 9,
              *e = afile + 9 + afile[8];
        boolean ax = false,
                aok = false;
        byte *fok = NULL,
              *tok = NULL;
        unsigned int cid = ESP.getChipId ();
        byte dow = bcdtime (now, datetime);
        while (p < e)
        {
          byte l = (*p & 0xF);
          byte c = (*p++ >> 4);
          if (p + l > e)
            return PSTR ("Invalid access file");
          if (c == 0x0)
          { // Padding, ignore
          } else if (c == 0xC) clockoverride = 1;
          else if (c == 0xD) deadlockoverride = 1;
          else if (c == 0xA)
          { // Allow
            if (!l)
              return PSTR ("Card blocked"); // Black list
            ax = true;
            if (l % 3)
              return PSTR ("Invalid allow list");
            byte n = l;
            while (n && !aok)
            {
              n -= 3;
              if ((p[n] << 16) + (p[n + 1] << 8) + p[n + 2] == cid)
                aok = 1;
            }
          } else if (c == 0xB)
          { // Barred
            if (l % 3)
              return PSTR ("Invalid barred list");
            byte n = l;
            while (n)
            {
              n -= 3;
              if ((p[n] << 16) + (p[n + 1] << 8) + p[n + 2] == cid)
                return PSTR ("Barred door");
            }
          } else if (c == 0xF)
          { // From
            if (fok)
              return PSTR ("Duplicate from time");
            if (l == 2)
              fok = p;
            else if (l == 4)
              fok = p + ((dow && dow < 6) ? 2 : 0);
            else if (l == 6)
              fok = p + (!dow ? 0 : dow < 6 ? 2 : 4);
            else if (l == 14)
              fok = p + dow * 2;
            else
              return PSTR ("Bad from time");        // Bad time
          } else if (c == 0x2)
          { // To
            if (tok)
              return PSTR ("Duplicate to time");
            if (l == 2)
              tok = p;
            else if (l == 4)
              tok = p + ((dow && dow < 6) ? 2 : 0);
            else if (l == 6)
              tok = p + (!dow ? 0 : dow < 6 ? 2 : 4);
            else if (l == 14)
              tok = p + dow * 2;
            else
              return PSTR ("Bad to time");
          } else if (c == 0xE)
          { // Expiry
            if (l == 1)
              xdays = *p;
            else
            {
              xoff = p - afile - 8;
              xlen = l;
            }
          } else
            return PSTR ("Unknown access code");     // Unknown access code
          p += l;
        }
        if (ax && !aok)
          return PSTR ("Not allowed door");   // Not on allow list
        if (xoff)
        {
          if (*datetime < 0x20)
          { // Clock not set
            if (!clockoverride)
              return PSTR("Date not set");
          }
          else if (memcmp (datetime, afile + xoff, xlen) > 0)
            return PSTR("Expired");       // expired
        }
        if (fok || tok)
        { // Time check
          if (*datetime < 0x20)
          { // Clock not set
            if (!clockoverride)
              return PSTR("Date not set");
          }
          else if (fok && tok && memcmp (fok, tok, 2) > 0)
          { // reverse
            if (memcmp (datetime + 4, fok, 2) < 0 && memcmp (datetime + 4, tok, 2) >= 0)
              return PSTR ("Outside time");
          } else
          {
            if (fok && memcmp (datetime + 4, fok, 2) < 0)
              return PSTR ("Too soon");
            if (tok && memcmp (datetime + 4, tok, 2) >= 0)
              return PSTR ("Too late");
          }
        }
      }
      if (!deadlockoverride && doordeadlock && door < 5)
        return PSTR ("");      // Quiet about this as normal for system controlled disarm
      if (*datetime >= 0x20 && xdays && xoff && xlen <= 7 && NFC.authenticated)
      { // Update expiry
        now += 86400 * xdays;
        bcdtime (now, datetime);
        if (memcmp (datetime, afile + 8 + xoff, xlen) > 0)
        { // Changed expiry
          byte buf[30];
          buf[1] = 0xA;
          buf[2] = xoff;
          buf[3] = 0;
          buf[4] = 0;
          buf[5] = xlen;
          buf[6] = 0;
          buf[7] = 0;
          memcpy (buf + 8, datetime, xlen);
          memcpy (afile + 8 + xoff, datetime, xlen);
          if (NFC.desfire_dx (0x3D, sizeof (buf), buf, 8 + xlen, 0xFF) < 0)
            return PSTR ("Expiry update failed");
          if (NFC.desfire (0xC7, 0, buf, sizeof (buf), err, 0) < 0)
            return PSTR ("Expiry commit failed");
        }
      }
      // Allowed
      if (doorstate == DOOR_OPEN && door >= 5)
        Door_prop (NULL);
      return NULL;
    }
    if (door == 3)
    {
      if (!NFC.secure)
        return PSTR ("");      // Don't make a fuss, control system may allow this, and it is obvious
      if (doordeadlock)
        return PSTR ("Door deadlocked");
      return NULL;
    }
    if (offlinemode)
    {
      if (!fallback)
        return PSTR ("Offline, and no fallback");
      if (!checkfob (fallback, id))
        return PSTR ("Offline, and fallback not matched");
      return NULL;
    }
    return PSTR ("");            // Just not allowed
  }

  const char *
  Door_setting (const char *tag, const byte * value, size_t len)
  { // Called for commands retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
#define t(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char*)value;return t;}}while(0)
    app_settings
#undef t
#undef s
    return NULL;              // Done
  }

  boolean
  Door_command (const char *tag, const byte * message, size_t len)
  { // Called for incoming MQTT messages
    if (!door)
      return false;             // No door control in operation
    if (!len || *message != len - 1)
      message = NULL;           // Not sensible access file
    if (!strcasecmp_P (tag, PSTR ("deadlock")))
      return Door_deadlock (message);
    if (!strcasecmp_P (tag, PSTR ("lock")))
      return Door_lock (message);
    if (!strcasecmp_P (tag, PSTR ("unlock")))
      return Door_unlock (message);
    if (!strcasecmp_P (tag, PSTR ("prop")))
      return Door_prop (message);
    if (!strcasecmp_P (tag, PSTR ("access")))
      return Door_access (message);
    return false;
  }

  boolean
  Door_setup (ESPRevK & revk)
  {
    if (!door)
      return false;             // No door control in operation
    if (Input_direct (IOPEN))
      doorstate = DOOR_OPEN;
    else
      doorstate = DOOR_LOCKING;
    lock[0].timeout = 1000;
    lock[1].timeout = 1000;
    return true;
  }

  boolean
  Door_loop (ESPRevK & revk, boolean force)
  {
    if (!door)
      return false;             // No door control in operation
    unsigned long now = millis ();
    static unsigned long doornext = 0;
    static byte lastdoorstate = -1;
    boolean iopen = Input_direct (IOPEN);
    if ((int) (doornext - now) < 0)
    {
      doornext = now + doorpoll;
      { // Check locks
        int l;
        for (l = 0; l < 2; l++)
        {
          byte last = lock[l].state;
          boolean o = Output_get (OUNLOCK + l),
                  i = Input_direct (IUNLOCK + l);
          if (!Output_active (OUNLOCK + l))
          {
            if (!Input_active (IUNLOCK + l))
              lock[l].state = (o ? LOCK_UNLOCKED : LOCK_LOCKED);    // No input or output, just track output
            else
              lock[l].state = (i ? LOCK_UNLOCKED : LOCK_LOCKED);    // No output, just track input
          } else
          { // Lock state tracking
            if (((iopen && last == LOCK_LOCKING) || lock[l].o) && !o)
            { // Change to lock - timer constantly restarted if door is open as it will not actually engage
              lock[l].timeout = ((now + doorlock) ? : 1);
              lock[l].state = LOCK_LOCKING;
            } else if (o && !lock[l].o)
            { // Change to unlock
              lock[l].timeout = ((now + doorunlock) ? : 1);
              lock[l].state = LOCK_UNLOCKING;
            }
            if (lock[l].timeout)
            { // Timeout running
              if (lock[l].i != i)
                lock[l].timeout = ((now + lockdebounce) ? : 1); // Allow some debounce before ending timeout
              if ((int) (lock[l].timeout - now) <= 0)
              { // End of timeout
                lock[l].timeout = 0;
                lock[l].state =
                  ((i == o
                    || !Input_active (IUNLOCK + l)) ? o ? LOCK_UNLOCKED : LOCK_LOCKED : o ? LOCK_UNLOCKFAIL : LOCK_LOCKFAIL);
              }
            } else if (lock[l].i != i)       // Input state change
              lock[l].state = ((i == o) ? i ? LOCK_UNLOCKED : LOCK_LOCKED : i ? LOCK_FORCED : LOCK_FAULT);
          }
          lock[l].o = o;
          lock[l].i = i;
          if (doordebug && (force || last != lock[l].state))
            revk.state (l ? F ("deadlock") : F ("lock"), lock[l].timeout ? F ("%s %dms") : F ("%s"), lockstates[lock[l].state],
                        (int) (lock[l].timeout - now));
        }
      }
      static long doortimeout = 0;
      // Door states
      if (iopen)
      { // Open
        if (doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED && doorstate != DOOR_OPEN)
        { // We have moved to open state, this can cancel the locking operation
          doorstate = DOOR_OPEN;
          if (lock[0].state == LOCK_LOCKING || lock[0].state == LOCK_LOCKFAIL)
            Output_set (OUNLOCK + 0, 1);     // Cancel lock
          if (lock[1].state == LOCK_LOCKING || lock[1].state == LOCK_LOCKFAIL)
            Output_set (OUNLOCK + 1, 1);     // Cancel deadlock
        }
      } else
      { // Closed
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
          doorstate =
            ((doorstate == DOOR_OPEN || doorstate == DOOR_NOTCLOSED || doorstate == DOOR_PROPPED
              || doorstate == DOOR_CLOSED) ? DOOR_CLOSED : DOOR_UNLOCKED);
      }
      if (doorstate != lastdoorstate)
      { // State change - set timerout
        if (doorstate == DOOR_OPEN)
          doortimeout = (now + doorprop ? : 1);
        else if (doorstate == DOOR_CLOSED)
          doortimeout = (now + doorclose ? : 1);
        else if (doorstate == DOOR_UNLOCKED)
          doortimeout = (now + dooropen ? : 1);
        else
          doortimeout = 0;
        Output_set (OBEEP, doorstate == DOOR_UNLOCKED && doorbeep ? 1 : 0);
      } else if (doortimeout && (int) (doortimeout - now) < 0)
      { // timeout
        Output_set (OBEEP, 0);
        doortimeout = 0;
        if (doorstate == DOOR_OPEN)
          doorstate = DOOR_NOTCLOSED;
        else if (doorstate == DOOR_UNLOCKED || doorstate == DOOR_CLOSED)
        { // Time to lock the door
          if (doordeadlock)
            Door_deadlock (NULL);
          else
            Door_lock (NULL);
        }
      }
      static long exit1 = 0;    // Main exit button
      if (Input_direct (IEXIT1))
      {
        if (!exit1)
        {
          exit1 = (now + doorexit ? : 1);
          if (door >= 2 && !doordeadlock)
            Door_unlock (NULL);
        }
      } else
        exit1 = 0;
      static long exit2 = 0;    // Secondary exit button
      if (Input_direct (IEXIT2))
      {
        if (!exit2)
        {
          exit2 = (now + doorexit ? : 1);
          if (door >= 2 && !doordeadlock)
            Door_unlock (NULL);
        }
      } else
        exit2 = 0;
      // Check faults
      if (lock[0].state == LOCK_UNLOCKFAIL)
        Door_fault = PSTR ("Lock stuck");
      else if (lock[1].state == LOCK_UNLOCKFAIL)
        Door_fault = PSTR ("Deadlock stuck");
      else if (lock[0].state == LOCK_FAULT)
        Door_fault = PSTR ("Lock fault");
      else if (lock[1].state == LOCK_FAULT)
        Door_fault = PSTR ("Deadlock fault");
      else if (exit1 && (int) (exit1 - now) < 0)
        Door_fault = PSTR ("Exit stuck");
      else if (exit2 && (int) (exit2 - now) < 0)
        Door_fault = PSTR ("Ranger stuck");
      else
        Door_fault = NULL;
      // Check tampers
      if (lock[0].state == LOCK_FORCED)
        Door_tamper = PSTR ("Lock forced");
      else if (lock[1].state == LOCK_FORCED)
        Door_tamper = PSTR ("Deadlock forced");
      else if (iopen && (lock[0].state == LOCK_LOCKED || lock[1].state == LOCK_LOCKED))
        Door_tamper = PSTR ("Door forced");
      else
        Door_tamper = NULL;
      // Beep
      if (Door_tamper || Door_fault || doorstate == DOOR_AJAR || doorstate == DOOR_NOTCLOSED)
        Output_set (OBEEP, ((now - doortimeout) & 512) ? 1 : 0);
      if (force || doorstate != lastdoorstate)
      {
        NFC_led (strlen (doorled[doorstate]), doorled[doorstate]);
        revk.state (F ("door"), doortimeout
                    && doordebug ? F ("%s %dms") : F ("%s"), doorstates[doorstate], (int) (doortimeout - now));
        lastdoorstate = doorstate;
      }
      Output_set (OERROR, Door_tamper || Door_fault);
    }
    return true;
  }
