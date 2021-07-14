// Alarm management
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "alarm";
#include "SS.h"
#include "desfireaes.h"
#include "alarm.h"
#include "nfc.h"
#include "door.h"
#include "input.h"
#include "output.h"
#include <esp_mesh.h>
#include <esp_http_client.h>
#ifdef  CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif
const char *alarm_fault = NULL;
const char *alarm_tamper = NULL;

// Alarm control

#define i(x) area_t state_##x;  // system wide aggregated input states
#define s(x) area_t state_##x;  // system wide calculated states
#define c(x) area_t control_##x;        // local control flags
#include "states.m"

// TODO keypad UI
// TODO commands to clean latched states

#define settings		\
	area(areafault)		\
	area(areatamper)	\
	area(areaenter)		\
	area(areaarm)		\
	area(areabell)		\
	area(areadisarm)	\
	area(engineering)	\
	area(armed)		\

#define area(n) area_t n;
#define s(n) char *n;
#define ss(n) char *n;
settings
#undef area
#undef s
#undef ss
const char *alarm_command(const char *tag, jo_t j)
{
   // TODO ARM and DISARM commands
   return NULL;
}

void alarm_arm(area_t a, const char *why)
{                               // Arm
   why = why;
   a &= areaarm;
   if (((state_armed | control_arm) & a & ~control_disarm) == a)
      return;                   // All armed
   ESP_LOGD(TAG, "Arm %X %s", a, why);
   control_arm |= a;
   control_disarm &= ~a;
   door_check();
}

void alarm_disarm(area_t a, const char *why)
{                               // Disarm
   why = why;
   a &= areadisarm;
   if (!((state_armed | control_arm) & a & ~control_disarm))
      return;                   // Not armed
   ESP_LOGD(TAG, "Disarm %X %s", a, why);
   control_arm &= ~a;
   control_disarm |= a;
   door_check();
}

void alarm_init(void)
{
#include "states.m"
   revk_register("area", 0, sizeof(areafault), &areafault, AREAS, SETTING_BITFIELD | SETTING_LIVE | SETTING_SECRET);    // TODO something has to be set here to work?
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD|SETTING_LIVE);
#define s(n) revk_register(#n,0,0,&n,NULL,0);
#define ss(n) revk_register(#n,0,0,&n,NULL,SETTING_SECRET);
   settings
#undef area
#undef s
#undef ss
       control_arm = armed;     // Arm from flash state
}

// JSON functions
void jo_area(jo_t j, const char *tag, area_t area)
{                               // Store area
   char set[sizeof(area_t) * 8 + 1] = "",
       *p = set;
   for (int b = 0; AREAS[b]; b++)
      if (area & (1ULL << (sizeof(area_t) * 8 - b - 1)))
         *p++ = AREAS[b];
   *p = 0;
   if (p > set)
      jo_string(j, tag, set);
}

area_t jo_read_area(jo_t j)
{                               // Read area
   if (jo_here(j) != JO_STRING)
      return 0;
   area_t a = 0;
   char val[sizeof(area_t) * 8 + 1];
   jo_strncpy(j, val, sizeof(val));
   for (char *p = val; *p; p++)
   {
      char *d = strchr(AREAS, *p);
      if (d)
         a |= (1ULL << (sizeof(area_t) * 8 - 1 - (d - AREAS)));
   }
   return a;

}

const char *system_makereport(jo_t j)
{                               // Make the report from leaf to root for out states...
#define i(x) area_t x=0;        // what we are going to send
#include "states.m"
   {                            // Inputs
      uint64_t latch = input_latch;
      input_latch = 0;
      uint64_t flip = input_flip;
      input_flip = 0;
      for (int i = 0; i < MAXINPUT; i++)
      {
         if (latch & (1ULL << i))
         {                      // State is active (or has been, even if briefly)
#define i(x) x|=input##x[i];
#include "states.m"
         }
         if (flip & (1ULL << i))
         {                      // State has changed, so causes presense
            presence |= inputtamper[i];
            presence |= inputaccess[i];
         }
      }
   }
   // Extras
   char bell = bell_latch;
   bell_latch = 0;
   if (bell)
      doorbell |= areabell;
   extern const char *last_fault;
   if (last_fault && strcmp(last_fault, "{}"))
      fault |= areafault;
   extern const char *last_tamper;
   if (last_tamper && strcmp(last_tamper, "{}"))
      tamper |= areatamper;
#define i(x) jo_area(j,#x,x);
#define c(x) jo_area(j,#x,control_##x);
#include "states.m"
   return NULL;
}

#define i(x) static area_t report_##x=0;        // The collated reports
#define c(x) i(x)
#include "states.m"

