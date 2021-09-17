// Logical outputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "output";
#include "SS.h"
#include "output.h"

#include <driver/gpio.h>

// Output ports
#define BITFIELDS "-"
#define PORT_INV 0x40
#define port_mask(p) ((p)&63)
static uint8_t output[MAXOUTPUT];
static uint8_t power[MAXOUTPUT];        /* fixed outputs */
static char *outputname[MAXOUTPUT];
static uint16_t outputpulse[MAXOUTPUT]; // Timeout in s/10

#define i(x,c) area_t output##x[MAXOUTPUT];
#define s(x,c) i(x,c)
#include "states.m"

static output_t output_state = 0;       // Port state
static output_t output_raw = 0; // Actual output
output_t output_forced = 0;     // Output forced externally
output_t output_pulsed = 0;     // Output pulse timed out
output_t output_mask = 0;       // Configure outputs
static uint32_t report_next = 0;        // When to report output

int output_active(int p)
{
   if (p < 1 || p > MAXOUTPUT)
      return -1;
   p--;
   if (output[p])
      return 1;
   return 0;
}

static void output_write(int p)
{                               // Write current (combined) state (p from 0)
   if (output[p])
   {
      output_t v = (((output_state | output_forced) & ~output_pulsed) >> p) & 1;
      output_raw = (output_raw & ~(1ULL << p)) | (v << p);
      gpio_hold_dis(port_mask(output[p]));
      gpio_set_level(port_mask(output[p]), (output[p] & PORT_INV) ? 1 - v : v);
      gpio_hold_en(port_mask(output[p]));
   }
}

void output_set(int p, int v)
{
   if (p < 1 || p > MAXOUTPUT)
      return;
   p--;
   if (v)
      output_state |= (1ULL << p);
   else
      output_state &= ~(1ULL << p);
   output_write(p);
}

int output_get(int p)
{
   if (p < 1 || p > MAXOUTPUT)
      return -1;
   p--;
   if (((output_state | output_forced) & ~output_pulsed) & (1ULL << p))
      return 1;
   return 0;
}

const char *output_command(const char *tag, jo_t j)
{
   if (!strcmp(tag, "connect"))
      report_next = uptime() + 1;       // Report
   const char *e = NULL;
   if (!strncmp(tag, TAG, strlen(TAG)))
   {                            // Set output
      if (!e)
         jo_skip(j);
      if (!e)
         e = jo_error(j, NULL);
      if (e)
         return e;
      int i = atoi(tag + strlen(TAG));
      if (!i)
      {                         // Object expected}
         if (jo_here(j) != JO_OBJECT)
            e = "Expecting JSON object";
         while (jo_here(j))
         {
            jo_next(j);
            if (jo_here(j) == JO_TAG)
            {
               int i = 0;
               for (i = 0; i < MAXOUTPUT && jo_strcmp(j, outputname[i]); i++);
               if (i == MAXOUTPUT)
                  e = "Unknown output";
               else
               {
                  jo_type_t t = jo_next(j);
                  if (t >= JO_TRUE)
                  {
                     if (!output[i - 1])
                        e = "Trying to set unconfigured output";
                     else if (t == JO_TRUE)
                        output_set(i, 1);
                     else if (t == JO_FALSE)
                        output_set(i, 0);
                  } else if (t != JO_NULL)
                     e = "Expecting boolean or null entries";
               }
            }
         }
      } else
      {                         // Single entry outputN
         jo_type_t t = jo_here(j);
         if (i > MAXOUTPUT)
            e = "Output too high";
         else if (!output[i - 1])
            e = "Output not active";
         else if (t == JO_TRUE)
            output_set(i, 1);
         else if (t == JO_FALSE)
            output_set(i, 0);
         else
            e = "Expecting boolean";
      }
      if (!e)
         e = "";
   }
   return e;
}

