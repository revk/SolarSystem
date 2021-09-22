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
#include "keypad.h"
#include <esp_mesh.h>
#include <esp_http_client.h>
#ifdef  CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif
#include "freertos/semphr.h"

// Alarm control

#define i(t,x,c) area_t state_##x;      // system wide aggregated input states
#define s(t,x,c) area_t state_##x;      // system wide calculated states
#define c(t,x) area_t control_##x;      // local control flags
#include "states.m"
const char *state_name[] = {
#define i(t,x,c) #x,
#define s(t,x,c) #x,
#include "states.m"
};

static SemaphoreHandle_t node_mutex = NULL;     // protect changes to node list

typedef struct node_s {
   mac_t mac;                   // Node MAC
   uint8_t online:1;            // Is on line
   uint8_t missed:2;            // Missed report count
   uint8_t reported:1;          // Has reported
   uint8_t bigflash:1;          // Has more than 4MiB flash
   uint8_t part:3;              // Received parts
   area_t display;              // Areas for which this is a display
} node_t;
static node_t *node = NULL;
static int nodes = 0;
static int nodes_online = 0;
static int nodes_reported = 0;
int64_t summary_next = 0;       // Send summary cycle
int64_t report_next = 0;        // Send report cycle
uint32_t last_summary = 0;      // When last summary (uptime)
uint32_t control_summary = 0;   // When next send control summary

static SemaphoreHandle_t display_mutex = NULL;
typedef struct display_s display_t;     // Display message list
struct display_s {
   display_t *next;
   mac_t mac;                   // Source MAC
   area_t area;                 // Impacted area
   uint8_t priority:4;          // Impacted priority 
   uint8_t seen:1;              // Has been seen in this report cycle
   char text[];                 // The display text (malloc)
};
static display_t *display = NULL;
#define MAX_ROOT_DISPLAY (MAX_LEAF_DISPLAY*2)

#define settings		\
	area(arealed)		\
	area(areaenter)		\
	area(areaarm)		\
	area(areastrongarm)	\
	area(areadisarm)	\
	area(areadeadlock)	\
	areanl(areakeypad)	\
	area(areakeydisarm)	\
	area(areakeyarm)	\
	area(engineer)		\
	area(armed)		\
	u16(armcancel)		\
	u16(armdelay)		\
	u16(alarmdelay)		\
	u16(alarmhold)		\
        u8(meshcycle,3)		\
        u8(meshwarmup,60)	\
        u8(meshflap,10)	\
	u8(meshdied,240)	\
	area(smsarm)		\
	area(smsdisarm)		\
	area(smsarmfail)	\
	area(smsalarm)		\
	area(smspanic)		\
	area(smsfire)		\
	arean(mixand,MAX_MIX)	\
	arean(mixset,MAX_MIX)	\

#define area(n) area_t n;
#define areanl(n) area_t n;
#define arean(n,q) area_t n[q];
#define s(n,d) char *n;
#define sn(n,q) char *n[q];
#define u16(n) uint16_t n;
#define u8(n,d) uint16_t n;
settings
#undef area
#undef areanl
#undef arean
#undef s
#undef sn
#undef u16
#undef u8
#define c(t,x) static area_t report_##x=0;      // The collated reports
#define i(t,x,l) static area_t report_##x=0;    // The collated reports
#include "states.m"
static area_t held_prearm = 0;
static void task(void *pvParameters);
static void node_online(const mac_t mac);
static void sms_event(const char *tag, jo_t);
static void set_outputs(void);

const char *alarm_command(const char *tag, jo_t j)
{
   if (!strcmp(tag, "connect"))
   {
      if (esp_mesh_is_root())
      {
         control_summary = 0;
         for (int i = 0; i < nodes; i++)
            if (node[i].online)
               node_online(node[i].mac);
      }
      return NULL;
   }
   if (!strcmp(tag, "arm"))
   {
      jo_t e = jo_make("Remote");
      alarm_arm(jo_read_area(j), &e);
      return "";
   }
   if (!strcmp(tag, "strongarm"))
   {
      jo_t e = jo_make("Remote");
      alarm_strongarm(jo_read_area(j), &e);
      return "";
   }
   if (!strcmp(tag, "disarm"))
   {
      jo_t e = jo_make("Remote");
      alarm_disarm(jo_read_area(j), &e);
      return "";
   }
   return NULL;
}

area_t alarm_armed(void)
{                               // What areas are, in effect, armed
   return (state_armed | control_arm | control_strongarm) & ~control_disarm;
}

void alarm_arm(area_t a, jo_t * jp)
{                               // Arm
   if (!(a & ~alarm_armed()))
   {
      jo_free(jp);
      return;                   // All armed
   }
   area_t a2 = (andset(alarm_armed() | a) & ~alarm_armed());
   jo_t j = NULL;
   if (jp)
   {
      j = *jp;
      *jp = NULL;
   }
   if (!j)
      j = jo_make(NULL);
   ESP_LOGD(TAG, "Arm %X", a);
   control_arm |= a;            // Each area only in one control
   control_strongarm &= ~a;
   control_disarm &= ~a;
   door_check();
   jo_area(j, "areas", a);
   if (a2 & ~a)
      jo_area(j, "also", a2 & ~a);
   if (smsarm & a2)
      sms_event("Armed", j);
   alarm_event("arm", &j, ioteventarm);
}

