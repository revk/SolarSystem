// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Input button logic

#ifndef	INPUTS
#define INPUTS 1
#endif

#define USE_INPUT

#include <ESP8266RevK.h>

extern boolean input_setting(const char *setting, const byte *value, size_t len);
extern boolean input_cmnd(const char*suffix, const byte *message, size_t len);
extern void input_setup(ESP8266RevK&);
extern void input_loop(ESP8266RevK&);
