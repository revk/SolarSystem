// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System
// ESP-12F based for use with PN532

// Wiring for ESP-12F
// PN532 connnections (in addition to GND/3V3)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

#define PINS  ((1 << 12) | (1 << 13) | (1 << 14) | (1 << ss))

#include <ESP8266RevK.h>
#include "Reader532.h"
#include "PN532RevK.h"
#include "PN532_SPI.h"
#include "Output.h"

PN532_SPI pn532spi(SPI, ss);
PN532RevK nfc(pn532spi);
boolean reader532ok = false;
const char* reader532_fault = false;

#define app_settings  \
  s(reader532);   \

#define s(n) const char *n=NULL
  app_settings
#undef s

#define readertimeout 100

  const char* reader532_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean reader532_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!reader532ok)return false; // Not configured
    if (!strcmp_P(tag, "nfc"))
    {
      byte res[100], rlen = sizeof(res);
      byte ok = nfc.inData((byte*)message, len, res, &rlen);
      if (!ok)
        revk.info(F("nfc"), rlen, res);
      else revk.error(F("nfc"), F("failed %02X (%d bytes sent %02X %02X %02X...)"), ok, len, message[0], message[1], message[2]);
      return true;
    }
    return false;
  }

  boolean reader532_setup(ESP8266RevK&revk)
  {
    if (!reader532)return false; // Not configured
    debugf("GPIO pin available %X for PN532", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      reader532_fault = PSTR("Reader532 pins (SPI) not available");
      reader532 = NULL;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X", gpiomap);
    SPI.begin();
    nfc.begin();
    SPI.setFrequency(100000);
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata)
    { // no reader
      reader532_fault = PSTR("PN532 failed");
      reader532 = NULL;
      return false;
    }
    nfc.setPassiveActivationRetries(1);
    nfc.SAMConfig();

    debug("PN532 OK");
    reader532ok = true;
    return true;
  }

#define MAX_UID 7
  static char tid[MAX_UID * 2 + 1] = {}; // text ID

  boolean reader532_loop(ESP8266RevK&revk, boolean force)
  {
    if (!reader532ok)return false; // Not configured
    long now = (millis() ? : 1); // Allowing for wrap, and using 0 to mean not set
    static long cardcheck = 0;
    static boolean held = false;
    if ((int)(cardcheck - now) < 0)
    {
      cardcheck = now + readerpoll;
      static long found = 0;
      if (found)
      {
        // TODO MIFARE 4 byte ID dont show as connected, and constantly show read target, grrr.
        if (!nfc.diagnose6(readertimeout) || nfc.inListPassiveTarget())
        { // still here
          if (!held && (int)(now - found) > holdtime)
          {
#ifdef USE_OUTPUT
            if (fallback && !strcmp(fallback, tid))
              output_safe_set(true);
#endif
            revk.event(F("held"), F("%s"), tid); // Previous card gone
            held = 1;
          }
        } else
        { // gone
          if (held)
          {
#ifdef USE_OUTPUT
            if (fallback && !strcmp(fallback, tid))
              output_safe_set(false);
#endif
            revk.event(F("gone"), F("%s"), tid); // Previous card gone
            held = false;
          }
          found = 0;
        }
      } else {
        byte uid[MAX_UID] = {}, uidlen = 0;
        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidlen, readertimeout))
        {
          found = (now ? : 1);
          int n;
          for (n = 0; n < uidlen && n * 2 < sizeof(tid); n++)sprintf_P(tid + n * 2, PSTR("%02X"), uid[n]);
          revk.event(F("id"), F("%s"), tid);
        }
      }
    }
    return true;
  }
