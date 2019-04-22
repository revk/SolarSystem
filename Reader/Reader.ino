// RFID Card reader for Solar System
// ESP-12F based for use with MFRC522 to send on card read

// Witing for ESP-12F
// RC522 connnections (in addition to GND/3V3)
// GPIO2  RST (not actually needed)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

#include <ESP8266RevK.h>

ESP8266RevK revk(__FILE__, "Build: " __DATE__ " " __TIME__);

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
  long now = millis();
  static long cardcheck = 0;
  if ((int)(cardcheck - now) < 0)
  {
    cardcheck = now + 10;
    if (rfid.PICC_IsNewCardPresent())
    {
      if (rfid.PICC_ReadCardSerial())
      {
        MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
        revk.pub("stat", "id", "%02X%02X%02X%02X", rfid.uid.uidByte[0], rfid.uid.uidByte[1], rfid.uid.uidByte[2], rfid.uid.uidByte[3]);
	// TODO report once for press and separately for held
      }
    }
  }
}
