// NFC reader interface - working with door control
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "nfc";
#include "SS.h"
const char *nfc_fault = NULL;
const char *nfc_tamper = NULL;

#include "door.h"
#include "pn532.h"
#include "desfireaes.h"
#include <driver/gpio.h>

#define port_mask(p) ((p)&0x3F)
#define	BITFIELDS "-"
#define PORT_INV 0x40
#define GPIO_INV 0x80           // No SETTING bit

inline int16_t gpio_mask(uint8_t p)
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
  u1(nfccommit) \
  gpio(nfcred) \
  gpio(nfcamber) \
  gpio(nfcgreen) \
  gpio(nfccard) \
  gpio(nfctamper) \
  gpio(nfcbell) \
  t(mqttbell) \
  u1(itamper) \
  u16(nfcpoll,50) \
  u16(nfchold,3000) \
  u16(nfcholdpoll,500) \
  u16(nfcledpoll,100) \
  u16(nfciopoll,200) \
  b(nfcbus,1) \
  ba(aes,17,3) \
  b(aid,3) \
  io(nfctx) \
  io(nfcrx) \
  io(nfcpower) \
  u8(nfcuart,1) \
  t(led)	\

#define i8(n,d) int8_t n;
#define io(n) uint8_t n;
#define gpio(n) uint8_t n;
#define u8(n,d) uint8_t n;
#define u16(n,d) uint16_t n;
#define b(n,l) uint8_t n[l];
#define ba(n,l,a) uint8_t n[a][l];
#define u1(n) uint8_t n;
#define t(n) const char*n=NULL;
settings
#undef t
#undef i8
#undef io
#undef gpio
#undef u8
#undef u16
#undef b
#undef ba
#undef u1
    pn532_t * pn532 = NULL;
uint8_t nfcmask = 0,
    nfcinvert = 0;
df_t df;
SemaphoreHandle_t nfc_mutex = NULL;     // PN532 has low level message mutex, but this is needed for DESFire level.

static char held = 0;           // Card was held, also flags pre-loaded for remote card logic
static uint8_t ledpattern[20] = "";

const char *nfc_led(int len, const void *value)
{
   if (!len)
      len = strlen(value = led);        // Default
   revk_info("led", "%.*s", len, value);
   if (len > sizeof(ledpattern))
      len = sizeof(ledpattern);
   if (len < sizeof(ledpattern))
      ledpattern[len] = 0;
   if (len)
      memcpy(ledpattern, value, len);
   return "";
}

