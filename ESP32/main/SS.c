// Solar System ESP32 app

#include "../components/ESP32RevK/revk.h"

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
#define m(x) extern void x##_init(void); x##_init();
   modules
#undef m
}
