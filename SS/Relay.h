// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output logic

#define  USE_RELAY
extern const char* relayfault;

#include <ESP8266RevK.h>

extern const char* relay_setting(const char *tag, const byte *value, size_t len);
extern boolean relay_command(const char*tag, const byte *message, size_t len);
extern boolean relay_setup(ESP8266RevK&);
extern boolean relay_loop(ESP8266RevK&, boolean);
