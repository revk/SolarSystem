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

#define FILE3                   // Check for old file 3 access file
#define MINAFILE  32            // Minimum we read from afile before checking length

#include "nfc.h"
#include "input.h"
#include "output.h"

uint8_t afile[256];             // Access file saved

#define settings  \
  u8(door,0);   \
  u32(doorunlock,1000); \
  u32(doorlock,3000); \
  u32(dooropen,5000); \
  u32(doorclose,500); \
  u32(doorprop,60000); \
  u32(doorexit,60000); \
  u32(doorpoll,100); \
  u32(lockdebounce,100); \
  u1(doordebug); \
  u1(doorsilent); \
  t(fallback); \
  t(blacklist); \
  t(ragSTART); \

#define u32(n,d) uint32_t n;
#define u16(n,d) uint16_t n;
#define u8(n,d) uint8_t n;
#define u1(n) uint8_t n;
#define t(n) const char*n=NULL;
settings
#undef t
#undef u32
#undef u16
#undef u8
#undef u1
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
char *doorled[] = {
   door_states
};

#undef d

struct {
   uint8_t o,
    i;
   uint8_t state;
   int64_t timeout;
} lock[2] = {
   0
};

uint8_t doorstate = -1;
uint8_t doordeadlock = true;
const char *doorwhy = NULL;

const char *door_access(const uint8_t * a)
{                               // Confirm access
   if (!a)
      return "";                // No action
   if (*a == *afile && !memcmp(a + 1, afile + 1, *afile))
      return "";                // Same
   if (!df.keylen)
      return "";
   xSemaphoreTake(nfc_mutex, portMAX_DELAY);
   memcpy(afile, a, *a + 1);
   const char *e = df_write_data(&df, 0x0A, 'B', DF_MODE_CMAC, 0, *afile + 1, afile);
   if (!e)
      e = df_commit(&df);
   xSemaphoreGive(nfc_mutex);
   if (!e)
      return "";
   return e;
}

const char *door_unlock(const uint8_t * a, const char *why)
{                               // Unlock the door - i.e. exit button, entry allowed, etc.
   if (why && !doorwhy)
      doorwhy = why;
   doordeadlock = false;
   output_set(OUNLOCK + 0, 1);
   output_set(OUNLOCK + 1, 1);
   return door_access(a);
}

const char *door_deadlock(const uint8_t * a)
{                               // Deadlock the door - i.e. move to alarm armed, no exit/entry
   doordeadlock = true;
   output_set(OUNLOCK + 0, 0);
   output_set(OUNLOCK + 1, 0);
   return door_access(a);
}

const char *door_lock(const uint8_t * a)
{                               // Lock the door - i.e. move to normal locked operation
   doordeadlock = false;
   output_set(OUNLOCK + 0, 0);
   output_set(OUNLOCK + 1, 1);
   return door_access(a);
}

const char *door_prop(const uint8_t * a)
{                               // Allow door propping
   if (doorstate != DOOR_OPEN && doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED)
      return false;
   doorstate = DOOR_PROPPED;
   return door_access(a);
}

static int checkfob(const char *fobs, const char *id)
{                               // is fob in list
   if (!fobs || !id)
      return 0;
   int l = strlen(id),
       n = 0;
   if (l && id[l - 1] == '+')
      l--;                      // Don't check secure tag suffix
   while (*fobs)
   {
      n++;
      const char *p = fobs;
      while (*p && *p != ' ')
         p++;
      if (p - fobs == l && !memcmp(fobs, id, l))
         return n;
      while (*p && *p == ' ')
         p++;
      fobs = p;
   }
   return 0;
}

