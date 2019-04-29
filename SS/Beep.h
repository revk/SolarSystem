// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Beep button logic

#define USE_INPUT
extern const char* beep_fault;

#include <ESP8266RevK.h>

extern const char * beep_setting(const char *tag, const byte *value, size_t len);
extern boolean beep_command(const char*tag, const byte *message, size_t len);
extern boolean beep_setup(ESP8266RevK&);
extern boolean beep_loop(ESP8266RevK&, boolean);
