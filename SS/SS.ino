// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// This is the general purpose ESP8266 application for use with Solar System
// It includes inputs GPIO2 (and GPIO5 and GPIO4 on ESP-12F)
// It includes relay outputs (via serial for relay module up to 4 relays)
// It includes NFC reader (only on ESP-12F as uses SPI)
// It includes laser ranger as exit button using TX and GPIO0 for I2C)

#include <ESP8266RevK.h>

#ifdef ARDUINO_ESP8266_NODEMCU
// SPI devices only on ESP-12F
#include "Reader532.h"
#include "Reader532.h"
#endif
#include "Relay.h"
#include "Input.h"
#include "Ranger0X.h"
#include "Ranger1X.h"
#include "Keypad.h"

static boolean force = true;

ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__);

const char* app_setting(const char *tag, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM, return true if setting is OK
  const char *ret;
#ifdef	USE_READER522
  if ((ret = reader522_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_READER532
  if ((ret = reader532_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_RELAY
  if ((ret = relay_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_INPUT
  if ((ret = input_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_RANGER0X
  if ((ret = ranger0x_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_RANGER1X
  if ((ret = ranger1x_setting(tag, value, len)))return ret;
#endif
#ifdef  USE_KEYPAD
  if ((ret = keypad_setting(tag, value, len)))return ret;
#endif
  return false; // Failed
}

boolean app_command(const char*tag, const byte *message, size_t len)
{ // Called for incoming MQTT messages, return true if message is OK
  if (!strcasecmp_P(tag, PSTR("connect")))force = true;
#ifdef	USE_READER522
  if (reader522_command(tag, message, len))return true;
#endif
#ifdef	USE_READER532
  if (reader532_command(tag, message, len))return true;
#endif
#ifdef	USE_RELAY
  if (relay_command(tag, message, len))return true;
#endif
#ifdef	USE_INPUTS
  if (input_command(tag, message, len))return true;
#endif
#ifdef	USE_RANGER0X
  if (ranger0x_command(tag, message, len))return true;
#endif
#ifdef	USE_RANGER1X
  if (ranger1x_command(tag, message, len))return true;
#endif
#ifdef  USE_KEYPAD
  if (keypad_command(tag, message, len))return true;
#endif
  return false; // Failed
}

void setup()
{
#ifdef USE_READER522
  reader522_setup(revk);
#endif
#ifdef USE_READER532
  reader532_setup(revk);
#endif
#ifdef USE_RELAY
  relay_setup(revk);
#endif
#ifdef USE_INPUT
  input_setup(revk);
#endif
#ifdef USE_RANGER0X
  ranger0x_setup(revk);
#endif
#ifdef USE_RANGER1X
  ranger1x_setup(revk);
#endif
#ifdef USE_KEYPAD
  keypad_setup(revk);
#endif
}

void loop()
{
  revk.loop();
#ifdef USE_READER522
  reader522_loop(revk, force);
#endif
#ifdef USE_READER532
  reader532_loop(revk, force);
#endif
#ifdef USE_RELAY
  relay_loop(revk, force);
#endif
#ifdef USE_INPUT
  input_loop(revk, force);
#endif
#ifdef USE_RANGER0X
  ranger0x_loop(revk, force);
#endif
#ifdef USE_RANGER1X
  ranger1x_loop(revk, force);
#endif
#ifdef USE_KEYPAD
  keypad_loop(revk, force);
#endif
  force = false;
  delay(1);
}
