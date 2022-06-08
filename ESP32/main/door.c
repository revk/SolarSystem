// Solar System
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "door";
#include "SS.h"
#include "alarm.h"
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
#define IOPEN 2
#define IUNLOCK 3
#define OUNLOCK 1
#define OBEEP 3
#define OERROR 4

#define FILE3                   // Check for old file 3 access file
#define MINAFILE  40            // Minimum we read from afile before checking length

#include "nfc.h"
#include "input.h"
#include "output.h"
#include "door.h"

uint8_t afile[256];             // Access file saved

#define settings  \
  u8(doorauto,0);   \
  u32(doorunlock,1000); \
  u32(doorlock,3000); \
  u32(dooropen,5000); \
  u32(doorclose,500); \
  u32(doorprop,60000); \
  u32(doorexit,3000); \
  u32(doorpoll,100); \
  u32(doordebounce,200); \
  b(doordebug); \
  b(doorexitarm); \
  b(doorexitdisarm); \
  b(doorcatch); \
  ta(fallback,10); \
  ta(blacklist,10); \
  s(dooriotdead)	\
  s(dooriotundead)	\
  s(dooriotunlock)	\

#define u32(n,d) uint32_t n;
#define u16(n,d) uint16_t n;
#define u8(n,d) uint8_t n;
#define b(n) uint8_t n;
#define ta(n,c) const char*n[c]={};
#define s(n) char *n;
settings
#undef ta
#undef u32
#undef u16
#undef u8
#undef b
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
const char *doorwhy = NULL;

static area_t door_deadlocked(void)
{
   return areadeadlock & alarm_armed();
}

const char *door_access(const char *id, const uint8_t * a)
{                               // Confirm access
   if (!a)
      return "";                // No action
   // TODO check id if set
   // TODO store cached id/afile if not on line
   if (*a == *afile && !memcmp(a + 1, afile + 1, *afile))
      return "";                // Same
   if (!df.keylen)
      return "";                // Not logged in
   xSemaphoreTake(nfc_mutex, portMAX_DELAY);
   memcpy(afile, a, *a + 1);
   const char *e = df_write_data(&df, 0x0A, 'B', DF_MODE_CMAC, 0, *afile + 1, afile);
   if (!e)
      e = df_commit(&df);
   xSemaphoreGive(nfc_mutex);
   if (!e)
   {
      nfc_retry();
      return "";
   }
   return e;
}

void door_check(void)
{
   if (doorauto >= 2)
      output_set(OUNLOCK + 1, door_deadlocked()? 0 : 1);
   if (areadeadlock)
   {                            // Work out if deadlock is final - as the normal deadlock state pre-empts actual setting so deadlock engaged can be waited on
      uint8_t unlock = !(areadeadlock & state_armed & ~control_disarm); // Delay deadlock state until confirmed
      static uint8_t was = -1;
      if (unlock != was)
      {
         was = unlock;
         if (!unlock && *dooriotdead)
            revk_mqtt_send_str_clients(dooriotdead, 0, 2);
         else if (unlock && *dooriotundead)
            revk_mqtt_send_str_clients(dooriotundead, 0, 2);
      }
   }
}

const char *door_unlock(const char *id, const uint8_t * a, const char *why)
{                               // Unlock the door - i.e. exit button, entry allowed, etc.
   if (doorauto >= 2)
   {
      if (why && !doorwhy)
         doorwhy = why;
      ESP_LOGD(TAG, "Unlock %s", why ? : "?");
      output_set(OUNLOCK + 0, 1);
      output_set(OUNLOCK + 1, 1);
   }
   return door_access(id, a);
}

const char *door_lock(const char *id, const uint8_t * a, const char *why)
{                               // Lock the door - i.e. move to normal locked operation
   if (doorauto >= 2)
   {
      ESP_LOGD(TAG, "Lock %s", why ? : "?");
      output_set(OUNLOCK + 0, 0);
   }
   return door_access(id, a);
}

const char *door_prop(const char *id, const uint8_t * a, const char *why)
{                               // Allow door propping
   if (doorstate != DOOR_OPEN && doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED)
      return false;
   jo_t j = jo_make(NULL);
   if (why)
      jo_string(j, "trigger", why);
   alarm_event("propped", &j, iotstatedoor);
   doorstate = DOOR_PROPPED;
   return door_access(id, a);
}

