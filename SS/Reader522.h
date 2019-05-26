// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System using RC522 via SPI

#define  USE_READER522
extern const char* reader522_fault;
extern const char* reader522_tamper;

#include <ESPRevK.h>

extern const char* reader522_setting(const char *tag, const byte *value, size_t len);
extern boolean reader522_command(const char*tag, const byte *message, size_t len);
extern boolean reader522_setup(ESPRevK&);
extern boolean reader522_loop(ESPRevK&, boolean);
