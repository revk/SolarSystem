// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// This is the general purpose ESP application for use with Solar System
// Configurable inputs
// Configurable outputs
// Serial relay control outputs
// I2C Range finder inputs for motion and touch free input (e.g. exit button)
// PN532 card readers on SPI


#include <ESPRevK.h>

#ifdef ARDUINO_ESP8266_NODEMCU
// SPI devices only on ESP-12F
#include "nfc.h"
unsigned int gpiomap = 0x1703F; // Pins available (ESP-12F)
#else
unsigned int gpiomap = 0xF; // Pins available (ESP-01)
#endif

#include "Ranger0X.h"
#include "Ranger1X.h"
#include "Keypad.h"
#include "Input.h"
#include "Output.h"

static boolean force = true;
boolean insafemode = false;
unsigned safemodestart = 0;

#define app_settings  \
  s(sda,0);   \
  s(scl,2);   \
  s(ss,16);   \
  s(beeper,0); \
  s(holdtime,3000); \
  s(safemode,60); \
  t(fallback); \
  s(rangerdebug,0); \
  s(rangerpoll,100); \
  s(rangerhold,1000); \
  s(rangermargin,50); \
  s(readerpoll,10); \

#define s(n,d) unsigned int n=d;
#define t(n) const char*n=NULL;
  app_settings
#undef s
#undef t

  ESPRevK revk(__FILE__, __DATE__ " " __TIME__);

  const char* app_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM, return true if setting is OK
    const char *ret;
    revk.restart(3000); // Any setting change means restart
#define s(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
#define t(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char*)value;return t;}}while(0)
    app_settings
#undef s
#undef t
#ifdef	USE_NFC
    if ((ret = nfc_setting(tag, value, len)))return ret;
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
#ifdef  USE_OUTPUT
    if ((ret = output_setting(tag, value, len)))return ret;
#endif
    debug("Bad setting");
    revk.restart(-1); // cancel restart - unknown/invalid setting
    return false; // Failed
  }

  boolean app_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages, return true if message is OK
    if (!strcasecmp_P(tag, PSTR("connect"))) {
      force = true;
      insafemode = false;
      safemodestart = 0;
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("disconnect"))) {
      if (safemode)
        safemodestart = (millis() + safemode * 1000 ? : 1);
      return true;
    }
#ifdef	USE_NFC
    if (nfc_command(tag, message, len))return true;
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
#ifdef  USE_OUTPUT
    if (output_command(tag, message, len))return true;
#endif
    return false; // Failed
  }

  void setup()
  {
    // Trying to find ways to make WiFi 100% reliable!
    // WiFi.setSleepMode(WIFI_NONE_SLEEP);
    // WiFi.setOutputPower(17);
    // WiFi.setPhyMode(WIFI_PHY_MODE_11G);
#ifdef USE_NFC
    nfc_setup(revk);
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
#ifdef USE_OUTPUT
    output_setup(revk);
#endif
    if (safemode)
      safemodestart = (millis() + safemode * 1000 ? : 1);
  }

  void loop()
  {
    revk.loop();
    { // Fault check
      const char*fault =
#ifdef USE_NFC
        nfc_fault ? :
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
#ifdef USE_OUTPUT
        output_fault ? :
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
    { // Tamper check
      const char*tamper =
#ifdef USE_NFC
        nfc_tamper ? :
#endif
#ifdef USE_RELAY
        relay_tamper ? :
#endif
#ifdef USE_RANGER0X
        ranger0x_tamper ? :
#endif
#ifdef USE_RANGER1X
        ranger1x_tamper ? :
#endif
#ifdef USE_KEYPAD
        keypad_tamper ? :
#endif
#ifdef USE_BEEP
        beep_tamper ? :
#endif
#ifdef USE_INPUT
        input_tamper ? :
#endif
#ifdef USE_OUTPUT
        output_tamper ? :
#endif
        NULL;
      static const char *lasttamper = NULL;
      if (force || tamper != lasttamper)
      {
        lasttamper = tamper;
        if (tamper)revk.state(F("tamper"), F("1 %S"), tamper);
        else revk.state(F("tamper"), F("0"));
      }
    }
    if (safemodestart && (int)(safemodestart - millis()) < 0)
    {
      force = true;
      insafemode = true;
      safemodestart = 0;
    }
#ifdef USE_NFC
    nfc_loop(revk, force);
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
#ifdef USE_OUTPUT
    output_loop(revk, force);
#endif
#ifdef USE_INPUT
    input_loop(revk, force);
#endif
    force = false;
  }
