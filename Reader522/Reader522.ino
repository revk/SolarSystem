// RFID Card reader for Solar System
// ESP-12F based for use with MFRC522 to send on card read

// Witing for ESP-12F
// RC522 connnections (in addition to GND/3V3)
// GPIO2  RST (not actually needed)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

// TODO
// HoldTime as a setting
// More checks on MiFare card validity (key as a setting)

#define HOLDTIME 3000
#define RELEASETIME 1000

#include <ESP8266RevK.h>

ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__);

#define RST 2 // SPI
#define SS 16 // SPI

#include <SPI.h>
#include <MFRC522.h>
MFRC522 rfid(SS, RST); // Instance of the class

boolean app_setting(const char *setting, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return false; // Done
}

boolean app_cmnd(const char*suffix, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

void setup()
{
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
}

void loop()
{
  revk.loop();
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
