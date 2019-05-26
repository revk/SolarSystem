// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

#define	USE_KEYPAD
extern const char* keypad_fault;
extern const char* keypad_tamper;

const char* keypad_setting(const char *tag, const byte *value, size_t len);
boolean keypad_command(const char*tag, const byte *message, size_t len);
boolean keypad_setup(ESPRevK &revk);
boolean keypad_loop(ESPRevK &revk, boolean);