void door_act(fob_t * fob)
{                               // Act on fob (unlock/lock/arm/disarm)
   jo_t make(void) {            // Create JSON for arm/disarm
      jo_t e = jo_make(NULL);
      jo_string(e, "reason", "fob");
      jo_string(e, "id", fob->id);
      if (fob->nameset)
         jo_string(e, "name", fob->name);
      if (fob->smsset)
         jo_string(e, "sms", fob->sms);
      return e;
   }
   if (fob->strongok && fob->longheld && (fob->strong & areastrong & ~((state_armed | control_strong) & ~control_disarm)))
   {                            // Simple, we can arm (Not using alarm_armed as that includes what we are trying, and failing, to control_arm)
      // Allowing if door open, strong arm is an override, so yeh, suffer the consequences
      if (doorauto >= 5)
      {
         jo_t e = make();
         alarm_strong(fob->strong & areastrong, &e);
         door_lock(fob->id, NULL, "fob");
         fob->stronged = 1;
      }
   }
   if (fob->armok && fob->held && (fob->arm & areaarm & ~alarm_armed()))
   {                            // Simple, we can arm
      if (doorstate == DOOR_OPEN || doorstate == DOOR_PROPPED || doorstate == DOOR_NOTCLOSED)
         return;                // Not if open
      if (doorauto >= 5)
      {
         jo_t e = make();
         alarm_arm(fob->arm & areaarm, &e);
         door_lock(fob->id, NULL, "fob");
         fob->armed = 1;
      }
   }
   if (fob->held)
      return;                   // Other actions were done already
   if (!fob->override && door_deadlocked() && (!fob->enterok || !fob->disarmok ||       //
                                               (fob->disarmok &&        //
                                                (areadeadlock & andset(alarm_armed() & ~(areadisarm & fob->disarm))))))
      return;                   // Same check as Deadlocked - cannot enter or cannot disarm or cannot disarm enough... Unless override
   if (fob->disarmok && (alarm_armed() & fob->disarm & areadisarm))
   {
      if (doorauto >= 5)
      {
         jo_t e = make();
         alarm_disarm(fob->disarm & areadisarm, &e);
         fob->disarmed = 1;
      }
   }
   if ((doorstate == DOOR_OPEN || doorstate == DOOR_NOTCLOSED) && !fob->held && fob->propok)
   {
      if (doorauto >= 4)
      {
         door_prop(fob->id, NULL, "fob");
         fob->propped = 1;
      }
   }
   if (fob->enterok || fob->override)
   {
      if (doorauto >= 4)
      {
         door_unlock(fob->id, NULL, "fob");
         fob->unlocked = 1;
      } else
         fob->unlockok = 1;
   } else if (fob->deny)
   {
      if (doorauto >= 4)
         door_lock(fob->id, NULL, "fob");
   }
}

