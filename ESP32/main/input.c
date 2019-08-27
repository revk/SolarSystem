// Logical inputs
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "input";
#include "SS.h"
const char *input_fault = NULL;
const char *input_tamper = NULL;

#include <driver/gpio.h>

// Input ports
#define MAXINPUT 26
#define	BITFIELDS "-"
#define	PORT_INV 0x40
#define	port_mask(p) ((p)&63)
static uint8_t input[MAXINPUT];

// Other settings
#define settings	\
u16 (inputhold, 100);	\
u16 (inputpoll, 10);	\

#define u16(n,v) uint16_t n
settings
#undef u16
static uint64_t input_raw = 0;
static uint64_t input_stable = 0;
static uint64_t input_invert = 0;
static uint64_t input_hold[MAXINPUT] = { };

static volatile char reportall = 0;

int
input_active (int p)
{
   if (p < 1 || p > MAXINPUT)
      return 0;
   p--;
   if (!input[p])
      return 0;
   return 1;
}

int
input_get (int p)
{
   if (p < 1 || p > MAXINPUT)
      return -1;
   p--;
   if (!input[p])
      return 0;
   if (input_raw & (1ULL << p))
      return 1;
   return 0;
}

const char *
input_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strcmp (tag, "connect"))
      reportall = 1;
   return NULL;
}

static void
task (void *pvParameters)
{                               // Main RevK task
   pvParameters = pvParameters;
   // Scan inputs
   while (1)
   {
      // Check inputs
      int64_t now = esp_timer_get_time ();
      char report = reportall;
      reportall = 0;
      int i;
      for (i = 0; i < MAXINPUT; i++)
         if (input[i])
         {
            int v = gpio_get_level (port_mask (input[i]));
            if ((1ULL << i) & input_invert)
               v = 1 - v;
            char changed = report;
            if ((input_hold[i] < now) && (report || v != ((input_stable >> i) & 1)))
            {                   // Change of stable state
               input_stable = ((input_stable & ~(1ULL << i)) | ((uint64_t) v << i));
               changed = 1;
            }
            if (changed)
            {
               char tag[20];
               sprintf (tag, "%s%d", TAG, i + 1);
               revk_state (tag, "%d", (input_stable >> i) & 1);
            }
            if (v != ((input_raw >> i) & 1))
            {                   // Change of raw state
               input_raw = ((input_raw & ~(1ULL << i)) | ((uint64_t) v << i));
               input_hold[i] = now + inputhold * 1000;
            }
         }
      // Sleep
      usleep ((inputpoll ? : 1) * 1000);
   }
}

void
input_init (void)
{
   revk_register (TAG, MAXINPUT, sizeof (*input), &input, BITFIELDS, SETTING_BITFIELD | SETTING_SET);
#define u16(n,v) revk_register(#n,0,sizeof(n),&n,#v,0);
   settings
#undef u16
      // Check config
   int i,
     p;
   for (i = 0; i < MAXINPUT; i++)
      if (input[i])
      {
         const char *e = port_check (p = port_mask (input[i]), TAG, 1);
         if (e)
         {
            input[i] = 0;
            status (input_fault = e);
         } else
         {
            REVK_ERR_CHECK (gpio_reset_pin (p));
            REVK_ERR_CHECK (gpio_set_pull_mode (p, GPIO_PULLUP_ONLY));
            REVK_ERR_CHECK (gpio_set_direction (p, GPIO_MODE_INPUT));
            if (input[i] & PORT_INV)
               input_invert |= (1ULL << i);     // TODO can this not be done at hardware level?
         }
      }
   static TaskHandle_t task_id = NULL;
   xTaskCreatePinnedToCore (task, TAG, 16 * 1024, NULL, 1, &task_id, tskNO_AFFINITY);   // TODO stack, priority, affinity check?
}
