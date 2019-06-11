// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#include <ESPRevK.h>
#include <Wire.h>
#include <VL53L0X.h>
const char* Ranger0X_fault = NULL;
const char* Ranger0X_tamper = NULL;

#define PINS ((1<<sda) | (1<<scl))

VL53L0X sensor0x;
boolean ranger0xok = false;

#define app_settings  \
  s(ranger0x,0);   \
  s(ranger0xmax,2000); \
  s(ranger0xaddress,0x29); \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  const char* Ranger0X_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean Ranger0X_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!ranger0xok)return false; // Ranger not configured
    return false;
  }

  boolean Ranger0X_setup(ESPRevK&revk)
  {
    if (!ranger0x)return false; // Ranger not configured
    if (sda < 0 || scl < 0 || sda == scl)
    {
      Ranger0X_fault = PSTR("Define SDA/SCL to use I2C");
      ranger0x = NULL;
      return false;
    }
    debugf("GPIO pin available %X for VL53L0X", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      Ranger0X_fault = PSTR("Pins (I2C) not available");
      ranger0x = NULL;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X after SDA=%d/SCL=%d", gpiomap, sda, scl);
    Wire.begin(sda, scl);
    Wire.beginTransmission((byte)ranger0xaddress);
    if (Wire.endTransmission())
    {
      Ranger0X_fault = PSTR("VL53L0X failed");
      ranger0x = NULL;
      return false;
    }
    sensor0x.init();
    sensor0x.setMeasurementTimingBudget(rangerpoll * 1000);
    sensor0x.setSignalRateLimit(0.25);
    sensor0x.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensor0x.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    sensor0x.startContinuous(rangerpoll);

    debug("VL53L0X OK");
    ranger0xok = true;
    return true;
  }

  boolean Ranger0X_loop(ESPRevK&revk, boolean force)
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
      if (range == 65535)Ranger0X_fault = PSTR("VL53L0X read failed");
      else Ranger0X_fault = NULL;
      if (range)
      {
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
        if (rangerdebug && range < ranger0xmax)
          revk.state(F("range"), F("%d"), range);
      }
    }
    return true;
  }
