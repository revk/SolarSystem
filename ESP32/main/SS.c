// Solar System ESP32 app
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "SS";

#include "SS.h"
#include "alarm.h"
#include <driver/gpio.h>
#include <esp_mesh.h>

#ifdef	CONFIG_REVK_APCONFIG
#warning	You do not want door controller running CONFIG_REVK_APCONFIG
#endif

#ifdef	CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED
#warning CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED may not be a good idea
#endif
#ifdef	CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED
#warning CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED may not be a good idea
#endif

#ifndef CONFIG_ESP32_WIFI_STATIC_TX_BUFFER
#warning CONFIG_ESP32_WIFI_STATIC_TX_BUFFER may be better
#else
#if CONFIG_ESP32_WIFI_STATIC_RX_BUFFER_NUM+CONFIG_ESP32_WIFI_DYNAMIC_RX_BUFFER_NUM+CONFIG_ESP32_WIFI_STATIC_TX_BUFFER_NUM > 30
#warning Maybe fewer wifi buffers
#endif
#endif

#ifndef	CONFIG_REVK_MESH
#error	CONFIG_REVK_MESH requried
#endif

// Common
static const char *port_inuse[MAX_PORT];

#define modules		\
	m(alarm)	\
	m(input)	\
	m(output)	\
	m(keypad)	\
	m(door)		\
	m(nfc)		\

// Other settings
#define settings  	\
  	io(tamper) 	\
	s(name)		\
	b(iotstatedoor)	\
	b(iotstateinput)\
	b(iotstateoutput)\
	b(iotstatefault)\
	b(iotstatetamper)\
	b(iotstatesystem)\
	b(ioteventfob)	\


#define io(n) static uint8_t n;
#define area(n) area_t n;
#define	s(n) char *n;
#define	sa(n,a) char *n[a];
#define b(n) uint8_t n;
#define bd(n,d)         static revk_bindata_t *n;
settings
#undef io
#undef area
#undef s
#undef sa
#undef bd
#undef b
#define port_mask(p) ((p)&63)
#define BITFIELDS "-"
#define PORT_INV 0x40
static esp_reset_reason_t reason = -1;  // Restart reason

const char *controller_fault = NULL;
const char *controller_tamper = NULL;

const char *last_fault = NULL;
const char *last_tamper = NULL;

static void status_report(int force)
{                               // Report status change
   int faults = 0;
   int tampers = 0;
   {                            // Faults
      jo_t j = jo_object_alloc();
#define m(n) extern const char *n##_fault;if(n##_fault){jo_string(j,#n,n##_fault);faults++;}
      modules m(controller)
#undef m
      if (!faults && force && reason >= 0)
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
         {
            jo_t j = jo_object_alloc();
            jo_string(j, "controller", r);
            revk_event("warning", &j);
         }
         reason = -1;           // Just once
      }
      const char *fault = jo_rewind(j);
      if (strcmp(fault ? : "", last_fault ? : "") || force)
      {
         free((void *) last_fault);
         last_fault = strdup(fault);
         revk_state_copy("fault", &j, iotstatefault);
         if (faults)
            latch_fault |= areafault;
      }
      jo_free(&j);              // safe to call even if freed by revk_state
   }
   {                            // Tampers
      jo_t j = jo_object_alloc();
#define m(n) extern const char *n##_tamper;if(n##_tamper){jo_string(j,#n,n##_tamper);tampers++;}
      modules m(controller)
#undef m
      const char *tamper = jo_rewind(j);
      if (strcmp(tamper ? : "", last_tamper ? : "") || force)
      {
         free((void *) last_tamper);
         last_tamper = strdup(tamper);
         revk_state_copy("tamper", &j, iotstatetamper);
         latch_presence |= areatamper;  // Change is presence
         if (faults)
            latch_tamper |= areatamper;
      }
      jo_free(&j);              // safe to call even if freed by revk_state
   }
   // TODO moved to alarm module...
   if (tampers)
      revk_blink(1, 1, "R-");
   else if (faults)
      revk_blink(1, 5, "M-");
   else if (esp_mesh_is_root())
      revk_blink(1, 5, revk_offline()? "GR-" : "G-");
   else if (revk_offline())
      revk_blink(1, 5, "C-");
   else
      revk_blink(0, 0, "RYGCBM");
}

