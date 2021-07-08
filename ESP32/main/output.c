// Logical outputs
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "output";
#include "SS.h"
const char *output_fault = NULL;
const char *output_tamper = NULL;

#include <driver/gpio.h>

// Output ports
#define MAXOUTPUT 10
#define BITFIELDS "-"
#define PORT_INV 0x40
#define port_mask(p) ((p)&63)
static uint8_t output[MAXOUTPUT];
static char *outputname[MAXOUTPUT];
static uint8_t power[MAXOUTPUT];        /* fixed outputs */
static char *powername[MAXOUTPUT];

#define i(x) s(x)
#define s(x) static area_t output##x[MAXOUTPUT];
#include "states.m"

static uint64_t output_state = 0;       // Port state
static uint64_t output_state_set = 0;   // Output has been set
static uint8_t output_changed = 0;

int output_active(int p)
{
   if (p < 1 || p > MAXOUTPUT)
      return -1;
   p--;
   if (output[p])
      return 1;
   return 0;
}

void output_set(int p, int v)
{
   if (p < 1 || p > MAXOUTPUT)
      return;
   p--;
   if (v)
   {
      v = 1;
      if ((output_state & (1ULL << p)) && (output_state_set & (1ULL << p)))
         return;                // No change
      output_state |= (1ULL << p);
      if (output[p])
         output_changed = 1;
   } else
   {
      if (!(output_state & (1ULL << p)) && (output_state_set & (1ULL << p)))
         return;                // No change
      output_state &= ~(1ULL << p);
      if (output[p])
         output_changed = 1;
   }
   if (output[p])
   {
      gpio_hold_dis(port_mask(output[p]));
      gpio_set_level(port_mask(output[p]), (output[p] & PORT_INV) ? 1 - v : v);
      gpio_hold_en(port_mask(output[p]));
   }
   output_state_set |= (1ULL << p);
}

int output_get(int p)
{
   if (p < 1 || p > MAXOUTPUT)
      return -1;
   p--;
   if (!(output_state_set & (1ULL << p)))
      return -1;
   if (output_state & (1ULL << p))
      return 1;
   return 0;
}

const char *output_command(const char *tag, jo_t j)
{
   if (!strcmp(tag, "connect"))
      output_changed = 1;       // Report
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
      {                         // Array expected}
         if (jo_here(j) != JO_ARRAY)
            e = "Expecting JSON array";
         jo_next(j);
         int i = 0;
         jo_type_t t = jo_here(j);
         while (t && i < MAXOUTPUT)
         {
            i++;
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
            t = jo_next(j);
         }
         if (!e && t)
            e = "Too many outputs";
      } else
      {                         // Single entry
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
{                               // Main RevK task
   esp_task_wdt_add(NULL);
   pvParameters = pvParameters;
   // Scan inputs
   while (1)
   {
      esp_task_wdt_reset();
      if (output_changed)
      {                         // JSON
         output_changed = 0;
         jo_t j = jo_create_alloc();
         jo_array(j, NULL);
         int t = MAXOUTPUT;
         while (t && (!output[t - 1] || !((output_state_set >> (t - 1)) & 1)))
            t--;
         for (int i = 0; i < t; i++)
            if (output[i] && ((output_state_set >> i) & 1))
               jo_bool(j, NULL, (output_state >> i) & 1);
            else
               jo_null(j, NULL);
         revk_state_copy(TAG, &j, iotstateoutput ? iot : NULL);
      }
      usleep(100000);
   }
}

void output_init(void)
{
   revk_register("output", MAXOUTPUT, sizeof(*output), &output, BITFIELDS, SETTING_BITFIELD | SETTING_SET | SETTING_SECRET);
   revk_register("outputgpio", MAXOUTPUT, sizeof(*output), &output, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
   revk_register("outputname", MAXOUTPUT, 0, &outputname, NULL, 0);
   revk_register("power", MAXOUTPUT, sizeof(*power), &power, BITFIELDS, SETTING_BITFIELD | SETTING_SET | SETTING_SECRET);
   revk_register("powergpio", MAXOUTPUT, sizeof(*power), &power, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
   revk_register("powername", MAXOUTPUT, 0, &powername, NULL, 0);
#define i(x) s(x)
#define s(x) revk_register("output"#x, MAXOUTPUT, sizeof(*output##x), &output##x, AREAS, SETTING_BITFIELD);
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
            {
               status(output_fault = e);
               output[i] = 0;
            } else
            {                   // Set up output pin
               c.pin_bit_mask |= (1ULL << p);
               REVK_ERR_CHECK(gpio_set_level(p, (output[i] & PORT_INV) ? 1 : 0));
               REVK_ERR_CHECK(gpio_hold_en(p));
            }
         }
         if (power[i])
         {
            const char *e = port_check(p = port_mask(power[i]), TAG, 0);
            if (e)
               status(output_fault = e);
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
   revk_task(TAG, task, NULL);
}