void alarm_strongarm(area_t a, jo_t * jp)
{                               // Strong arm
   if (!(a & ~((state_armed | control_strongarm) & ~control_disarm)))   // Not using alarm_armed as that includes what we are trying, and failing, to control_arm
   {
      jo_free(jp);
      return;                   // All armed
   }
   area_t a2 = (andset(alarm_armed() | a) & ~alarm_armed());
   jo_t j = NULL;
   if (jp)
   {
      j = *jp;
      *jp = NULL;
   }
   if (!j)
      j = jo_make(NULL);
   ESP_LOGD(TAG, "Strong arm %X", a);
   control_strongarm |= a;      // Each area only in one control
   control_arm &= ~a;
   control_disarm &= ~a;
   door_check();
   jo_area(j, "areas", a);
   if (a2 & ~a)
      jo_area(j, "also", a2 & ~a);
   if (smsarm & a2)
      sms_event("Strong armed", j);
   alarm_event("strongarm", &j, ioteventarm);
}

void alarm_disarm(area_t a, jo_t * jp)
{                               // Disarm
   if (!(a & alarm_armed()))
   {
      jo_free(jp);
      return;                   // Not armed
   }
   area_t a2 = (alarm_armed() & ~andset(alarm_armed() & ~a));
   jo_t j = NULL;
   if (jp)
   {
      j = *jp;
      *jp = NULL;
   }
   if (!j)
      j = jo_make(NULL);
   ESP_LOGD(TAG, "Disarm %X", a);
   control_disarm |= a;         // Each area only in one control
   control_arm &= ~a;
   control_strongarm &= ~a;
   door_check();
   jo_area(j, "areas", a);
   if (a2 & ~a)
      jo_area(j, "also", a2 & ~a);
   if (smsdisarm & a2)
      sms_event("Disarmed", j);
   alarm_event("disarm", &j, ioteventarm);
}

void alarm_boot(void)
{
   node_mutex = xSemaphoreCreateBinary();
   xSemaphoreGive(node_mutex);
   display_mutex = xSemaphoreCreateBinary();
   xSemaphoreGive(display_mutex);
   revk_register("area", 0, sizeof(arealed), &arealed, AREAS, SETTING_BITFIELD | SETTING_LIVE | SETTING_SECRET);        // Will control if shown in dump!
   revk_register("sms", 0, sizeof(smsalarm), &smsalarm, AREAS, SETTING_BITFIELD | SETTING_LIVE | SETTING_SECRET);
   revk_register("mix", sizeof(mixand) / sizeof(*mixand), sizeof(*mixand), &mixand, AREAS, SETTING_BITFIELD | SETTING_LIVE | SETTING_SECRET);
#define area(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD|SETTING_LIVE);
#define areanl(n) revk_register(#n,0,sizeof(n),&n,AREAS,SETTING_BITFIELD);
#define arean(n,q) revk_register(#n,q,sizeof(*n),&n,AREAS,SETTING_BITFIELD|SETTING_LIVE);
#define s(n,d) revk_register(#n,0,0,&n,#d,SETTING_LIVE);
#define sn(n,q) revk_register(#n,q,0,&n,NULL,SETTING_LIVE);
#define u16(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_LIVE);
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_LIVE);
   settings;
#undef area
#undef areanl
#undef arean
#undef s
#undef sn
#undef u16
#undef u8
   // Pick up flash stored state to get started
   state_armed = armed;
   state_engineer = engineer;
   set_outputs();               // Ensure output states set so when output starts it knows expected state
}

void alarm_start(void)
{
   revk_task(TAG, task, NULL);
}

// JSON functions
char *area_list(char set[sizeof(area_t) * 8 + 1], area_t area)
{
   char *p = set;
   for (int b = 0; AREAS[b]; b++)
      if (area & (1ULL << (sizeof(area_t) * 8 - b - 1)))
         *p++ = AREAS[b];
   *p = 0;
   return set;
}

void jo_area(jo_t j, const char *tag, area_t area)
{                               // Store area
   char set[sizeof(area_t) * 8 + 1] = "";
   if (area)
      jo_string(j, tag, area_list(set, area));
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

static int mesh_find_child(const mac_t mac, char insert)
{
   xSemaphoreTake(node_mutex, portMAX_DELAY);
   int l = 0,
       m = 0,
       h = nodes - 1,
       d = -1;
   while (l <= h)
   {
      m = (l + h) / 2;
      d = memcmp(mac, node[m].mac, 6);
      if (d < 0)
         h = m - 1;
      else if (d > 0)
         l = m + 1;
      else
         break;
   }
   if (d)
   {
      m = -1;                   // Not found
      if (insert)
      {
         if (nodes >= meshmax)
            ESP_LOGE(TAG, "Too many children (%d)", nodes);
         else
         {                      // Insert
            m = l;
            ESP_LOGD(TAG, "Added leaf %02X%02X%02X%02X%02X%02X at %d/%d", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], m, nodes + 1);
            if (m < nodes)
               memmove(&node[m + 1], &node[m], (nodes - m) * sizeof(node_t));
            nodes++;
            memset(&node[m], 0, sizeof(node_t));
            memcpy(node[m].mac, mac, 6);
         }
      }
   }
   xSemaphoreGive(node_mutex);
   return m;
}

