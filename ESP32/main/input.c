// Logical inputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "input";
#include "SS.h"
#include "input.h"
#include "alarm.h"

#include <driver/gpio.h>

// Input ports
#define	BITFIELDS "-"
#define	PORT_INV 0x40
#define	port_mask(p) ((p)&63)
static uint8_t in[MAXINPUT];
static uint8_t inhold[MAXINPUT];     // Time held left for debounce
static uint8_t intime[MAXINPUT];     // Time active so far
static uint8_t infunc[MAXINPUT];     // Input functions
static uint8_t inputfuncs;      // Combined input funcs
#define i(t,x,c) area_t in##x[MAXINPUT];
#define c(t,x) area_t in##x[MAXINPUT];
#include "states.m"
char *inname[MAXINPUT];

// Other settings
#define settings	\
u8 (inpoll, 10);	\

#define u8(n,v) uint8_t n
settings
#undef u8
static input_t input_raw = 0;
static input_t input_invert = 0;
input_t input_stable = 0;
input_t input_latch = 0;        // holds resettable state of input
input_t input_flip = 0;         // holds flipped flag for each input, i.e. state has changed

static uint32_t report_next = 0;

int input_active(int p)
{                               // Port from 1
   if (p < 1 || p > MAXINPUT)
      return 0;
   p--;
   if (!in[p])
      return 0;
   return 1;
}

int input_get(int p)
{                               // Port from 1
   if (p < 1 || p > MAXINPUT)
      return -1;
   p--;
   if (input_stable & (1ULL << p))
      return 1;
   return 0;
}

int input_func_active(uint8_t f)
{                               // Does a function exist at all (expected to be one bit set)
   if (inputfuncs & f)
      return 1;
   return 0;
}

int input_func_all(uint8_t f)
{                               // Are all inputs for a function set (expected to be one bit set)
   for (int p = 0; p < MAXINPUT; p++)
      if ((infunc[p] & f) && !(input_stable & (1ULL << p)))
         return 0;
   return 1;
}

const char *input_func_any(uint8_t f)
{                               // Are any inputs for a function set (expected to be one bit set) - returns name of most recently active input
   char *found = NULL;
   uint8_t best = 0xFF;
   for (int p = 0; p < MAXINPUT; p++)
      if ((infunc[p] & f) && (input_stable & (1ULL << p)) && intime[p] <= best)
      {
         best = intime[p];
         found = inname[p];
      }
   return found;
}

const char *input_command(const char *tag, jo_t j)
{
   if (!strcmp(tag, "connect"))
      report_next = uptime() + 1;
   return NULL;
}

static void task(void *pvParameters)
{                               // Input poll
   esp_task_wdt_add(NULL);
   pvParameters = pvParameters;
   int poll = (inpoll ? : 1);
   static uint8_t input_hold[MAXINPUT] = { 0 };
   // Scan inputs
   while (1)
   {
      esp_task_wdt_reset();
      // Check inputs
      input_t was = input_stable;
      for (int i = 0; i < MAXINPUT; i++)
         if (in[i])
         {
            int p = port_mask(in[i]),
                v;
            if (p < LOGIC_PORT)
               v = gpio_get_level(p);
            else if (p >= LOGIC_PORT2 && (in[i] & PORT_INV))
               v = ((logical_gpio >> (16 + p - LOGIC_PORT)) & 1);       // Non invertable logical GPIO, i.e. extra ones
            else
               v = ((logical_gpio >> (p - LOGIC_PORT)) & 1);    // Logical GPIO, e.g. NFC ports, etc.
            if ((1ULL << i) & input_invert)
               v = 1 - v;
            if (v != ((input_raw >> i) & 1))
            {                   // Change of raw state
               input_raw = ((input_raw & ~(1ULL << i)) | ((input_t) v << i));
               input_hold[i] = (inhold[i] ? : 100);  // Start countdown
            }
            if (input_hold[i])
            {                   // counting down
               if (input_hold[i] < poll)
                  input_hold[i] = 0;
               else
                  input_hold[i] -= poll;
               if (!input_hold[i])      // hold done
               {
                  input_stable = ((input_stable & ~(1ULL << i)) | ((input_t) v << i));
                  if (v)
                  {
                     jo_t make(void) {
                        jo_t e = jo_make(NULL);
                        jo_string(e, "reason", inname[i]);
                        return e;
                     }
                     if (indisarm[i] && (indisarm[i] & alarm_armed()))
                     {          // Disarm
                        jo_t e = make();
                        alarm_disarm(indisarm[i], &e);
                     } else
                     {          // Arm - i.e. allows for same button to do both
                        if (inarm[i])
                        {
                           jo_t e = make();
                           alarm_arm(inarm[i], &e);
                        }
                        if (instrong[i])
                        {
                           jo_t e = make();
                           alarm_strong(instrong[i], &e);
                        }
                     }
                  }
               }
            }
            if (!(input_stable & (1ULL << i)))
               intime[i] = 0;
            else if (intime[i] < 255)
               intime[i]++;
         }
      uint32_t now = uptime();
      if (was != input_stable || now > report_next)
      {                         // JSON
         report_next = now + 3600;
         input_latch |= (input_stable & ~was);  // Latch rising edges
         input_flip |= (input_stable ^ was);    // Latch any change
         jo_t j = jo_make(NULL);
         for (int i = 0; i < MAXINPUT; i++)
            if (in[i] && *inname[i])
               jo_bool(j, inname[i], (input_stable >> i) & 1);
         revk_state_clients("input", &j, debug | (iotstateinput << 1));
      }
      // Sleep
      usleep(poll * 1000);
   }
}

