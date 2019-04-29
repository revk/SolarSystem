// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System using RC522 via SPI

#define  USE_READER522
extern const char* reader522fault;

#ifndef	HOLDTIME
#define HOLDTIME 3000
#endif

#ifndef	REALASETIME
#define RELEASETIME 1000
#endif

#include <ESP8266RevK.h>

extern const char * reader522_setting(const char *tag, const byte *value, size_t len);
extern boolean reader522_command(const char*tag, const byte *message, size_t len);
extern boolean reader522_setup(ESP8266RevK&);
extern boolean reader522_loop(ESP8266RevK&, boolean);
