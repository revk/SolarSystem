// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESP8266RevK.h>
#include "Ranger.h"

#define app_settings  \
  s(ranger);   \

#define s(n) const char *n=NULL
  app_settings
#undef s

  const char* ranger_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean ranger_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger)return false; // Ranger not configured
    return false;
  }

  boolean ranger_setup(ESP8266RevK&revk)
  {
    if (!ranger)return false; // Ranger not configured
    return true;
  }

  boolean ranger_loop(ESP8266RevK&revk)
  {
    if (!ranger)return false; // Ranger not configured
    return true;
  }
