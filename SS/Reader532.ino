// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System
// ESP-12F based for use with PN532

// Wiring for ESP-12F
// PN531 connnections (in addition to GND/3V3)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

#include <ESP8266RevK.h>
#include "Reader532.h"
#include <PN532_SPI.h>
#include "PN532.h"

PN532_SPI pn532spi(SPI, 16);
PN532 nfc(pn532spi);

#define app_settings  \
  s(reader532);   \

#define s(n) const char *n=NULL
  app_settings
#undef s

  const char* reader532_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean reader532_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    return false;
  }

  boolean reader532_setup(ESP8266RevK&revk)
  {
    if (!reader532)return false; // Not configured
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata)
    { // no reader
      revk.error(F("reader532"), F("NP532 not present"));
      reader532 = NULL;
      return false;
    }
    nfc.setPassiveActivationRetries(1);
    nfc.SAMConfig();
    return true;
  }

  static byte lastlen = 0;
  static byte lastuid[7] = {};
  static void report(const __FlashStringHelper *tag)
  {
    char tid[15];
    int n;
    for (n = 0; n < lastlen && n * 2 < sizeof(tid); n++)sprintf(tid + n * 2, "%02X", lastuid[n]);
    revk.state(tag, F("%s"), tid);
  }

  boolean reader532_loop(ESP8266RevK&revk, boolean force)
  {
    if (!reader532)return false; // Not configured
    long now = (millis() ? : 1); // Allowing for wrap, and using 0 to mean not set
    static long cardcheck = 0;
    if ((int)(cardcheck - now) < 0)
    {
      cardcheck = now + 100;
      static long first = 0;
      static long last = 0;
      static boolean held = false;
      byte uid[7] = {}, uidlen = 0;
      if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidlen))
      {
        last = now;
        if (!first || uidlen != lastlen || memcmp(lastuid, uid, uidlen))
        {
          if (held)
            report(F("gone"));
          first = now;
          held = false;
          memcpy(lastuid, uid, lastlen = uidlen);
          report(F("id"));
        } else if (!held && first && (int)(now - first) > HOLDTIME)
        {
          held = true;
          report(F("held"));
        }
      } else if (last && (int)(now - last) > RELEASETIME)
      {
        if (held)
          report(F("gone"));
        first = 0;
        last = 0;
        held = false;
      }
    }
    return true;
  }
