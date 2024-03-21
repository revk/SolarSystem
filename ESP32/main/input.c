// Logical inputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "input";
#include "SS.h"
#include "input.h"
#include "output.h"
#include "alarm.h"

#include <driver/gpio.h>

static input_t input_raw = 0;
static input_t input_invert = 0;
input_t input_stable = 0;
input_t input_latch = 0;        // holds resettable state of input
input_t input_flip = 0;         // holds flipped flag for each input, i.e. state has changed
static uint8_t intime[MAXINPUT];        // Time active so far
static uint8_t inputfuncs;      // Combined input funcs
static uint32_t report_next = 0;

int
input_active (int p)
{                               // Port from 1
   if (p < 1 || p > MAXINPUT)
      return 0;
   p--;
   if (!ingpio[p].set)
      return 0;
   return 1;
}

int
input_get (int p)
{                               // Port from 1
   if (p < 1 || p > MAXINPUT)
      return -1;
   p--;
   if (input_stable & (1ULL << p))
      return 1;
   return 0;
}

int
input_func_active (uint8_t f)
{                               // Does a function exist at all (expected to be one bit set)
   if (inputfuncs & f)
      return 1;
   return 0;
}

int
input_func_all (uint8_t f)
{                               // Are all inputs for a function set (expected to be one bit set)
   for (int p = 0; p < MAXINPUT; p++)
      if ((infunc[p] & f) && !(input_stable & (1ULL << p)))
         return 0;
   return 1;
}

const char *
input_func_any (uint8_t f)
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

const char *
input_command (const char *tag, jo_t j)
{
   if (!strcmp (tag, "connect"))
      report_next = uptime () + 1;
   return NULL;
}

