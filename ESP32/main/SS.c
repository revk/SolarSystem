// Solar System ESP32 app
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "SS";

#include "SS.h"
#include <driver/gpio.h>

#ifdef	CONFIG_REVK_APCONFIG
#warning	You do not want door controller running CONFIG_REVK_APCONFIG
#endif

// Common
static const char *port_inuse[MAX_PORT];

#define modules		\
	m(input)	\
	m(output)	\
	m(ranger)	\
	m(keypad)	\
	m(door)		\
	m(nfc)		\

// Other settings
#define settings  	\
  	io(tamper) 	\
	s(name)		\
	area(area)	\
	s(iothost)	\
	s(iotuser)	\
	s(iotpass)	\
	bd(iotcert,NULL)\
	b(iotstatedoor)	\
	b(iotstateinput)\
	b(iotstateoutput)\
	b(iotstatefault)\
	b(iotstatetamper)\
	b(ioteventfob)	\


#define io(n) static uint8_t n;
#define area(n) area_t n;
#define	s(n) char *n;
#define b(n) uint8_t n;
#define bd(n,d)         static revk_bindata_t *n;
settings
#undef io
#undef area
#undef s
#undef bd
#undef b
#define port_mask(p) ((p)&63)
#define BITFIELDS "-"
#define PORT_INV 0x40
static esp_reset_reason_t reason = -1;  // Restart reason

#if 0
#define i(x) s(x)
#define s(x) static area_t local##x=0; static area_t global##x=0;
states
#define i
#define s
#endif
const char *controller_fault = NULL;
const char *controller_tamper = NULL;

lwmqtt_t iot = NULL;
void iot_init(jo_t j);          // Called for wifi connect

static void status_report(int force)
{                               // Report status change
   static char *lastfault = NULL;
   int faults = 0;
   static char *lasttamper = NULL;
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
            char *res = jo_finisha(&j);
            if (res)
            {
               revk_event("warning", "%s", res);
               free(res);
            }
         }
         reason = -1;           // Just once
      }
      const char *fault = jo_rewind(j);
      if (strcmp(fault ? : "", lastfault ? : "") || force)
      {
         if (lastfault)
            free(lastfault);
         lastfault = strdup(fault);
         revk_statej("fault", &j, iotstatefault ? iot : NULL);
      }
      jo_free(&j);              // safe to call even if freed by revk_statej
   }
   {                            // Tampers
      jo_t j = jo_object_alloc();
#define m(n) extern const char *n##_tamper;if(n##_tamper){jo_string(j,#n,n##_tamper);tampers++;}
      modules m(controller)
#undef m
      const char *tamper = jo_rewind(j);
      if (strcmp(tamper ? : "", lasttamper ? : "") || force)
      {
         if (lasttamper)
            free(lasttamper);
         lasttamper = strdup(tamper);
         revk_statej("tamper", &j, iotstatetamper ? iot : NULL);
      }
      jo_free(&j);              // safe to call even if freed by revk_statej
   }
   if (tampers)
      revk_blink(1, 1);
   else if (faults)
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

const char *app_callback(const char *prefix, const char *target, const char *suffix, jo_t j)
{
   const char *e = NULL;
   if (prefix && !strcmp(prefix, prefixcommand))
   {                            // Commands
      if (target && (!strcmp(target, revk_id) || !strcmp(target, "*")))
      {                         // To us
#define m(x) extern const char * x##_command(const char *,jo_t); jo_rewind(j);if(!e)e=x##_command(suffix,j);
         modules;
#undef m
      }
      if (!target)
      {                         // System commands
         if (!strcmp(suffix, "wifi"))
            iot_init(j);
         else if (strcmp(suffix, "restart"))
            lwmqtt_end(&iot);
         else if (!strcmp(suffix, "connect"))
         {
            status_report(1);
            status_report(0);
         }
      }
   }
   return e;
}

void iot_rx(void *arg, char *topic, unsigned short len, unsigned char *payload)
{
   arg = arg;
   if (topic)
   {                            // Message - we have limited support for IoT based messages, when configured to accept them
      if (!strncmp(topic, "command/", 8))
      {
         char *c = strrchr(topic, '/');
         if (c && *c)
         {
            c++;
            // Commands we handle
            // TODO
         }

      }

   } else if (!payload)
      ESP_LOGI(TAG, "IoT closed (mem:%d)", esp_get_free_heap_size());
   else
   {
      ESP_LOGI(TAG, "IoT open %s", payload);
      char topic[100];
      snprintf(topic, sizeof(topic), "command/%s/%s/#", revk_appname(), revk_id);
      lwmqtt_subscribe(iot, topic);
      snprintf(topic, sizeof(topic), "state/%s/%s", revk_appname(), revk_id);
      lwmqtt_send_full(iot, -1, topic, -1, (void *) "{\"up\":true}", 1, 0);
   }
}

void iot_init(jo_t j)
{
   if (!*iothost)
      return;
   char gw[16] = "",
       slave = 0;
   const char *host = iothost;
   if (j && jo_here(j) == JO_OBJECT)
   {
      jo_type_t t;
      while ((t = jo_next(j)))
         if (t == JO_TAG)
         {
            if (jo_strcmp(j, "slave"))
            {
               if (jo_next(j) == JO_TRUE)
                  slave = 1;
            } else if (jo_strcmp(j, "gw"))
            {
               if (jo_next(j) == JO_STRING)
                  jo_strncpy(j, gw, sizeof(gw));
            }
         }
   }
   if (slave && *gw)
      iothost = gw;
   if (iot)
      lwmqtt_end(&iot);
   char topic[100];
   snprintf(topic, sizeof(topic), "state/%s/%s", revk_appname(), revk_id);
   lwmqtt_client_config_t config = {
      .client = revk_id,
      .hostname = host,
      .username = iotuser,
      .password = iotpass,
      .callback = &iot_rx,
      .topic = topic,
      .plen = -1,
      .retain = 1,
      .payload = (void *) "{\"up\":false}",
   };
   if (iotcert->len)
   {
      config.ca_cert_pem = (void *) iotcert->data;
      config.ca_cert_len = iotcert->len;
   }
   extern revk_bindata_t *clientcert,
   *clientkey;
   if (clientkey->len && clientcert->len)
   {
      config.client_cert_pem = (void *) clientcert->data;
      config.client_cert_len = clientcert->len;
      config.client_key_pem = (void *) clientkey->data;
      config.client_key_len = clientkey->len;
   }
   iot = lwmqtt_client(&config);
}

void app_main()
{
   reason = esp_reset_reason();
   revk_init(&app_callback);
   revk_register("iot", 0, 0, &iothost, NULL, SETTING_SECRET);  // iot group
#define io(n) revk_register(#n,0,sizeof(n),&n,BITFIELDS,SETTING_SET|SETTING_BITFIELD);
#define s(n) revk_register(#n,0,0,&n,NULL,0);
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD);
#define bd(n,d)         revk_register(#n,0,0,&n,d,SETTING_BINDATA);
#define b(n)          revk_register(#n,0,1,&n,NULL,SETTING_BOOLEAN);
   settings
#undef io
#undef area
#undef s
#undef bd
#undef b
   int p;
   for (p = 6; p <= 11; p++)
      port_check(p, "Flash", 0);        // Flash chip uses 6-11
#define m(x) extern void x##_init(void); x##_init();
   modules
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
