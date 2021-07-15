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

area_t latch_fault = 0;         // From board fault
area_t latch_tamper = 0;        // From board tamper
area_t latch_warning = 0;       // From board tamper
area_t latch_presence = 0;      // From board tamper
static uint32_t summary_next = 0;       // When to report summary

// TODO keypad UI
// TODO commands to clean latched states

#define settings		\
	area(areawarning)	\
	area(areafault)		\
	area(areatamper)	\
	area(areaenter)		\
	area(areaarm)		\
	area(areabell)		\
	area(areadisarm)	\
	area(engineer)		\
	area(armed)		\
	u16(armcancel)		\
	u16(alarmdelay)		\
	u16(alarmhold)		\

#define area(n) area_t n;
#define s(n) char *n;
#define ss(n) char *n;
#define u16(n) uint16_t n;
settings
#undef area
#undef s
#undef ss
#undef u16
const char *alarm_command(const char *tag, jo_t j)
{
   if (!strcmp(tag, "connect"))
      summary_next = uptime() + 1;      // Report
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
#define u16(n) revk_register(#n,0,sizeof(n),&n,NULL,0);
   settings
#undef area
#undef s
#undef ss
#undef u16
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
      input_t latch = input_latch;
      input_latch = 0;
      input_t flip = input_flip;
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
   // Latched from local fault or tamper
   area_t latch = latch_fault;
   latch_fault = 0;
   fault |= latch;
   latch = latch_tamper;
   latch_tamper = 0;
   tamper |= latch;
   latch = latch_presence;
   latch_presence = 0;
   presence |= latch;
   latch = latch_warning;
   latch_warning = 0;
   warning |= latch;
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
   // simple latched states - cleared by re-arming
   state_tampered = ((state_tampered & ~report_arm) | report_tamper);
   state_faulted = ((state_faulted & ~report_arm) | report_fault);
   state_alarmed = ((state_alarmed & ~report_arm) | state_alarm);
   // arming normally holds off for presence (obviously) but also tamper and access - forcing armed is possible
   state_armed = ((state_armed | report_forcearm | (report_arm & ~state_presence & ~(state_tamper & ~engineer) & ~state_access)) & ~report_disarm);
   // prearm if any not armed yet
   state_prearm = (report_arm & ~state_armed);
   // Alarm based only on presence, but change of tamper or access trips presence anyway. Basically you can force arm with tamper and access
   state_prealarm = (((state_prealarm | state_presence) & state_armed) & ~state_alarm);
   static uint16_t timer1 = 0;  // Pre alarm timer - ideally per area, but this will be fine
   if (!state_prealarm)
      timer1 = 0;
   else if (!alarmdelay || (timer1 += meshcycle) > alarmdelay)
      state_alarm = ((state_alarm | state_prealarm) & state_armed);
   static uint16_t timer2 = 0;  // Post alarm timer - ideally per area, but this will be fine
   if (state_prealarm)
      timer2 = 0;
   else if (alarmhold && (timer2 += meshcycle) > alarmhold)
      state_alarm = 0;
   // Fixed
   state_engineer = engineer;   // From flash - could be changed live though, so set here

   // Send summary
#define i(x) jo_area(j,#x,state_##x);report_##x=0;
#define c(x) report_##x=0;
#define s(x) jo_area(j,#x,state_##x);
#include "states.m"
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
         {                      // Unknown?
         }
      }
   }
   return NULL;
}

const char *system_summary(jo_t j)
{                               // Alarm state - process summary of output states
   if (esp_mesh_is_root())
   {                            // We are root, so we have updated anyway, but let's report to IoT
      const char *json = jo_rewind(j);
      if (json)
      {
         json = strchr(json, ',');      // Skip time as that changes every time, duh
         static unsigned int last = 0;  // using a CRC is a lot less memory than a copy of this or of the states
         unsigned int crc = 0;
         if (json)
            crc = df_crc(strlen(json), (void *) json);
         uint32_t now = uptime();
         if (last != crc || now > summary_next)
         {                      // Changed
            summary_next = now + 60;
            last = crc;
            jo_t c = jo_copy(j);
            revk_state_copy("system", &c, iotstatesystem);
         }
      }
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
            {                   // Unknown?
            }
         }
      }
#define i(x) state_##x=x;
#define s(x) i(x)
#include "states.m"
   }
   // Clear control bits when actioned
   control_arm &= ~state_armed;
   control_forcearm &= ~state_armed;
   control_disarm &= state_armed;
   static uint16_t timer = 0;
   if (!control_arm)
      timer = 0;
   else if (armcancel && (timer += meshcycle) > armcancel)
   {                            // Cancel arming (ideally per area, but this is good enough)
      control_arm = 0;
      // TODO event on arming times out
   }
   // Outputs
   output_t forced = 0;
   for (int i = 0; i < MAXOUTPUT; i++)
   {
#define i(x) if(output##x[i]&state_##x)forced|=(1ULL<<i);
#define s(x) i(x)
#include "states.m"
   }
   output_forced = forced;

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