static void task(void *pvParameters)
{
   esp_task_wdt_add(NULL);
   pvParameters = pvParameters;
   int64_t nextpoll = 0;
   int64_t nextled = 0;
   int64_t nexttamper = 0;
   char id[22];
   const char *noaccess = NULL;
   int64_t found = 0;
   uint8_t ledlast = 0xFF;
   uint8_t ledpos = 0;
   while (1)
   {
      esp_task_wdt_reset();
      usleep(1000);
      int64_t now = esp_timer_get_time();
      // Regular tasks
      // Check tamper
      if (nexttamper < now)
      {                         // Check tamper
         nexttamper += (uint64_t) nfciopoll *1000LL;
         int p3 = -1;
         if (pn532)
         {                      // Connected, get port
            p3 = pn532_read_GPIO(pn532);
            if (p3 < 0)
               p3 = pn532_read_GPIO(pn532);     // Try again
            if (p3 < 0)
            {
               pn532 = pn532_end(pn532);
               status(nfc_fault = "Failed");
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
               ESP_LOGE(TAG, "NFC re-init");
               pn532 = pn532_init(nfcuart, port_mask(nfctx), port_mask(nfcrx), nfcmask);
               if (pn532)
               {                // All good!
                  df_init(&df, pn532, pn532_dx);
                  ledlast = 0xFF;
                  status(nfc_fault = NULL);
               } else
               {                // Failed
                  on = 0;
                  if (nfcpower)
                     gpio_set_level(port_mask(nfcpower), (nfcpower & PORT_INV) ? 1 : 0);        // Off
                  wait = 2000 / nfciopoll;      // off wait
               }
            } else
            {                   // Off, so turn on
               on = 1;
               if (nfcpower)
                  gpio_set_level(port_mask(nfcpower), (nfcpower & PORT_INV) ? 0 : 1);   // On
               wait = 200 / nfciopoll;  // on wait
            }
         }
         if (!pn532)
            continue;           // No point doing any more
         if (p3 >= 0)
         {                      // Inputs
            p3 ^= nfcinvert;
            if (nfctamper)
            {                   // Check tamper
               if (p3 & (1 << gpio_mask(nfctamper)))
                  status(nfc_tamper = "Tamper");
               else
                  status(nfc_tamper = NULL);
            }
            if (nfcbell)
            {
               static uint8_t bell = 0;
               if (p3 & (1 << gpio_mask(nfcbell)))
               {
                  if (!bell)
                  {
                     bell = 1;
                     if (mqttbell)
                     {
                        char *topic = strdup(mqttbell);
                        char *data = strchr(topic, ' ');
                        if (data)
                           *data++ = 0;
                        revk_raw(NULL, topic, data ? strlen(data) : 0, data, 0);
                        free(topic);
                     }
                     revk_info("bell", "pushed");
                  }
               } else
                  bell = 0;
            }
         }
      }
      // LED
      void blink(uint8_t p) {   // Blink an LED
         if (!p)
            return;             //Port not set
         if (ledlast & (1 << gpio_mask(p)))
            return;             //Already set
         pn532_write_GPIO(pn532, (ledlast ^= (1 << gpio_mask(p))) ^ nfcinvert); //Blink on
         nextled = now + (uint64_t) nfcledpoll *1000LL;
      }
      if (nextled < now)
      {                         // Check LED
         nextled = now + (uint64_t) nfcledpoll *1000LL;
         static int count = 0;
         if (count)
            count--;            // We are repeating existing pattern for a while
         if (!count)
         {                      // Next colour
            ledpos++;
            if (ledpos >= sizeof(ledpattern) || !ledpattern[ledpos] || !*ledpattern)
               ledpos = 0;      // Wrap
            uint8_t newled = 0;
            while (ledpos < sizeof(ledpattern) && ledpattern[ledpos] && isdigit(ledpattern[ledpos]))
               count = count * 10 + ledpattern[ledpos++] - '0';
            while (ledpos < sizeof(ledpattern) && ledpattern[ledpos])
            {                   // Check combined colours
               if (nfcred && ledpattern[ledpos] == 'R')
                  newled |= (1 << gpio_mask(nfcred));
               if (nfcamber && ledpattern[ledpos] == 'A')
                  newled |= (1 << gpio_mask(nfcamber));
               if (nfcgreen && ledpattern[ledpos] == 'G')
                  newled |= (1 << gpio_mask(nfcgreen));
               if (nfccard && found && *id != '*')
                  newled |= (1 << gpio_mask(nfccard));
               if (ledpos + 1 >= sizeof(ledpattern) || ledpattern[ledpos + 1] != '+')
                  break;        // Combined LED pattern with +
               ledpos += 2;
            }
            if (newled != ledlast)
               pn532_write_GPIO(pn532, (ledlast = newled) ^ nfcinvert);
         }
      }
      // Card
      if (nextpoll < now)
      {                         // Check for card
         nextpoll = now + (uint64_t) nfcpoll *1000LL;
         if (found && !pn532_Present(pn532))
         {                      // Card gone
            ESP_LOGI(TAG, "gone %s", id);
            if (held && nfchold)
               revk_event("gone", "%s", id);
            found = 0;
            held = 0;
         }
         if (found)
         {
            nextpoll = now + (int64_t) nfcholdpoll *1000LL;     // Periodic check for card held
            if (!held && nfchold && found < now)
            {                   // Card has been held for a while, report
               revk_event("held", "%s", id);
               blink(nfcamber);
               held = 1;
            }
            continue;           // Waiting for card to go
         }
         // Check for new card
         df.keylen = 0;         // New card
         int cards = pn532_Cards(pn532);
         if (cards > 0)
         {
            xSemaphoreTake(nfc_mutex, portMAX_DELAY);
            nextpoll = now + (int64_t) nfcholdpoll *1000LL;     // Periodic check for card held
            noaccess = "";      // Assume no auto access (not an error)
            uint8_t aesid = 0;
            const char *e = NULL;
            uint8_t *ats = pn532_ats(pn532);
            uint32_t crc = 0;
            if (cards > 1)
               strcpy(id, "*Multiple");
            else
            {
               pn532_nfcid(pn532, id);
               if (!held && aes[0][0] && (aid[0] || aid[1] || aid[2]) && *ats && ats[1] == 0x75)
               {                // DESFire
                  // Select application
                  if (!e)
                     e = df_select_application(&df, aid);
                  if (!e && aes[1][0])
                  {             // Get key to work out which AES
                     uint8_t version = 0;
                     e = df_get_key_version(&df, 1, &version);
                     if (!e && version)
                     {
                        for (aesid = 0; aesid < sizeof(aes) / sizeof(*aes) && aes[aesid][0] != version; aesid++);
                        if (aesid == sizeof(aes) / sizeof(*aes))
                           e = "Unknown key version";
                     }
                  }
                  // Authenticate
                  if (!e)
                     e = df_authenticate(&df, 1, aes[aesid] + 1);
                  uint8_t uid[7];       // Real ID
                  if (!e)
                     e = df_get_uid(&df, uid);
                  if (!e)
                     snprintf(id, sizeof(id), "%02X%02X%02X%02X%02X%02X%02X+", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]); // Set UID with + to indicate secure, regardless of access allowed, etc.
               }
            }
            // Door check
            if (e)
               noaccess = e;    // NFC or DESFire error
            else
               noaccess = door_fob(id, &crc);   // Access from door control
            void log(void) {    // Log and count
               // Log
               uint8_t buf[10];
               buf[0] = revk_binid >> 16;
               buf[1] = revk_binid >> 8;
               buf[2] = revk_binid;
               bcdtime(0, buf + 3);
               if (buf[3] == 0x19)
                  revk_error(TAG, "Clock not set");
               else if ((e = df_write_data(&df, 1, 'C', DF_MODE_CMAC, 0, 10, buf)))
                  return;
               // Count
               if ((e = df_credit(&df, 2, DF_MODE_CMAC, 1)))
                  return;
               // Commit
               if ((e = df_commit(&df)))
                  return;
               // Key update
               if (aesid)
               {
                  revk_info("aes", "Key update %02X->%02X", *aes[aesid], *aes[0]);
                  e = df_change_key(&df, 1, aes[0][0], aes[aesid] + 1, aes[0] + 1);
               }
            }
            if (e && strstr(e, "TIMEOUT"))
            {
               blink(nfcamber); // Read ID OK
               ESP_LOGI(TAG, "Retry %s %s", id, e);
               nextpoll = 0;    // Try again immediately
            } else
            {                   // Processing door
               if (e)
                  ESP_LOGI(TAG, "Error %s %s", id, e);
               else
                  ESP_LOGI(TAG, "ID %s", id);
               if (!e && df.keylen && nfccommit)
                  log();        // Log before reporting or opening door
               if (!noaccess)
               {                // Access is allowed!
                  blink(nfcred);        // Not allowed
                  door_unlock(NULL, "fob");     // Door system was happy with fob, let 'em in
               } else if (door >= 4)
                  blink(nfcgreen);      // Allowed
               else if (nfccard && *id != '*')
                  blink(nfccard);
               else
                  blink(nfcamber);      // Read OK but we don't know if allowed or not as needs back end to advise
               nextled = now + 200000LL;
               // Report
               if (door >= 4 || !noaccess)
               {                // Autonomous door control
                  if (noaccess && *noaccess == '*')
                     revk_event("noaccess", "%s %08X %s", id, crc, noaccess + 1);
                  else if (noaccess && *noaccess)
                     revk_event("nfcfail", "%s %08X %s", id, crc, noaccess);
                  else
                     revk_event(noaccess ? "id" : "access", "%s %08lX%s", id, crc, *ats && ats[1] == 0x75 ? " DESFire" : *ats && ats[1] == 0x78 ? " ISO" : "");
               } else
                  revk_event("id", "%s%s", id, *ats && ats[1] == 0x75 ? " DESFire" : *ats && ats[1] == 0x78 ? " ISO" : "");
               if (!e && df.keylen && !nfccommit)
               {
                  log();        // Can log after reporting / opening
                  if (e && !strstr(e, "TIMEOUT"))
                     revk_error(TAG, "%s", e);  // Log new error anyway, unless simple timeout
               }
               found = now + (uint64_t) nfchold *1000LL;
            }
            xSemaphoreGive(nfc_mutex);
         }
      }
   }
}

