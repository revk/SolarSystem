// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Input buttons

#include <ESP8266RevK.h>
#include "Input.h"
const char* inputfault = false;

#define MAX_PIN 17
#define PINHOLD 250

int pin[MAX_PIN] = {};
unsigned long inputs = 0;

#define app_settings  \
  s(input,0);   \
  s(input1,-1);   \
  s(input2,-1);   \
  s(input3,-1);   \

#define s(n,d) unsigned int n=d;
  app_settings
#undef s

  const char* input_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):0);return t;}}while(0)
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
    debugf("GPIO available %X for %d inputs", gpiomap, input);
    gpiomap &= ~(1 << 0); // Dont use GPIO0 as general input because flash mode
    int i;
    for (i = 0; i < input; i++)
    {
      if (!gpiomap)
      {
        inputfault = PSTR("Input pins not available");
        input = NULL;
        return false;
      }
      int p;
      if (i == 0 && input1 >= 0)p = input1; // presets
      else if (i == 1 && input2 >= 0)p = input2;
      else if (i == 2 && input3 >= 0)p = input3;
      else for (p = 0; p < MAX_PIN && !(gpiomap & (1 << p)); p++); // Find a pin
      pin[i] = p;
      debugf("Input %d pin %d", i + 1, p);
      gpiomap &= ~(1 << p);
    }
    debugf("GPIO remaining %X", gpiomap);
    for (i = 0; i < input; i++)
      pinMode(pin[i], INPUT_PULLUP);
    debug("Input OK");
    return true;
  }

  boolean input_loop(ESP8266RevK&revk, boolean force)
  {
    if (!input)return false; // No inputs defined
    unsigned long now = millis();
    unsigned long pincheck = 0;
    unsigned long pinhold[MAX_PIN] = {};

    if ((int)(pincheck - now) < 0)
    {
      pincheck = now + 10;
      int p;
      for (p = 0; p < input; p++)
      {
        if (force || !pinhold[p] || (int)(pinhold[p] - now) < 0)
        {
          pinhold[p] = 0;
          int r = digitalRead(pin[p]);
          if (force || r != ((inputs & (1 << p)) ? HIGH : LOW))
          {
            pinhold[p] = ((now + PINHOLD) ? : 1);
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
