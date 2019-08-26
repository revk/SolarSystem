// NFC reader interface - working with door control
static const char TAG[] = "nfc";
#include "SS.h"
const char *nfc_fault = NULL;
const char *nfc_tamper = NULL;

#include "door.h"
#include "pn532.h"
#include "desfireaes.h"

#define port_mask(p) ((p)&127)

// Other settings
#define settings  \
  u1(nfccommit); \
  i8(nfcred,1); \
  i8(nfcgreen,0); \
  i8(nfctamper,3); \
  u16(nfcpoll,50); \
  u16(nfchold,3000); \
  b(nfcbus,1); \
  ba(aes,17,3); \
  b(aid,3); \
  p(nfctx); \
  p(nfcrx); \
  u8(nfcuart,2); \

#define i8(n,d) int8_t n;
#define u8(n,d) uint8_t n;
#define u16(n,d) uint16_t n;
#define b(n,l) uint8_t n[l];
#define ba(n,l,a) uint8_t n[a][l];
#define u1(n) uint8_t n;
#define p(n) uint8_t n;
settings
#undef i8
#undef u8
#undef u16
#undef b
#undef ba
#undef u1
#undef p
   pn532_t * pn532 = NULL;
df_t df;

static char held = 0;           // Card was held, also flags pre-loaded for remote card logic
static char ledpattern[10] = "";

static void
task (void *pvParameters)
{
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
      usleep (1000);
      int64_t now = esp_timer_get_time ();
      int ready = pn532_ready (pn532);
      if (ready > 0)
      {                         // Check ID response
         df.keylen = 0;         // New card
         int cards = pn532_Cards (pn532);
         ready = -1;            // We cleared command
         if (cards > 0)
         {
            noaccess = "";      // Assume no auto access (not an error)
            uint8_t aesid = 0;
            const char *e = NULL;
            uint8_t *ats = pn532_ats (pn532);
            uint32_t crc = 0;
            if (cards > 1)
               strcpy (id, "Multiple");
            else
            {
               pn532_nfcid (pn532, id);
               if (!held && aes[0][0] && (aid[0] || aid[1] || aid[2]) && *ats && ats[1] == 0x75)
               {                // DESFire
                  // Select application
                  if (!e)
                     e = df_select_application (&df, aid);
                  if (!e && aes[1][0])
                  {             // Get key to work out which AES
                     uint8_t version = 0;
                     e = df_get_key_version (&df, 1, &version);
                     if (!e && version)
                     {
                        for (aesid = 0; aesid < sizeof (aes) / sizeof (*aes) && aes[aesid][0] != version; aesid++);
                        if (aesid == sizeof (aes) / sizeof (*aes))
                           e = "Unknown key version";
                     }
                  }
                  // Authenticate
                  if (!e)
                     e = df_authenticate (&df, 1, aes[aesid] + 1);
                  uint8_t uid[7];       // Real ID
                  if (!e)
                     e = df_get_uid (&df, uid);
                  if (!e)
                     snprintf (id, sizeof (id), "%02X%02X%02X%02X%02X%02X%02X+", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);       // Set UID with + to indicate secure, regardless of access allowed, etc.
               }
            }
            // Door check
            if (e)
            {                   // NFC or DESFire error
               noaccess = e;
            } else
               noaccess = door_fob (id, &crc);  // Access from door control
            void log (void)
            {                   // Log and count
               // Log
               uint8_t buf[10];
               buf[0] = revk_binid >> 16;
               buf[1] = revk_binid >> 8;
               buf[2] = revk_binid;
               bcdtime (0, buf + 3);
               if (buf[3] == 0x19)
                  revk_error (TAG, "Clock not set");
               else if ((e = df_write_data (&df, 1, 'C', DF_MODE_CMAC, 0, 10, buf)))
                  return;
               // Count
               if ((e = df_credit (&df, 2, DF_MODE_CMAC, 1)))
                  return;
               // Commit
               if ((e = df_commit (&df)))
                  return;
               // Key update
               if (aesid)
                  e = df_change_key (&df, 1, aes[0][0], aes[aesid] + 1, aes[0] + 1);
            }
            if (e && !strcmp (e, "PN532_ERR_TIMEOUT"))
               nextpoll = 0;    // Try again
            else
            {                   // Processing door
               if (!e && df.keylen && nfccommit)
                  log ();       // Log before reporting or opening door
               if (!noaccess)
               {                // Access is allowed!
                  pn532_write_GPIO (pn532, ledlast = (nfcgreen >= 0 && !(ledlast & (1 << nfcgreen)) ? (1 << nfcgreen) : 0));    // Blink green
                  door_unlock (NULL);   // Door system was happy with fob, let 'em in
               } else if (door >= 4)
                  pn532_write_GPIO (pn532, ledlast = (nfcred >= 0 && !(ledlast & (1 << nfcred)) ? (1 << nfcred) : 0));  // Blink red
               nextled = now + 200000;
               // Report
               if (door >= 4 || !noaccess)
               {                // Autonomous door control
                  if (noaccess && *noaccess == '*')
                     revk_event ("noaccess", "%s %08X %s", id, crc, noaccess + 1);
                  else if (noaccess && *noaccess)
                     revk_event ("nfcfail", "%s %08X %s", id, crc, noaccess);
                  else
                     revk_event (noaccess ? "id" : "access", "%s %08lX%s", id, crc, *ats && ats[1] == 0x75 ? " DESFire" : *ats
                                 && ats[1] == 0x78 ? " ISO" : "");
               } else
                  revk_event ("id", "%s%s", id, *ats && ats[1] == 0x75 ? " DESFire" : *ats && ats[1] == 0x78 ? " ISO" : "");
               if (!e && df.keylen && !nfccommit)
               {
                  log ();       // Can log after reporting / opening
                  if (e && strcmp (e, "PN532_ERR_TIMEOUT"))
                     revk_error (TAG, "%s", e); // Log new error anyway, unless simple timeout
               }
               found = now + (uint64_t) nfchold *1000;
               nextpoll = now + 100000;
            }
         }
      }
      if (found && !held && found < now)
      {
         revk_event ("held", "%s", id);
         held = 1;
      }
      if (ready >= 0)
         continue;              // We cannot talk to card for LED/tamper as waiting for reply
      if (nextled < now)
      {                         // Check LED
         ledpos++;
         if (ledpos >= sizeof (ledpattern) || !ledpattern[ledpos] || !*ledpattern)
            ledpos = 0;
         uint8_t newled = 0;
         // We are assuming exactly two LEDs, one at a time (back to back) on P30 and P31
         if (nfcred >= 0 && ledpattern[ledpos] == 'R')
            newled = (1 << nfcred);
         if (nfcgreen >= 0 && ledpattern[ledpos] == 'G')
            newled = (1 << nfcgreen);
         if (newled != ledlast)
            pn532_write_GPIO (pn532, ledlast = newled);
         nextled = now + 100000;
      }
      if (nexttamper < now && nfctamper >= 0)
      {                         // Check tamper
         nexttamper = now + 1000000;
         int p3 = pn532_read_GPIO (pn532);
         if (p3 < 0)
         {                      // Failed
            // Try init again
            pn532_end (pn532);
            pn532 = pn532_init (nfcuart, port_mask (nfctx), port_mask (nfcrx), (1 << nfcred) | (1 << nfcgreen));
            if (!pn532)
               status (nfc_fault = "Failed");
            else
               status (nfc_fault = NULL);
         } else
         {                      // Check tamper
            if (p3 & (1 << nfctamper))
               status (nfc_tamper = "Tamper");
            else
               status (nfc_tamper = NULL);
         }
      }
      if (nextpoll < now)
      {                         // Check for card
         nextpoll = now + (uint64_t) nfcpoll *1000;
         if (found && !pn532_Present (pn532))
         {
            if (held && nfchold)
               revk_event ("gone", "%s", id);
            found = 0;
            held = 0;
            nextpoll = now + 100000;
         }
         if (!found)
         {
            pn532_ILPT_Send (pn532);
            continue;           // Cant do LED/tamper until we get reply
         }
      }
   }
}

