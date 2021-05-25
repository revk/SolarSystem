// Solar System ESP32 app
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "SS";

#include "SS.h"
#include <driver/gpio.h>

#ifdef	CONFIG_REVK_APCONFIG
#error	You do not want door controller running CONFIG_REVK_APCONFIG
#endif

// Common
static const char *port_inuse[MAX_PORT];

#define modules	\
	m(input)	\
	m(output)	\
	m(nfc)		\
	m(ranger)	\
	m(keypad)	\
	m(door)		\

// Other settings
#define settings  \
  io(tamper) \

#define io(n) uint8_t n;
settings
#undef io
#define port_mask(p) ((p)&63)
#define BITFIELDS "-"
#define PORT_INV 0x40
static esp_reset_reason_t reason = -1;  // Restart reason

const char *controller_fault = NULL;
const char *controller_tamper = NULL;

static void status_report(int force)
{                               // Report status change
   static const char *lastfault = NULL;
   static const char *lasttamper = NULL;
   {                            // Faults
      const char *fault = NULL;
      const char *module = NULL;
      int faults = 0;
#define m(n) extern const char *n##_fault;if(n##_fault){fault=n##_fault;module=#n;faults++;}
      modules m(controller)
#undef m
      if (!fault && force && reason >= 0)
      {
         const char *r = NULL;
         if (reason == ESP_RST_POWERON)
            r = "Power on";
         else if (reason == ESP_RST_INT_WDT)
            r = "Int watchdog";
         else if (reason == ESP_RST_TASK_WDT)
            r = "Watchdog";
         else if (reason == ESP_RST_PANIC)
            r = "Panic";
         else if (reason == ESP_RST_BROWNOUT)
            r = "Brownout";
         else
            r = "Restart";
         if (r)
            revk_event("warning", "%s", r);
         reason = -1;           // Just once
      }
      if (lastfault != fault || force)
      {
         lastfault = fault;
         if (faults > 1)
            revk_state("fault", "1 %s: %s (+%d other)", module, fault, faults - 1);
         else if (fault)
            revk_state("fault", "1 %s: %s", module, fault);
         else
            revk_state("fault", "0");
      }
   }
   {                            // Tampers
      const char *tamper = NULL;
      const char *module = NULL;
      int tampers = 0;
#define m(n) extern const char *n##_tamper;if(n##_tamper){tamper=n##_tamper;module=#n;tampers++;}
      modules m(controller)
#undef m
      if (lasttamper != tamper || force)
      {
         lasttamper = tamper;
         if (tampers > 1)
            revk_state("tamper", "1 %s: %s (+%d other)", module, tamper, tampers);
         else if (tamper)
            revk_state("tamper", "1 %s: %s", module, tamper);
         else
            revk_state("tamper", "0");
      }
   }
   if (lasttamper)
      revk_blink(1, 1);
   else if (lastfault)
      revk_blink(1, 5);
   else
      revk_blink(0, 0);
}

// External
const char *port_check(int p, const char *module, int in)
{                               // Check port is OK
   if (p < 0 || p >= MAX_PORT || !GPIO_IS_VALID_GPIO(p))
   {
      revk_error("port", "Port %d is not valid", p);
      if (p < 0 || p >= MAX_PORT)
         return "Bad GPIO port number";
      return "Invalid GPIO port";
   }
   if (!in && !GPIO_IS_VALID_OUTPUT_GPIO(p))
   {
      revk_error("port", "Port %d is not valid for output", p);
      return "Bad GPIO for output";
   }
   if (port_inuse[p])
   {
      revk_error("port", "Port %d is already in use by %s so cannot be used by %s", p, port_inuse[p], module);
      return "GPIO clash";
   }
   port_inuse[p] = module;
   return NULL;                 // OK
}

const char *app_command(const char *tag, unsigned int len, const unsigned char *value)
{
   const char *e = NULL;
#define m(x) extern const char * x##_command(const char *tag,unsigned int len,const unsigned char *value); if(!e)e=x##_command(tag,len,value);
   modules;
#undef m
   if (!strcmp(tag, "connect"))
   {
      status_report(1);
      status_report(0);
   }
   return e;
}

void app_main()
{
   reason = esp_reset_reason();
   revk_init(&app_command);
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
   settings
#undef io
   int p;
   for (p = 6; p <= 11; p++)
      port_check(p, "Flash", 0);        // Flash chip uses 6-11
#define m(x) extern void x##_init(void); x##_init();
   modules
#undef m
       // Main loop, if needed
       if (!tamper)
      return;                   // Not blinking or tamper checking
   if (tamper)
   {
      port_check(port_mask(tamper), "Tamper", 1);
      gpio_reset_pin(port_mask(tamper));
      gpio_set_pull_mode(port_mask(tamper), GPIO_PULLUP_ONLY);
   }
   while (1)
   {                            // Tamper
      if (tamper)
      {
         if (gpio_get_level(port_mask(tamper)) ^ ((tamper & PORT_INV) ? 1 : 0))
         {
            if (!controller_tamper)
               status(controller_tamper = "Tamper");
         } else
         {
            if (controller_tamper)
               status(controller_tamper = NULL);
         }
      }
      usleep(100000);
   }
}

void status(const char *ignored)
{
   ignored = ignored;
   status_report(0);
}

uint8_t bcdtime(time_t now, uint8_t datetime[7])
{
   if (!now)
      now = time(0);
   struct tm *t;
   t = localtime(&now);
   int v = t->tm_year + 1900;
   datetime[0] = (v / 1000) * 16 + (v / 100 % 10);
   datetime[1] = (v / 10 % 10) * 16 + (v % 10);
   v = t->tm_mon + 1;
   datetime[2] = (v / 10) * 16 + (v % 10);
   v = t->tm_mday;
   datetime[3] = (v / 10) * 16 + (v % 10);
   v = t->tm_hour;
   datetime[4] = (v / 10) * 16 + (v % 10);
   v = t->tm_min;
   datetime[5] = (v / 10) * 16 + (v % 10);
   v = t->tm_sec;
   datetime[6] = (v / 10) * 16 + (v % 10);
   return t->tm_wday;
}