static void report_state(void)
{
   if (!aid[0] && !aid[1] && !aid[2])
      return;
   if (revk_offline())
      return;
   char vers[sizeof(aes) / sizeof(*aes) * 2 + 1];
   int i;
   for (i = 0; i < sizeof(aes) / sizeof(*aes); i++)
      sprintf(vers + i * 2, "%02X", aes[i][0]);
   while (i && !aes[i - 1][0])
      i--;
   vers[i * 2] = 0;
   revk_state("aes", "%02X%02X%02X %s", aid[0], aid[1], aid[2], vers);
}

const char *nfc_command(const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strcmp(tag, "connect"))
      report_state();
   if (!pn532)
      return NULL;              // Not running
   if (!strcmp(tag, "restart"))
   {
      if (nfcpower)
         gpio_set_level(port_mask(nfcpower), (nfcpower & PORT_INV) ? 1 : 0);    // Off
   }
   if (nfcmask && !strcmp(tag, "led"))
      return nfc_led(len, value);
   if (!strcmp(tag, TAG))
   {
      if (!len)
         held = 1;
      else
      {
         uint8_t buf[256];
         if (len > sizeof(buf))
            return "Too big";
         memcpy(buf, value, len);
         const char *err = NULL;
         xSemaphoreTake(nfc_mutex, portMAX_DELAY);
         int l = pn532_dx(pn532, len, buf, sizeof(buf), &err);
         xSemaphoreGive(nfc_mutex);
         if (l < 0)
            return err ? : "?";
         revk_raw(prefixinfo, TAG, l, buf, 0);
      }
      return "";
   }
   return NULL;
}