const char *door_fob(char *id, uint32_t * crcp)
{                               // Consider fob
   // Return NULL is access allowed
   // Return string if not
   // - Empty string if not allowed but not an error (e.g. not autonomous door control)
   // - String starting * if not allowed based on access rules
   // - Other string is not allowed based on NFC or DESFire error of some sort
   if (crcp)
      *crcp = 0;
   if (!door)
      return "";                // just not allowed
   if (blacklist && checkfob(blacklist, id))
   {
      if (door >= 4 && df.keylen)
      {                         // Zap the access file
         *afile = 1;
         afile[1] = 0xA0;       // Blacklist
         if (crcp)
            *crcp = df_crc(*afile, afile + 1);
         if (df.keylen)
         {
            const char *e = df_write_data(&df, 0x0A, 'B', DF_MODE_CMAC, 0, *afile + 1, afile);
            if (!e)
               e = df_commit(&df);      // Commit the change, as we will not commit later as access not allowed
            if (e)
               return e;
         }
         return "*Blacklist (zapped)";
      }
      return "*Blacklisted fob";
   }
   if (door >= 4)
   {                            // Autonomous door control logic - check access file and times, etc
      if (!df.keylen)
         return "";             // Don't make a fuss, control system may allow this, and it is obvious
      // Just read file A
      const char *e = df_read_data(&df, 0x0A, DF_MODE_CMAC, 0, MINAFILE, afile);
      if (!e && *afile + 1 > MINAFILE)
         e = df_read_data(&df, 0x0A, DF_MODE_CMAC, 0, *afile + 1 - MINAFILE, afile + MINAFILE);
      if (e)
         return e;
      if (crcp)
         *crcp = df_crc(*afile, afile + 1);
      // Check access file (expected to exist)
      time_t now = time(0);
      uint8_t datetime[7];      // BCD date time
      int xoff = 0,
          xlen = 0,
          xdays = 0;            // Expiry data
      char clockoverride = 0;   // Override time/expiry if clock not set
      char deadlockoverride = 0;        // Override deadlock
      if (*afile)
      {                         // Check access
         uint8_t *p = afile + 1,
             *e = afile + 1 + *afile;
         uint8_t ax = false,
             aok = false;
         uint8_t *fok = NULL,
             *tok = NULL;
         uint8_t dow = bcdtime(now, datetime);
         while (p < e)
         {
            uint8_t l = (*p & 0xF);
            uint8_t c = (*p++ >> 4);
            if (p + l > e)
               return "*Invalid access file";
            if (c == 0x0)
            {                   // Padding, ignore
            } else if (c == 0xC)
               clockoverride = 1;
            else if (c == 0xD)
               deadlockoverride = 1;
            else if (c == 0xA)
            {                   // Allow
               if (!l)
                  return "*Card blocked";       // Black list
               ax = true;
               if (l % 3)
                  return "*Invalid allow list";
               uint8_t n = l;
               while (n && !aok)
               {
                  n -= 3;
                  if ((p[n] << 16) + (p[n + 1] << 8) + p[n + 2] == revk_binid)
                     aok = 1;
               }
            } else if (c == 0xB)
            {                   // Barred
               if (l % 3)
                  return "*Invalid barred list";
               uint8_t n = l;
               while (n)
               {
                  n -= 3;
                  if ((p[n] << 16) + (p[n + 1] << 8) + p[n + 2] == revk_binid)
                     return "*Barred door";
               }
            } else if (c == 0xF)
            {                   // From
               if (fok)
                  return "*Duplicate from time";
               if (l == 2)
                  fok = p;
               else if (l == 4)
                  fok = p + ((dow && dow < 6) ? 2 : 0);
               else if (l == 6)
                  fok = p + (!dow ? 0 : dow < 6 ? 2 : 4);
               else if (l == 14)
                  fok = p + dow * 2;
               else
                  return "*Bad from time";      // Bad time
            } else if (c == 0x2)
            {                   // To
               if (tok)
                  return "*Duplicate to time";
               if (l == 2)
                  tok = p;
               else if (l == 4)
                  tok = p + ((dow && dow < 6) ? 2 : 0);
               else if (l == 6)
                  tok = p + (!dow ? 0 : dow < 6 ? 2 : 4);
               else if (l == 14)
                  tok = p + dow * 2;
               else
                  return "*Bad to time";
            } else if (c == 0xE)
            {                   // Expiry
               if (l == 1)
                  xdays = *p;
               else
               {
                  xoff = p - afile;
                  xlen = l;
                  if (memcmp(datetime, p, l) > 0)
                     return "*Expired"; // expired
               }
            } else
               return "*Unknown access code";   // Unknown access code
            p += l;
         }
         if (ax && !aok)
            return "*Not allowed door"; // Not on allow list
         if (xoff)
         {
            if (*datetime < 0x20)
            {                   // Clock not set
               if (!clockoverride)
                  return "*Date not set";
            } else if (memcmp(datetime, afile + xoff, xlen) > 0)
               return "*Expired";       // expired
         }
         if (fok || tok)
         {                      // Time check
            if (*datetime < 0x20)
            {                   // Clock not set
               if (!clockoverride)
                  return "*Time not set";
            } else if (fok && tok && memcmp(fok, tok, 2) > 0)
            {                   // reverse
               if (memcmp(datetime + 4, fok, 2) < 0 && memcmp(datetime + 4, tok, 2) >= 0)
                  return "*Outside time";
            } else
            {
               if (fok && memcmp(datetime + 4, fok, 2) < 0)
                  return "*Too soon";
               if (tok && memcmp(datetime + 4, tok, 2) >= 0)
                  return "*Too late";
            }
         }
      }
      if (!deadlockoverride && doordeadlock && door < 5)
         return "";             // Quiet about this as normal for system controlled disarm
      if (*datetime >= 0x20 && xdays && xoff && xlen <= 7 && df.keylen)
      {                         // Update expiry
         now += 86400LL * (int64_t) xdays;
         bcdtime(now, datetime);
         if (memcmp(datetime, afile + xoff, xlen) > 0)
         {                      // Changed expiry
            memcpy(afile + xoff, datetime, xlen);
            if (crcp)
               *crcp = df_crc(*afile, afile + 1);
            const char *e = df_write_data(&df, 0x0A, 'B', DF_MODE_CMAC, xoff, xlen, datetime);
            if (e)
               revk_error("fob", "%s Expiry update failed: %s", id, e);
            else
               revk_info("fob", "%s Expiry updated", id);
            // We don't really care if this fails, as we get a chance later, this means the access is allowed so will log and commit later
         }
      }
      // Allowed
      if (doorstate == DOOR_OPEN && door >= 5)
         door_prop(NULL);
      return NULL;
   }
   if (door == 3)
   {
      if (!df.keylen)
         return "";             // Don't make a fuss, control system may allow this, and it is obvious
      if (doordeadlock)
         return "*Door deadlocked";
      return NULL;
   }
   if (revk_offline())
   {
      if (!fallback)
         return "*Offline, and no fallback";
      if (!checkfob(fallback, id))
         return "*Offline, and fallback not matched";
      return NULL;
   }
   return "";                   // Just not allowed
}

