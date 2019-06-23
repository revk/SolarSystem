// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESPRevK.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <VL53L1X.h>
const char* Ranger_fault = NULL;
const char* Ranger_tamper = NULL;

#define PINS ((1<<sda) | (1<<scl))

VL53L0X sensor0x;
VL53L1X sensor1x;

#define app_settings  \
  s(rangerdebug,0); \
  s(rangerpoll,50); \
  s(rangerhold,1000); \
  s(rangermargin,50); \
  s(rangerset,300); \
  s(rangermax,1000); \
  s(rangeraddress,0x29); \
  s(ranger,-1);   \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  const char* Ranger_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean Ranger_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (ranger < 0)return false; // Ranger not configured
    return false;
  }

  boolean Ranger_setup(ESPRevK&revk)
  {
    if (ranger < 0)return false; // Ranger not configured
    if (sda < 0 || scl < 0 || sda == scl)
    {
      Ranger_fault = PSTR("Define SDA/SCL to use I2C");
      ranger = -1;
      return false;
    }
    debugf("GPIO pin available %X for Ranger", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      Ranger_fault = PSTR("Pins (I2C) not available");
      ranger = -1;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X after SDA=%d/SCL=%d", gpiomap, sda, scl);
    Wire.begin(sda, scl);
    Wire.beginTransmission((byte)rangeraddress);
    if (Wire.endTransmission())
    {
      Ranger_fault = PSTR("Failed");
      ranger = -1;
      return false;
    }
    if (ranger == 0)
    {
      sensor0x.init();
      sensor0x.setMeasurementTimingBudget(rangerpoll * 1000);
      sensor0x.startContinuous(rangerpoll);
      debug("VL53L0X OK");
    } else
    {
      sensor1x.init();
      sensor1x.setDistanceMode(VL53L1X::Long);
      sensor1x.setMeasurementTimingBudget(rangerpoll * 1000);
      sensor1x.startContinuous(rangerpoll);
      debug("VL53L1X OK");
    }
    return true;
  }

  boolean Ranger_loop(ESPRevK&revk, boolean force)
  {
    if (ranger < 0)return false; // Ranger not configured
    long now = millis();
    static long next = 0;
    if ((int)(next - now) < 0)
    {
      next = now + rangerpoll;
      static boolean buttonshort = 0;
      static boolean buttonlong = 0;
      static unsigned int last = 0;
      static long endlong = 0;
      int range = 0;
      if (ranger == 0)range = sensor0x.readRangeContinuousMillimeters();
      else range = sensor1x.readRangeContinuousMillimeters();
      if (range == 65535)Ranger_fault = PSTR("Read failed");
      else Ranger_fault = NULL;
      if (range)
      {
        if (range > rangermax)range = rangermax;
        if (range < ranger && last < ranger)
        {
          if (force || !buttonshort)
          {
            buttonshort = true;
            revk.state(F("input8"), F("1 %dmm"), range);
          }
        } else if (force || range > ranger + rangermargin)
        {
          if (force || buttonshort)
          {
            buttonshort = false;
            revk.state(F("input8"), F("0 %dmm"), range);
          }
        }
        static int lastdelta = 0;
        int delta = range - last;
        if ((delta > 0 && lastdelta > 0 && delta + lastdelta >= rangermargin) || (delta < 0 && lastdelta < 0 && delta + lastdelta <= rangermargin))
        {
          if (force || !buttonlong)
          {
            buttonlong = true;
            revk.state(F("input9"), F("1 %dmm"), range);
          }
          endlong = now + rangerhold;
        } else if ((int)(endlong - now) < 0)
        {
          if (force || buttonlong)
          {
            buttonlong = false;
            revk.state(F("input9"), F("0 %dmm"), range);
          }
        }
        if (rangerdebug && (range < rangermax || last < rangermax))
          revk.state(F("range"), F("%dmm"), range);
        last = range;
        lastdelta = delta;
      }
    }
    return true;
  }