// External
const char *port_check(int p, const char *module, int in)
{                               // Check port is OK
   if (p < 0 || p >= MAX_PORT || !GPIO_IS_VALID_GPIO(p))
   {
      jo_t j = jo_object_alloc();
      jo_string(j, "description", "Port not valid");
      jo_string(j, "module", module);
      jo_int(j, "port", p);
      revk_error("port", &j);
      if (p < 0 || p >= MAX_PORT)
         return "Bad GPIO port number";
      return "Invalid GPIO port";
   }
   if (!in && !GPIO_IS_VALID_OUTPUT_GPIO(p))
   {
      jo_t j = jo_object_alloc();
      jo_string(j, "description", "Port not valid for output");
      jo_string(j, "module", module);
      jo_int(j, "port", p);
      revk_error("port", &j);
      return "Bad GPIO for output";
   }
   if (port_inuse[p])
   {
      jo_t j = jo_object_alloc();
      jo_string(j, "description", "Port clash");
      jo_string(j, "module", module);
      jo_string(j, "clash", port_inuse[p]);
      revk_error("port", &j);
      return "GPIO clash";
   }
   port_inuse[p] = module;
   return NULL;                 // OK
}

const char *app_callback(int client, const char *prefix, const char *target, const char *suffix, jo_t j)
{
   const char *e = NULL;
   if (!client && prefix && !strcmp(prefix, "mesh") && suffix)
      return system_mesh(suffix, j);
   if (client || !prefix || target || strcmp(prefix, prefixcommand))
      return NULL;              // Not for us or not a command from main MQTT
#define m(x) extern const char * x##_command(const char *,jo_t); jo_rewind(j);if(!e)e=x##_command(suffix,j);
   modules;
#undef m
   if (!strcmp(suffix, "connect"))
   {
      status_report(1);
      status_report(0);
   }
   return e;
}

uint8_t iotcopy;                // group heading
void app_main()
{
   reason = esp_reset_reason();
   revk_init(&app_callback);
   revk_register("iot", 0, 0, &iotcopy, "true", SETTING_BOOLEAN | SETTING_SECRET);      // iot group
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
#define s(n) revk_register(#n,0,0,&n,NULL,0);
#define sa(n,a) revk_register(#n,a,0,&n,NULL,0);
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD);
#define bd(n,d)         revk_register(#n,0,0,&n,d,SETTING_BINDATA);
#define b(n)          revk_register(#n,0,1,&n,NULL,SETTING_BOOLEAN);
   settings
#undef io
#undef area
#undef s
#undef sa
#undef bd
#undef b
   int p;
   for (p = 6; p <= 11; p++)
      port_check(p, "Flash", 0);        // Flash chip uses 6-11
#define m(x) extern void x##_init(void); x##_init();
   modules;
#undef m
   // Main loop, if needed
   if (!tamper)
      return;                   // Not tamper checking, nothing to do.
   if (tamper)
   {
      port_check(port_mask(tamper), "Tamper", 1);
      gpio_reset_pin(port_mask(tamper));
      gpio_set_direction(port_mask(tamper), GPIO_MODE_INPUT);
      gpio_set_pull_mode(port_mask(tamper), GPIO_PULLUP_ONLY);
   }
   while (1)
   {                            // Tamper
      if (tamper)
      {
         if (gpio_get_level(port_mask(tamper)) ^ ((tamper & PORT_INV) ? 1 : 0))
         {
            if (!controller_tamper)
               status(controller_tamper = "Main board tamper switch");
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

uint8_t bcdutctime(time_t now, uint8_t datetime[7])
{
   if (!now)
      now = time(0);
   struct tm *t;
   t = gmtime(&now);
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

uint8_t bcdlocaltime(time_t now, uint8_t datetime[7])
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
