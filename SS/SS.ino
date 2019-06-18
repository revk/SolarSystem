// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// This is the general purpose ESP application for use with Solar System
// Configurable inputs and outputs, including serial relay control outputs
// I2C Range finder inputs for motion and touch free input (e.g. exit button)
// PN532 card readers on SPI or HSU
// TODO - acting as Max Reader on Galaxy bus

// List of modules
#define modules \
  m(NFC);    \
  m(Ranger0X); \
  m(Ranger1X); \
  m(Keypad); \
  m(Input); \
  m(Output);

#include <ESPRevK.h>

#define m(n)extern const char *n##_fault,*n##_tamper; \
  extern const char * n##_setting(const char *tag, const byte *value, size_t len); \
  extern boolean n##_command(const char*tag, const byte *message, size_t len); \
  extern boolean n##_setup(ESPRevK&); \
  extern boolean n##_loop(ESPRevK&, boolean);
modules
#undef n

#ifdef ARDUINO_ESP8266_NODEMCU
unsigned int gpiomap = 0x1F03F; // Pins available (ESP-12F/12S)
#else
unsigned int gpiomap = 0xF; // Pins available (ESP-01)
#endif

static boolean force = true;
boolean insafemode = false;
unsigned safemodestart = 0;

#define app_settings  \
  s(sda,-1);   \
  s(scl,-1);   \
  s(ss,-1);   \
  s(bustx,-1); \
  s(busrx,-1); \
  s(busde,-1); \
  s(holdtime,3000); \
  s(safemode,60); \
  t(fallback); \
  s(rangerdebug,0); \
  s(rangerpoll,50); \
  s(rangerhold,1000); \
  s(rangermargin,50); \
  s(readerpoll,50); \
  s(pwm,1000); \


#define s(n,d) int n=d;
#define t(n) const char*n=NULL;
  app_settings
#undef s
#undef t

  ESPRevK revk(__FILE__, __DATE__ " " __TIME__);

  const char* app_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM, return true if setting is OK
    const char *ret;
    revk.restart(3000); // Any setting change means restart
#define s(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
#define t(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char*)value;return t;}}while(0)
    app_settings
#undef s
#undef t
#define m(n) if((ret=n##_setting(tag,value,len)))return ret
    modules
#undef m
    debug("Bad setting");
    revk.restart(-1); // cancel restart - unknown/invalid setting
    return false; // Failed
  }

  boolean app_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages, return true if message is OK
    if (!strcasecmp_P(tag, PSTR("connect"))) {
      force = true;
      insafemode = false;
      safemodestart = 0;
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("disconnect"))) {
      if (safemode)
        safemodestart = (millis() + safemode * 1000 ? : 1);
      return true;
    }
#define m(n) if(n##_command(tag,message,len))return true;
    modules
#undef m
    return false; // Failed
  }

  void setup()
  {
    // Trying to find ways to make WiFi 100% reliable!
#define m(n)     n##_setup(revk);
    modules
#undef m
    analogWriteFreq(pwm);
    if (safemode)
      safemodestart = (millis() + safemode * 1000 ? : 1);
  }

  void loop()
  {
    revk.loop();
    { // Fault check
      const char *fault = NULL, *prefix = NULL;
#define m(n) do{if(!fault&&n##_fault){fault=n##_fault;prefix=#n;}}while(0);
      modules
#undef m
      static const char *lastfault = NULL;
      if (force || fault != lastfault)
      {
        lastfault = fault;
        if (fault)revk.state(F("fault"), F("1 %S: %S"), prefix, fault);
        else revk.state(F("fault"), F("0"));
      }
    }
    { // Tamper check
      const char*tamper = NULL, *prefix = NULL;
#define m(n) do{if(!tamper&&n##_tamper){tamper=n##_tamper;prefix=#n;}}while(0);
      modules
#undef m
      static const char *lasttamper = NULL;
      if (force || tamper != lasttamper)
      {
        lasttamper = tamper;
        if (tamper)revk.state(F("tamper"), F("1 %S: %S"), prefix, tamper);
        else revk.state(F("tamper"), F("0"));
      }
    }
    if (safemodestart && (int)(safemodestart - millis()) < 0)
    {
      force = true;
      insafemode = true;
      safemodestart = 0;
    }
#define m(n) n##_loop(revk,force);
    modules
#undef m
    force = false;
  }
