// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Output button logic

#define USE_OUTPUT
extern const char* output_fault;

#include <ESPRevK.h>

extern const char * output_setting(const char *tag, const byte *value, size_t len);
extern boolean output_command(const char*tag, const byte *message, size_t len);
extern boolean output_setup(ESPRevK&);
extern boolean output_loop(ESPRevK&, boolean);
extern void output_safe_set(boolean);