void mesh_send_report(void)
{                               // Make the report from leaf to root for out states...
#define MAX (MESH_MPS-MESH_PAD)
   jo_t report = NULL;
   uint8_t count = 0;
   uint8_t parts = 0;
   void start(void) {
      if (report)
         return;
      report = jo_object_alloc();
      jo_array(report, "report");
      count = 0;
      parts++;
   }
   void stop(void) {
      if (!report)
         return;
      jo_close(report);
      if (!count)
         return;
      jo_string(report, "@", nodename);
   }
   void send(void) {
      if (!report)
         return;
      stop();
      revk_mesh_send_json(NULL, &report);       // Split
   }
   static area_t was_prearm = 0;
   {                            // Inputs
      input_t latch = input_latch;
      input_latch = 0;
      input_t flip = input_flip;
      input_flip = 0;
      for (int i = 0; i < MAXINPUT; i++)
      {
#define i(t,x,c) area_t x=0;    // what we are going to send
#include "states.m"
         area_t trigger = (inputpresence[i] | inputaccess[i] | (inputtamper[i] & ~state_engineer));
         if ((latch | input_stable) & (1ULL << i))
         {                      // State is active (or has been, even if briefly)
#define i(t,x,c) x|=input##x[i];
#include "states.m"
            if ((trigger & state_prearm) && !(trigger & was_prearm))
            {
               jo_t e = jo_make(NULL);
               jo_string(e, "input", inputname[i]);
#define i(x) if(input##x[i]&state_prearm)jo_area(e,#x,input##x[i]&(state_armed|state_prearm));
               i(presence);     // Only these relate to alarm/trigger
               i(access);
               i(tamper);
#undef i
               alarm_event("inhibit", &e, ioteventarm);
            }
         }
         if (flip & (1ULL << i))
         {                      // State has changed, so causes presence and event logging
            if (trigger & state_armed)
            {                   // Event log
               if (trigger & (state_alarm | state_prealarm))
               {                // Event while alarmed as well
                  jo_t e = jo_make(NULL);
                  jo_string(e, "input", inputname[i]);
#define i(x) if(input##x[i]&state_armed)jo_area(e,#x,input##x[i]&(state_armed|state_prearm));
                  i(presence);  // Only these relate to alarm/trigger
                  i(access);
                  i(tamper);
#undef i
                  alarm_event("trigger", &e, ioteventarm);
               } else
                  input_flip |= (1ULL << i);    // Defer trigger, maintain that there is an input to trip
            }
            presence |= inputtamper[i];
            presence |= inputaccess[i];
            if ((latch | input_stable) & (1ULL << i))
            {                   // Event reporting regardless
#define i(x) if(input##x[i]){jo_t e = jo_make(NULL);jo_string(e, "input", inputname[i]);jo_area(e,#x,input##x[i]);alarm_event(#x, &e, 0);}
               i(warning);
               i(tamper);
               i(fault);
               i(panic);
#undef i
            }
         }
         if (
#define i(t,x,c) x||
#include "states.m"
               0)
         {                      // Report for input
            jo_t j = jo_object_alloc();
            jo_string(j, "@", inputname[i]);
#define i(t,x,c) if(#x)jo_area(j,#t,x);
#include "states.m"
            if (jo_len(j) + jo_len(report) > MAX - 25)
               send();
            if (!report)
               start();
            jo_json(report, NULL, j);
            jo_free(&j);
            count++;
         }
      }
   }
   was_prearm = state_prearm;
   if (jo_len(report) > MAX - 60 - 25)
      send();
   if (!report)
      start();
   stop();
   // Control (not area only in one control)
#define c(t,x) jo_area(report,#t,control_##x);
#include "states.m"
   jo_int(report, "#", parts);
   revk_mesh_send_json(NULL, &report);
}

static void node_offline(const mac_t mac)
{
}

static void node_online(const mac_t mac)
{
   if (memcmp(mac, revk_mac, 6))
   {
      revk_send_sub(0, mac);
      revk_send_sub(1, mac);
      jo_t j = jo_object_alloc();
      jo_null(j, "connect");
      revk_mesh_send_json(mac, &j);
   }
}

static void mesh_now_root(void)
{
}

static void mesh_now_leaf(void)
{
}

