// GPS time reference
// Copyright © 2019-22 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "gps";
#include "SS.h"

#include "desfireaes.h"
#include "alarm.h"
#include "gps.h"
#include <driver/gpio.h>

#define port_mask(p) ((p)&0x3F)
#define	BITFIELDS "-"
#define PORT_INV 0x40
#define GPIO_INV 0x80           // No SETTING bit

// Other settings
#define settings  \
  io(gpstx) \
  io(gpsrx) \
  io(gpstick) \

#define i8(n,d) int8_t n;
#define io(n) uint8_t n;
#define gpio(n) uint8_t n;
#define u8(n,d) uint8_t n;
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
#undef u16
#undef b
#undef bap
#undef u1
static void task(void *pvParameters)
{
   pvParameters = pvParameters;
   while (1)
   {
      esp_task_wdt_reset();
      usleep(1000000);
   }
}

const char *gps_command(const char *tag, jo_t j)
{
   if (!gpstx || !gpsrx)
      return NULL;              // Not running
   return NULL;
}

void gps_boot(void)
{
   revk_register("gps", 0, sizeof(gpstx), &gpstx, BITFIELDS, SETTING_SET | SETTING_BITFIELD | SETTING_SECRET);  // parent setting
#define i8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_SIGNED);
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
#define gpio(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_BITFIELD);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n,l) revk_register(#n,0,sizeof(n),n,NULL,SETTING_BINDATA|SETTING_HEX);
#define bap(n,l,a) revk_register(#n,a,sizeof(n[0]),n,NULL,SETTING_BINDATA|SETTING_HEX|SETTING_SECRET);
#define u1(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
#define t(n,d) revk_register(#n,0,0,&n,d,0);
   settings
#undef t
#undef io
#undef gpio
#undef i8
#undef u8
#undef u16
#undef b
#undef bap
#undef u1
       if (gpstx && gpsrx)
   {
      const char *e = port_check(port_mask(gpstx), TAG, 0);
      if (!e)
         e = port_check(port_mask(gpsrx), TAG, 1);
      if (e)
         logical_gpio |= logical_GPSFault;
      else
      {

      }
   } else if (gpsrx || gpstx)
      logical_gpio |= logical_GPSFault;
}

void gps_start(void)
{
   if (gpstx && gpsrx)
      revk_task(TAG, task, NULL);
}
