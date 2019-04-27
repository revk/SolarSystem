// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Input buttons

#include <ESP8266RevK.h>
#include "Input.h"

#define app_settings  \
  s(input);   \

#define s(n) const char *n=NULL
  app_settings
#undef s

  const char* input_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean input_commannd(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!input)return false; // No inputs defined
    return false;
  }

  boolean input_setup(ESP8266RevK&revk)
  {
    if (!input)return false; // No inputs defined
    // TODO
    return true;
  }

  boolean input_loop(ESP8266RevK&revk)
  {
    if (!input)return false; // No inputs defined
    // TODO
    return true;
  }
