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
unsigned int gpiomap = 0x1703F; // Pins available (ESP-12F)
#else
unsigned int gpiomap = 0xF; // Pins available (ESP-01)
#endif

#include "Relay.h"
#include "Ranger0X.h"
#include "Ranger1X.h"
#include "Keypad.h"
#include "Beep.h"
#include "Input.h"

static boolean force = true;

#define app_settings  \
  s(sda,0);   \
  s(scl,2);   \
  s(ss,16);   \
  s(rst,2); \
  s(beeper,0); \
  s(holdtime,3000); \
  s(releasetime,1000); \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__);

  const char* app_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM, return true if setting is OK
    const char *ret;
    revk.restart(3000); // Any setting change means restart
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):0);return t;}}while(0)
    app_settings
#undef s
#ifdef	USE_READER522
    if ((ret = reader522_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_READER532
    if ((ret = reader532_setting(tag, value, len)))return ret;
#endif
#ifdef	USE_RELAY
    if ((ret = relay_setting(tag, value, len)))return ret;
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
#ifdef  USE_BEEP
    if ((ret = beep_setting(tag, value, len)))return ret;
#endif
#ifdef  USE_INPUT
    if ((ret = input_setting(tag, value, len)))return ret;
#endif
    debug("Bad setting");
    revk.restart(-1); // cancel restart - unknown/invalid setting
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
#ifdef	USE_RANGER0X
    if (ranger0x_command(tag, message, len))return true;
#endif
#ifdef	USE_RANGER1X
    if (ranger1x_command(tag, message, len))return true;
#endif
#ifdef  USE_KEYPAD
    if (keypad_command(tag, message, len))return true;
#endif
#ifdef  USE_BEEP
    if (beep_command(tag, message, len))return true;
#endif
#ifdef  USE_INPUT
    if (input_command(tag, message, len))return true;
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
#ifdef USE_RANGER0X
    ranger0x_setup(revk);
#endif
#ifdef USE_RANGER1X
    ranger1x_setup(revk);
#endif
#ifdef USE_KEYPAD
    keypad_setup(revk);
#endif
    // Leave until last as needs to know which GPIO are available
#ifdef USE_BEEP
    beep_setup(revk);
#endif
#ifdef USE_INPUT
    input_setup(revk);
#endif
  }

  void loop()
  {
    revk.loop();
    { // Fault check
      const char*fault =
#ifdef USE_READER522
        reader522_fault ? :
#endif
#ifdef USE_READER532
        reader532_fault ? :
#endif
#ifdef USE_RELAY
        relay_fault ? :
#endif
#ifdef USE_RANGER0X
        ranger0x_fault ? :
#endif
#ifdef USE_RANGER1X
        ranger1x_fault ? :
#endif
#ifdef USE_KEYPAD
        keypad_fault ? :
#endif
#ifdef USE_BEEP
        beep_fault ? :
#endif
#ifdef USE_INPUT
        input_fault ? :
#endif
        NULL;
      static const char *lastfault = NULL;
      if (force || fault != lastfault)
      {
        lastfault = fault;
        if (fault)revk.state(F("fault"), F("1 %S"), fault);
        else revk.state(F("fault"), F("0"));
      }
    }
#ifdef USE_READER522
    reader522_loop(revk, force);
#endif
#ifdef USE_READER532
    reader532_loop(revk, force);
#endif
#ifdef USE_RELAY
    relay_loop(revk, force);
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
#ifdef USE_NEEP
    beep_loop(revk, force);
#endif
#ifdef USE_INPUT
    input_loop(revk, force);
#endif
    force = false;
    delay(1);
  }