static int check_online(const char *target)
{
   if (!target || strlen(target) != 12)
      return -1;
   mac_t mac;
   for (int n = 0; n < sizeof(mac); n++)
      mac[n] = (((target[n * 2] & 0xF) + (target[n * 2] > '9' ? 9 : 0)) << 4) + ((target[1 + n * 2] & 0xF) + (target[1 + n * 2] > '9' ? 9 : 0));
   int child = mesh_find_child(mac, 1);
   if (child < 0)
      return child;
   if (!node[child].online)
   {                            // Is there risk of a race in any way? mutex?
      node[child].online = 1;
      node[child].missed = 0;
      node[child].reported = 0;
      nodes_online++;
      node_online(mac);
   }
   return child;
}

area_t andset(area_t a)
{                               // Mix and set
   for (int s = 0; s < MAX_MIX; s++)
   {
      if (mixand[s] & ~a)
         a &= ~mixset[s];
      else
         a |= mixset[s];
   }
   return a;
}

static void mesh_send_summary(void)
{                               // Process reports received, and make summary
   jo_t j = jo_object_alloc();
   jo_datetime(j, "summary", time(0));
#define i(t,x,c) area_t was_##x=state_##x;
#define s(t,x,c) area_t was_##x=state_##x;
#include "states.m"
#define i(t,x,c) state_##x=report_##x;  // Set aggregate states anyway (done by summary anyway)
#include "states.m"
   // Make system states
   // simple latched states - cleared by re-arming
   // arming normally holds off for presence (obviously) but also tamper and access - forcing armed is possible
   // prearm if any not armed yet
   state_prearm = (andset(report_arm | state_armed) & ~state_armed & ~report_disarm);   // and/set to ensure we see implied arming areas
   static uint16_t timer1 = 0;
   if (!state_prearm || (state_prearm & (state_presence | (state_tamper & ~engineer) | state_access)))
   {                            // No prearm, or prearm waiting - restart timer
      timer1 = 0;
      state_armed = andset((state_armed | report_strongarm) & ~report_disarm);  // Apply strongarm anyway
   } else if (!armdelay || (timer1 += meshcycle) > armdelay)
      state_armed = andset((state_armed | state_prearm | report_strongarm) & ~report_disarm);   // Prearm is clean and ready to apply
   static uint16_t timer2 = 0;  // Pre arm hold
   held_prearm = ((held_prearm | state_prearm) & ~state_armed); // Hold pre arm so we can see on display
   if (!state_prearm)
      timer2 = 0;
   else if ((timer2 += meshcycle) > armcancel)
      held_prearm = 0;
   // What changed
   area_t new_armed = (state_armed & ~was_armed);
   // Arming clears latched states
   state_tampered = ((state_tampered & ~new_armed) | report_tamper);
   state_faulted = ((state_faulted & ~new_armed) | report_fault);
   state_alarmed = ((state_alarmed & ~new_armed) | state_alarm);
   // Alarm based only on presence, but change of tamper or access trips presence anyway. Basically you can force arm with tamper and access
   state_prealarm = (((state_prealarm | state_presence) & state_armed) & ~state_alarm);
   static uint16_t timer3 = 0;  // Pre alarm timer - ideally per area, but this will be fine
   if (!state_prealarm)
      timer3 = 0;
   else if (!alarmdelay || (timer3 += meshcycle) > alarmdelay)
      state_alarm = ((state_alarm | state_prealarm) & state_armed);
   state_alarm &= state_armed;
   static uint16_t timer4 = 0;  // Post alarm timer - ideally per area, but this will be fine
   if (state_prealarm)
      timer4 = 0;
   else if (alarmhold && (timer4 += meshcycle) > alarmhold)
      state_alarm = 0;
   // Fixed
   state_engineer = engineer;   // From flash - could be changed live though, so set here
   // Send summary
#define i(t,x,c) jo_area(j,#t,state_##x);report_##x=0;
#define s(t,x,c) jo_area(j,#t,state_##x);
#define c(t,x) report_##x=0;
#include "states.m"
   const mac_t addr = { 255, 255, 255, 255, 255, 255 };
   revk_mesh_send_json(addr, &j);
   if (esp_mesh_is_root() && !revk_link_down())
   {                            // Report to control
      uint32_t now = uptime();
      was_presence = state_presence;    // Not doing these
      was_access = state_access;        // Not doing these
      static uint8_t lastnodes = 0;
      static display_t *lastdisplay = NULL;
      if (now > control_summary || nodes != lastnodes || display != lastdisplay
#define i(t,x,c) ||was_##x!=state_##x
#define s(t,x,c) ||was_##x!=state_##x
#include "states.m"
          )
      {
         lastdisplay = display;
         lastnodes = nodes;
         control_summary = now + 3600;
         j = jo_make("");
         jo_string(j, "root", nodename);
         jo_int(j, "nodes", nodes);
         if (nodes_online < nodes)
            jo_int(j, "offline", nodes - nodes_online);
         if (nodes < meshmax)
            jo_int(j, "missing", meshmax - nodes);
         char set[sizeof(area_t) * 8 + 1] = "";
         if (display)
            jo_stringf(j, "status", "%c%s %s %s", toupper(*state_name[display->priority]), state_name[display->priority] + 1, area_list(set, display->area), display->text);
#define i(t,x,c) if(strcmp(#x,"access")&&strcmp(#x,"presence"))jo_area(j,#x,state_##x); // Using full name to control
#define s(t,x,c) jo_area(j,#x,state_##x);
#include "states.m"
         revk_state_clients("system", &j, 1 | (iotstatesystem << 1));
      }
   }

   void new_event(const char *event, priority_t p, area_t mask, area_t sms) {
      if (!mask)
         return;
      jo_t j = jo_make("");
      jo_area(j, "areas", mask);
      jo_array(j, "triggers");
      xSemaphoreTake(display_mutex, portMAX_DELAY);
      for (display_t * d = display; d; d = d->next)
         if (d->priority == p && (d->area & mask))
            jo_string(j, NULL, d->text);
      xSemaphoreGive(display_mutex);
      if (sms & mask)
         sms_event(event, j);
      alarm_event(event, &j, ioteventarm);
   }
   static area_t lastalarm = -1;
   if (lastalarm != state_alarm)
   {
      new_event("alarm", priority_presence, state_alarm & ~lastalarm, smsalarm);
      lastalarm = state_alarm;
   }
   static area_t lastpanic = -1;
   if (lastpanic != state_panic)
   {
      new_event("panic", priority_panic, state_panic & ~lastpanic, smspanic);
      lastpanic = state_panic;
   }
   static area_t lastfire = -1;
   if (lastfire != state_fire)
   {
      new_event("fire", priority_fire, state_fire & ~lastfire, smsfire);
      lastfire = state_fire;
   }
}