static void
task (void *pvParameters)
{                               // Input poll
   esp_task_wdt_add (NULL);
   pvParameters = pvParameters;
   int poll = (inpoll ? : 1);
   static uint8_t input_hold[MAXINPUT] = { 0 };
#ifdef  CONFIG_REVK_LED_STRIP
   for (int i = 0; i < MAXINPUT; i++)
      if (inrgb[i])
         led_set (inrgb[i], ((1ULL << i) & input_raw) ? 'G' : 'R');
#endif
   // Scan inputs
   while (1)
   {
      esp_task_wdt_reset ();
      // Check inputs
      input_t was = input_stable;
      for (int i = 0; i < MAXINPUT; i++)
         if (ingpio[i].set)
         {
            int p = ingpio[i].num,
               v;
            if (p < LOGIC_PORT)
               v = gpio_get_level (p);
            else if (p >= LOGIC_PORT2 && ingpio[i].invert)
               v = ((logical_gpio >> (16 + p - LOGIC_PORT)) & 1);       // Non invertable logical GPIO, i.e. extra ones
            else
               v = ((logical_gpio >> (p - LOGIC_PORT)) & 1);    // Logical GPIO, e.g. NFC ports, etc.
            if ((1ULL << i) & input_invert)
               v = 1 - v;
            if (v != ((input_raw >> i) & 1))
            {                   // Change of raw state
               input_raw = ((input_raw & ~(1ULL << i)) | ((input_t) v << i));
               input_hold[i] = (inhold[i] ? : 100) + poll + 1;  // Start countdown
               // We add poll as next step takes off poll
               // We add 1 so a poll size hold does expect two samples to be the same, less than poll will work on one sample
            }
            if (input_hold[i])
            {                   // counting down
               if (input_hold[i] < poll)
                  input_hold[i] = 0;
               else
                  input_hold[i] -= poll;
               if (!input_hold[i])      // hold done
               {
#ifdef  CONFIG_REVK_LED_STRIP
                  if (inrgb[i])
                     led_set (inrgb[i], v ? 'R' : 'G');
#endif
                  input_stable = ((input_stable & ~(1ULL << i)) | ((input_t) v << i));
                  if (v)
                  {
                     jo_t make (void)
                     {
                        jo_t e = jo_make (NULL);
                        jo_string (e, "reason", inname[i]);
                        return e;
                     }
                     if (indisarm[i] && (indisarm[i] & alarm_armed ()))
                     {          // Disarm
                        jo_t e = make ();
                        alarm_disarm (indisarm[i], &e);
                     } else
                     {          // Arm - i.e. allows for same button to do both
                        if (inarm[i])
                        {
                           jo_t e = make ();
                           alarm_arm (inarm[i], &e);
                        }
                        if (instrong[i])
                        {
                           jo_t e = make ();
                           alarm_strong (instrong[i], &e);
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
      uint32_t now = uptime ();
      if (was != input_stable || now > report_next)
      {                         // JSON
         report_next = now + 3600;
         input_latch |= (input_stable & ~was);  // Latch rising edges
         input_flip |= (input_stable ^ was);    // Latch any change
         jo_t j = jo_make (NULL);
         for (int i = 0; i < MAXINPUT; i++)
            if (ingpio[i].set && *inname[i])
               jo_bool (j, inname[i], (input_stable >> i) & 1);
         revk_state_clients ("input", &j, debug | (iotstateinput << 1));
      }
      // Sleep
      usleep (poll * 1000);
   }
}

void
input_boot (void)
{
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
         if (ingpio[i].set)
         {
            const char *e = port_check (p = ingpio[i].num, TAG, 1);
            if (e)
               ingpio[i].set = 0;
            else
            {
               if (p < LOGIC_PORT)
               {
#ifdef 	CONFIG_IDF_TARGET_ESP32
                  if (p >= 34)
                     I.pin_bit_mask |= (1ULL << p);     // Do not have pull up/down on ESP32
                  else
#endif
                  if (infunc[i] & INPUT_FUNC_P)
                     D.pin_bit_mask |= (1ULL << p);     // Pull down
                  else
                     U.pin_bit_mask |= (1ULL << p);     // Pull up
#ifdef 	CONFIG_IDF_TARGET_ESP32
                  if (p == 7 || p == 8)
                     gpio_reset_pin (p);
                  if (p != 20)
                     REVK_ERR_CHECK (gpio_hold_dis (p));
#endif
               }
               if (p < LOGIC_PORT2 && ingpio[i].invert)
               {                // Inverted
                  input_invert |= (1ULL << i);
                  if (p >= LOGIC_PORT)
                     logical_gpio |= (1ULL << (p - LOGIC_PORT));        // Init off
               }
            }
         }
      if (I.pin_bit_mask)
         REVK_ERR_CHECK (gpio_config (&I));
      if (U.pin_bit_mask)
         REVK_ERR_CHECK (gpio_config (&U));
      if (D.pin_bit_mask)
         REVK_ERR_CHECK (gpio_config (&D));
   }
   // Init state
   for (int i = 0; i < MAXINPUT; i++)
      if (ingpio[i].set)
      {
         int p = ingpio[i].num,
            v;
         if (p < LOGIC_PORT)
            v = gpio_get_level (p);
         else if (p >= LOGIC_PORT2 && ingpio[i].invert)
            v = ((logical_gpio >> (16 + p - LOGIC_PORT)) & 1);  // Non invertable logical GPIO, i.e. extra ones
         else
            v = ((logical_gpio >> (p - LOGIC_PORT)) & 1);       // Logical GPIO, e.g. NFC ports, etc.
         if ((1ULL << i) & input_invert)
            v = 1 - v;
         input_raw = ((input_raw & ~(1ULL << i)) | ((input_t) v << i));
      }
   input_stable = input_raw;
}

void
input_start (void)
{
   int i;
   for (i = 0; i < MAXINPUT && !ingpio[i].set; i++);
   if (i == MAXINPUT)
      return;
   revk_task (TAG, task, NULL, 3);
}
