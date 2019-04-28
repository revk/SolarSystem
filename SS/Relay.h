// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output logic

#ifndef	RELAYS
#define RELAYS 1
#endif

#define	USE_RELAY

#include <ESP8266RevK.h>

extern const char* relay_setting(const char *tag, const byte *value, size_t len);
extern boolean relay_command(const char*tag, const byte *message, size_t len);
extern boolean relay_setup(ESP8266RevK&);
extern boolean relay_loop(ESP8266RevK&, boolean);