static void task(void *pvParameters)
{                               // Output poll
   pvParameters = pvParameters;
   esp_task_wdt_add(NULL);
   static output_t output_last = 0;     // Last reported
   static output_t output_last_pulsed = 0;      // Last reported
   static uint16_t output_hold[MAXOUTPUT] = { };
   // Set outputs to their current state
   for (int i = 0; i < MAXOUTPUT; i++)
      if (output[i])
      {
         output_mask |= (1ULL << i);
         output_write(i);
      }
   // Scan outputs
   while (1)
   {
      esp_task_wdt_reset();
      uint32_t now = uptime();
      output_t output_mix = ((output_state | output_forced) & output_mask);
      for (int i = 0; i < MAXOUTPUT; i++)
         if (!(output_mix & (1ULL << i)))
         {
            output_hold[i] = 0;
            output_pulsed &= ~(1ULL << i);
         } else if (output_hold[i] && !--output_hold[i])
            output_pulsed |= (1ULL << i);
      output_mix &= ~output_pulsed;
      if (output_mix != output_raw)
         for (int i = 0; i < MAXOUTPUT; i++)
            if ((output_mix ^ output_raw) & (1ULL << i))
            {
               if (output_mix & (1ULL << i))
                  output_hold[i] = outputpulse[i];
               output_write(i); // Update output state
            }
      if (output_mix != output_last || output_pulsed != output_last_pulsed || now > report_next)
      {
         output_last = output_mix;
         output_last_pulsed = output_pulsed;
         report_next = now + 3600;
         jo_t j = jo_make(NULL);
         int t = MAXOUTPUT;
         while (t && !output[t - 1])
            t--;
         for (int i = 0; i < t; i++)
            if (output[i] && *outputname[i])
            {
               if (output_pulsed & (1ULL << i))
                  jo_null(j, outputname[i]);    // Distinct state for false but should be true
               else
                  jo_bool(j, outputname[i], (output_mix >> i) & 1);
            }
         revk_state_clients("output", &j, debug | (iotstateoutput << 1));
      }
      usleep(100000);           // 100 ms (timers assume this)
   }
}

void output_boot(void)
{
   revk_register("output", MAXOUTPUT, sizeof(*output), &output, BITFIELDS, SETTING_BITFIELD | SETTING_SET | SETTING_SECRET);
   revk_register("outputgpio", MAXOUTPUT, sizeof(*output), &output, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
   revk_register("outputname", MAXOUTPUT, 0, &outputname, NULL, 0);
   revk_register("outputpulse", MAXOUTPUT, sizeof(*outputpulse), &outputpulse, NULL, 0);
   revk_register("power", MAXOUTPUT, sizeof(*power), &power, BITFIELDS, SETTING_BITFIELD | SETTING_SET | SETTING_SECRET);
   revk_register("powergpio", MAXOUTPUT, sizeof(*power), &power, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
#define i(x,c) revk_register("output"#x, MAXOUTPUT, sizeof(*output##x), &output##x, AREAS, SETTING_BITFIELD);
#define s(x,c) i(x,c)
#include "states.m"
   {                            // GPIO
    gpio_config_t c = { mode:GPIO_MODE_OUTPUT };
      int i,
       p;
      for (i = 0; i < MAXOUTPUT; i++)
      {
         if (output[i])
         {
            const char *e = port_check(p = port_mask(output[i]), TAG, 0);
            if (e)
               output[i] = 0;
            else
            {                   // Set up output pin
               c.pin_bit_mask |= (1ULL << p);
               REVK_ERR_CHECK(gpio_set_level(p, (output[i] & PORT_INV) ? 1 : 0));
            }
         }
         if (power[i])
         {
            const char *e = port_check(p = port_mask(power[i]), TAG, 0);
            if (e)
               power[i] = 0;
            else
            {                   // Set up power output pin
               c.pin_bit_mask |= (1ULL << p);
               REVK_ERR_CHECK(gpio_hold_dis(p));
               REVK_ERR_CHECK(gpio_set_level(p, (power[i] & PORT_INV) ? 0 : 1));
               REVK_ERR_CHECK(gpio_set_drive_capability(p, GPIO_DRIVE_CAP_3));
            }
         }
      }
      if (c.pin_bit_mask)
         REVK_ERR_CHECK(gpio_config(&c));
   }
}

void output_start(void)
{
   int i;
   for (i = 0; i < MAXOUTPUT && !output[i]; i++);
   if (i == MAXOUTPUT)
      return;
   revk_task(TAG, task, NULL);
}