static void mesh_send_display(void)
{                               // Update display devices
   // Cleanup
   xSemaphoreTake(display_mutex, portMAX_DELAY);
   display_t **dp = &display;
   while (*dp)
   {
      display_t *d = *dp;
      if (!d->seen && (d->priority != priority_presence || !(d->area & state_prealarm)))
      {                         // Gone - but we retain presence in prealarm state so shown on alarm event
         *dp = d->next;
         free(d);
      } else
         dp = &d->next;
   }
   xSemaphoreGive(display_mutex);
   xSemaphoreTake(node_mutex, portMAX_DELAY);
   for (int i = 0; i < nodes; i++)
      if (node[i].display && node[i].online)
      {
         jo_t j = jo_object_alloc();
         jo_array(j, "display");
         int count = 0;
         char set[sizeof(area_t) * 8 + 1] = "";
         for (display_t * d = display; d; d = d->next)
            if (d->seen && (d->area & node[i].display) && count++ < MAX_LEAF_DISPLAY)
               jo_stringf(j, NULL, "%c%s: %s\n%s", toupper(*state_name[d->priority]), state_name[d->priority] + 1, area_list(set, d->area & node[i].display), d->text);
         revk_mesh_send_json(node[i].mac, &j);
      }
   xSemaphoreGive(node_mutex);
}

static void mesh_handle_report(const char *target, jo_t j)
{                               // Alarm state - process a report from a device - aggregate them
   int child = check_online(target);
   if (child < 0)
      return;
   node[child].part++;
   char dev[17] = "";
   jo_type_t t;
   jo_rewind(j);
   for (jo_next(j); (t = jo_here(j)) > JO_CLOSE; jo_skip(j))
      if (t == JO_TAG)
      {
#define c(t,x) if(!jo_strcmp(j,#x)||!jo_strcmp(j,#t)){jo_next(j);report_##x|=jo_read_area(j);} else
#include "states.m"
         if (!jo_strcmp(j, "@"))
         {
            jo_next(j);
            jo_strncpy(j, dev, sizeof(dev));
         } else if (!jo_strcmp(j, "#"))
         {                      // Last part of report
            jo_next(j);
            if (node[child].part == jo_read_int(j))
            {                   // Got all parts
               node[child].missed = 0;
               if (!node[child].reported)
               {
                  node[child].reported = 1;
                  nodes_reported++;
               }
            }
            node[child].part = 0;
         }
      }
   jo_rewind(j);
   jo_next(j);                  // report tag
   if (jo_next(j) == JO_ARRAY)
   {                            // Handle report inputs
      while ((t = jo_next(j)) == JO_OBJECT)
      {
         char id[17] = "";
         while ((t = jo_next(j)) && t != JO_CLOSE)
            if (t == JO_TAG)
            {                   // fields in report
               void add_display(priority_t p, area_t a) {
                  // Some filtering
                  if ((p == priority_access || p == priority_presence) && !(a & (state_armed | state_prearm | held_prearm | state_prealarm)))
                     return;
                  // Add display
                  char text[35];
                  snprintf(text, sizeof(text), "%s: %s", dev, id);
                  xSemaphoreTake(display_mutex, portMAX_DELAY);
                  display_t **dp = &display;
                  int count = 0;
                  while (dp)
                  {
                     display_t *d = *dp;
                     if (d && d->priority == p && d->area == a && !memcmp(d->mac, target, sizeof(d->mac)) && !strcmp(d->text, text))
                     {          // Match/update
                        d->seen = 1;
                        break;
                     }
                     if (!d || d->priority < p)
                     {          // New
                        display_t *n = malloc(sizeof(*n) + strlen(text) + 1);
                        memset(n, 0, sizeof(*n));
                        memcpy(n->mac, target, sizeof(n->mac));
                        n->priority = p;
                        n->area = a;
                        n->seen = 1;
                        strcpy(n->text, text);
                        n->next = d;
                        *dp = n;
                        break;
                     }
                     dp = &d->next;
                     count++;
                  }
                  while (dp && count++ < MAX_ROOT_DISPLAY)
                     dp = &(*dp)->next;
                  while (dp && *dp)
                  {             // Too many
                     display_t *d = *dp;
                     *dp = d->next;
                     free(d);
                  }
                  xSemaphoreGive(display_mutex);
               }
#define i(t,x,l) if(!jo_strcmp(j,#t)){jo_next(j);area_t a=jo_read_area(j);report_##x|=a;add_display(priority_##x,a);continue;} else
#include "states.m"
               if (!jo_strcmp(j, "@"))
               {                // ID
                  jo_next(j);
                  jo_strncpy(j, id, sizeof(id));
                  continue;
               }
            }
      }
   }
}

