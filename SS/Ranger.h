// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#define	USE_RANGER

#include <ESP8266RevK.h>

extern const char * ranger_setting(const char *tag, const byte *value, size_t len);
extern boolean ranger_command(const char*tag, const byte *message, size_t len);
extern boolean ranger_setup(ESP8266RevK&);
extern boolean ranger_loop(ESP8266RevK&);
