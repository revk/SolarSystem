// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Output button logic

#define USE_OUTPUT
extern const char* output_fault;

#include <ESP8266RevK.h>

extern const char * output_setting(const char *tag, const byte *value, size_t len);
extern boolean output_command(const char*tag, const byte *message, size_t len);
extern boolean output_setup(ESP8266RevK&);
extern boolean output_loop(ESP8266RevK&, boolean);
extern void output_safe_set(boolean);
