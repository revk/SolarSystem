// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output

#include <ESP8266RevK.h>
#include "relay.h"

const char* relay_setting(const char *tag, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return NULL; // Done
}

boolean relay_command(const char*tag, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

boolean relay_setup(ESP8266RevK&revk)
{
#ifndef REVKDEBUG
  Serial.begin(9600);	// Serial relay control uses 9600
#endif
  return false;
}

boolean relay_loop(ESP8266RevK&revk)
{
  return false;
}
