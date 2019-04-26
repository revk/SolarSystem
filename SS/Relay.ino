// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output

#include <ESP8266RevK.h>
#include "relay.h"

boolean relay_setting(const char *setting, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return false; // Done
}

boolean relay_cmnd(const char*suffix, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

void relay_setup(ESP8266RevK&revk)
{
#ifndef REVKDEBUG
  Serial.begin(9600);	// Serial relay control uses 9600
#endif
}

void relay_loop(ESP8266RevK&revk)
{
}
