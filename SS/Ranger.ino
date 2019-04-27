// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESP8266RevK.h>
#include "Ranger.h"

const char* ranger_setting(const char *tag, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return NULL; // Done
}

boolean ranger_command(const char*tag, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

boolean ranger_setup(ESP8266RevK&revk)
{
  return false;
}

boolean ranger_loop(ESP8266RevK&revk)
{
  return false;
}
