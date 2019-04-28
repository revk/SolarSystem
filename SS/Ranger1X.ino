// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESP8266RevK.h>
#include "Ranger1X.h"
#include <Wire.h>
#include <VL53L1X.h>

VL53L1X sensor1x;

#define MAXRANGE 2000	// Maximum range
#define LONGHOLD 1000	// How long to hold movement input
#define MARGIN 50	// Movement margin

#define app_settings  \
  s(ranger1x);   \

#define s(n) unsigned int n=0;
  app_settings
#undef s

  const char* ranger1x_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):0);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean ranger1x_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger1x)return false; // Ranger not configured
    return false;
  }

  boolean ranger1x_setup(ESP8266RevK&revk)
  {
    if (!ranger1x)return false; // Ranger not configured

    Wire.begin();

    if (!sensor1x.init())
    { revk.error(F("ranger1x"), F("VL53L1X not present"));
      ranger1x = NULL;
      return false;
    }
    //sensor1x.setAddress(1x29);
    sensor1x.setTimeout(500);
    sensor1x.setMeasurementTimingBudget(50000);
    sensor1x.setDistanceMode(VL53L1X::Long);
    return true;
  }

  boolean ranger1x_loop(ESP8266RevK&revk, boolean force)
  {
    if (!ranger1x)return false; // Ranger not configured
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
      unsigned int range = sensor1x.read();
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
      if (range < ranger1x)
      {
        if (force || !buttonshort)
        {
          buttonshort = true;
          revk.state(F("ranger0"), F("1"));
        }
      } else if (force || range > ranger1x + MARGIN)
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
      } else if ((int)(endlong - now) < 0)
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
