// NFC reader interface - working with door control
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "nfc";
#include "SS.h"

#ifndef	CONFIG_REVK_LOCK_CPU1
#warning	Better with CONFIG_REVK_LOCK_CPU1
#endif

#include "desfireaes.h"
#include "alarm.h"
#include "nfc.h"
#include "door.h"
#include "pn532.h"
#include <driver/uart.h>
#include <driver/gpio.h>
#include <aes/esp_aes.h>

#define port_mask(p) ((p)&0x3F)
#define	BITFIELDS "-"
#define PORT_INV 0x40
#define GPIO_INV 0x80           // No SETTING bit

int16_t
gpio_mask (uint8_t p)
{
   if (!p)
      return -1;                // Invalid (bit set if port is set)
   p &= 0x7F;                   // Does not have SETTING, so just invert at top bit
   if (p >= 30 && p <= 35)
      return p - 30;
   if (p >= 71 && p <= 72)
      return p + 6 - 71;        // Yes, does not work for one byte port with two bits
   return -1;                   // Invalid
}

// Other settings
#define settings  \
  gpio(nfcred) \
  gpio(nfcamber) \
  gpio(nfcgreen) \
  gpio(nfccard) \
  io(nfctx) \
  io(nfcrx) \
  io(nfcpower) \
  u16(nfcpoll,50) \
  u16(nfcholdpoll,500) \
  u8(nfchold,6) \
  u8(nfclonghold,20) \
  u16(nfcledpoll,100) \
  u16(nfciopoll,200) \
  u8(nfcuart,1) \
  u8f(nfcbaud,CONFIG_NFC_BAUD_CODE) \
  t(nfcmqttbell,NULL) \
  bap(aes,18,3) \
  b(aid,3) \
  t(ledIDLE,"3R3-") \

#define i8(n,d) int8_t n;
#define io(n) uint8_t n;
#define gpio(n) uint8_t n;
#define u8(n,d) uint8_t n;
#define u8f(n,d) uint8_t n;
#define u16(n,d) uint16_t n;
#define b(n,l) uint8_t n[l];
#define bap(n,l,a) uint8_t n[a][l];
#define u1(n) uint8_t n;
#define t(n,d) const char*n=NULL;
settings
#undef t
#undef i8
#undef io
#undef gpio
#undef u8
#undef u8f
#undef u16
#undef b
#undef bap
#undef u1
   pn532_t * pn532 = NULL;
uint8_t nfcmask = 0,
   nfcinvert = 0;
df_t df;
SemaphoreHandle_t nfc_mutex = NULL;     // PN532 has low level message mutex, but this is needed for DESFire level.

static uint8_t ledpattern[20] = "";

static fob_t fob = { 0 };       // Current card state

uint8_t
key_type (uint8_t keyid)
{                               // Get key type
   if (keyid >= sizeof (aes) / sizeof (*aes))
      return 0;
   return aes[keyid][0];
}

uint8_t
key_ver (uint8_t keyid)
{                               // Get key version
   if (keyid >= sizeof (aes) / sizeof (*aes))
      return 0;
   if (!aes[keyid][0])
      return 0;
   return aes[keyid][1];        // The key version
}

uint8_t *
key_aes (uint8_t keyid, uint8_t temp[16])
{                               // Get actual key
   if (keyid >= sizeof (aes) / sizeof (*aes))
      return NULL;
   if (!aes[keyid][0])
      return NULL;
   if (aes[keyid][0] == 1)
      return aes[keyid] + 2;    // Direct key use
   // Encrypted key
   if (aes[keyid][0] != 2)
      return NULL;
   if (!temp)
      return NULL;
   // Encrypt key based on fobid.
   uint8_t id[16] = { 0 };
   strncpy ((char *) id, fob.id, 16);
   esp_aes_context ctx;
   esp_aes_init (&ctx);
   esp_err_t err = esp_aes_setkey (&ctx, aes[keyid] + 2, 128);
   if (!err)
      err = esp_internal_aes_encrypt (&ctx, id, temp);
   esp_aes_free (&ctx);
   if (err)
      return NULL;
   return temp;
}

