// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESP8266RevK.h>
#include "Ranger0X.h"
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor0x;

#define MAXRANGE 2000	// Maximum range
#define LONGHOLD 1000	// How long to hold movement input
#define MARGIN 50	// Movement margin

#define app_settings  \
  s(ranger0x);   \

#define s(n) unsigned int n=0;
  app_settings
#undef s

  const char* ranger0x_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):0);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean ranger0x_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger0x)return false; // Ranger not configured
    return false;
  }

  boolean ranger0x_setup(ESP8266RevK&revk)
  {
    if (!ranger0x)return false; // Ranger not configured

    Wire.begin();

    if (!sensor0x.init())
    { revk.error(F("ranger0x"), F("VL53L0X not present"));
      ranger0x = NULL;
      return false;
    }
    sensor0x.setAddress(0x29);
    sensor0x.setTimeout(500);
    sensor0x.startContinuous();

    return true;
  }

  boolean ranger0x_loop(ESP8266RevK&revk, boolean force)
  {
    if (!ranger0x)return false; // Ranger not configured
    long now = millis();
    static long next = 0;
    if ((int)(next - now) < 0)
    {
      next = now + 10;
      static boolean buttonshort = 0;
      static boolean buttonlong = 0;
      static boolean fault = false;
      static unsigned int last = 0;
      static long endlong = 0;
      unsigned int range = sensor0x.readRangeContinuousMillimeters();
      if (range == 65535 && !fault)
      {
        fault = true;
        revk.state(F("fault"), F("0"));
      } else if (range != 65535 && fault)
      {
        fault = false;
        revk.state(F("fault"), F("0"));
      }
      if (range > MAXRANGE)range = MAXRANGE;
      if (range < ranger0x)
      {
        if (force || !buttonshort)
        {
          buttonshort = true;
          revk.state(F("ranger0"), F("1"));
        }
      } else if (force || range > ranger0x + MARGIN)
      {
        if (force || buttonshort)
        {
          buttonshort = false;
          revk.state(F("ranger0"), F("0"));
        }
      }
      if (range > last + MARGIN || last > range + MARGIN)
      {
        if (force || !buttonlong)
        {
          buttonlong = true;
          revk.state(F("ranger1"), F("1"));
        }
        endlong = now + LONGHOLD;
      } else if((int)(endlong - now) < 0)
      {
        if (force || buttonlong)
        {
          buttonlong = false;
          revk.state(F("ranger1"), F("0"));
        }
      }
      last = range;
      //revk.state(F("range"), F("%d"), range);
    }
    return true;
  }
