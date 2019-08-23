// Solar System ESP32 app

#include "SS.h"
#include <driver/gpio.h>

// Common
static const char *port_inuse[MAX_PORT];

// External
int
port_ok (int p, const char *module)
{                               // Check port is OK
   if (p < 0 || p >= MAX_PORT || !GPIO_IS_VALID_GPIO (p))
   {
      revk_error ("port", "Port %d is not valid", p);
      return 0;
   }
   if (port_inuse[p])
   {
      revk_error ("port", "Port %d is already in use by %s so cannot be used by %s", p, port_inuse[p], module);
      return 0;
   }
   port_inuse[p] = module;
   return 1;
}

#define modules	\
	m(input)	\
	m(output)	\
	m(nfc)		\
	m(ranger)	\
	m(keypad)	\
	m(door)		\

const char *
app_command (const char *tag, unsigned int len, const unsigned char *value)
{
   const char *e = NULL;
#define m(x) extern const char * x##_command(const char *tag,unsigned int len,const unsigned char *value); if(!e)e=x##_command(tag,len,value);
   modules
#undef m
      return "";                // Unknown
}

void
app_main ()
{
   revk_init (&app_command);
   int p;
   for (p = 6; p <= 11; p++)
      port_ok (p, "Flash");     // Flash chip uses 6-11
#define m(x) extern void x##_init(void); x##_init();
   modules
#undef m
}