const char *
nfc_led (int len, const void *value)
{
   if (!len)
      len = strlen (value = ledIDLE);   // Default
   jo_t j = jo_object_alloc ();
   jo_stringf (j, "sequence", "%.*s", len, value);
   revk_info_clients ("led", &j, debug | (ioteventfob << 1));
   if (len > sizeof (ledpattern))
      len = sizeof (ledpattern);
   if (len < sizeof (ledpattern))
      ledpattern[len] = 0;
   if (len)
      memcpy (ledpattern, value, len);
   return "";
}

static void
fobevent (const uint8_t * ats, const uint8_t ver[28])
{
   jo_t j = jo_make (NULL);
   if (*fob.id)
   {
      if (fob.secureset)
         jo_bool (j, "secure", fob.secure);
      jo_string (j, "id", fob.id);
      if (!fob.secure && ats && *ats)
         jo_base16 (j, "ats", ats, *ats);
      if (ver)
         jo_base16 (j, "mifare-version", ver, 28);
      if (fob.nameset)
         jo_string (j, "name", fob.name);
      if (fob.smsset)
         jo_string (j, "sms", fob.sms);
      if (fob.verset)
         jo_stringf (j, "ver", "%02X", fob.ver);
      if (fob.keyupdated)
         jo_bool (j, "keyupdated", fob.keyupdated);
      if (fob.fail)
         jo_string (j, "fail", fob.fail);
      if (fob.deny)
         jo_string (j, "deny", fob.deny);
      if (fob.updated)
         jo_bool (j, "updated", fob.updated);
      if (fob.logged)
         jo_bool (j, "logged", fob.logged);
      if (fob.counted)
         jo_bool (j, "counted", fob.counted);
      if (fob.checked)
         jo_bool (j, "checked", fob.checked);
      if (fob.held)
         jo_bool (j, "held", fob.held);
      if (fob.longheld)
         jo_bool (j, "longheld", fob.longheld);
      if (fob.gone)
         jo_bool (j, "gone", fob.gone);
      if (fob.block)
         jo_bool (j, "block", fob.block);
      if (fob.blacklist)
         jo_bool (j, "blacklist", fob.blacklist);
      if (fob.fallback)
         jo_bool (j, "fallback", fob.fallback);
      if (fob.unlocked)
         jo_bool (j, "unlocked", fob.unlocked);
      if (fob.disarmed)
         jo_bool (j, "disarmed", fob.disarmed);
      if (fob.armed)
         jo_bool (j, "isarmed", fob.armed);
      if (fob.stronged)
         jo_bool (j, "stronged", fob.stronged);
      if (fob.unlockok)
         jo_bool (j, "unlockok", fob.unlockok);
      if (fob.enterok)
         jo_bool (j, "enterok", fob.enterok);
      if (fob.propok)
         jo_bool (j, "propok", fob.propok);
      if (fob.propped)
         jo_bool (j, "propped", fob.propped);
      if (fob.disarmok)
         jo_area (j, "disarmok", fob.disarm & areadisarm);
      if (fob.armok)
         jo_area (j, "armok", fob.arm & areaarm);
      if (fob.strongok)
         jo_area (j, "strongok", fob.strong & areaarm);
      if (fob.armlate)
         jo_bool (j, "armlate", fob.armlate);
      if (fob.afile)
         jo_stringf (j, "crc", "%08X", fob.crc);
      if (fob.override)
         jo_bool (j, "override", fob.override);
      if (door_state_name ())
         jo_string (j, "door", door_state_name ());
   }
   if (fob.remote)
      jo_bool (j, "remote", 1);
   alarm_event ("fob", &j, ioteventfob);
}

