// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Relay output

#include <ESP8266RevK.h>
#include "relay.h"
const char* relay_fault = false;

#define PINS  ((1<<1))  // Tx (GPIO1)

#define app_settings  \
  s(relay);   \

#define s(n) unsigned int n=0;
  app_settings
#undef s

  unsigned long relaynext = 0;
  unsigned long output = 0;
  unsigned long override = 0;

  void relay_safe_relay(boolean enable)
  { // Set relay safe mode operation
    if (enable)
      override = 1;
    else
      override = 0;
    relaynext = millis();
  }

  const char* relay_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):0);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean relay_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!relay)
      return false; // Relay not configured
    if (!strncasecmp(tag, PSTR("output"), 6) && tag[6] > '0' && tag[6] <= '0' + relay)
    {
      if (len && *message == '1')
        output |= (1 << (tag[6] - '1'));
      else
        output &= ~(1 << (tag[6] - '1'));
      relaynext = millis();
      return true;
    }
    return false;
  }

  boolean relay_setup(ESP8266RevK&revk)
  {
    if (!relay)return false; // Relay not configured
    if ((gpiomap & PINS) != PINS)
    {
      relay_fault = PSTR("Relay pin (Tx) not available");
      relay = NULL;
      return false;
    }
    gpiomap &= ~PINS;
#ifndef REVKDEBUG
    Serial.begin(9600);	// Serial relay control uses 9600
#endif
    debug("Relay OK");
    return true;
  }

  boolean relay_loop(ESP8266RevK&revk, boolean force)
  {
    if (!relay)return false; // Relay not configured
    unsigned long now = millis();
    if ((int)(relaynext - now) < 0)
    {
      relaynext = now + 1000; // Periodically send all relays
      unsigned long relays = output;
      if (insafemode)relays = override; // Safe mode, normall means relays off but can be overridden
      int n;
      for (n = 0; n < relay && n < 8; n++)
      {
        byte msg[5];
        msg[0] = 0xA0;
        msg[1] = n + 1;
        msg[2] = ((relays & (1 << n)) ? 1 : 0);
        msg[3] = msg[0] + msg[1] + msg[2];
        msg[4] = 0x0A;
        Serial.write(msg, 5);
      }
    }
    return true;
  }
