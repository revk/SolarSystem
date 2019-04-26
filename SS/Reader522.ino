// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System
// Functions for RC522 via SPI

// GPIO2  RST (not actually needed)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

#include <ESP8266RevK.h>
#include "Reader522.h"
#include <SPI.h>
#include <MFRC522.h>

#define RST 2 // SPI
#define SS 16 // SPI
MFRC522 rfid(SS, RST); // Instance of the class

boolean reader522_setting(const char *setting, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return false; // Done
}

boolean reader522_cmnd(const char*suffix, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

void reader522_setup(ESP8266RevK&revk)
{
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void reader522_loop(ESP8266RevK&revk)
{
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
          revk.stat("gone", "%02X%02X%02X%02X", id[0], id[1], id[2], id[3]); // Edge case of change card after hold before release time
        first = now;
        held = false;
        memcpy(id, rfid.uid.uidByte, 4);
        revk.stat("id", "%02X%02X%02X%02X", id[0], id[1], id[2], id[3]);
      } else if (!held && first && (int)(now - first) > HOLDTIME)
      {
        held = true;
        revk.stat( "held", "%02X%02X%02X%02X", id[0], id[1], id[2], id[3]);
      }
    } else if (last && (int)(now - last) > RELEASETIME)
    {
      if (held)
        revk.stat("gone", "%02X%02X%02X%02X", id[0], id[1], id[2], id[3]);
      first = 0;
      last = 0;
      held = false;
    }
  }
}
