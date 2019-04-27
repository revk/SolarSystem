// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Input buttons

#include <ESP8266RevK.h>
#include "Input.h"

const char* input_setting(const char *tag, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return NULL; // Done
}

boolean input_commannd(const char*tag, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

boolean input_setup(ESP8266RevK&revk)
{
  return false;
}

boolean input_loop(ESP8266RevK&revk)
{
  return false;
}
