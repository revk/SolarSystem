// Logical outputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "output";
#include "SS.h"
#include "output.h"
#include "input.h"

#include <driver/gpio.h>

static output_t output_state = 0;       // Port state
static output_t output_raw = 0; // Actual output
output_t output_forced = 0;     // Output forced externally
output_t output_pulsed = 0;     // Output pulse timed out meaning output is inverted from its logical state
output_t output_mask = 0;       // Configure outputs
static uint8_t outputfuncs;     // Combined outputs of all
static uint8_t outputfuncset;   // Logical state of output funcs
static uint32_t report_next = 0;        // When to report output

#ifdef	CONFIG_REVK_LED_STRIP
led_strip_handle_t rgb = NULL;
void
led_set (int led, char c)
{
   if (!rgb || led > rgbs)
      return;
   revk_led (rgb, led, 255, revk_rgb (c));
}
#endif

int
output_active (int p)
{
   if (p < 1 || p > MAXOUTPUT)
      return -1;
   p--;
   if (outgpio[p].set)
      return 1;
   return 0;
}

static void
output_write (int p)
{                               // Write current (combined) state (p from 0)
   if (outgpio[p].set)
   {
      output_t v = (((output_state | output_forced) ^ output_pulsed) >> p) & 1;
      output_raw = (output_raw & ~(1ULL << p)) | (v << p);
      gpio_hold_dis (outgpio[p].num);
      gpio_set_level (outgpio[p].num, outgpio[p].invert - v);
      gpio_hold_en (outgpio[p].num);
#ifdef  CONFIG_REVK_LED_STRIP
      if (outrgb[p])
         led_set (outrgb[p], v ? 'R' : 'G');
#endif
   }
}

void
output_set (int p, int v)
{
   if (p < 1 || p > MAXOUTPUT)
      return;
   p--;
   if (v)
      output_state |= (1ULL << p);
   else
      output_state &= ~(1ULL << p);
   output_write (p);
}

int
output_func_active (uint8_t f)
{                               // Does a function exist at all (expected to be one bit set)
   if (outputfuncs & f)
      return 1;
   return 0;
}

void
output_func_set (uint8_t f, int v)
{                               // Set all outputs for a function set (expected to be one bit set)
   if (v)
      outputfuncset |= f;
   else
      outputfuncset &= ~f;
   for (int p = 0; p < MAXOUTPUT; p++)
      if (outfunc[p] & f)
         output_set (p + 1, v); // Yes, output_set is using p starting at 1, this really needs fixing some time as so annoying.
}

int
output_func_get (uint8_t f)
{
   return (outputfuncset & f) ? 1 : 0;
}

int
output_get (int p)
{
   if (p < 1 || p > MAXOUTPUT)
      return -1;
   p--;
   if (((output_state | output_forced) ^ output_pulsed) & (1ULL << p))
      return 1;
   return 0;
}

const char *
output_command (const char *tag, jo_t j)
{
   if (!strcmp (tag, "connect"))
      report_next = uptime () + 1;      // Report
   const char *e = NULL;
   if (!strncmp (tag, TAG, strlen (TAG)))
   {                            // Set output
      if (!e)
         jo_skip (j);
      if (!e)
         e = jo_error (j, NULL);
      if (e)
         return e;
      int i = atoi (tag + strlen (TAG));
      if (!i)
      {                         // Object expected}
         if (jo_here (j) != JO_OBJECT)
            e = "Expecting JSON object";
         while (jo_here (j))
         {
            jo_next (j);
            if (jo_here (j) == JO_TAG)
            {
               int i = 0;
               for (i = 0; i < MAXOUTPUT && jo_strcmp (j, outname[i]); i++);
               if (i == MAXOUTPUT)
                  e = "Unknown output";
               else
               {
                  jo_type_t t = jo_next (j);
                  if (t >= JO_TRUE)
                  {
                     if (!outgpio[i - 1].set)
                        e = "Trying to set unconfigured output";
                     else if (t == JO_TRUE)
                        output_set (i, 1);
                     else if (t == JO_FALSE)
                        output_set (i, 0);
                  } else if (t != JO_NULL)
                     e = "Expecting boolean or null entries";
               }
            }
         }
      } else
      {                         // Single entry outputN
         jo_type_t t = jo_here (j);
         if (i > MAXOUTPUT)
            e = "Output too high";
         else if (!outgpio[i - 1].set)
            e = "Output not active";
         else if (t == JO_TRUE)
            output_set (i, 1);
         else if (t == JO_FALSE)
            output_set (i, 0);
         else
            e = "Expecting boolean";
      }
      if (!e)
         e = "";
   }
   return e;
}