void nfc_init(void)
{
#define i8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
#define gpio(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_BITFIELD);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINARY|SETTING_HEX);
#define ba(n,l,a) revk_register(#n,a,sizeof(n[0]),n,NULL,SETTING_BINARY|SETTING_HEX);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define t(n) revk_register(#n,0,0,&n,NULL,0);
   settings
#undef t
#undef io
#undef gpio
#undef i8
#undef u8
#undef u16
#undef b
#undef ba
#undef u1
       // Set up ports */
       nfcmask = 0;             /* output mask for NFC */
   if (nfcred)
      nfcmask |= (1 << gpio_mask(nfcred));
   if (nfcamber)
      nfcmask |= (1 << gpio_mask(nfcamber));
   if (nfcgreen)
      nfcmask |= (1 << gpio_mask(nfcgreen));
   if (nfccard)
      nfcmask |= (1 << gpio_mask(nfccard));
   if (nfcred & GPIO_INV)
      nfcinvert |= (1 << gpio_mask(nfcred));
   if (nfcamber & GPIO_INV)
      nfcinvert |= (1 << gpio_mask(nfcamber));
   if (nfcgreen & GPIO_INV)
      nfcinvert |= (1 << gpio_mask(nfcgreen));
   if (nfccard & GPIO_INV)
      nfcinvert |= (1 << gpio_mask(nfccard));
   if (nfctamper & GPIO_INV)
      nfcinvert |= (1 << gpio_mask(nfctamper));
   if (nfcbell & GPIO_INV)
      nfcinvert |= (1 << gpio_mask(nfcbell));
   if (nfcpower)
   {
      gpio_set_level(port_mask(nfcpower), (nfcpower & PORT_INV) ? 0 : 1);
      gpio_set_direction(port_mask(nfcpower), GPIO_MODE_OUTPUT);
      usleep(100000);
   }
   nfc_led(0, NULL);
   if (nfctx && nfcrx)
   {
      const char *e = port_check(port_mask(nfctx), TAG, 0);
      if (!e)
         e = port_check(port_mask(nfcrx), TAG, 1);
      if (e)
         status(nfc_fault = e);
      else
      {
         nfc_mutex = xSemaphoreCreateBinary();
         xSemaphoreGive(nfc_mutex);
         pn532 = pn532_init(nfcuart, port_mask(nfctx), port_mask(nfcrx), nfcmask);
         if (!pn532)
            status(nfc_fault = "Failed to start PN532");
         df_init(&df, pn532, pn532_dx); // Start anyway, er re-try init
         revk_task(TAG, task, pn532);
      }
   } else if (nfcrx || nfctx)
      status(nfc_fault = "Set nfctx, and nfcrx");
   report_state();
}
