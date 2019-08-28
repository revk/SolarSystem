// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"
const char *keypad_fault = NULL;
const char *keypad_tamper = NULL;

#include "galaxybus.h"

#define port_mask(p) ((p)&127)

#define settings  \
  p(keypadtx) \
  p(keypadrx) \
  p(keypadde) \
  p(keypadre) \
  u8(keypaduart,1) \
  u8h(keypadaddress,10)	\
  u8h(keypadmaster,11)	\
  b(keypadbdegug)	\

#define u8(n,d) uint8_t n;
#define u8h(n,d) uint8_t n;
#define b(n) uint8_t n;
#define p(n) uint8_t n;
settings
#undef u8
#undef u8h
#undef b
#undef p
const char *
keypad_command (const char *tag, unsigned int len, const unsigned char *value)
{
   // TODO
   return NULL;
}

static void
task (void *pvParameters)
{
   galaxybus_t *g = pvParameters;
   while (1)
   {
      sleep (1);
      uint8_t buf[20];
      int len=galaxybus_poll (g,keypadaddress, sizeof (buf), buf);
      if(len>0)ESP_LOG_BUFFER_HEX_LEVEL ("Keypad", buf, len, ESP_LOG_INFO);
      
      // TODO
   }
}

void
keypad_init (void)
{
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8h(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_HEX);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
   settings
#undef u8
#undef u8h
#undef b
#undef p
      if (keypadtx && keypadrx && keypadde)
   {
      const char *err = port_check (port_mask (keypadtx), TAG,0);
      if (!err)
         port_check (port_mask (keypadrx), TAG,1);
      if (!err)
         port_check (port_mask (keypadde), TAG,0);
      if (!err && keypadre)
         port_check (port_mask (keypadre), TAG,0);
      if (err)
         status (keypad_fault = err);
      else
      {
         galaxybus_t *g =
            galaxybus_init (keypaduart, port_mask (keypadtx), port_mask (keypadrx), port_mask (keypadde),
                          keypadre ? port_mask (keypadre) : -1, keypadmaster);
         if (!g)
            status (keypad_fault = "Init fail");
         else
         {
            static TaskHandle_t task_id = NULL;
            xTaskCreatePinnedToCore (task, TAG, 16 * 1024, g, 1, &task_id, 1);  // TODO stack, priority, affinity check?
         }
      }
   } else if (keypadtx || keypadrx || keypadde)
      status (keypad_fault = "Set keypadtx, keypadrx and keypadde");
}
