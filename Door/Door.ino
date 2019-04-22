// Door control
// ESP-01 based with serial controlled relay board

#include <ESP8266RevK.h>

ESP8266RevK revk(__FILE__, "Build: " __DATE__ " " __TIME__);

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
 Serial.begin(9600);	// Relay board uses 9600
 // TODO
}

void loop()
{
  revk.loop();
  long now = millis();
 // TODO
}
