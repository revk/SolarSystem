// Logical outputs

#include "SS.h"

#define MAXOUTPUT 20
static uint8_t output[MAXOUTPUT];

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
   // TODO other settings
}
