// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System using PN532 via SPI

#ifndef	HOLDTIME
#define HOLDTIME 3000
#endif

#ifndef	REALASETIME
#define RELEASETIME 1000
#endif

#define	USE_READER532

#include <ESP8266RevK.h>

extern boolean reader532_setting(const char *setting, const byte *value, size_t len);
extern boolean reader532_cmnd(const char*suffix, const byte *message, size_t len);
extern void reader532_setup(ESP8266RevK&);
extern void reader532_loop(ESP8266RevK&);
