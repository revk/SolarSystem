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
   while (1)
   {
      sleep (1);
#if 0
      // TODO check for cards
      uint8_t buf[100];
      int l = pn532_InListPassiveTarget (pn532, sizeof (buf), buf);
      revk_info ("nfc", "ILPT %d", l);
#endif
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
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINARY);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
   settings
#undef u8
#undef b
#undef u1
#undef p
      if (nfctx && nfcrx && nfcuart && port_ok (port_mask (nfctx), "nfctx") && port_ok (port_mask (nfcrx), "nfcrx"))
   {
      pn532 = pn532_init (port_mask (nfcuart), port_mask (nfctx), port_mask (nfcrx), 0);        // TODO P3
      if (!pn532)
         revk_error ("nfc", "Failed to start PN532");
      else
         xTaskCreatePinnedToCore (nfc_task, "nfc", 16 * 1024, NULL, 1, &nfc_task_id, tskNO_AFFINITY);   // TODO stack, priority, affinity check?
   } else if (nfcrx || nfctx || nfcuart)
      revk_error ("nfc", "Set nfcuart, nfctx, and nfcrx");
}
