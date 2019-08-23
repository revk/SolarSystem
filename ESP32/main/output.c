// Logical outputs

#include "SS.h"
#include <driver/gpio.h>

// Output ports
#define MAXOUTPUT 26
#define BITFIELDS "-"
#define PORT_INV 0x40
#define port_mask(p) ((p)&63)
static uint8_t output[MAXOUTPUT];

static uint64_t output_invert = 0;

const char *
output_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strncmp (tag, "output", 6))
   {                            // Set output
      int index = atoi (tag + 6);
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
   revk_register ("output", MAXOUTPUT, sizeof (*output), &output, "-", SETTING_BITFIELD | SETTING_SET);
   int i,
     p;
   for (i = 0; i < MAXOUTPUT; i++)
      if (output[i] && port_ok (p = port_mask (output[i]), "output"))
      {
         REVK_ERR_CHECK (gpio_reset_pin (p));
         if (output[i] & PORT_INV)
	 {
            output_invert |= (1ULL << i);       // TODO can this not be done at hardware level?
	    REVK_ERR_CHECK(gpio_set_level(p,1));
	 }
         REVK_ERR_CHECK (gpio_set_direction (p, GPIO_MODE_OUTPUT));
      } else
         output[i] = 0;
}