void
nfc_retry (void)
{
   fob.recheck = 1;
   ESP_LOGD (TAG, "NFC permissions re-check");
}

static void
task (void *pvParameters)
{
   esp_task_wdt_add (NULL);
   if (!nfciopoll)
      nfciopoll = 100;          // Should not happen
   pvParameters = pvParameters;
   int64_t found = 0;
   int64_t nextpoll = 0;        // Timers
   int64_t nextled = 0;
   uint8_t countled = 0;
   int64_t nextio = 0;
   uint8_t ledlast = 0xFF;
   uint8_t ledpos = 0;
   uint8_t retry = 0;
   uint8_t holdpolls = 0;
   uart_set_line_inverse (nfcuart, ((PORT_INV & nfctx) ? UART_SIGNAL_TXD_INV : 0) + ((PORT_INV & nfcrx) ? UART_SIGNAL_RXD_INV : 0));    // Allow for inverted pins
   gpio_set_pull_mode (port_mask (nfcrx), (PORT_INV & nfcrx) ? GPIO_PULLUP_ONLY : GPIO_FLOATING);       // If inverted rx, set pull up, as will be a FET
   while (1)
   {
      esp_task_wdt_reset ();
      usleep (1000);
      int64_t now = esp_timer_get_time ();
      // Regular tasks
      if (nextio < now)
      {                         // Check tamper
         nextio += (int64_t) nfciopoll *1000LL;
         int p3 = -1;
         if (pn532)
         {                      // Connected, get port
            p3 = pn532_read_GPIO (pn532);
            if (p3 >= 0)
            {
               logical_gpio = ((logical_gpio & ~0xFF) | p3);
               retry = 0;
            } else
            {
               nextio = now;    // Try again right away
               ESP_LOGD (TAG, "Retry %d", retry + 1);
               if (retry++ >= 10)
               {                // We don't expect this in normal operation, but some flash operations seem to stall serial a bit
                  pn532 = pn532_end (pn532);
                  logical_gpio |= logical_NFCFault;
               }
            }
         }
         if (!pn532)
         {                      // In failed state
            static uint8_t wait = 0,
               on = 1;
            if (wait)
               wait--;
            if (wait)
               continue;
            if (on)
            {                   // Try talking to it
               ESP_LOGE (TAG, "NFC re-init");
               pn532 = pn532_init (nfcuart, nfcbaud, port_mask (nfctx), port_mask (nfcrx), nfcmask);
               if (!pn532)
                  pn532 = pn532_init (nfcuart, nfcbaud, port_mask (nfctx), port_mask (nfcrx), nfcmask);
               if (pn532)
               {                // All good!
                  df_init (&df, pn532, pn532_dx);
                  ledlast = 0xFF;
                  logical_gpio &= ~logical_NFCFault;
               } else
               {                // Failed
                  on = 0;
                  gpio_reset_pin (port_mask (nfcrx));   // Don't drive
                  gpio_reset_pin (port_mask (nfctx));   // Don't drive
                  if (nfcpower)
                     gpio_set_level (port_mask (nfcpower), (nfcpower & PORT_INV) ? 1 : 0);      // Off
                  wait = 1000 / nfciopoll;      // off wait
               }
            } else
            {                   // Off, so turn on
               on = 1;
               if (nfcpower)
                  gpio_set_level (port_mask (nfcpower), (nfcpower & PORT_INV) ? 0 : 1); // On
               wait = 500 / nfciopoll;  // on wait
            }
         }
         if (!pn532)
            continue;           // No point doing any more
      }
      // LED
      void blink (char r, char a, char g)
      {                         // Blink an LED
         uint8_t newled = 0;
         if (nfcred && r)
            newled |= (1 << gpio_mask (nfcred));
         if (nfcamber && a)
            newled |= (1 << gpio_mask (nfcamber));
         if (nfcgreen && g)
            newled |= (1 << gpio_mask (nfcgreen));
         if (newled != ledlast)
            pn532_write_GPIO (pn532, (ledlast = newled) ^ nfcinvert);
         pn532_write_GPIO (pn532, ledlast ^ nfcinvert);
         countled = 5;          // Hold for a bit
         ledpos = 0;
      }
      if (nextled < now)
      {                         // Check LED
         nextled = now + (int64_t) nfcledpoll *1000LL;
         if (countled)
            countled--;         // We are repeating existing pattern for a while
         if (!countled)
         {                      // Next colour
            ledpos++;
            if (ledpos >= sizeof (ledpattern) || !ledpattern[ledpos] || !*ledpattern)
               ledpos = 0;      // Wrap
            uint8_t newled = 0;
            while (ledpos < sizeof (ledpattern) && ledpattern[ledpos] && isdigit (ledpattern[ledpos]))
               countled = countled * 10 + ledpattern[ledpos++] - '0';
            while (ledpos < sizeof (ledpattern) && ledpattern[ledpos])
            {                   // Check combined colours
               if (nfcred && ledpattern[ledpos] == 'R')
                  newled |= (1 << gpio_mask (nfcred));
               if (nfcamber && ledpattern[ledpos] == 'A')
                  newled |= (1 << gpio_mask (nfcamber));
               if (nfcgreen && ledpattern[ledpos] == 'G')
                  newled |= (1 << gpio_mask (nfcgreen));
               if (nfccard && found)
               {                // Blinky
                  if (holdpolls & 1)
                     newled &= ~(1 << gpio_mask (nfccard));
                  else
                     newled |= (1 << gpio_mask (nfccard));
               }
               if (ledpos + 1 >= sizeof (ledpattern) || ledpattern[ledpos + 1] != '+')
                  break;        // Combined LED pattern with +
               ledpos += 2;
            }
            if (state_prearm && (ledpos & 1) && nfcred)
               newled ^= (1 << gpio_mask (nfcred));     // Extra blinky when prearm... Not a nice bodge
            if (newled != ledlast)
               pn532_write_GPIO (pn532, (ledlast = newled) ^ nfcinvert);
         }
      }
      // Card
      if (nextpoll < now)
      {                         // Check for card
         nextpoll = now + (int64_t) nfcpoll *1000LL;    // Default polling
         if (found && !pn532_Present (pn532))
         {                      // Card gone
            ESP_LOGI (TAG, "gone %s", fob.id);
            fob.gone = 1;
            if (fob.remote || (fob.held && nfchold) || (fob.longheld && nfclonghold) || fob.deny || fob.fail)
               fobevent (NULL, NULL);   // Report as was help, or was still failed
            memset (&fob, 0, sizeof (fob));
            found = 0;
            holdpolls = 0;
         }
         if (found && !fob.recheck)
         {
            if (holdpolls < 255)
               holdpolls++;
            nextpoll = now + (int64_t) nfcholdpoll *1000LL;     // Polling while held
            if (!fob.remote && !fob.held && nfchold && holdpolls >= nfchold)
            {                   // Card has been held for a while, report
               fob.held = 1;
               fob.deny = NULL; // Re-evaluate as held
               door_fob (&fob);
               door_act (&fob); // Action from held
               fobevent (NULL, NULL);
            }
            if (!fob.remote && !fob.longheld && nfclonghold && holdpolls >= nfclonghold)
            {                   // Card has been held for a while, report
               fob.longheld = 1;
               fob.deny = NULL; // Re-evaluate as long held
               door_fob (&fob);
               door_act (&fob); // Action from long held
               fobevent (NULL, NULL);
            }
            continue;           // Waiting for card to go
         }
         // Check for new card
         df_deauth (&df);
         int cards = 1;
         if (!fob.recheck)
            cards = pn532_Cards (pn532);
         if (cards > 1)
         {
            ESP_LOGI (TAG, "Release 2nd card");
            pn532_release (pn532, 2);
            nextpoll = 0;
         } else if (cards > 0)
         {                      // Check for new card
            if (fob.recheck)
               memset (&fob, 0, sizeof (fob));  // Fresh start
            xSemaphoreTake (nfc_mutex, portMAX_DELAY);
            nextpoll = now + (int64_t) nfcholdpoll *1000LL;     // Polling while held
            const char *e = NULL;
            uint8_t *ats = pn532_ats (pn532);
            pn532_nfcid (pn532, fob.id);
            if (*ats && ats[1] == 0x78)
               fob.iso = 1;
            if (!fob.remote && key_type (0) && (aid[0] || aid[1] || aid[2]) && *ats && ats[1] == 0x75)
            {                   // DESFire
               fob.secureset = 1;       // We checked security
               // Select application
               if (!e)
                  e = df_select_application (&df, aid);
               if (e)
               {
                  if (!strstr (e, "TIMEOUT"))
                     e = NULL;  // Just treat as insecure
               } else
               {
                  if (!e && key_type (1))
                  {             // We have more than one key, get key version
                     uint8_t version = 0;
                     e = df_get_key_version (&df, 1, &version);
                     if (!e)
                     {
                        fob.ver = version;
                        fob.verset = 1;
                     }
                     if (!e && version)
                     {
                        uint8_t aesid;
                        for (aesid = 0; aesid < sizeof (aes) / sizeof (*aes) && key_ver (aesid) != version; aesid++);
                        if (aesid == sizeof (aes) / sizeof (*aes))
                           e = "Unknown key version";
                        else
                           fob.aesid = aesid;
                     }
                  }
                  // Authenticate
                  if (!e)
                  {
                     uint8_t temp[16];
                     e = df_authenticate (&df, 1, key_aes (fob.aesid, temp));
                     if (e)
                     {          // Log key version as auth failed
                        uint8_t version = 0;
                        if (!df_get_key_version (&df, 1, &version))
                        {
                           fob.ver = version;
                           fob.verset = 1;
                        }
                     } else
                     {          // Authenticated so version is as expected
                        fob.ver = key_ver (fob.aesid);
                        fob.verset = 1;
                        fob.secure = 1;
                        if (key_type (fob.aesid) == 1)
                        {       // Only get UID if key type 1 as someone could have fed us one UID then linked to a real fob fooling us, but if encrypted key based on UID, it must be right so we can assume so and save time
                           uint8_t uid[7];      // Real ID
                           if (!e)
                              e = df_get_uid (&df, uid);
                           if (e)
                              fob.secure = 0;   // Err, WTF?
                           else
                              snprintf (fob.id, sizeof (fob.id), "%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3],
                                        uid[4], uid[5], uid[6]);
                        }
                     }
                  }
               }
            }
            // Door check
            if (e)
               fob.fail = e;
            else
               door_fob (&fob);
            void log (void)
            {                   // Log and count
               if (fob.log)
               {                // Log
                  uint8_t buf[13];
                  buf[0] = revk_binid >> 40;
                  buf[1] = revk_binid >> 32;
                  buf[2] = revk_binid >> 24;
                  buf[3] = revk_binid >> 16;
                  buf[4] = revk_binid >> 8;
                  buf[5] = revk_binid;
                  bcdutctime (0, buf + 6);
                  if (buf[6] == 0x19)
                     ESP_LOGI (TAG, "Clock not set when logging");
                  else if ((e = df_write_data (&df, 1, 'C', DF_MODE_CMAC, 0, 13, buf)))
                     return;
               }
               if (fob.count)
               {                // Count
                  if ((e = df_credit (&df, 2, DF_MODE_CMAC, 1)))
                     return;
               }
               if (fob.log || fob.count || fob.updated)
               {                // Commit
                  if ((e = df_commit (&df)))
                  {
                     fob.updated = 0;
                     return;
                  }
               }
               // Key update
               if (fob.aesid && key_type (0))
               {
                  uint8_t temp1[16],
                    temp2[16];
                  e = df_change_key (&df, 1, key_ver (0), key_aes (fob.aesid, temp1), key_aes (0, temp2));
                  if (!e)
                  {
                     fob.aesid = 0;
                     fob.ver = key_ver (0);
                     fob.keyupdated = 1;
                  }
               }
               if (!e)
               {
                  if (fob.log)
                     fob.logged = 1;
                  if (fob.count)
                     fob.counted = 1;
               }
            }
            if (e && strstr (e, "TIMEOUT"))
            {
               blink (0, 1, 0); // Read ID OK
               ESP_LOGI (TAG, "Retry %s %s", fob.id, e);
               nextpoll = 0;    // Try again immediately
               memset (&fob, 0, sizeof (fob));
            } else
            {                   // Processing door
               if (fob.fail)
                  ESP_LOGI (TAG, "Fail %s: %s", fob.id, fob.fail);
               else if (fob.deny)
                  ESP_LOGI (TAG, "Deny %s: %s", fob.id, fob.deny);
               else
                  ESP_LOGI (TAG, "Read %s", fob.id);
               if (!e && df_isauth (&df) && fob.commit)
                  log ();       // Log before reporting or opening door
               if (fob.enterok)
                  blink (0, 1, 1);
               else if (fob.deny)
                  blink (1, 0, 1);
               else
                  blink (0, 1, 0);
               if (!e)
                  door_act (&fob);
               uint8_t ver[28];
               fobevent (ats, fob.secureset && !fob.secure && !df_get_version (&df, ver) ? ver : NULL); // Report - as may need updating
               if (!e && df_isauth (&df) && !fob.commit)
               {
                  log ();       // Can log after reporting / opening
                  if (e && !strstr (e, "TIMEOUT"))
                     fob.fail = e;      // Will log when gone
               }
               found = now;
            }
            xSemaphoreGive (nfc_mutex);
         }
      }
   }
}

