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
static uint8_t input[MAXINPUT];
static uint8_t inputhold[MAXINPUT];
#define i(t,x,c) area_t input##x[MAXINPUT];
#define c(t,x) area_t input##x[MAXINPUT];
#include "states.m"
char *inputname[MAXINPUT];

// Other settings
#define settings	\
u8 (inputpoll, 10);	\

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
{
   if (p < 1 || p > MAXINPUT)
      return 0;
   p--;
   if (!input[p])
      return 0;
   return 1;
}

void input_set(int p, int v)
{                               // For locally set inputs

   if (p < 1 || p > MAXINPUT)
      return;
   p--;
   if (v)
      input_raw |= (1ULL << p);
   else
      input_raw &= ~(1ULL << p);
}

int input_get(int p)
{
   if (p < 1 || p > MAXINPUT)
      return -1;
   p--;
   if (input_raw & (1ULL << p))
      return 1;
   return 0;
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
   int poll = (inputpoll ? : 1);
   static uint8_t input_hold[MAXINPUT] = { };
   // Init state
   for (int i = 0; i < MAXINPUT; i++)
      if (input[i])
      {
         int p = port_mask(input[i]),
             v;
         if (p < LOGIC_PORT)
            v = gpio_get_level(port_mask(input[i]));
         else if (p >= LOGIC_PORT2 && (input[i] & PORT_INV))
            v = ((logical_gpio >> (16 + p - LOGIC_PORT)) & 1);  // Non invertable logical GPIO, i.e. extra ones
         else
            v = ((logical_gpio >> (p - LOGIC_PORT)) & 1);       // Logical GPIO, e.g. NFC ports, etc.
         if ((1ULL << i) & input_invert)
            v = 1 - v;
         input_raw = ((input_raw & ~(1ULL << i)) | ((input_t) v << i));
      }
   input_stable = input_raw;
   // Scan inputs
   while (1)
   {
      esp_task_wdt_reset();
      // Check inputs
      input_t was = input_stable;
      for (int i = 0; i < MAXINPUT; i++)
         if (input[i])
         {
            int p = port_mask(input[i]),
                v;
            if (p >= LOGIC_PORT)
               v = ((logical_gpio >> (p - LOGIC_PORT)) & 1);    // Logical GPIO, e.g. NFC ports, etc.
            else
               v = gpio_get_level(port_mask(input[i]));
            if ((1ULL << i) & input_invert)
               v = 1 - v;
            if (v != ((input_raw >> i) & 1))
            {                   // Change of raw state
               input_raw = ((input_raw & ~(1ULL << i)) | ((input_t) v << i));
               input_hold[i] = (inputhold[i] ? : 100);  // Start countdown
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
                        jo_string(e, "reason", "input");
                        jo_string(e, "input", inputname[i]);
                        return e;
                     }
                     if (inputarm[i])
                     {
                        jo_t e = make();
                        alarm_arm(inputarm[i], &e);
                     }
                     if (inputstrong[i])
                     {
                        jo_t e = make();
                        alarm_strong(inputstrong[i], &e);
                     }
                     if (inputdisarm[i])
                     {
                        jo_t e = make();
                        alarm_disarm(inputdisarm[i], &e);
                     }
                  }
               }
            }
         }
      uint32_t now = uptime();
      if (was != input_stable || now > report_next)
      {                         // JSON
         report_next = now + 3600;
         input_latch |= (input_stable & ~was);  // Latch rising edges
         input_flip |= (input_stable ^ was);    // Latch any change
         jo_t j = jo_make(NULL);
         int t = MAXINPUT;
         while (t && !input[t - 1])
            t--;
         for (int i = 0; i < t; i++)
            if (*inputname[i])
               jo_bool(j, inputname[i], (input_stable >> i) & 1);
         revk_state_clients("input", &j, debug | (iotstateinput << 1));
      }
      // Sleep
      usleep(poll * 1000);
   }
}

void input_boot(void)
{
   revk_register("input", MAXINPUT, sizeof(*input), &input, BITFIELDS, SETTING_BITFIELD | SETTING_SET | SETTING_SECRET);
   revk_register("inputgpio", MAXINPUT, sizeof(*input), &input, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
   revk_register("inputhold", MAXINPUT, sizeof(*inputhold), &inputhold, NULL, SETTING_LIVE);
   revk_register("inputname", MAXINPUT, 0, &inputname, NULL, SETTING_LIVE);
#define i(t,x,c) revk_register("input"#x, MAXINPUT, sizeof(*input##x), &input##x, AREAS, SETTING_BITFIELD|SETTING_LIVE);
#define c(t,x) revk_register("input"#x, MAXINPUT, sizeof(*input##x), &input##x, AREAS, SETTING_BITFIELD|SETTING_LIVE);
#include "states.m"
#define u8(n,v) revk_register(#n,0,sizeof(n),&n,#v,0);
   settings
#undef u8
   {                            // GPIO
    gpio_config_t c = { mode: GPIO_MODE_INPUT, pull_up_en:GPIO_PULLUP_ENABLE };
      int i,
       p;
      for (i = 0; i < MAXINPUT; i++)
         if (input[i])
         {
            const char *e = port_check(p = port_mask(input[i]), TAG, 1);
            if (e)
               input[i] = 0;
            else
            {
               if (p < MAX_PORT)
               {
                  c.pin_bit_mask |= (1ULL << p);
                  if (p == 7 || p == 8)
                     gpio_reset_pin(p);
                  if (p != 20)
                     REVK_ERR_CHECK(gpio_hold_dis(p));
               }
               if (p < LOGIC_PORT2 && (input[i] & PORT_INV))
               {                // Inverted
                  input_invert |= (1ULL << i);
                  if (p >= LOGIC_PORT)
                     logical_gpio |= (1ULL << (p - LOGIC_PORT));        // Init off
               }
            }
         }
      if (c.pin_bit_mask)
         REVK_ERR_CHECK(gpio_config(&c));
   }
}

void input_start(void)
{
   int i;
   for (i = 0; i < MAXINPUT && !input[i]; i++);
   if (i == MAXINPUT)
      return;
   revk_task(TAG, task, NULL);
}
