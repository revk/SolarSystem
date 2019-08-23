// NFC reader interface - working with door control
static const char TAG[] = "NFC";

#include "SS.h"
#include "door.h"
#include "../components/ESP32RevK/pn532.h"

#define port_mask(p) ((p)&127)

// Other settings
#define settings  \
  u1(nfccommit); \
  u8(nfcred,1); \
  u8(nfcgreen,0); \
  u8(nfctamper,3); \
  u8(nfcpoll,50); \
  b(nfcbus,1); \
  b(aes,16); \
  b(aid,3); \
  p(nfctx); \
  p(nfcrx); \
  p(nfcuart); \

#define u8(n,d) uint8_t n;
#define b(n,l) uint8_t n[l];
#define u1(n) uint8_t n;
#define p(n) uint8_t n;
settings
#undef u8
#undef b
#undef u1
#undef p
static TaskHandle_t nfc_task_id = NULL;
static pn532_t *pn532 = NULL;

static void
nfc_task (void *pvParameters)
{                               // Main RevK task
   pvParameters = pvParameters;
   int64_t nextpoll = 0;
   int64_t nextled = 0;
   int64_t nexttamper = 0;
   while (1)
   {
      usleep (1000);            // TODO work out how long to sleep for
      int64_t now = esp_timer_get_time ();
      int ready = pn532_ready (pn532);
      if (ready > 0)
      {                         // Check ID response
         int cards = pn532_Cards (pn532);
         if (cards)
            revk_info ("nfc", "Cards %d nfcidlen=%d atslen=%s", cards, *pn532_nfcid (pn532), *pn532_ats (pn532));
         ready = -1;
      }
      if (ready >= 0)
         continue;              // We cannot talk to card for LED/tamper as waiting for reply
      if (nextpoll < now)
      {                         // Check for card
         nextpoll = now + nfcpoll * 1000;
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
   // TODO
   return NULL;
}

void
nfc_init (void)
{
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINARY|SETTING_HEX);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
   settings
#undef u8
#undef b
#undef u1
#undef p
      if (nfctx && nfcrx && nfcuart && port_ok (port_mask (nfctx), "nfctx") && port_ok (port_mask (nfcrx), "nfcrx"))
   {
      pn532 = pn532_init (port_mask (nfcuart), port_mask (nfctx), port_mask (nfcrx), (1 << nfcred) | (1 << nfcgreen));
      if (!pn532)
         revk_error ("nfc", "Failed to start PN532");
      else
         xTaskCreatePinnedToCore (nfc_task, "nfc", 16 * 1024, NULL, 1, &nfc_task_id, tskNO_AFFINITY);   // TODO stack, priority, affinity check?
   } else if (nfcrx || nfctx || nfcuart)
      revk_error ("nfc", "Set nfcuart, nfctx, and nfcrx");
}