const char *door_fob(fob_t * fob)
{                               // Consider fob - sets details of action in the fob object
   if (!fob || doorauto < 3 || fob->fail || fob->deny)
      return NULL;              // Do nothing
   // Check the card security
   time_t now = time(0);
   uint8_t datetime[7];         // BCD date time
   int xoff = 0,
       xlen = 0,
       xdays = 0;               // Expiry data
   const char *afilecheck(void) {       // Read Afile
      fob->checked = 1;
      const char *e = df_read_data(&df, 0x0A, DF_MODE_CMAC, 0, MINAFILE, afile);        // Initial
      if (!e && *afile + 1 > MINAFILE)
         e = df_read_data(&df, 0x0A, DF_MODE_CMAC, MINAFILE, *afile + 1 - MINAFILE, afile + MINAFILE);  // More data
      if (e)
      {
         fob->fail = e;
         return "Cannot access Afile";
      }
      fob->afile = 1;
      fob->crc = df_crc(*afile, afile + 1);
      // Check access file (expected to exist)
      if (*afile)
      {                         // Check access
         uint8_t *p = afile + 1,
             *e = afile + 1 + *afile;
         uint8_t *fok = NULL,
             *tok = NULL;
         uint8_t dow = bcdlocaltime(now, datetime);
         while (p < e)
         {                      // Scan the afile
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
               l = 0;           // Flag, so no length
            } else if (c == 0x0)
            {                   // Padding
               if (!l)
                  break;        // end of file
            } else if (c == 0x4)
            {                   // Name
               if (l)
               {
                  memcpy(fob->name, p, l);
                  fob->name[l] = 0;
                  fob->nameset = 1;
               }
            } else if (c == 0x9)
            {                   // Number
               if (l)
               {
                  uint8_t i = 0,
                      o = 0;
                  while (i < l && o < sizeof(fob->sms) - 1)
                  {
                     if ((p[i] & 0xF0) != 0xF0)
                        fob->sms[o++] = '0' + (p[i] >> 4);
                     if ((p[i] & 0x0F) != 0x0F && o < sizeof(fob->sms) - 1)
                        fob->sms[o++] = '0' + (p[i] & 0xF);
                     i++;
                  }
                  fob->sms[o] = 0;
                  fob->smsset = 1;
               }
            } else if (c == 0xA)
            {                   // Arm
               fob->arm = 0;
               for (int q = 0; q < l; q++)
                  fob->arm |= (p[q] << (24 - q * 8));
               fob->armset = 1;
            } else if (c == 0xB)
            {                   // Force arm
               fob->strong = 0;
               for (int q = 0; q < l; q++)
                  fob->strong |= (p[q] << (24 - q * 8));
               fob->strongset = 1;
            } else if (c == 0xC)
            {                   // Prop
               fob->prop = 0;
               for (int q = 0; q < l; q++)
                  fob->prop |= (p[q] << (24 - q * 8));
               fob->propset = 1;
            } else if (c == 0xD)
            {                   // Disarm
               fob->disarm = 0;
               for (int q = 0; q < l; q++)
                  fob->disarm |= (p[q] << (24 - q * 8));
               fob->disarmset = 1;
            } else if (c == 0xE)
            {                   // Enter
               fob->enter = 0;
               for (int q = 0; q < l; q++)
                  fob->enter |= (p[q] << (24 - q * 8));
               fob->enterset = 1;
            } else if (c == 0x1)
            {                   // From
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
                  return "Bad from time";       // Bad time
            } else if (c == 0x2)
            {                   // To
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
            } else if (c == 0x3)
            {                   // Expiry
               if (l == 1)
                  xdays = *p;
               else
               {
                  xoff = p - afile;
                  xlen = l;
                  if (memcmp(datetime, p, l) > 0)
                     return "Expired";  // expired
               }
            } else
               return "Unknown access code";    // Unknown access code
            p += l;
         }
         if (xoff)
         {
            if (*datetime < 0x20)
            {                   // Clock not set
               if (!fob->clock)
                  return "Date not set";
            } else if (memcmp(datetime, afile + xoff, xlen) > 0)
               return "Expired";        // expired
         }
         if (fok || tok)
         {                      // Time check
            const char *e = NULL;
            if (*datetime < 0x20)
            {                   // Clock not set
               if (!fob->clock)
                  e = "Time not set";
            } else if (fok && tok && memcmp(fok, tok, 2) > 0)
            {                   // reverse
               if (memcmp(datetime + 4, fok, 2) < 0 && memcmp(datetime + 4, tok, 2) >= 0)
                  e = "Outside time";
            } else
            {
               if (fok && memcmp(datetime + 4, fok, 2) < 0)
                  e = "Too soon";
               if (tok && memcmp(datetime + 4, tok, 2) >= 0)
                  e = "Too late";
            }
            if (e)
            {
               // Consider armlate to say if amrok even whilst out of time
               if (fob->armlate && (areaarm & fob->arm))
                  fob->armok = 1;
               return e;
            }
         }
      }
      if (fob->block)
         return "Card blocked";
      // We are OK and in time...
      if (areaarm & fob->arm)
         fob->armok = 1;        // Can arm if any areas can be armed
      if (areastrong & fob->strong)
         fob->strongok = 1;     // Can arm is any areas can be armed
      if (areadisarm & fob->disarm)
         fob->disarmok = 1;     // Can disarm if any areas can be disarmed, but will only do so if can enter when disarmed
      if (!(areaenter & ~fob->enter))
         fob->enterok = 1;      // Can enter if all enter areas covered
      if (!(areaenter & ~fob->prop))
         fob->propok = 1;       // Can prop if all enter areas covered
      if (door_deadlocked() && (!fob->enterok || !fob->disarmok ||      //
                                (fob->disarmok &&       //
                                 (areadeadlock & andset(alarm_armed() & ~(areadisarm & fob->disarm))))))
         return "Deadlocked";   // We could not enter, or could not disarm, or could not disarm enough to get in
      return NULL;
   }
   if (fob->secure && df.keylen)
      fob->deny = afilecheck();
   // Check fallback
   if (*fob->id)
      for (int i = 0; i < sizeof(fallback) / sizeof(*fallback); i++)
         if (!strcmp(fallback[i], fob->id))
         {
            fob->fallback = 1;
            fob->enterok = 1;
            break;
         }
   // Check blacklist
   if (*fob->id)
      for (int i = 0; i < sizeof(blacklist) / sizeof(*blacklist); i++)
         if (!strcmp(blacklist[i], fob->id))
         {
            fob->blacklist = 1;
            fob->enterok = 0;
            fob->propok = 0;
            fob->disarmok = 0;
            fob->armok = 0;
            fob->strongok = 0;
            if (!fob->deny)
               fob->deny = "Blacklist";
            if (fob->secure && df.keylen)
            {
               if (*afile != 1 || afile[1] != 0xFB)
               {
                  *afile = 1;
                  afile[1] = 0xFB;      // Blocked
                  fob->crc = df_crc(*afile, afile + 1);
                  const char *e = df_write_data(&df, 0x0A, 'B', DF_MODE_CMAC, 0, *afile + 1, afile);
                  if (!e)
                     fob->updated = 1;
               }
            }
            break;
         }
   if (fob->held)
      return NULL;
   if (!fob->deny && fob->secure && df.keylen && *datetime >= 0x20 && xdays && xoff && xlen <= 7)
   {                            // Update expiry
      now += 86400LL * (int64_t) xdays;
      bcdutctime(now, datetime);
      if (memcmp(datetime, afile + xoff, xlen) > 0)
      {                         // Changed expiry
         memcpy(afile + xoff, datetime, xlen);
         fob->crc = df_crc(*afile, afile + 1);
         if (!df_write_data(&df, 0x0A, 'B', DF_MODE_CMAC, xoff, xlen, datetime))
            fob->updated = 1;
         // We don't really care if this fails, as we get a chance later, this means the access is allowed so will log and commit later
      }
   }

   return NULL;
}