const char *
nfc_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strcmp (tag, "connect") && (aid[0] || aid[1] || aid[2]) && *aes[0])
   {
      char vers[sizeof (aes) / sizeof (*aes) * 2 + 1];
      int i;
      for (i = 0; i < sizeof (aes) / sizeof (*aes); i++)
         sprintf (vers + i * 2, "%02X", aes[i][0]);
      while (i && !aes[i - 1][0])
         i--;
      vers[i * 2] = 0;
      revk_info ("aes", "%02X%02X%02X %s", aid[0], aid[1], aid[2], vers);
   }
   if (!strcmp (tag, "led"))
   {
      if (len > sizeof (ledpattern))
         len = sizeof (ledpattern);
      if (len < sizeof (ledpattern))
         ledpattern[len] = 0;
      if (len)
         memcpy (ledpattern, value, len);
      return "";
   }
   if (!strcmp (tag, TAG) && len)
   {
      if (pn532_ready (pn532) >= 0)
         return "Busy";
      uint8_t buf[256];
      memcpy (buf, value, len);
      const char *err = NULL;
      int l = pn532_dx (&df, len, buf, sizeof (buf), &err);
      if (l < 0)
         return err ? : "?";
      revk_raw (prefixinfo, TAG, l, buf, 0);
      return "";
   }
   return NULL;
}

void
nfc_init (void)
{
#define i8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINARY|SETTING_HEX);
#define ba(n,l,a) revk_register(#n,a,sizeof(n[0]),n,NULL,SETTING_BINARY|SETTING_HEX);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
   settings
#undef i8
#undef u8
#undef u16
#undef b
#undef ba
#undef u1
#undef p
      if (nfctx && nfcrx)
   {
      const char *e = port_check (port_mask (nfctx), "nfctx", 0);
      if (!e)
         e = port_check (port_mask (nfcrx), "nfcrx", 1);
      if (e)
         status (nfc_fault = e);
      else
      {
         pn532 = pn532_init (nfcuart, port_mask (nfctx), port_mask (nfcrx), (1 << nfcred) | (1 << nfcgreen));
         if (!pn532)
            status (nfc_fault = "Failed to start PN532");
         else
         {
            df_init (&df, pn532, pn532_dx);
            static TaskHandle_t task_id = NULL;
            xTaskCreatePinnedToCore (task, TAG, 16 * 1024, NULL, 1, &task_id, tskNO_AFFINITY);  // TODO stack, priority, affinity check?
         }
      }
   } else if (nfcrx || nfctx)
      status (nfc_fault = "Set nfctx, and nfcrx");
}
