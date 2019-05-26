// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Laser ranger as button

#define	USE_RANGER0X
extern const char* ranger0x_fault;
extern const char* ranger0x_tamper;

#include <ESPRevK.h>

extern const char * ranger0x_setting(const char *tag, const byte *value, size_t len);
extern boolean ranger0x_command(const char*tag, const byte *message, size_t len);
extern boolean ranger0x_setup(ESPRevK&);
extern boolean ranger0x_loop(ESPRevK&, boolean);