static void
report_state (void)
{
   jo_t j = jo_object_alloc ();
   if (aid[0] || aid[1] || aid[2])
      jo_stringf (j, "aid", "%02X%02X%02X", aid[0], aid[1], aid[2]);
   jo_array (j, "ver");
   for (int i = 0; i < sizeof (aes) / sizeof (*aes) && key_type (i); i++)
      jo_stringf (j, NULL, "%02X", key_ver (i));
   revk_state_clients ("keys", &j, 1);
}

const char *
nfc_command (const char *tag, jo_t j)
{
   if (!strcmp (tag, "connect"))
      report_state ();
   if (!pn532)
      return NULL;              // Not running
   if (!strcmp (tag, "shutdown"))
   {
      if (nfctx)
         gpio_reset_pin (port_mask (nfctx));    // So not driving via data lines
      if (nfcrx)
         gpio_reset_pin (port_mask (nfcrx));    // So not driving via data lines
      if (nfcpower)
         gpio_set_level (port_mask (nfcpower), (nfcpower & PORT_INV) ? 1 : 0);  // Off
   }
   if (nfcmask && !strcmp (tag, "led"))
   {
      char temp[sizeof (ledpattern)];
      int l = jo_strncpy (j, temp, sizeof (temp));
      if (l < 0)
         return "Expecting JSON string";
      if (l > sizeof (temp))
         return "Too long";
      return nfc_led (l, temp);
   }
   if (!strcmp (tag, "nfcremote"))
   {                            // Direct NFC data
      fob.remote = 1;           // Disable normal working
      ESP_LOGI (TAG, "NFC access remote");
      return "";
   }
   if (!strcmp (tag, "nfc"))
   {
      if (jo_here (j) != JO_STRING)
         return "Expecting JSON string";
      if (!fob.remote)
         return "Send nfcremote first";
      uint8_t buf[256];
      int len = jo_strncpy16 (j, (char *) buf, sizeof (buf));
      if (len < 0 || len > sizeof (buf))
         return "Too big";
      const char *err = NULL;
      xSemaphoreTake (nfc_mutex, portMAX_DELAY);
      len = pn532_dx (pn532, len, buf, sizeof (buf), &err);
      xSemaphoreGive (nfc_mutex);
      jo_t i = jo_create_alloc ();
      if (len < 0)
         jo_string (i, NULL, err);
      else
         jo_base16 (i, NULL, buf, len);
      revk_info_clients (len < 0 ? "nfcerror" : "nfc", &i, 1);
      return "";
   }
   if (!strcmp (tag, "nfcdone"))
   {
      ESP_LOGI (TAG, "NFC access remote ended");
      fob.remote = 0;
      fob.recheck = 1;
      nfc_led (0, NULL);
      return "";
   }
   return NULL;
}