void input_boot(void)
{
   revk_register("in", MAXINPUT, sizeof(*in), &in, BITFIELDS, SETTING_BITFIELD | SETTING_SET | SETTING_SECRET);
   revk_register("ingpio", MAXINPUT, sizeof(*in), &in, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
   revk_register("inhold", MAXINPUT, sizeof(*inhold), &inhold, NULL, SETTING_LIVE);
   revk_register("infunc", MAXINPUT, sizeof(*infunc), &infunc, INPUT_FUNCS, SETTING_BITFIELD);
   revk_register("inname", MAXINPUT, 0, &inname, NULL, SETTING_LIVE);
#define i(t,x,c) revk_register("in"#x, MAXINPUT, sizeof(*in##x), &in##x, AREAS, SETTING_BITFIELD|SETTING_LIVE);
#define c(t,x) revk_register("in"#x, MAXINPUT, sizeof(*in##x), &in##x, AREAS, SETTING_BITFIELD|SETTING_LIVE);
#include "states.m"
#define u8(n,v) revk_register(#n,0,sizeof(n),&n,#v,0);
   settings
#undef u8
       inputfuncs = 0;
   for (int i = 0; i < MAXINPUT; i++)
      inputfuncs |= infunc[i];
   {                            // GPIO
    gpio_config_t I = { mode:GPIO_MODE_INPUT };
    gpio_config_t U = { mode: GPIO_MODE_INPUT, pull_up_en:GPIO_PULLUP_ENABLE };
    gpio_config_t D = { mode: GPIO_MODE_INPUT, pull_down_en:GPIO_PULLDOWN_ENABLE };
      int i,
       p;
      for (i = 0; i < MAXINPUT; i++)
         if (in[i])
         {
            const char *e = port_check(p = port_mask(in[i]), TAG, 1);
            if (e)
               in[i] = 0;
            else
            {
               if (p < MAX_PORT)
               {
                  if (p >= 34)
                     I.pin_bit_mask |= (1ULL << p);     // Do not have pull up/down
                  else if (infunc[i] & INPUT_FUNC_P)
                     D.pin_bit_mask |= (1ULL << p);     // Pull down
                  else
                     U.pin_bit_mask |= (1ULL << p);     // Pull up
                  if (p == 7 || p == 8)
                     gpio_reset_pin(p);
                  if (p != 20)
                     REVK_ERR_CHECK(gpio_hold_dis(p));
               }
               if (p < LOGIC_PORT2 && (in[i] & PORT_INV))
               {                // Inverted
                  input_invert |= (1ULL << i);
                  if (p >= LOGIC_PORT)
                     logical_gpio |= (1ULL << (p - LOGIC_PORT));        // Init off
               }
            }
         }
      if (I.pin_bit_mask)
         REVK_ERR_CHECK(gpio_config(&I));
      if (U.pin_bit_mask)
         REVK_ERR_CHECK(gpio_config(&U));
      if (D.pin_bit_mask)
         REVK_ERR_CHECK(gpio_config(&D));
   }
   // Init state
   for (int i = 0; i < MAXINPUT; i++)
      if (in[i])
      {
         int p = port_mask(in[i]),
             v;
         if (p < LOGIC_PORT)
            v = gpio_get_level(p);
         else if (p >= LOGIC_PORT2 && (in[i] & PORT_INV))
            v = ((logical_gpio >> (16 + p - LOGIC_PORT)) & 1);  // Non invertable logical GPIO, i.e. extra ones
         else
            v = ((logical_gpio >> (p - LOGIC_PORT)) & 1);       // Logical GPIO, e.g. NFC ports, etc.
         if ((1ULL << i) & input_invert)
            v = 1 - v;
         input_raw = ((input_raw & ~(1ULL << i)) | ((input_t) v << i));
      }
   input_stable = input_raw;
}

void input_start(void)
{
   int i;
   for (i = 0; i < MAXINPUT && !in[i]; i++);
   if (i == MAXINPUT)
      return;
   revk_task(TAG, task, NULL,4);
}
