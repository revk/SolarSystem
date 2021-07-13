// Alarm management
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "alarm";
#include "SS.h"
#include "alarm.h"
#include <esp_mesh.h>
const char *alarm_fault = NULL;
const char *alarm_tamper = NULL;

// Alarm control

#define i(x) area_t state_##x;  // system wide aggregated input states
#define s(x) area_t state_##x;  // system wide calculated states
#define c(x) area_t control_##x;        // local control flags
#include "states.m"

area_t lastarmed = 0;

// TODO keypad UI
// TODO commands to clean latched states

#define settings		\
	area(engineering)	\
	area(armed)		\
	area(areafault)		\
	area(areatamper)	\
	area(areaenter)		\
	area(areaarm)		\
	area(areadiarm)		\

#define area(n) area_t n;
settings
#undef area
const char *alarm_command(const char *tag, jo_t j)
{
   return NULL;
}

void alarm_init(void)
{
#include "states.m"
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD);
   settings
#undef area
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
   // Inputs
#define i(x) extern area_t input_latch_##x,input_now_##x;x=input_latch_##x;input_latch_##x=input_now_##x;
#include "states.m"
   // Extras
   extern const char *controller_fault;
   if (controller_fault && strcmp(controller_fault, "{}"))
      fault |= areafault;
   extern const char *controller_tamper;
   if (controller_tamper && strcmp(controller_tamper, "{}"))
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
   // TODO timers?
   state_alarm = (state_armed & state_presence);
   state_tampered |= report_tamper;
   state_faulted |= report_fault;
   state_alarmed |= state_alarm;
   state_prearm = report_arm;
   state_armed |= (report_arm & ~state_presence);
   state_armed &= ~report_unarm;
   state_alarmed &= ~report_unalarm;
   state_tampered &= ~report_untamper;
   state_faulted &= ~report_unfault;

   // Send summary
#define i(x) jo_area(j,#x,state_##x);report_##x=0;
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
         jo_next(j);
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
      return NULL;
   } else
   {                            // We are leaf, get the data
      jo_rewind(j);
      jo_type_t t;
      while ((t = jo_next(j)))
      {
         if (t == JO_TAG)
         {
#define i(x) if(!jo_strcmp(j,#x)){jo_next(j);state_##x=jo_read_area(j);} else
#define s(x) i(x)
#include "states.m"
            {
            }
         }
      }
   }
   // Clear control bits when actioned
   // TODO timer cancel arm
   control_arm &= ~state_armed;
   control_unarm &= state_armed;
   control_unfault &= state_faulted;
   control_unalarm &= state_alarmed;
   control_untamper &= state_tampered;
   // TODO Poke outputs maybe

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
   return NULL;
}