void
nfc_boot (void)
{
   revk_register ("nfc", 0, sizeof (nfctx), &nfctx, BITFIELDS, SETTING_SET | SETTING_BITFIELD | SETTING_SECRET);        // parent setting
#define str(x) #x
#define i8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
#define gpio(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_BITFIELD);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8f(n,d) revk_register(#n,0,sizeof(n),&n,str(d),SETTING_FIX);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINDATA|SETTING_HEX);
#define bap(n,l,a) revk_register(#n,a,sizeof(n[0]),n,NULL,SETTING_BINDATA|SETTING_HEX|SETTING_SECRET|SETTING_LIVE);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define t(n,d) revk_register(#n,0,0,&n,d,0);
   settings
#undef t
#undef io
#undef gpio
#undef i8
#undef u8
#undef u8f
#undef u16
#undef b
#undef bap
#undef u1
#undef str
      // Set up ports */
      nfcmask = 0;              /* output mask for NFC */
   if (nfcred)
      nfcmask |= (1 << gpio_mask (nfcred));
   if (nfcamber)
      nfcmask |= (1 << gpio_mask (nfcamber));
   if (nfcgreen)
      nfcmask |= (1 << gpio_mask (nfcgreen));
   if (nfccard)
      nfcmask |= (1 << gpio_mask (nfccard));
   if (nfcred & GPIO_INV)
      nfcinvert |= (1 << gpio_mask (nfcred));
   if (nfcamber & GPIO_INV)
      nfcinvert |= (1 << gpio_mask (nfcamber));
   if (nfcgreen & GPIO_INV)
      nfcinvert |= (1 << gpio_mask (nfcgreen));
   if (nfccard & GPIO_INV)
      nfcinvert |= (1 << gpio_mask (nfccard));
   if (nfcpower && !port_check (port_mask (nfcpower), TAG, 0))
   {
      gpio_reset_pin (port_mask (nfcpower));
      gpio_set_level (port_mask (nfcpower), (nfcpower & PORT_INV) ? 1 : 0);     // Off
      gpio_set_direction (port_mask (nfcpower), GPIO_MODE_OUTPUT);
   }
   if (nfctx && nfcrx)
   {
      nfc_mutex = xSemaphoreCreateBinary ();
      xSemaphoreGive (nfc_mutex);
      const char *e = port_check (port_mask (nfctx), TAG, 0);
      if (!e)
         e = port_check (port_mask (nfcrx), TAG, 1);
      if (e)
         logical_gpio |= logical_NFCFault;
      else
      {
         pn532 = pn532_init (nfcuart, nfcbaud, port_mask (nfctx), port_mask (nfcrx), nfcmask);
         if (!pn532)
            logical_gpio |= logical_NFCFault;
         df_init (&df, pn532, pn532_dx);        // Start anyway, er re-try init
      }
   } else if (nfcrx || nfctx)
      logical_gpio |= logical_NFCFault;
}

void
nfc_start (void)
{
   if (nfctx && nfcrx)
   {
      revk_task (TAG, task, pn532, 3);
      nfc_led (0, NULL);
   }
}
