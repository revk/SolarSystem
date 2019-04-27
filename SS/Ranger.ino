// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESP8266RevK.h>
#include "Ranger.h"
#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;

#define MAXRANGE 2000

#define app_settings  \
  s(ranger);   \

#define s(n) unsigned int n=0;
  app_settings
#undef s

  const char* ranger_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):0);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean ranger_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger)return false; // Ranger not configured
    return false;
  }

  boolean ranger_setup(ESP8266RevK&revk)
  {
    if (!ranger)return false; // Ranger not configured

    Wire.begin();

    sensor.init();
    sensor.setAddress(0x29);
    sensor.setTimeout(500);
    sensor.startContinuous();

    return true;
  }

  boolean ranger_loop(ESP8266RevK&revk)
  {
    long now = millis();
    if (!ranger)return false; // Ranger not configured
    static long next = 0;
    static unsigned int margin = 100;
    static unsigned int rangemax = 200;
    static boolean buttonshort = 0;
    static boolean buttonlong = 0;
    static boolean fault = false;
    if ((int)(next - now) < 0)
    {
      next = now + 10;
      unsigned int range = sensor.readRangeContinuousMillimeters();
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
      if (range < ranger && !buttonshort)
      {
        buttonshort = true;
        revk.state(F("ranger0"), F("1 %u %u"), range, ranger);
      } else if (range > ranger + margin && buttonshort)
      {
        buttonshort = false;
        revk.state(F("ranger0"), F("0 %u %u"), range, ranger);
      }
      if (range > rangemax)
        rangemax = range;
      else
        rangemax = (rangemax * 999 + range) / 1000;
      if (range + margin * 2 < rangemax && !buttonlong)
      {
        buttonlong = true;
        revk.state(F("ranger1"), F("1 %u %u"), range, rangemax);
      } else if (range + margin > rangemax && buttonlong)
      {
        buttonlong = false;
        revk.state(F("ranger1"), F("0 %u %u"), range, rangemax);
      }
      //revk.state(F("range"), F("%d"), range);
    }
    return true;
  }
