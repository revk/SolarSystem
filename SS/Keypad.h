// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

#define	USE_KEYPAD
extern boolean keypadfault;

const char* keypad_setting(const char *tag, const byte *value, size_t len);
boolean keypad_command(const char*tag, const byte *message, size_t len);
boolean keypad_setup(ESP8266RevK &revk);
boolean keypad_loop(ESP8266RevK &revk, boolean);
