// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#define	USE_RANGER1X
extern const char* ranger1x_fault;
extern const char* ranger1x_tamper;

#include <ESPRevK.h>

extern const char * ranger1x_setting(const char *tag, const byte *value, size_t len);
extern boolean ranger1x_command(const char*tag, const byte *message, size_t len);
extern boolean ranger1x_setup(ESPRevK&);
extern boolean ranger1x_loop(ESPRevK&);