static uint8_t resend;

const char *door_command(const char *tag, unsigned int len, const unsigned char *value)
{                               // Called for incoming MQTT messages
   if (!door)
      return false;             // No door control in operation
   if (!len || *value != len - 1)
      value = NULL;             // Not sensible access file (which is arg that we accept here)
   if (!strcasecmp(tag, "deadlock"))
      return door_deadlock(value);
   if (!strcasecmp(tag, "lock"))
      return door_lock(value);
   if (!strcasecmp(tag, "unlock"))
      return door_unlock(value, "remote");
   if (!strcasecmp(tag, "prop"))
      return door_prop(value);
   if (!strcasecmp(tag, "access"))
      return door_access(value);
   if (!strcasecmp(tag, "connect"))
      resend = 1;
   return false;
}

static void task(void *pvParameters)
{                               // Main RevK task
   sleep(1);
   esp_task_wdt_add(NULL);
   pvParameters = pvParameters;
   if (input_get(IOPEN))
   {
      doorstate = DOOR_OPEN;
      output_set(OUNLOCK + 0, 1);       // Start with unlocked doors
      output_set(OUNLOCK + 1, 1);
   } else
   {
      int64_t now = esp_timer_get_time();
      doorstate = DOOR_LOCKING;
      output_set(OUNLOCK + 0, 0);       // Start with locked doors
      output_set(OUNLOCK + 1, 0);
      lock[0].timeout = now + (int64_t) doorlock *1000LL;
      lock[1].timeout = now + (int64_t) doorlock *1000LL;
   }
   while (1)
   {
      esp_task_wdt_reset();
      usleep(1000);             // ms
      int64_t now = esp_timer_get_time();
      static uint64_t doornext = 0;
      static uint8_t lastdoorstate = -1;
      uint8_t iopen = input_get(IOPEN);
      if (doornext < now)
      {
         uint8_t force = resend;
         resend = 0;
         doornext = now + (int64_t) doorpoll *1000LL;
         {                      // Check locks
            int l;
            for (l = 0; l < 2; l++)
            {
               uint8_t last = lock[l].state;
               uint8_t o = output_get(OUNLOCK + l),
                   i = input_get(IUNLOCK + l);
               if (!output_active(OUNLOCK + l))
               {
                  if (!input_active(IUNLOCK + l))
                     lock[l].state = (o ? LOCK_UNLOCKED : LOCK_LOCKED); // No input or output, just track output
                  else
                     lock[l].state = (i ? LOCK_UNLOCKED : LOCK_LOCKED); // No output, just track input
               } else
               {                // Lock state tracking
                  if (((iopen && last == LOCK_LOCKING) || lock[l].o) && !o)
                  {             // Change to lock - timer constantly restarted if door is open as it will not actually engage
                     lock[l].timeout = now + (int64_t) doorlock *1000LL;
                     lock[l].state = LOCK_LOCKING;
                  } else if (o && !lock[l].o)
                  {             // Change to unlock
                     lock[l].timeout = now + (int64_t) doorunlock *1000LL;
                     lock[l].state = LOCK_UNLOCKING;
                  }
                  if (lock[l].timeout)
                  {             // Timeout running
                     if (lock[l].i != i)
                        lock[l].timeout = now + (int64_t) lockdebounce *1000LL; // Allow some debounce before ending timeout
                     if (lock[l].timeout <= now)
                     {          // End of timeout
                        lock[l].timeout = 0;
                        lock[l].state = ((i == o || !input_active(IUNLOCK + l)) ? o ? LOCK_UNLOCKED : LOCK_LOCKED : o ? LOCK_UNLOCKFAIL : LOCK_LOCKFAIL);
                     }
                  } else if (lock[l].i != i)    // Input state change
                     lock[l].state = ((i == o) ? i ? LOCK_UNLOCKED : LOCK_LOCKED : i ? LOCK_FORCED : LOCK_FAULT);
               }
               lock[l].o = o;
               lock[l].i = i;
               if (doordebug && (force || last != lock[l].state))
                  revk_state(l ? "deadlock" : "lock", lock[l].timeout ? "%s %dms" : "%s", lockstates[lock[l].state], (int) (lock[l].timeout - now) / 1000);
            }
         }
         static int64_t doortimeout = 0;
         // Door states
         if (iopen)
         {                      // Open
            if (doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED && doorstate != DOOR_OPEN)
            {                   // We have moved to open state, this can cancel the locking operation
               revk_event("open", "%s", doorwhy ? : "");
               doorwhy = NULL;
               doorstate = DOOR_OPEN;
               if (lock[0].state == LOCK_LOCKING || lock[0].state == LOCK_LOCKFAIL)
                  output_set(OUNLOCK + 0, 1);   // Cancel lock
               if (lock[1].state == LOCK_LOCKING || lock[1].state == LOCK_LOCKFAIL)
                  output_set(OUNLOCK + 1, 1);   // Cancel deadlock
            }
         } else
         {                      // Closed
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
         {                      // State change - set timeout
            if (doorstate == DOOR_OPEN)
               doortimeout = now + (int64_t) doorprop *1000LL;
            else if (doorstate == DOOR_CLOSED)
               doortimeout = now + (int64_t) doorclose *1000LL;
            else if (doorstate == DOOR_UNLOCKED)
               doortimeout = now + (int64_t) dooropen *1000LL;
            else
               doortimeout = 0;
            output_set(OBEEP, doorstate == DOOR_UNLOCKED && !doorsilent ? 1 : 0);
         } else if (doortimeout && doortimeout < now)
         {                      // timeout
            output_set(OBEEP, 0);
            doortimeout = 0;
            if (doorstate == DOOR_OPEN)
               doorstate = DOOR_NOTCLOSED;
            else if (doorstate == DOOR_UNLOCKED || doorstate == DOOR_CLOSED)
            {                   // Time to lock the door
               if (doorstate == DOOR_UNLOCKED)
                  revk_event("notopen", "%s", doorwhy ? : "");
               if (doordeadlock)
                  door_deadlock(NULL);
               else
                  door_lock(NULL);
               doorwhy = NULL;
            }
         }
         static int64_t exit1 = 0;      // Main exit button
         if (input_get(IEXIT1))
         {
            if (!exit1)
            {
               exit1 = now + (int64_t) doorexit *1000LL;
               if (door >= 2 && !doordeadlock)
                  door_unlock(NULL, "button");
            }
         } else
            exit1 = 0;
         static int64_t exit2 = 0;      // Secondary exit button
         if (input_get(IEXIT2))
         {
            if (!exit2)
            {
               exit2 = now + (int64_t) doorexit *1000LL;
               if (door >= 2 && !doordeadlock)
                  door_unlock(NULL, "ranger");
            }
         } else
            exit2 = 0;
         // Check faults
         if (lock[0].state == LOCK_UNLOCKFAIL)
            status(door_fault = "Lock stuck");
         else if (lock[1].state == LOCK_UNLOCKFAIL)
            status(door_fault = "Deadlock stuck");
         else if (lock[0].state == LOCK_FAULT)
            status(door_fault = "Lock fault");
         else if (lock[1].state == LOCK_FAULT)
            status(door_fault = "Deadlock fault");
         else if (exit1 && exit1 < now)
            status(door_fault = "Exit stuck");
         else if (exit2 && exit2 < now)
            status(door_fault = "Ranger stuck");
         else
            status(door_fault = NULL);
         // Check tampers
         if (lock[0].state == LOCK_FORCED)
            status(door_tamper = "Lock forced");
         else if (lock[1].state == LOCK_FORCED)
            status(door_tamper = "Deadlock forced");
         else if (iopen && (lock[0].state == LOCK_LOCKED || lock[1].state == LOCK_LOCKED))
            status(door_tamper = "Door forced");
         else
            status(door_tamper = NULL);
         // Beep
         if (door_tamper || door_fault || doorstate == DOOR_AJAR || doorstate == DOOR_NOTCLOSED)
            output_set(OBEEP, ((now - doortimeout) & (512 * 1024)) ? 1 : 0);
         if (force || doorstate != lastdoorstate)
         {
            nfc_led(strlen(doorled[doorstate]), doorled[doorstate]);
            revk_state("door", doortimeout && doordebug ? "%s %dms" : "%s", doorstates[doorstate], (int) (doortimeout - now) / 1000);
            lastdoorstate = doorstate;
         }
         output_set(OERROR, door_tamper || door_fault);
      }
   }
}

void door_init(void)
{
#define u32(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define t(n) revk_register(#n,0,0,&n,NULL,0);
#define d(n,l) revk_register("rag"#n,0,0,&doorled[DOOR_##n],#l,0);
   settings door_states
#undef t
#undef u32
#undef u16
#undef u8
#undef u1
#undef d
   nfc_led(strlen(ragSTART), ragSTART);
   if (!door)
       return;                  // No door control in operation
   revk_task(TAG, task, NULL);
}