static uint8_t resend;

const char *door_command(const char *tag, jo_t j)
{                               // Called for incoming MQTT messages
   if (!doorauto)
      return false;             // No door control in operation
   const char *e = NULL;
   char temp[256],
    tempid[22];
   const uint8_t *afile = NULL;
   const char *id = NULL;
   if (j)
   {
      if (jo_here(j) == JO_OBJECT)
      {                         // New format with id and afile
         while (jo_here(j))
         {
            jo_next(j);
            if (jo_here(j) == JO_TAG)
            {
               if (!jo_strcmp(j, "id"))
               {
                  int len = jo_strncpy(j, tempid, sizeof(tempid));
                  if (len > 0 && len < sizeof(tempid))
                     id = tempid;

               } else if (!jo_strcmp(j, "afile"))
               {
                  int len = jo_strncpy16(j, temp, sizeof(temp));
                  if (len < 0)
                     e = "Bad hex";
                  else if (len > 0 && *temp == len - 1)
                     afile = (uint8_t *) temp;
                  else
                     e = "Bad afile";
               }
            }
         }
      } else if (jo_here(j) == JO_STRING)
      {                         // Afile only
         int len = jo_strncpy16(j, temp, sizeof(temp));
         if (len < 0)
            e = "Bad hex";
         else if (len > 0 && *temp == len - 1)
            afile = (uint8_t *) temp;
         else
            e = "Bad afile";
      } else
         e = "Expecting JSON";
      if (!e)
         e = jo_error(j, NULL);
   }
   if (!strcasecmp(tag, "lock"))
      return e ? : door_lock(id, afile, "remote");
   if (!strcasecmp(tag, "unlock"))
      return e ? : door_unlock(id, afile, "remote");
   if (!strcasecmp(tag, "prop"))
      return e ? : door_prop(id, afile, "remote");
   if (!strcasecmp(tag, "access"))
      return e ? : door_access(id, afile);
   if (!strcasecmp(tag, "connect"))
      resend = 1;
   return NULL;
}

