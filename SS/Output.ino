// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Direct digital outputs

#include <ESPRevK.h>
const char* Output_fault = NULL;
const char* Output_tamper = NULL;

#define MAX_PIN 17
#define MAX_OUTPUT 9

char outputpin[MAX_OUTPUT] = {}; // GPIO pin or relay number
unsigned int outputinvert = 0;
unsigned int outputactive = 0;
unsigned int outputstate = 0;
unsigned int outputrelay = 0;
unsigned int outputbeep = 0;

unsigned long outputnext = 0;

#define app_settings  \
  s(output,0);   \

#define s(n,d) int n=d;
  app_settings
#undef s

  static void dooutput(int o)
  {
    unsigned long out = outputstate;
    out ^= outputinvert;
    if (outputactive & (1 << o))
    {
      if (outputrelay & (1 << o))
      {
        byte msg[5];
        msg[0] = 0xA0;
        msg[1] = outputpin[o];
        msg[2] = ((out & (1 << o)) ? 1 : 0);
        msg[3] = msg[0] + msg[1] + msg[2];
        msg[4] = 0x0A;
        Serial.write(msg, 5);
      }
      else if (outputbeep & (1 << o))
        analogWrite(outputpin[o], (out & (1 << o)) ? PWMRANGE / 2 : 0);
      else
        digitalWrite(outputpin[o], (out & (1 << o)) ? 1 : 0);
    }
  }

  void Output_set(int o, boolean v)
  { // Set an output externally
    o--; // Starts from 1
    if (!(outputactive & (1 << o)))return;
    if (v)
    {
      if (!(outputstate & (1 << o)))
      {
        outputstate |= (1 << o);
        dooutput(o);
      }
    }
    else
    {
      if (outputstate & (1 << o))
      {
        outputstate &= ~(1 << o);
        dooutput(o);
      }
    }
  }

  boolean Output_active(int o)
  { // Return if output is active
    o--; // Starts from 1
    if (!(outputactive & (1 << o)))return false;
    return true;
  }

  boolean Output_get(int o)
  { // Read an output state
    o--; // Starts from 1
    if (!(outputactive & (1 << o)))return false;
    if (outputstate & (1 << o))return true;
    return false;
  }

  const char* Output_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
    if (!strncasecmp_P(tag, PSTR("output"), 6) && isdigit(tag[6]))
    { // Define output pin
      int i = atoi(tag + 6) - 1;
      if (i < 0 || i >= MAX_OUTPUT)
        return NULL;
      // Clear
      outputactive &= ~(1 << i);
      outputbeep &= ~(1 << i);
      outputrelay &= ~(1 << i);
      outputinvert &= ~(1 << i);
      if (len)
      {
        if (*value == '+' || *value == '-')
        { // Polarity
          if (*value == '-')
            outputinvert |= (1 << i);
          value++;
          len--;
        }
        if (!len)
          return NULL;
        byte prefix = 0;
        if (!isdigit(*value))
        {
          prefix = *value++;
          len--;
        }
        if (!len)return NULL;
        int p = 0;
        while (len && isdigit(*value))
        {
          p = p * 10 + *value++ -'0';
          len--;
        }
        if (len || p > 255)return NULL;
        if (prefix == 'R')
          outputrelay |= (1 << i); // Relay
        else if (prefix == 'B')
          outputbeep |= (1 << i);
        outputpin[i] = p;
        outputactive |= (1 << i);
      }
      // Messy...
      const char*tagname[] = {PSTR("output1"), PSTR("output2"), PSTR("output3"), PSTR("output4"), PSTR("output5"), PSTR("output6"), PSTR("output7"), PSTR("output8"), PSTR("output9")};
      return tagname[i];
    }
#define s(n,d) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean Output_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!outputactive)return false; // No outputs defined
    if (!strncasecmp_P(tag, PSTR("output"), 6) && tag[6] > '0' && tag[6] <= '0' + MAX_OUTPUT)
    {
      int i = tag[6] - '0';
      if (!i || !(outputactive & (1 << (i - 1))))return false;
      if (len && *message == '1')
        Output_set(i, true);
      else
        Output_set(i, false);
      return true;
    }
    return false;
  }

  boolean Output_setup(ESPRevK&revk)
  {
    if (!outputactive && !output)return false; // No outputs defined
    debugf("GPIO available %X for %d outputs", gpiomap, output);
    if (outputrelay && !(gpiomap & (1 << 1)))
    {
      Output_fault = PSTR("Relay Tx not available");
      outputrelay = 0;
    }
    if (outputrelay)gpiomap &= ~(1 << 1); // Tx used
    int i;
    // Check assigned pins
    for (i = 0; i < MAX_OUTPUT; i++)
      if ((outputactive & (1 << i)) && !(outputrelay & (1 << i)))
      {
        if (!(gpiomap & (1 << outputpin[i])))
        { // Unusable
          Output_fault = PSTR("Pin assignment not available");
          outputactive &= ~(1 << i);
          continue;
        }
        gpiomap &= ~(1 << outputpin[i]);
      }
    if (output)
    { // Auto assign some pins (deprecated)
      for (i = 0; i < output; i++)
        if (!(outputactive & (1 << i)) && !(outputrelay & (1 << i)))
        {
          int p;
          for (p = 0; p < MAX_PIN && !(gpiomap & (1 << p)); p++); // Find a pin
          if (p == MAX_PIN)
          {
            Output_fault = PSTR("No output pins available to assign");
            break;
          }
          outputpin[i] = p;
          outputactive |= (1 << i);
          gpiomap &= ~(1 << outputpin[i]);
        }
    }
    debugf("GPIO remaining %X", gpiomap);
    for (i = 0; i < MAX_OUTPUT; i++)
      if (outputactive & (1 << i) && !(outputrelay & (1 << i)))
        pinMode(outputpin[i], OUTPUT);
#ifndef REVKDEBUG
    if (outputrelay)
      Serial.begin(9600);  // Serial relay control uses 9600
#endif
    debug("Output OK");
    return true;
  }

  boolean Output_loop(ESPRevK&revk, boolean force)
  {
    if (!outputactive)return false; // No outputs defined
    unsigned long now = millis();
    if ((int)(outputnext - now) < 0)
    {
      outputnext = now + 1000; // Periodically re-set all output just in case, not really necessary

      int i;
      for (i = 0; i < MAX_OUTPUT; i++) dooutput(i);

    }
    return true;
  }
