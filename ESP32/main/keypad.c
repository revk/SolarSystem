// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"
const char *keypad_fault = NULL;
const char *keypad_tamper = NULL;

#include "galaxybus.h"

#define settings  \
  p(keypadtx) \
  p(keypadrx) \
  p(keypadde) \
  p(keypadre) \
  u8(keypaduart,1) \
  u8h(keypadaddress,10)	\
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
   pvParameters = pvParameters;
   while (1)
   {
      sleep (1);
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
      // TODO
   static TaskHandle_t task_id = NULL;
   xTaskCreatePinnedToCore (task, TAG, 16 * 1024, NULL, 1, &task_id, 1);        // TODO stack, priority, affinity check?
}