static void task(void *pvParameters)
{                               // Main RevK task
   esp_task_wdt_add(NULL);
   pvParameters = pvParameters;
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
                     if (doorlock)
                        lock[l].timeout = now + (int64_t) doorlock *1000LL;
                     lock[l].state = LOCK_LOCKING;
                  } else if (o && !lock[l].o)
                  {             // Change to unlock
                     if (doorunlock)
                        lock[l].timeout = now + (int64_t) doorunlock *1000LL;
                     lock[l].state = LOCK_UNLOCKING;
                  }
                  if (lock[l].timeout)
                  {             // Timeout running
                     if (lock[l].i != i && doordebounce)
                        lock[l].timeout = now + (int64_t) doordebounce *1000LL; // Allow some debounce before ending timeout
                     if (lock[l].timeout <= now)
                     {          // End of timeout
                        lock[l].timeout = 0;
                        lock[l].state = ((i == o || !input_active(IUNLOCK + l)) ? o ? LOCK_UNLOCKED : LOCK_LOCKED : o ? LOCK_UNLOCKFAIL : LOCK_LOCKFAIL);
                     }
                  } else if (!doorunlock)
                  {             // Zero timeout means lock input only works when handle, e.g. Abloy EL56X. Treat as following o
                     lock[l].state = ((!i && iopen) ? LOCK_FAULT : o ? LOCK_UNLOCKED : LOCK_LOCKED);
                  } else if (lock[l].i != i)    // Input state change
                     lock[l].state = ((i == o) ? i ? LOCK_UNLOCKED : LOCK_LOCKED : i ? LOCK_FORCED : LOCK_FAULT);
               }
               lock[l].o = o;
               lock[l].i = i;
               if (doordebug && (force || last != lock[l].state))
               {
                  jo_t j = jo_make(NULL);
                  jo_string(j, "state", lockstates[lock[l].state]);
                  if (lock[l].timeout > now)
                     jo_int(j, "timeout", (lock[l].timeout - now) / 1000);
                  revk_state_clients(l ? "deadlock" : "lock", &j, debug | (iotstatedoor << 1));
               }
            }
         }
         static int64_t doortimeout = 0;
         // Door states
         if (iopen)
         {                      // Open
            if (doorstate != DOOR_NOTCLOSED && doorstate != DOOR_PROPPED && doorstate != DOOR_OPEN)
            {                   // We have moved to open state, this can cancel the locking operation
               const char *manual = input_func_any(INPUT_FUNC_M);       // If a manual input was found (uses input name)
               char forced = ((output_active(OUNLOCK) && ((!manual && lock[0].state == LOCK_LOCKED) || lock[0].state == LOCK_FORCED)) ||        //
                              (output_active(OUNLOCK + 1) && ((!manual && lock[1].state == LOCK_LOCKED) || lock[1].state == LOCK_FORCED)));
               if (!doorwhy)
                  doorwhy = (forced ? "forced" : manual ? : "manual");
               if (doorwhy)
               {
                  jo_t j = jo_make(NULL);
                  jo_string(j, "trigger", doorwhy);
                  alarm_event(forced ? "forced" : "open", &j, iotstatedoor);
                  doorwhy = NULL;
               }
               if (doorstate != DOOR_UNLOCKING && *dooriotunlock)
                  revk_mqtt_send_str_clients(dooriotunlock, 0, 2);      // We skipped unlocking...
               doorstate = DOOR_OPEN;
               if (doorauto >= 2 && !doorcatch)
               {
                  output_set(OUNLOCK + 0, 1);   // Cancel lock
                  output_set(OUNLOCK + 1, 1);   // Cancel deadlock
               }
               if (forced && !(logical_gpio & logical_DoorForce))
                  logical_gpio |= logical_DoorForce;
            }
            if (doorauto >= 2 && doorcatch)
               output_set(OUNLOCK + 0, 0);      // Door lock ready for when it closes
         } else
         {                      // Closed
            if (logical_gpio & logical_DoorForce)
               logical_gpio &= ~logical_DoorForce;
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
            {
               if (doorstate == DOOR_NOTCLOSED || doorstate == DOOR_PROPPED)
               {
                  jo_t j = jo_make(NULL);
                  alarm_event("closed", &j, iotstatedoor);
               }
               doorstate = ((doorstate == DOOR_OPEN || doorstate == DOOR_NOTCLOSED || doorstate == DOOR_PROPPED || doorstate == DOOR_CLOSED) ? DOOR_CLOSED : DOOR_UNLOCKED);
            }
         }
         if (doorstate != lastdoorstate)
         {                      // State change - set timeout
            if (doorstate == DOOR_OPEN && doorprop)
               doortimeout = now + (int64_t) doorprop *1000LL;
            else if (doorstate == DOOR_CLOSED && doorclose)
               doortimeout = now + (int64_t) doorclose *1000LL;
            else if (doorstate == DOOR_UNLOCKED && dooropen)
               doortimeout = now + (int64_t) dooropen *1000LL;
            else
               doortimeout = 0;
         } else if (doortimeout && doortimeout < now)
         {                      // Timeout happened
            if (doorauto >= 2)
               output_set(OBEEP, 0);
            if (doorstate == DOOR_OPEN)
            {
               doorstate = DOOR_NOTCLOSED;
               doortimeout = 0;
               jo_t j = jo_make(NULL);
               alarm_event("notclosed", &j, iotstatedoor);
            } else if (doorstate == DOOR_UNLOCKED || doorstate == DOOR_CLOSED)
            {                   // Time to lock the door
               if (doorstate == DOOR_UNLOCKED && doorwhy)
               {                // Only if doorwhy set, as can spam if locking failing
                  jo_t j = jo_make(NULL);
                  jo_string(j, "trigger", doorwhy);
                  alarm_event("notopen", &j, iotstatedoor);
               }
               door_lock(NULL, NULL, NULL);
               doorwhy = NULL;
            }
         }
         if (doorstate != lastdoorstate)
         {                      // Actions on state change
            if (doorstate == DOOR_NOTCLOSED)
               logical_gpio |= logical_DoorProp;        // Always a change of state - unauthorised propped
            else if (lastdoorstate == DOOR_NOTCLOSED)
               logical_gpio &= ~logical_DoorProp;       // Always a change of state - unauthorised propped
            if (doorstate == DOOR_UNLOCKED && *dooriotunlock)
               revk_mqtt_send_str_clients(dooriotunlock, 0, 2);
            if (doorauto >= 2)
               output_set(OBEEP, doorstate == DOOR_UNLOCKED ? 1 : 0);
         }
         static uint64_t exit = 0;      // Main exit button
         if (input_get(IEXIT1))
         {
            if (!exit)
            {                   // Pushed
               jo_t j = jo_make(NULL);
               exit = now + (int64_t) doorexit *1000LL; // Exit button timeout
               jo_area(j, "disarmok", doorexitdisarm & areadeadlock);
               if (door_deadlocked() && doorexitdisarm && !(alarm_armed() & ~(areadeadlock & areadisarm)))
               {
                  jo_t e = jo_make(NULL);
                  jo_string(e, "reason", "exit button");
                  alarm_disarm(areadeadlock & areadisarm, &e);
                  jo_bool(j, "disarmed", 1);
               }
               if (!door_deadlocked())
               {
                  if (doorauto >= 2)
                  {
                     door_unlock(NULL, NULL, "button");
                     jo_bool(j, "unlocked", 1);
                  } else
                     jo_bool(j, "unlockok", 1);
               }
               alarm_event("button", &j, iotstatedoor);
            } else if (doorexitarm && exit && exit < now)
            {                   // Held (not applicable if not arming allowed, so leaves to do exit stuck fault)
               exit = -1;       // Don't report stuck - this is max value as unsigned
               // Allowed even if door open, one assumes this is not used for an actual alarm if being set from inside
               jo_t j = jo_make(NULL);
               jo_bool(j, "held", 1);
               if (areadeadlock & areaarm)
               {
                  jo_area(j, "armok", areadeadlock & areaarm);
                  if (doorauto >= 2 && (areadeadlock & areaarm & ~alarm_armed()))
                  {
                     jo_t e = jo_make(NULL);
                     jo_string(e, "reason", "exit button");
                     alarm_arm(areadeadlock & areaarm, &e);
                     jo_bool(j, "armed", 1);
                     door_lock(NULL, NULL, "button");
                  }
               }
               alarm_event("button", &j, iotstatedoor);
            }
         } else
            exit = 0;

         // Check faults
         const char *fault = NULL;
         if (lock[0].state == LOCK_UNLOCKFAIL)
            fault = "Lock stuck";
         else if (lock[1].state == LOCK_UNLOCKFAIL)
            fault = "Deadlock stuck";
         else if (lock[0].state == LOCK_FAULT)
            fault = "Lock fault";
         else if (lock[1].state == LOCK_FAULT)
            fault = "Deadlock fault";
         else if (exit && exit < now)
            fault = "Exit stuck";
         if (fault)
         {
            if (!(logical_gpio & logical_LockFault))
            {                   // new fault
               jo_t j = jo_make(NULL);
               jo_string(j, "lock", fault);
               alarm_event("fault", &j, iotstatedoor);
               logical_gpio |= logical_LockFault;
            }
         } else if (logical_gpio & logical_LockFault)
            logical_gpio &= ~logical_LockFault;

         // Note that forced are not logged as tampers, and picked up directly for alarm from open/disengaged inputs showing as access
         // Beep
         if (doorauto >= 2 && (fault || doorstate == DOOR_AJAR || doorstate == DOOR_NOTCLOSED))
            output_set(OBEEP, ((now - doortimeout) & (512 * 1024)) ? 1 : 0);
         if (force || doorstate != lastdoorstate)
         {
            nfc_led(strlen(doorled[doorstate]), doorled[doorstate]);
            jo_t j = jo_make(NULL);
            jo_string(j, "state", doorstates[doorstate]);
            if (doorwhy && doorstate == DOOR_UNLOCKING)
               jo_string(j, "trigger", doorwhy);
            if (doortimeout > now)
               jo_int(j, "timeout", (doortimeout - now) / 1000);
            revk_state_clients("door", &j, debug | (iotstatedoor << 1));
            lastdoorstate = doorstate;
         }
         if (doorauto >= 2)
            output_set(OERROR, fault ? 1 : 0);
      }
   }
}

