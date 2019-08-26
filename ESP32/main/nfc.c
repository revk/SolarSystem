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
  u8(nfcred,1); \
  u8(nfcgreen,0); \
  u8(nfctamper,3); \
  u16(nfcpoll,50); \
  b(nfcbus,1); \
  ba(aes,17,3); \
  b(aid,3); \
  p(nfctx); \
  p(nfcrx); \
  u8(nfcuart,2); \

#define u8(n,d) uint8_t n;
#define u16(n,d) uint16_t n;
#define b(n,l) uint8_t n[l];
#define ba(n,l,a) uint8_t n[a][l];
#define u1(n) uint8_t n;
#define p(n) uint8_t n;
settings
#undef u8
#undef u16
#undef b
#undef ba
#undef u1
#undef p
   pn532_t * pn532 = NULL;
df_t df;

static void
task (void *pvParameters)
{
   pvParameters = pvParameters;
   int64_t nextpoll = 0;
   int64_t nextled = 0;
   int64_t nexttamper = 0;
   char id[22];
   const char *noaccess = NULL;
   char found = 0;
   while (1)
   {
      usleep (1000);            // TODO work out how long to sleep for
      int64_t now = esp_timer_get_time ();
      int ready = pn532_ready (pn532);
      if (ready > 0)
      {                         // Check ID response
         noaccess = "";         // Assume no auto access (not an error)
         int cards = pn532_Cards (pn532);
         if (cards)
         {
            uint8_t aesid = 0;
            const char *e = NULL;
            uint8_t *ats = pn532_ats (pn532);
            pn532_nfcid (pn532, id);
            if (aes[0][0] && (aid[0] || aid[1] || aid[2]) && *ats && ats[1] == 0x75)
            {                   // DESFire
               uint8_t uid[7];  // Real ID
               // Select application
               if (!e)
                  e = df_select_application (&df, aid);
               if (!e && aes[1][0])
               {                // Get key to work out which AES
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
               if (!e)
                  e = df_get_uid (&df, uid);
               if (!e)
                  snprintf (id, sizeof (id), "%02X%02X%02X%02X%02X%02X%02X+", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5],
                            uid[6]);
            }
            // Door check
            noaccess = door_fob (id);
            if (e)
            {                   // Error
               if (!strcmp (e, "Dx fail"))
                  e = pn532_err_to_name (pn532_lasterr (pn532));
               noaccess = e;
            }
            void log (void)
            {                   // Log and count
               if (e || !df.keylen)
                  return;
               // TODO
               if (aesid)
                  e = df_change_key (&df, 1, aes[0][0], aes[aesid] + 1, aes[0] + 1);
            }
            if (nfccommit)
               log ();
            if (!noaccess)
            {                   // Open door?
               // TODO commit?
               // TODO LED
               door_unlock (NULL);      // Door system was happy with fob, let 'em in
            }
            // Report
            if (noaccess && *noaccess)
               revk_info ("noaccess", "%s %s", id, noaccess);
            else
               revk_info (noaccess ? "id" : "access", "%s%s", id, *ats && ats[1] == 0x75 ? " DESFire" : *ats
                          && ats[1] == 0x78 ? " ISO" : "");
            // TODO, hold on MIFARE classic
            if (nfccommit)
               log ();
            found = 1;
         }
         ready = -1;
      }
      if (ready >= 0)
         continue;              // We cannot talk to card for LED/tamper as waiting for reply
      if (nextpoll < now)
      {                         // Check for card
         nextpoll = now + (uint64_t) nfcpoll *1000;
         if (found && !pn532_Present (pn532))
         {
            revk_info ("gone", "%s", id);
            found = 0;
         }
         if (!found)
            pn532_ILPT_Send (pn532);
      }
      if (nextled < now)
      {                         // Check LED
         nextled = now + 100000;
      }
      if (nexttamper < now)
      {                         // Check tamper
         nexttamper = now + 1000000;
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
   return NULL;
}

void
nfc_init (void)
{
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINARY|SETTING_HEX);
#define ba(n,l,a) revk_register(#n,a,sizeof(n[0]),n,NULL,SETTING_BINARY|SETTING_HEX);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
   settings
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
