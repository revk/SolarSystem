// Logical outputs
static const char TAG[] = "output";
#include "SS.h"
const char *output_fault = NULL;
const char *output_tamper = NULL;

#include <driver/gpio.h>

// Output ports
#define MAXOUTPUT 26
#define BITFIELDS "-"
#define PORT_INV 0x40
#define port_mask(p) ((p)&63)
static uint8_t output[MAXOUTPUT];

static uint64_t output_invert = 0;

int
output_active (int p)
{
   // TODO
   return 0;
}

void
output_set (int p)
{
   // TODO
}

int
output_get (int p)
{
   // TODO
   return -1;
}

const char *
output_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strncmp (tag, TAG, sizeof (TAG) - 1))
   {                            // Set output
      int index = atoi (tag + sizeof (TAG) - 1);
      if (index >= 1 && index <= MAXOUTPUT)
      {
         // TODO change output
         return "";             // Done
      }
   }
   return NULL;
}

void
output_init (void)
{
   revk_register (TAG, MAXOUTPUT, sizeof (*output), &output, "-", SETTING_BITFIELD | SETTING_SET);
   int i,
     p;
   for (i = 0; i < MAXOUTPUT; i++)
      if (output[i])
      {
         const char *e = port_check (p = port_mask (output[i]), TAG, 0);
         if (e)
         {
            status(output_fault = e);
            output[i] = 0;
         } else
         {
            REVK_ERR_CHECK (gpio_reset_pin (p));
            if (output[i] & PORT_INV)
            {
               output_invert |= (1ULL << i);    // TODO can this not be done at hardware level?
               REVK_ERR_CHECK (gpio_set_level (p, 1));
            }
            REVK_ERR_CHECK (gpio_set_direction (p, GPIO_MODE_OUTPUT));
         }
      }
}