void door_boot(void)
{
   extern char *ledIDLE;
   revk_register("door", 0, sizeof(doorauto), &doorauto, NULL, SETTING_SECRET); // Parent
   revk_register("led", 0, 0, &ledIDLE, NULL, SETTING_SECRET);  // Parent
#define u32(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define ta(n,c) revk_register(#n,c,0,&n,NULL,SETTING_LIVE);
#define d(n,l) revk_register("led"#n,0,0,&doorled[DOOR_##n],#l,0);
#define s(n) revk_register(#n,0,0,&n,NULL,SETTING_LIVE);
   settings door_states
#undef ta
#undef u32
#undef u16
#undef u8
#undef b
#undef d
#undef s
       // Initial states before output starts
    int64_t now = esp_timer_get_time();
   if (input_get(IOPEN))
   {
      doorstate = DOOR_OPEN;
      if (doorauto >= 2 && !doorcatch)
         output_set(OUNLOCK + 0, 1);    // Start with unlocked doors
      lock[0].state = LOCK_UNLOCKING;
      lock[1].state = LOCK_UNLOCKING;
   } else
   {
      doorstate = DOOR_LOCKING;
      if (doorauto >= 2 && doorcatch)
         output_set(OUNLOCK + 0, 0);    // Start with locked doors
      lock[0].state = LOCK_LOCKING;
      lock[1].state = LOCK_LOCKING;
   }
   if (doorauto >= 2 && doorcatch)
      output_set(OUNLOCK + 0, 0);       // Start with locked doors
   lock[0].timeout = now + 1000LL;
   lock[1].timeout = now + 1000LL;
}

void door_start(void)
{
   if (!doorauto)
      return;                   // No door control in operation
   revk_task(TAG, task, NULL);
}

const char *door_state_name(void)
{
   if (!doorauto)
      return NULL;
   return doorstates[doorstate];
}
