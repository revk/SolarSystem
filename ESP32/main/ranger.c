// Ranger ranger as exit button and motion detect
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
const char TAG[] = "ranger";
#include "SS.h"
const char *ranger_fault = NULL;
const char *ranger_tamper = NULL;

#include "vl53l0x.h"
#include "input.h"

#define BITFIELDS "-"
#define PORT_INV 0x40
#define port_mask(p) ((p)&63)

#define settings		\
	p(rangersda)		\
	p(rangerscl)		\
	u8(rangerport,0)	\
	u8(rangeraddress,0)	\
	u16(rangerclose,200)	\
	u16(rangerfar,1000)	\
	u16(rangerpoll,100)	\
	u8(rangerinputnear,8)	\
	u8(rangerinputfar,9)	\

#define	p(n)	uint8_t n;
#define u8(n,d)	uint8_t n;
#define u16(n,d)	uint16_t n;
settings
#undef p
#undef u16
#undef u8
static vl53l0x_t *vl53l0x = NULL;

const char *
ranger_command (const char *tag, unsigned int len, const unsigned char *value)
{
   // TODO
   return NULL;
}

static void
task (void *pvParameters)
{                               // Main RevK task
   pvParameters = pvParameters;
   while (1)
   {
      usleep (rangerpoll * 1000 ? : 1000);
   }
}

void
ranger_init (void)
{
#define p(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_BITFIELD|SETTING_SET);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
   settings
#undef p
#undef u16
#undef u8
      if (rangerscl && rangersda)
   {
      const char *e = port_check (port_mask (rangersda), TAG, 0);
      if (!e)
         e = port_check (port_mask (rangerscl), TAG, 0);
      if (e)
         status (ranger_fault = e);
      else
      {
         vl53l0x_t *vl53l0x = vl53l0x_init (rangerport, port_mask (rangerscl), port_mask (rangersda), rangeraddress, 0);
         if (!vl53l0x)
            status (ranger_fault= "Missing");
         else
         {                      // Start task
            static TaskHandle_t task_id = NULL;
            xTaskCreatePinnedToCore (task, TAG, 16 * 1024, NULL, 1, &task_id, tskNO_AFFINITY);  // TODO stack, priority, affinity check?
         }
      }
   } else if (rangersda || rangerscl)
      status (ranger_fault = "Set rangerscl and rangersda");
}
