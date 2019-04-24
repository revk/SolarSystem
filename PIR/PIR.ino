// PIR or laser sensor, TBA
// ESP-01 based with sensor board


#include <ESP8266RevK.h>

ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__);

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
 // TODO
}

void loop()
{
  revk.loop();
  long now = millis();
 // TODO
}
