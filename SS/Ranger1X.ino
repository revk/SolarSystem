// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESPRevK.h>
#include "Ranger1X.h"
#include <Wire.h>
#include <VL53L1X.h>

#define PINS ((1<<sda) | (1<<scl))

VL53L1X sensor1x;
boolean ranger1xok = false;
const char* ranger1x_fault = false;

#define app_settings  \
  s(ranger1x,0);   \
  s(ranger1xbudget,50); \
  s(ranger1xmax,2000); \
  s(ranger1xaddress,0x29); \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  const char* ranger1x_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean ranger1x_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger1xok)return false; // Ranger not configured
    return false;
  }

  boolean ranger1x_setup(ESPRevK&revk)
  {
    if (!ranger1x)return false; // Ranger not configured
    debugf("GPIO pin available %X for VL53L1X", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      ranger1x_fault = PSTR("Ranger1x pins (I2C) not available");
      keypad = NULL;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X after SDA=%d/SCL=%d", gpiomap, sda, scl);
    Wire.begin(sda, scl);
    Wire.setClock(400000); // use 400 kHz I2C
    Wire.beginTransmission((byte)ranger1xaddress);
    if (Wire.endTransmission())
    {
      ranger1x_fault = PSTR("VL53L1X failed");
      ranger1x = NULL;
      return false;
    }
    sensor1x.init();
    sensor1x.setDistanceMode(VL53L1X::Long);
    sensor1x.setMeasurementTimingBudget(ranger1xbudget * 1000);
    sensor1x.startContinuous(ranger1xbudget);
    sensor1x.setTimeout(10);

    debug("VL53L1X OK");
    ranger1xok = true;
    return true;
  }

  boolean ranger1x_loop(ESPRevK&revk, boolean force)
  {
    if (!ranger1xok)return false; // Ranger not configured
    long now = millis();
    static long next = 0;
    if ((int)(next - now) < 0)
    {
      next = now + rangerpoll;
      static boolean buttonshort = 0;
      static boolean buttonlong = 0;
      static unsigned int last = 0;
      static long endlong = 0;
      unsigned int range = sensor1x.read(false);
      if (sensor1x.timeoutOccurred()) ranger1x_fault = PSTR("VL53L1X Timeout");
      else ranger1x_fault = NULL;
      if (!range || range > ranger1xmax)range = ranger1xmax;
      if (range < ranger1x)
      {
        if (force || !buttonshort)
        {
          buttonshort = true;
          revk.state(F("input8"), F("1"));
        }
      } else if (force || range > ranger1x + rangermargin)
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
