// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Direct digital outputs

#include <ESP8266RevK.h>
#include "Output.h"
const char* output_fault = false;

#define MAX_PIN 17
#define MAX_OUTPUT 9

char outputpin[MAX_OUTPUT] = {};
unsigned int outputinvert = 0;
unsigned int outputactive = 0;
unsigned int outputstate = 0;
unsigned int outputoverride = 0;

unsigned long outputnext = 0;

#define app_settings  \
  s(output,0);   \

#define s(n,d) int n=d;
  app_settings
#undef s

  void output_safe_set(boolean enable)
  { // Set relay safe mode operation
    if (enable)
      outputoverride = 1;
    else
      outputoverride = 0;
    outputnext = millis();
  }

  const char* output_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
    if (!strncmp(tag, PSTR("output"), 6) && isdigit(tag[6]))
    { // Define output pin
      int i = atoi(tag + 6) - 1;
      if (i < 0 || i >= MAX_OUTPUT)
        return NULL;
      if (!len)
      { // inactive
        outputactive &= ~(1 << i);
        return NULL;
      }
      outputinvert &= ~(1 << i);
      if (*value == '+' || *value == '-')
      { // active
        if (*value == '-')
          outputinvert |= (1 << i);
        value++;
        len--;
      }
      if (!len)
        return NULL;
      int p = 0;
      while (len && isdigit(*value))
      {
        p = p * 10 + *value++ -'0';
        len--;
      }
      if (len || p >= MAX_PIN)
        return NULL;
      outputpin[i] = p;
      outputactive |= (1 << i);
      // Messy...
      const char*tagname[] = {PSTR("output1"), PSTR("output2"), PSTR("output3"), PSTR("output4"), PSTR("output5"), PSTR("output6"), PSTR("output7"), PSTR("output8"), PSTR("output9")};
      return tagname[i];
    }
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean output_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!outputactive)return false; // No outputs defined
    if (!strncasecmp(tag, PSTR("output"), 6) && tag[6] > '0' && tag[6] <= '0' + MAX_OUTPUT)
    {
      int i = tag[6] - '1';
      if (!(outputactive & (1 << i)))return false;
      if (len && *message == '1')
        outputstate |= (1 << i);
      else
        outputstate &= ~(1 << i);
      outputnext = millis();
      return true;
    }
    return false;
  }

  boolean output_setup(ESP8266RevK&revk)
  {
    if (!outputactive && !output)return false; // No outputs defined
    debugf("GPIO available %X for %d outputs", gpiomap, output);
    int i;
    // Check assigned pins
    for (i = 0; i < MAX_OUTPUT; i++)
      if (outputactive & (1 << i))
      {
        if (!(gpiomap & (1 << outputpin[i])))
        { // Unusable
          output_fault = PSTR("Output pin assignment not available");
          outputactive &= ~(1 << i);
          continue;
        }
        gpiomap &= ~(1 << outputpin[i]);
      }
    if (output)
    { // Auto assign some pins (deprecated)
      for (i = 0; i < output; i++)
        if (!(outputactive & (1 << i)))
        {
          int p;
          for (p = 0; p < MAX_PIN && !(gpiomap & (1 << p)); p++); // Find a pin
          if (p == MAX_PIN)
          {
            output_fault = PSTR("No output pins available to assign");
            break;
          }
          outputpin[i] = p;
          outputactive |= (1 << i);
          gpiomap &= ~(1 << outputpin[i]);
        }
    }
    debugf("GPIO remaining %X", gpiomap);
    for (i = 0; i < MAX_OUTPUT; i++)
      if (outputactive & (1 << i))
        pinMode(outputpin[i], OUTPUT);
    debug("Output OK");
    return true;
  }

  boolean output_loop(ESP8266RevK&revk, boolean force)
  {
    if (!outputactive)return false; // No outputs defined
    unsigned long now = millis();
    if ((int)(outputnext - now) < 0)
    {
      outputnext = now + 1000; // Periodically re-set all output just in case, not really necessary
      unsigned long out = outputstate;
      if (insafemode)out = outputoverride; // Safe mode, normall means relays off but can be overridden
      out ^= outputinvert;
      int i;
      for (i = 0; i < MAX_OUTPUT; i++)
        if (outputactive & (1 << i))
          digitalWrite(outputpin[i], (out & (1 << i)) ? 1 : 0);
    }
    return true;
  }
