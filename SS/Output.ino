// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Direct digital outputs

#include <ESP8266RevK.h>
#include "Output.h"
const char* output_fault = false;

#define MAX_PIN 17

char outputpin[MAX_PIN] = {};
unsigned long outputs = 0;
unsigned long outputnext = 0;
unsigned long outputoverride = 0;

#define app_settings  \
  s(output,0);   \
  s(output1,-1);   \
  s(output2,-1);   \
  s(output3,-1);   \
  s(outputhold,500); \
  s(outputpoll,10); \

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
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean output_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!output)return false; // No outputs defined
    if (!strncasecmp(tag, PSTR("output"), 6) && tag[6] > '0' && tag[6] <= '0' + output)
    {
      if (len && *message == '1')
        outputs |= (1 << (tag[6] - '1'));
      else
        outputs &= ~(1 << (tag[6] - '1'));
      outputnext = millis();
      return true;
    }
    return false;
  }

  boolean output_setup(ESP8266RevK&revk)
  {
    if (!output)return false; // No outputs defined
    debugf("GPIO available %X for %d outputs", gpiomap, output);
    int i;
    for (i = 0; i < output; i++)outputpin[i] = -1;
    outputpin[0] = output1; // Presets (0 means not preset as we don't use 0 anyway)
    outputpin[1] = output2;
    outputpin[2] = output3;
    for (i = 0; i < output; i++)
    {
      if (!gpiomap)
      {
        output_fault = PSTR("Output pins not available");
        output = NULL;
        return false;
      }
      int p = outputpin[i];
      if (p < 0) for (p = 0; p < MAX_PIN && !(gpiomap & (1 << p)); p++); // Find a pin
      if (p == MAX_PIN || !(gpiomap & (1 << p)))
      {
        output_fault = PSTR("Output pin assignment not available");
        output = NULL;
        return false;
      }
      outputpin[i] = p;
      debugf("Output %d pin %d", i + 1, p);
      gpiomap &= ~(1 << p);
    }
    debugf("GPIO remaining %X", gpiomap);
    for (i = 0; i < output; i++)
      pinMode(outputpin[i], OUTPUT);
    debug("Output OK");
    return true;
  }

  boolean output_loop(ESP8266RevK&revk, boolean force)
  {
    if (!output)return false; // No outputs defined
    unsigned long now = millis();
    if ((int)(outputnext - now) < 0)
    {
      outputnext = now + 1000; // Periodically send all output
      unsigned long out = outputs;
      if (insafemode)outputs = outputoverride; // Safe mode, normall means relays off but can be overridden
      int i;
      for (i = 0; i < output && i < 8; i++)
        digitalWrite(outputpin[i], (out & (1 << i)) ? 1 : 0);
    }
    return true;
  }
