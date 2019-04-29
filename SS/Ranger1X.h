// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#define	USE_RANGER1X
extern const char* ranger1x_fault;

#include <ESP8266RevK.h>

extern const char * ranger1x_setting(const char *tag, const byte *value, size_t len);
extern boolean ranger1x_command(const char*tag, const byte *message, size_t len);
extern boolean ranger1x_setup(ESP8266RevK&);
extern boolean ranger1x_loop(ESP8266RevK&);
