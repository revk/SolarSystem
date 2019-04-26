// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output logic

#ifndef	RELAYS
#define RELAYS 1
#endif

#define	USE_RELAY

#include <ESP8266RevK.h>

extern boolean relay_setting(const char *setting, const byte *value, size_t len);
extern boolean relay_cmnd(const char*suffix, const byte *message, size_t len);
extern void relay_setup(ESP8266RevK&);
extern void relay_loop(ESP8266RevK&);
