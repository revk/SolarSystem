// Solar System ESP32 app
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "SS";

#include "SS.h"
#include "alarm.h"
#include <driver/gpio.h>
#include <esp_mesh.h>

#ifndef	CONFIG_REVK_MESH
#error	Needs CONFIG_REVK_MESH
#endif

#if	CONFIG_REVK_OTAAUTO != 0
#error	Do not auto upgrade (OTAAUTO should be 0)
#endif

#ifdef	CONFIG_REVK_APMODE
#warning	You do not want door controller running CONFIG_REVK_APMODE
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

// Output first to minimise startup
// Input before door
#define modules		\
	m(output)	\
	m(input)	\
	m(keypad)	\
	m(door)		\
	m(nfc)		\
	m(alarm)	\
	m(gps)		\

// Other settings
#define settings  	\
  	io(tamper) 	\
	bl(iotstatedoor)	\
	bl(iotstateinput)\
	bl(iotstateoutput)\
	bl(iotstatesystem)\
	bl(ioteventfob)	\
	bl(ioteventarm)	\
	bl(iotkeypad)	\
	bl(iotgps)	\
	s(iottopic)	\
	bl(debug)	\

#define io(n) static uint8_t n;
#define area(n) area_t n;
#define	s(n) char *n;
#define	sa(n,a) char *n[a];
#define b(n) uint8_t n;
#define bl(n) uint8_t n;
#define bd(n,d)         static revk_bindata_t *n;
settings
#undef io
#undef area
#undef s
#undef sa
#undef bd
#undef b
#undef bl
#define port_mask(p) ((p)&63)
#define BITFIELDS "-"
#define PORT_INV 0x40
   uint32_t logical_gpio = 0;   // Logical GPIO (from GPIO 48 to 63 and -48 to -63, covers NFC, keypad, GPS, etc.)

// External
const char *
port_check (uint8_t p, const char *module, uint8_t in)
{                               // Check port is OK
   if (p < LOGIC_PORT)
   {                            // Physical port
      if (in && !(gpio_ok (p) & 2))
      {
         ESP_LOGE (TAG, "Port %d not input (%s)", p, module);
         jo_t j = jo_object_alloc ();
         jo_string (j, "description", "Port not valid");
         jo_string (j, "module", module);
         jo_int (j, "port", p);
         revk_error_clients ("port", &j, 1);
         return "Bad GPIO for input";
      }
      if (!in && !(gpio_ok (p) & 1))
      {
         ESP_LOGE (TAG, "Port %d not output (%s)", p, module);
         jo_t j = jo_object_alloc ();
         jo_string (j, "description", "Port not valid for output");
         jo_string (j, "module", module);
         jo_int (j, "port", p);
         revk_error_clients ("port", &j, 1);
         return "Bad GPIO for output";
      }
   }
   static uint64_t port_inuse = 0;
   if (p < LOGIC_PORT && (port_inuse & (1ULL << p)))
   {                            // Only check clash on real GPIOs
      ESP_LOGE (TAG, "Port %d clash (%s)", p, module);
      jo_t j = jo_object_alloc ();
      jo_string (j, "description", "Port clash");
      jo_string (j, "module", module);
      revk_error_clients ("port", &j, 1);
      return "GPIO clash";
   }
   port_inuse |= (1ULL << p);
   return NULL;                 // OK
}

const char *
app_callback (int client, const char *prefix, const char *target, const char *suffix, jo_t j)
{
   const char *e = NULL;
   if (!client && prefix && !strcmp (prefix, "mesh"))
   {
      alarm_rx (target, j);
      return NULL;
   }
#ifdef	CONFIG_FREERTOS_USE_TRACE_FACILITY
   if (client && prefix && !strcmp (prefix, "command") && suffix && !strcmp (suffix, "ps"))
      return revk_command ("ps", j);
#endif
   if (!prefix || target || strcmp (prefix, prefixcommand) || (client && strcmp (suffix, "connect")))
      return NULL;              // Not for us or not a command from main MQTT
#define m(x) extern const char * x##_command(const char *,jo_t); jo_rewind(j);if(!e)e=x##_command(suffix,j);
   modules;
#undef m
   return e;
}

uint8_t iotcopy;                // group heading
void
app_main ()
{
   revk_boot (&app_callback);
   revk_register ("iot", 0, 0, &iotcopy, "true", SETTING_BOOLEAN | SETTING_SECRET);     // iot group
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
#define s(n) revk_register(#n,0,0,&n,NULL,0);
#define sa(n,a) revk_register(#n,a,0,&n,NULL,0);
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD);
#define bd(n,d)         revk_register(#n,0,0,&n,d,SETTING_BINDATA);
#define b(n)          revk_register(#n,0,1,&n,NULL,SETTING_BOOLEAN);
#define bl(n)          revk_register(#n,0,1,&n,NULL,SETTING_BOOLEAN|SETTING_LIVE);
   settings
#undef io
#undef area
#undef s
#undef sa
#undef bd
#undef b
#undef bl
#ifdef  CONFIG_IDF_TARGET_ESP32
      port_check (1, "Serial", 0);
   port_check (3, "Serial", 0);
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
   port_check (43, "Serial", 0);
   port_check (44, "Serial", 0);
#endif
#define m(x) extern void x##_boot(void); ESP_LOGI(TAG,"Boot "#x); x##_boot();
   modules;
#undef m
   // Start
   revk_start ();
#define m(x) extern void x##_start(void); ESP_LOGI(TAG,"Start "#x); x##_start();
   modules;
#undef m
   // Main loop, if needed
}

uint8_t
bcdutctime (time_t now, uint8_t datetime[7])
{
   if (!now)
      now = time (0);
   struct tm *t;
   t = gmtime (&now);
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

uint8_t
bcdlocaltime (time_t now, uint8_t datetime[7])
{
   if (!now)
      now = time (0);
   struct tm *t;
   t = localtime (&now);
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