const char *system_makesummary(jo_t j)
{                               // Process reports received, and make summary
#define i(x) state_##x=report_##x;      // Set aggregate states anyway (done by summary anyway)
#include "states.m"

   // Make system states
   // simple latched states
   state_tampered |= report_tamper;
   state_faulted |= report_fault;
   state_alarmed |= state_alarm;
   // arming normally holds off for presence (obviously) but also tamper and access - forcing armed is possible
   state_armed |= (report_arm & ~state_presence & ~(state_tamper & ~engineering) & ~state_access);
   // disarm
   state_armed &= ~report_disarm;
   // prearm if not armed yet
   state_prearm = (report_arm & ~state_armed);
   // Alarm based only on presence, but change of tamper or access trips presence anyway. Basically you can force arm with tamper and access
   state_prealarm = (state_armed & state_presence);
   // TODO delay for alarm from prealarm
   state_alarm |= state_prealarm;

   uint64_t forced = 0;
   for (int i = 0; i < MAXOUTPUT; i++)
   {
#define i(x) if(output##x[i]&state_##x)forced|=(1ULL<<i);
#define s(x) i(x)
#include "states.m"
   }
   output_forced = forced;

   // Send summary
#define i(x) jo_area(j,#x,state_##x);report_##x=0;
#define c(x) report_##x=0;
#define s(x) jo_area(j,#x,state_##x);
#include "states.m"
   jo_area(j, "engineering", engineering);
   return NULL;
}

const char *system_report(jo_t j)
{                               // Alarm state - process a report from a device - aggregate them
   jo_rewind(j);
   jo_type_t t;
   while ((t = jo_next(j)))
   {
      if (t == JO_TAG)
      {
#define i(x) if(!jo_strcmp(j,#x)){jo_next(j);report_##x|=jo_read_area(j);} else
#define c(x) i(x)
#include "states.m"
         {
         }
      }
   }
   return NULL;
}

const char *system_summary(jo_t j)
{                               // Alarm state - process summary of output states
   if (esp_mesh_is_root())
   {                            // We are root, so we have updated anyway, but let's report to IoT
      if (iotstatesystem)
      {
         jo_t c = jo_copy(j);
         revk_state_copy("system", &c, -1);
      }
      // TODO reporting to cloud, if changed...
   } else
   {                            // We are leaf, get the data
#define i(x) area_t x=0;        // Zero if not specified
#define s(x) i(x)
#include "states.m"
      jo_rewind(j);
      jo_type_t t;
      while ((t = jo_next(j)))
      {
         if (t == JO_TAG)
         {
#define i(x) if(!jo_strcmp(j,#x)){jo_next(j);x=jo_read_area(j);} else
#define s(x) i(x)
#include "states.m"
            {
            }
         }
      }
#define i(x) state_##x=x;
#define s(x) i(x)
#include "states.m"
   }
   // Clear control bits when actioned
   // TODO timer cancel arm
   control_arm &= ~state_armed;
   control_disarm &= state_armed;
   // TODO Poke outputs maybe

   // Store armed state
   static area_t lastarmed = -1;
   if (lastarmed != state_armed)
   {
      jo_t j = jo_object_alloc();
      jo_area(j, "armed", state_armed);
      revk_setting(j);
      jo_free(&j);
      lastarmed = state_armed;
      door_check();
   }
   return NULL;
}

const char *system_mesh(const char *suffix, jo_t j)
{                               // Note, some of these fill in j and used by library
   if (!strcmp(suffix, "makereport"))
      return system_makereport(j);
   else if (!strcmp(suffix, "makesummary"))
      return system_makesummary(j);
   else if (!strcmp(suffix, "report"))
      return system_report(j);
   else if (!strcmp(suffix, "summary"))
      return system_summary(j);
   else if (!strcmp(suffix, "noquorum"))
      status(alarm_fault = "No quorum");
   else if (!strcmp(suffix, "quorum"))
      status(alarm_fault = "Missing devices");
   else if (!strcmp(suffix, "fullhouse"))
      status(alarm_fault = NULL);
   else if (!strcmp(suffix, "other"))
   {
      // TODO messages to clear latched states
   }
   return NULL;
}

void send_sms(const char *to, const char *fmt, ...)
{
   if (!to || !*to)
      return;
   char *v = NULL;
   va_list ap;
   va_start(ap, fmt);
   vasprintf(&v, fmt, ap);
   va_end(ap);
   jo_t j = jo_object_alloc();
   jo_string(j, "target", to);
   jo_string(j, "message", v);
   free(v);
   revk_mqtt_send("sms", 1, NULL, &j);
}
