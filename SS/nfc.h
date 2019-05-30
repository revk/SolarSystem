// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System using PN532 via SPI

#define  USE_NFC
extern const char* nfc_fault;
extern const char* nfc_tamper;

#include <ESPRevK.h>

extern const char* nfc_setting(const char *tag, const byte *value, size_t len);
extern boolean nfc_command(const char*tag, const byte *message, size_t len);
extern boolean nfc_setup(ESPRevK&);
extern boolean nfc_loop(ESPRevK&, boolean);
