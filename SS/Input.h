// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Input button logic

#define USE_INPUT
extern const char* inputfault;

#include <ESP8266RevK.h>

extern const char * input_setting(const char *tag, const byte *value, size_t len);
extern boolean input_command(const char*tag, const byte *message, size_t len);
extern boolean input_setup(ESP8266RevK&);
extern boolean input_loop(ESP8266RevK&, boolean);
