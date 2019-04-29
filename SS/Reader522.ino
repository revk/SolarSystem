// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System
// Functions for RC522 via SPI

// GPIO2  RST (not actually needed)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

#define PINS  ((1<<rst) | (1 << 12) | (1 << 13) | (1 << 14) | (1 << ss))

#include <ESP8266RevK.h>
#include "Reader522.h"
#include <SPI.h>
#include <MFRC522.h>

#define app_settings  \
  s(reader522);   \

#define s(n) const char *n=NULL
  app_settings
#undef s

  MFRC522 rfid(ss, rst); // Instance of the class
  boolean reader522ok = false;
  const char* reader522_fault = false;

  const char* reader522_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
    app_settings
#undef s
    return NULL; // Done
  }

  boolean reader522_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!reader522ok)return false; // Not configured
    return false;
  }

  boolean reader522_setup(ESP8266RevK&revk)
  {
    if (!reader522)return false; // Not configured
    debugf("GPIO pin available %X for RC522", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      reader522_fault = PSTR("Reader522 pins (SPI) not available");
      reader522 = NULL;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X", gpiomap);
    SPI.begin(); // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
    int v = rfid.PCD_ReadRegister(rfid.VersionReg);
    if (!v || v == 0xFF)
    { // Failed
      reader522_fault = PSTR("RC522 failed");
      reader522 = NULL;
      return false;
    }
    debug("RC522 OK");
    reader522ok = true;
    return true;
  }

  boolean reader522_loop(ESP8266RevK&revk)
  {
    if (!reader522ok)return false; // Not configured
    long now = (millis() ? : 1); // Allowing for wrap, and using 0 to mean not set
    static long cardcheck = 0;
    if ((int)(cardcheck - now) < 0)
    {
      cardcheck = now + 100;
      static byte id[4] = {};
      static long first = 0;
      static long last = 0;
      static boolean held = false;
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
      {
        last = now;
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
        if (!first || memcmp(id, rfid.uid.uidByte, 4))
        {
          if (held)
            revk.event(F("gone"), F("%02X%02X%02X%02X"), id[0], id[1], id[2], id[3]); // Edge case of change card after hold before release time
          first = now;
          held = false;
          memcpy(id, rfid.uid.uidByte, 4);
          revk.event(F("id"), F("%02X%02X%02X%02X"), id[0], id[1], id[2], id[3]);
        } else if (!held && first && (int)(now - first) > holdtime)
        {
          held = true;
          revk.event(F("held"), F("%02X%02X%02X%02X"), id[0], id[1], id[2], id[3]);
        }
      } else if (last && (int)(now - last) > releasetime)
      {
        if (held)
          revk.state(F("gone"), F("%02X%02X%02X%02X"), id[0], id[1], id[2], id[3]);
        first = 0;
        last = 0;
        held = false;
      }
    }
    return true;
  }