static void
task (void *pvParameters)
{                               // Output poll
   esp_task_wdt_add (NULL);
   pvParameters = pvParameters;
   static output_t output_last = 0;     // Last reported
   static output_t output_last_pulsed = 0;      // Last reported
   static uint16_t output_hold[MAXOUTPUT] = { 0 };
   // Set outputs to their current state
   for (int i = 0; i < MAXOUTPUT; i++)
      if (outgpio[i].set)
      {
         output_mask |= (1ULL << i);
         output_write (i);
      }
   // Scan outputs
   while (1)
   {
      esp_task_wdt_reset ();
      uint32_t now = uptime ();
      output_t output_mix = ((output_state | output_forced) & output_mask);
      for (int i = 0; i < MAXOUTPUT; i++)
         if ((output_mix & (1ULL << i)) ? outpulse[i] < 0 : outpulse[i] > 0)
         {                      // Cancel pulsed output
            output_hold[i] = 0;
            output_pulsed &= ~(1ULL << i);
         } else if (output_hold[i] && !--output_hold[i])
            output_pulsed |= (1ULL << i);       // End of pulse time
      output_mix ^= output_pulsed;
      if (output_mix != output_raw)
         for (int i = 0; i < MAXOUTPUT; i++)
            if ((output_mix ^ output_raw) & (1ULL << i))
            {                   // State has changed
               if ((output_mix & (1ULL << i)) ? outpulse[i] > 0 : outpulse[i] < 0)
                  output_hold[i] = (outpulse[i] > 0 ? outpulse[i] : -outpulse[i]);      // Start of pulse time
               output_write (i);        // Update output state
            }
      if (output_mix != output_last || output_pulsed != output_last_pulsed || now > report_next)
      {
         output_last = output_mix;
         output_last_pulsed = output_pulsed;
         report_next = now + 3600;
         jo_t j = jo_make (NULL);
         for (int i = 0; i < MAXOUTPUT; i++)
            if (outgpio[i].set && *outname[i])
            {
               if (output_pulsed & (1ULL << i))
                  jo_null (j, outname[i]);      // Distinct state for output pulse timeout
               else
                  jo_bool (j, outname[i], (output_mix >> i) & 1);
            }
         revk_state_clients ("output", &j, debug | (iotstateoutput << 1));
      }
      usleep (100000);          // 100 ms (timers assume this)
#ifndef CONFIG_REVK_BLINK_LIB
      if (blink[0].set)
      {
         uint32_t colour = revk_blinker ();
         if (!blink[1].set)
            revk_gpio_set (blink[0], (colour >> 31) & 1);
         else if (blink[0].num != blink[1].num)
         {                      // Separate RGB on
            revk_gpio_set (blink[0], (colour >> 29) & 1);
            revk_gpio_set (blink[1], (colour >> 27) & 1);
            revk_gpio_set (blink[2], (colour >> 25) & 1);
         }
#ifdef  CONFIG_REVK_LED_STRIP
         else if(rgb)
         {
            revk_led (rgb, 0, 255, colour);
            led_strip_refresh (rgb);
         }
#endif
      }
#endif
   }
}

void
output_boot (void)
{
   outputfuncset = 0;
   outputfuncs = 0;
   for (int i = 0; i < MAXOUTPUT; i++)
      outputfuncs |= outfunc[i];
   {                            // GPIO
    gpio_config_t c = { mode:GPIO_MODE_OUTPUT };
      int i,
        p;
      for (i = 0; i < MAXOUTPUT; i++)
      {
         if (outgpio[i].set)
         {
            const char *e = port_check (p = outgpio[i].num, TAG, 0);
            if (e)
               outgpio[i].set = 0;
            else
            {                   // Set up output pin
               c.pin_bit_mask |= (1ULL << p);
               REVK_ERR_CHECK (gpio_set_level (p, outgpio[i].invert));
            }
         }
         if (powergpio[i].set)
         {
            const char *e = port_check (p = powergpio[i].num, TAG, 0);
            if (e)
               powergpio[i].set = 0;
            else
            {                   // Set up power output pin
               c.pin_bit_mask |= (1ULL << p);
               if (p != 20)
                  REVK_ERR_CHECK (gpio_hold_dis (p));
               REVK_ERR_CHECK (gpio_set_level (p, 1 - powergpio[i].invert));
               REVK_ERR_CHECK (gpio_set_drive_capability (p, GPIO_DRIVE_CAP_3));
            }
         }
      }
      if (c.pin_bit_mask)
         REVK_ERR_CHECK (gpio_config (&c));
   }
#ifdef  CONFIG_REVK_LED_STRIP
#ifndef CONFIG_REVK_BLINK_LIB
   if (blink[0].set && blink[1].set && blink[0].num == blink[1].num)
   {
      led_strip_config_t strip_config = {
         .strip_gpio_num = blink[0].num,
         .max_leds = rgbs + 1,  // The number of LEDs in the strip,
         .led_pixel_format = LED_PIXEL_FORMAT_GRB,      // Pixel format of your LED strip
         .led_model = LED_MODEL_WS2812, // LED strip model
         .flags.invert_out = blink[0].invert,
      };
      led_strip_rmt_config_t rmt_config = {
         .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
         .resolution_hz = 10 * 1000 * 1000,     // 10MHz
#ifdef  CONFIG_IDF_TARGET_ESP32S3
         .flags.with_dma = true,
#endif
      };
      REVK_ERR_CHECK (led_strip_new_rmt_device (&strip_config, &rmt_config, &rgb));
      for (int i = 0; i < MAXOUTPUT; i++)
         if (powerrgb[i])
            led_set (powerrgb[i], powergpio[i].invert ? 'B' : 'M');
   } else
      for (int b = 0; b < 3; b++)
         revk_gpio_output (blink[b],0);
#endif
#endif
}

void
output_start (void)
{
#ifdef CONFIG_REVK_BLINK_LIB    // If not defined we need output task to blink the status LED, so run task anyway
   int i;
   for (i = 0; i < MAXOUTPUT && !outgpio[i].set; i++);
   if (i == MAXOUTPUT)
      return;
#endif
   revk_task (TAG, task, NULL, 3);
}
