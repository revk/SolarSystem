// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Input buttons

#include <ESP8266RevK.h>
#include "Input.h"
const char* input_fault = false;

#define MAX_PIN 17

int inputpin[MAX_PIN] = {};
unsigned long inputs = 0;

#define app_settings  \
  s(input,0);   \
  s(input1,0);   \
  s(input2,0);   \
  s(input3,0);   \
  s(inputhold,500); \
  s(inputpoll,10); \
  s(inputinvert,0); \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  const char* input_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean input_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!input)return false; // No inputs defined
    return false;
  }

  boolean input_setup(ESP8266RevK&revk)
  {
    if (!input)return false; // No inputs defined
    unsigned int map = gpiomap & ~((1 << 0) | (1 << 1) || (1 << 2)); // Dont use GPIO0, GPI1, nor GPI2 as general input because bad if tied low at boot.
    debugf("GPIO available %X for %d inputs", map, input);
    int i;
    inputpin[0] = input1; // Presets (0 means not preset as we don't use 0 anyway)
    inputpin[1] = input2;
    inputpin[2] = input3;
    for (i = 0; i < input; i++)
    {
      if (!map)
      {
        input_fault = PSTR("Input pins not available");
        input = NULL;
        return false;
      }
      int p = inputpin[i];
#ifdef ARDUINO_ESP8266_NODEMCU
      if (!p) for (p = 1; p < MAX_PIN && !((map | (1 << 1) | (1 << 3)) & (1 << p)); p++); // Find a pin (skip tx/rx)
#endif
      if (!p) for (p = 1; p < MAX_PIN && !(map & (1 << p)); p++); // Find a pin
      if (p == MAX_PIN || !(map & (1 << p)))
      {
        input_fault = PSTR("Input pin assignment not available");
        input = NULL;
        return false;
      }
      inputpin[i] = p;
      debugf("Input %d pin %d", i + 1, p);
      gpiomap &= ~(1 << p);
      map &= ~(1 << p);
    }
    debugf("GPIO remaining %X", gpiomap);
    for (i = 0; i < input; i++)
      pinMode(inputpin[i], INPUT_PULLUP);
    debug("Input OK");
    return true;
  }

  boolean input_loop(ESP8266RevK&revk, boolean force)
  {
    if (!input)return false; // No inputs defined
    unsigned long now = millis();
    static unsigned long pincheck = 0;
    static unsigned long pinhold[MAX_PIN] = {};

    if ((int)(pincheck - now) < 0)
    {
      pincheck = now + inputpoll;
      int p;
      for (p = 0; p < input; p++)
      {
        if (force || !pinhold[p] || (int)(pinhold[p] - now) < 0)
        {
          pinhold[p] = 0;
          int r = digitalRead(inputpin[p]);
          if (inputinvert & (1 << p))r = 1 - r;
          if (force || r != ((inputs & (1 << p)) ? HIGH : LOW))
          {
            pinhold[p] = ((now + inputhold) ? : 1);
            char tag[7];
            strcpy_P(tag, PSTR("inputX"));
            tag[5] = '1' + p;
            revk.state(tag, r == HIGH ? F("1") : F("0"));
            if (r == HIGH)inputs |= (1 << p);
            else inputs &= ~(1 << p);
          }
        }
      }
    }
    return true;
  }
