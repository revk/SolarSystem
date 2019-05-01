// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESP8266RevK.h>
#include <Wire.h>
#include <VL53L0X.h>
#include "Ranger0X.h"
const char* ranger0x_fault = false;

#define PINS ((1<<sda) | (1<<scl))

VL53L0X sensor0x;
boolean ranger0xok = false;

#define app_settings  \
  s(ranger0x,0);   \
  s(ranger0xbudget,50); \
  s(ranger0xmax,2000); \
  s(ranger0xaddress,0x29); \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  const char* ranger0x_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean ranger0x_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger0xok)return false; // Ranger not configured
    return false;
  }

  boolean ranger0x_setup(ESP8266RevK&revk)
  {
    if (!ranger0x)return false; // Ranger not configured
    debugf("GPIO pin available %X for VL53L0X", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      ranger0x_fault = PSTR("Ranger0x pins (I2C) not available");
      keypad = NULL;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X after SDA=%d/SCL=%d", gpiomap, sda, scl);
    Wire.begin(sda, scl);
    Wire.beginTransmission((byte)ranger0xaddress);
    if (Wire.endTransmission())
    {
      ranger0x_fault = PSTR("VL53L0X failed");
      ranger0x = NULL;
      return false;
    }
    sensor0x.init();
    sensor0x.setMeasurementTimingBudget(ranger0xbudget * 1000);
    sensor0x.startContinuous();
    sensor0x.setTimeout(10); // continuous

    debug("VL53L0X OK");
    ranger0xok = true;
    return true;
  }

  boolean ranger0x_loop(ESP8266RevK&revk, boolean force)
  {
    if (!ranger0xok)return false; // Ranger not configured
    long now = millis();
    static long next = 0;
    if ((int)(next - now) < 0)
    {
      next = now + rangerpoll;
      static boolean buttonshort = 0;
      static boolean buttonlong = 0;
      static unsigned int last = 0;
      static long endlong = 0;
      unsigned int range = sensor0x.readRangeContinuousMillimeters();
      if (range == 65535)ranger0x_fault = PSTR("VL53L0X read failed");
      else ranger0x_fault = NULL;
      if (range > ranger0xmax)range = ranger0xmax;
      if (range < ranger0x)
      {
        if (force || !buttonshort)
        {
          buttonshort = true;
          revk.state(F("input8"), F("1"));
        }
      } else if (force || range > ranger0x + rangermargin)
      {
        if (force || buttonshort)
        {
          buttonshort = false;
          revk.state(F("input8"), F("0"));
        }
      }
      if (range > last + rangermargin || last > range + rangermargin)
      {
        if (force || !buttonlong)
        {
          buttonlong = true;
          revk.state(F("input9"), F("1"));
        }
        endlong = now + rangerhold;
      } else if ((int)(endlong - now) < 0)
      {
        if (force || buttonlong)
        {
          buttonlong = false;
          revk.state(F("input9"), F("0"));
        }
      }
      last = range;
      if (rangerdebug)
        revk.state(F("range"), F("%d"), range);
    }
    return true;
  }