static void set_outputs(void)
{                               // Outputs set based on state
   output_t forced = 0;
   for (int i = 0; i < MAXOUTPUT; i++)
   {
#define i(t,x,c) if(output##x[i]&state_##x)forced|=(1ULL<<i);
#define s(t,x,c) if(output##x[i]&state_##x)forced|=(1ULL<<i);
#include "states.m"
   }
   output_forced = forced;
}

static void mesh_handle_summary(const char *target, jo_t j)
{                               // Alarm state - process summary of output states
   last_summary = uptime();
   report_next = esp_timer_get_time() + 1000000LL * meshcycle / 4 + 1000000LL * meshcycle * esp_random() / (1ULL << 32) / 2;    // Fit in reports random slots
   check_online(target);
   if (!esp_mesh_is_root())
   {                            // We are leaf, get the data
#define i(t,x,c) area_t x=0;    // Zero if not specified
#define s(t,x,c) area_t x=0;    // Zero if not specified
#include "states.m"
      jo_rewind(j);
      jo_type_t t;
      while ((t = jo_next(j)))
         if (t == JO_TAG)
         {
            if (!jo_strcmp(j, "summary"))
            {
               jo_next(j);
               if (jo_here(j) == JO_STRING)
               {
                  time_t new = jo_read_datetime(j);
                  if (new > 1000000000)
                  {
                     time_t now = time(0);
                     time_t diff = new - now;
                     if (diff > 60 || diff < -60)
                     {          // Big change
                        if (now > 1000000000 && diff < -300)
                        {
                           struct tm tm;
                           gmtime_r(&new, &tm);
                           ESP_LOGE(TAG, "Replay attack? diff=%d new=%04d-%02d-%02dT%02d:%02d:%02dZ", (int) diff, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                           // return; // Note we are seeing this in normal working, so something is amiss... Investigate
                        }
                        struct timeval tv = { new, 0 };
                        if (settimeofday(&tv, NULL))
                           ESP_LOGE(TAG, "Time set %d failed", (int) new);
                     } else if (diff)
                     {
                        struct timeval delta = { diff, 0 };
                        adjtime(&delta, NULL);
                     }
                  }
               }
            } else
#define i(t,x,c) if(!jo_strcmp(j,#x)||!jo_strcmp(j,#t)){jo_next(j);x=jo_read_area(j);} else
#define s(t,x,c) if(!jo_strcmp(j,#x)||!jo_strcmp(j,#t)){jo_next(j);x=jo_read_area(j);} else
#include "states.m"
            {                   // Unknown?
            }
         }
#define i(t,x,c) state_##x=x;
#define s(t,x,c) state_##x=x;
#include "states.m"
   }
   // Clear control bits when actioned
   control_arm &= ~state_armed;
   control_strongarm &= ~state_armed;
   control_disarm &= state_armed;
   static uint16_t timer = 0;
   if (!control_arm)
      timer = 0;
   else if (armcancel && (timer += meshcycle) > armcancel)
   {                            // Cancel arming (ideally per area, but this is good enough)
      if (smsarmfail & control_arm)
      {
         jo_t j = jo_make(NULL);
         jo_area(j, "areas", control_arm);
         sms_event("Arm failed", j);
         alarm_event("armfail", &j, ioteventarm);
      }
      control_arm = 0;
      door_check();
   }
   set_outputs();

   // Store armed state
   static area_t lastarmed = -1;
   if (lastarmed != state_armed)
   {
      jo_t j = jo_object_alloc();
      if (state_armed)
         jo_area(j, "armed", state_armed);
      else
         jo_null(j, "armed");
      const char *er = revk_setting(j);
      if (er && *er)
         ESP_LOGE(TAG, "Setting error %s (%s)", er, jo_debug(j));
      jo_free(&j);
      door_check();
      lastarmed = state_armed;
   }
}

static void task(void *pvParameters)
{                               // 
   esp_task_wdt_add(NULL);
   pvParameters = pvParameters;
   uint32_t isroot = 0;
   uint32_t flapping = 0;
   int wasonline = 0;
   node = malloc(sizeof(*node) * meshmax);
   mesh_find_child(revk_mac, 1);        // We count as a child
   node[0].online = 1;          // Us
   nodes_online++;
   while (1)
   {
      esp_task_wdt_reset();
      {                         // Set LED mode
         int r = 1;
         if ((isroot && !revk_link_down() && nodes_online == meshmax) || (!isroot && esp_mesh_is_device_active()))
            r = 3;
         const char *led = "G";
#define i(t,x,c) if((state_##x&(arealed?:(area_t)-1))&&*#c)led=#c;
#define s(t,x,c) if((state_##x&(arealed?:(area_t)-1))&&*#c)led=#c;
#include "states.m"
         revk_blink(r, r, led);
      }
      // Waiting
      uint64_t now = esp_timer_get_time();
      if (!esp_mesh_is_device_active())
      {
         sleep(1);
         continue;
      }
      int64_t next = report_next;
      if (isroot && next > summary_next)
         next = summary_next;
      if (next > now + 1000000LL * meshcycle / 4)
         next = now + 1000000LL * meshcycle / 4;        // Max time we wait, as we can change the waiting on rx of summary
      if (next > now)
      {
         usleep(next - now);
         now = next;
      }
      // Periodic
      if (isroot && summary_next <= now)
      {                         // Summary reporting cycle
         summary_next = now + 1000000LL * meshcycle;
         // Check off line
         for (int n = 0; n < nodes; n++)
            if (!node[n].reported && node[n].online)
            {                   // Gone off line
               if (node[n].missed < 3)
                  node[n].missed++;     // Allow some missed
               else
               {
                  node[n].online = 0;
                  nodes_online--;
                  if (!memcmp(node[n].mac, revk_mac, 6))
                  {             // Should not happen
                     ESP_LOGE(TAG, "Self offline");
                     continue;
                  }
                  char mac[13];
                  sprintf(mac, "%02X%02X%02X%02X%02X%02X", node[n].mac[0], node[n].mac[1], node[n].mac[2], node[n].mac[3], node[n].mac[4], node[n].mac[5]);
                  revk_send_unsub(0, node[n].mac);
                  revk_send_unsub(1, node[n].mac);
                  char *topic;  // Tell IoT
                  asprintf(&topic, "state/%s/%s", appname, mac);
                  revk_mqtt_send_raw(topic, 1, "{\"up\":false}", -1);
                  free(topic);
                  node_offline(node[n].mac);
                  flapping = uptime() + meshflap;       // Ignore changes for a moment
               }
            }
         static char missed = 0;
         if (nodes_reported >= nodes_online)
         {                      // We have a full set, make a report - the off line logic means we may miss a report if a device goes off line
            mesh_send_summary();
            mesh_send_display();
            missed = 0;
            // Clear reports
            for (int n = 0; n < nodes; n++)
               node[n].reported = 0;
            nodes_reported = 0;
            // Clear display
            xSemaphoreTake(display_mutex, portMAX_DELAY);
            for (display_t * d = display; d; d = d->next)
               d->seen = 0;
            xSemaphoreGive(display_mutex);
         } else
            missed += nodes_online - nodes_reported;
      }
      if (report_next <= now)
      {                         // Periodic send to root - even to self
         report_next = now + 1000000LL * meshcycle;
         mesh_send_report();
         if (meshdied && last_summary + meshdied < uptime())
            revk_restart("No summaries", 0);
      }
      if (esp_mesh_is_root())
      {
         if (!isroot)
         {                      // We have become root
            mesh_now_root();
            isroot = uptime();
            wasonline = 0;
            // Clear down
            for (int i = 0; i < nodes; i++)
            {
               node[i].online = (memcmp(node[i].mac, revk_mac, 6) ? 0 : 1);     // All that are not us
               node[i].missed = 0;
               node[i].reported = 0;
            }
            nodes_online = 1;   // Us
            nodes_reported = 0;
            revk_mqtt_init();
            report_next = 0;    // Send report from us to us
            summary_next = now + 1000000LL * meshcycle; // Start reporting cycle
         }
         if (uptime() - isroot > meshwarmup)
         {                      // Checking network
            if (flapping < uptime())
            {                   // Consider changes
               if (wasonline != nodes_online)
               {                // Change to mesh
                  wasonline = nodes_online;
                  // Simple missing nodes picked up by control
                  if (nodes_online == 1 && meshmax > 1)
                     logical_gpio |= logical_MeshFault; // Lonely
                  else
                     logical_gpio &= ~logical_MeshFault;
               }
            }
         }
      } else
      {
         if (isroot)
         {                      // We are no longer root
            nodes_online = 1;   // Us
            revk_mqtt_close("Not root");
            freez(node);
            mesh_now_leaf();
         }
      }
   }
}

void alarm_event(const char *event, jo_t * jp, char copy)
{                               // Send an event
   revk_event_clients(event, jp, 1 | (copy ? 2 : 0));
#if 0
   jo_t o = jo_object_alloc();
   jo_string(o, copy ? "event+" : "event", event);
   jo_json(o, NULL, *jp);       // Add object content in line
   jo_free(jp);
   revk_mesh_send_json(NULL, &o);
#endif
}

#if 0                           // Sending via root loses the sender ID in topic. Maybe work out how to send from right sender later
void mesh_handle_event(jo_t j)
{
   if (!esp_mesh_is_root())
      return;
   char copy = !jo_strcmp(j, "event+");
   if (jo_next(j) != JO_STRING)
      return;
   char event[20];
   jo_strncpy(j, event, sizeof(event));
   jo_next(j);
   char *m;
   asprintf(&m, "{%s", jo_debug(j));    // Slightly message usage of jo_debug...
   jo_t o = jo_parse_str(m);
   revk_event_clients(event, &o, 1 | (copy ? 2 : 0));
   free(m);
   // We can, at this point, pick up some types of event if we need
}
#endif

void mesh_handle_capability(const char *target, jo_t j)
{
   int child = check_online(target);
   if (child < 0)
      return;
   area_t display = 0;
   uint8_t bigflash = 0;
   jo_type_t t;
   while ((t = jo_next(j)))
      if (t == JO_TAG)
      {
         if (!jo_strcmp(j, "display"))
         {
            jo_next(j);
            display = jo_read_area(j);
            continue;
         }
         if (!jo_strcmp(j, "flash"))
         {
            jo_next(j);
            if (jo_read_int(j) > 4 * 1024 * 1024)
               bigflash = 1;
            continue;
         }
      }
   node[child].display = display;
   node[child].bigflash = bigflash;
}

void alarm_rx(const char *target, jo_t j)
{
   ESP_LOGD(TAG, "Rx JSON %s %s", target, jo_rewind(j) ? : "?");
   jo_rewind(j);
   if (jo_here(j) != JO_OBJECT)
      return;
   jo_next(j);
   if (jo_here(j) != JO_TAG)
      return;
#if 0
   if (!jo_strcmp(j, "event") || !jo_strcmp(j, "event+"))
   {
      mesh_handle_event(j);
      return;
   }
#endif
   if (!jo_strcmp(j, "capability"))
   {
      if (esp_mesh_is_root())
         mesh_handle_capability(target, j);
      return;
   }
   if (!jo_strcmp(j, "report"))
   {
      mesh_handle_report(target, j);
      return;
   }
   if (!jo_strcmp(j, "summary"))
   {
      mesh_handle_summary(target, j);
      return;
   }
   if (!jo_strcmp(j, "display"))
   {
      keypad_display_update(j);
      return;
   }
   if (!jo_strcmp(j, "connect"))
   {
      revk_command("status", NULL);     // For up message
      app_callback(0, prefixcommand, NULL, "connect", j);
      // Send capability
      jo_t j = jo_object_alloc();
      jo_null(j, "capability");
      if (areakeypad)
         jo_area(j, "display", areakeypad);
      jo_int(j, "flash", spi_flash_get_chip_size());
      revk_mesh_send_json(NULL, &j);
      return;
   }
}

void send_sms(const char *fmt, ...)
{
   char *v = NULL;
   va_list ap;
   va_start(ap, fmt);
   vasprintf(&v, fmt, ap);
   va_end(ap);
   jo_t j = jo_object_alloc();
   jo_string(j, "message", v);
   free(v);
   revk_mqtt_send("sms", 1, NULL, &j);
}

static void sms_event(const char *tag, jo_t j)
{
   char areas[sizeof(area_t) * 8 + 1] = "";
   char also[sizeof(area_t) * 8 + 1] = "";
   char ts[21] = "";
   char id[15] = "";
   char node[30] = "";
   char name[30] = "";
   char reason[20] = "";
   jo_rewind(j);
   jo_type_t t;
   while ((t = jo_next(j)))
      if (t == JO_TAG)
      {
         if (!jo_strcmp(j, "ts"))
         {
            jo_next(j);
            jo_strncpy(j, ts, sizeof(ts));
            continue;
         }
         if (!jo_strcmp(j, "reason"))
         {
            jo_next(j);
            jo_strncpy(j, reason, sizeof(reason));
            continue;
         }
         if (!jo_strcmp(j, "node"))
         {
            jo_next(j);
            jo_strncpy(j, node, sizeof(node));
            continue;
         }
         if (!jo_strcmp(j, "name"))
         {
            jo_next(j);
            jo_strncpy(j, name, sizeof(name));
            continue;
         }
         if (!jo_strcmp(j, "id"))
         {
            jo_next(j);
            jo_strncpy(j, id, sizeof(id));
            continue;
         }
         if (!jo_strcmp(j, "areas"))
         {
            jo_next(j);
            jo_strncpy(j, areas, sizeof(areas));
            continue;
         }
         if (!jo_strcmp(j, "also"))
         {
            jo_next(j);
            jo_strncpy(j, also, sizeof(also));
            continue;
         }
      }
   send_sms("%s\n%s: %s%s%s\n%s\n%s %s %s", ts, tag, areas, *also ? "+" : "", also, node, reason, id, name);
}
