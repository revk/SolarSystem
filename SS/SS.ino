// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// This is the general purpose ESP8266 application for use with Solar System
// It includes inputs GPIO2 (and GPIO5 and GPIO4 on ESP-12F)
// It includes relay outputs (via serial for relay module up to 4 relays)
// It includes NFC reader (only on ESP-12F as uses SPI)
// It includes laser ranger as exit button using TX and GPIO0 for I2C)

#include <ESP8266RevK.h>

#ifdef ARDUINO_ESP8266_NODEMCU
#include "Reader532.h"		// SPI pins need the ESP-12F/S
#define INPUTS 3
#endif
#define RELAYS 4
#include "Relay.h"
#include "Input.h"
#include "Ranger.h"

ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__);

const char* app_setting(const char *tag, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM, return true if setting is OK
  const char *ret;
#ifdef	USE_READER532
  if ((ret = reader532_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_RELAY
  if ((ret = relay_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_INPUT
  if ((ret = input_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_RANGER
  if ((ret = ranger_setting(tag, value, len)))return ret;
#endif
  return false; // Failed
}

boolean app_command(const char*tag, const byte *message, size_t len)
{ // Called for incoming MQTT messages, return true if message is OK
#ifdef	USE_READER532
  if (reader532_command(tag, message, len))return true;
#endif
#ifdef	USE_RELAY
  if (relay_command(tag, message, len))return true;
#endif
#ifdef	USE_INPUTS
  if (input_command(tag, message, len))return true;
#endif
#ifdef	USE_RANGER
  if (ranger_command(tag, message, len))return true;
#endif
  return false; // Failed
}

void setup()
{
#ifdef USE_READER532
  reader532_setup(revk);
#endif
#ifdef USE_RELAY
  relay_setup(revk);
#endif
#ifdef USE_INPUT
  input_setup(revk);
#endif
#ifdef USE_RANGER
  ranger_setup(revk);
#endif
}

void loop()
{
  revk.loop();
#ifdef USE_READER532
  reader532_loop(revk);
#endif
#ifdef USE_RELAY
  relay_loop(revk);
#endif
#ifdef USE_INPUT
  input_loop(revk);
#endif
#ifdef USE_RANGER
  ranger_loop(revk);
#endif
}
