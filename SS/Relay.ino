// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output

#include <ESP8266RevK.h>
#include "relay.h"
boolean relayfault = false;

#define app_settings  \
  s(relay);   \

#define s(n) const char *n=NULL
  app_settings
#undef s

  const char* relay_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean relay_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!relay)return false; // Relay not configured
    return false;
  }

  boolean relay_setup(ESP8266RevK&revk)
  {
    if (!relay)return false; // Relay not configured
#ifndef REVKDEBUG
    Serial.begin(9600);	// Serial relay control uses 9600
#endif
    return true;
  }

  boolean relay_loop(ESP8266RevK&revk, boolean force)
  {
    if (!relay)return false; // Relay not configured
    return true;
  }
