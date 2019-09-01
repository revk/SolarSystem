// Ranger ranger as exit button and motion detect
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "ranger";
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
	p(rangerxshut)		\
	u8(rangerport,0)	\
	u8(rangeraddress,0x29)	\
	u16(rangernear,200)	\
	u16(rangerfar,1000)	\
  	u16(rangermargin,50)	\
	u16(rangerpoll,100)	\
	u16(rangerhold,1000)	\
	u8(rangerinputnear,8)	\
	u8(rangerinputfar,9)	\
	b(rangerdebug)		\

#define	p(n)	uint8_t n;
#define u8(n,d)	uint8_t n;
#define u16(n,d)	uint16_t n;
#define b(n)	uint8_t n;
settings
#undef p
#undef u16
#undef u8
#undef b
static char newforce;
const char *
ranger_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strcmp (tag, "connect"))
      newforce = 1;
   return NULL;
}

static void
task (void *pvParameters)
{                               // Main RevK task
   vl53l0x_t *v = pvParameters;
   void doinit ()
   {
      const char *err;
      if ((err = vl53l0x_init (v)))
      {
         status (ranger_fault = err);
         do
            sleep (1);
         while (vl53l0x_init (v));
         status (ranger_fault = NULL);
      }
   }
   doinit ();
   vl53l0x_setSignalRateLimit (v, 2);   // This helps avoid sunlight / errors, default is 0.25 (MCPS)
   vl53l0x_setMeasurementTimingBudget (v, rangerpoll * 950);
   vl53l0x_startContinuous (v, rangerpoll);
   char *inputnear = NULL;
   if (rangerinputnear)
      asprintf (&inputnear, "input%d", rangerinputnear);
   char *inputfar = NULL;
   if (rangerinputfar)
      asprintf (&inputfar, "input%d", rangerinputfar);
   char buttonnear = 0,
      buttonfar = 0;
   uint32_t last = 0;
   int64_t next = 0;
   int64_t endlong = 0;
   while (1)
   {
      int64_t now = esp_timer_get_time ();
      if (next > now)
         usleep ((now - next) / 1000);
      next = now + (int64_t) rangerpoll *1000LL;
      uint32_t range = vl53l0x_readRangeContinuousMillimeters (v);
      if (range > rangerfar)
         range = rangerfar;
      char force = newforce;
      newforce = 0;
      if (rangerinputnear)
      {
         char change = force;
         if (range < rangernear && last < rangernear)
         {                      // Two polls below set for input 8
            if (!buttonnear)
            {
               buttonnear = 1;
               change = 1;
            }
         } else if (range > rangernear && last > rangernear)
         {                      // Two polls above, so unset input 8
            if (force || buttonnear)
            {
               buttonnear = 0;
               change = 1;
            }
         }
         if (change)
         {
            revk_state (inputnear, "%d %dmm", buttonnear, range);
            input_set (rangerinputnear, buttonnear);
         }
      }
      if (rangerinputfar)
      {
         char change = force;
         int32_t delta = range - last;
         static int32_t lastdelta = 0;
         if ((delta > 0 && lastdelta > 0 && delta + lastdelta >= rangermargin)
             || (delta < 0 && lastdelta < 0 && delta + lastdelta <= -rangermargin))
         {                      // Moved (consistently) rangermargin over two polls
            if (!buttonfar)
            {
               buttonfar = 1;
               change = 1;
            }
            endlong = now + (int64_t) rangerhold *1000LL;
         } else if (endlong < now)
         {                      // Not moved, and we have reached timeout for motion
            if (buttonfar)
            {
               buttonfar = 0;
               change = 1;
            }
         }
         if (change)
         {
            revk_state (inputfar, "%d %dmm", buttonfar, range);
            input_set (9, buttonfar);
         }
         lastdelta = delta;
      }
      if (rangerdebug && (range < rangerfar || last < rangerfar))
         revk_state ("range", "%dmm", range);
      last = range;
      if (vl53l0x_i2cFail (v))
         doinit ();
   }
}

void
ranger_init (void)
{
#define p(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_BITFIELD|SETTING_SET);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u16(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN);
   settings
#undef p
#undef u16
#undef u8
#undef b
      if (rangerscl && rangersda)
   {
      const char *e = port_check (port_mask (rangersda), TAG, 0);
      if (!e)
         e = port_check (port_mask (rangerscl), TAG, 0);
      if (e)
         status (ranger_fault = e);
      else
      {
         vl53l0x_t *vl53l0x =
            vl53l0x_config (rangerport, port_mask (rangerscl), port_mask (rangersda), rangerxshut ? port_mask (rangerxshut) : -1,
                            rangeraddress, 0);
         if (!vl53l0x)
            status (ranger_fault = "Missing");
         else
            revk_task (TAG, task, vl53l0x);
      }
   } else if (rangersda || rangerscl)
      status (ranger_fault = "Set rangerscl and rangersda");
}
