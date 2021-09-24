// NFC operation (door control)
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#include "pn532.h"
extern pn532_t *pn532;
extern df_t df;
extern SemaphoreHandle_t nfc_mutex;

extern const char *nfc_led(int len, const void *value);
extern void nfc_retry(void);

typedef struct {
   char id[22];                 // Initial card ID (insecure), hex null terminated
   char name[16];               // Name from fob
   char sms[21];		// Number on fob
   const char *fail;            // NFC fail message
   const char *deny;            // Door system entry deny message
   uint32_t crc;                // Afile CRC
   area_t enter;                // Enter allow areas
   area_t arm;                  // Arm allow areas
   area_t strongarm;            // Force arm allow areas
   area_t disarm;               // Disarm allow areas
   area_t prop;                 // Prop allowed
   uint8_t aesid;               // The AES key index used
   uint8_t ver;                 // AES key version on fob
   uint8_t verset:1;            // Ver is set
   uint8_t enterset:1;          // Afile has enter setting
   uint8_t propset:1;           // Afile has prop setting
   uint8_t nameset:1;           // Afile has name set
   uint8_t smsset:1;           // Afile has sms set
   uint8_t armset:1;            // Afile has arm setting
   uint8_t strongarmset:1;      // Afile has strongarm setting
   uint8_t disarmset:1;         // Afile has disarm setting
   uint8_t iso:1;               // Fob looks like an ISO card
   uint8_t secureset:1;         // If secure is set
   uint8_t secure:1;            // Set if DESFire, AID selected, AES done, and real UID retrieved
   uint8_t held:1;              // Card was held
   uint8_t longheld:1;          // Card was held even more
   uint8_t gone:1;              // Card has gone
   uint8_t keyupdated:1;        // AES key successfully updated
   uint8_t updated:1;           // Afile successfully updated
   uint8_t checked:1;           // Door system has checked the card
   uint8_t fallback:1;          // Card matches fallback list
   uint8_t blacklist:1;         // Card matches blacklist list
   uint8_t afile:1;             // Afile present (crc is set) and access permissions checked (deny is set)
   uint8_t allowset:1;          // Afile has allow setting
   uint8_t armlate:1;           // Afile says late arming allowed
   uint8_t override:1;          // Afile says block card
   uint8_t block:1;             // Afile says block card
   uint8_t commit:1;            // Afile says commit before opening
   uint8_t clock:1;             // Afile says time ignore if not set
   uint8_t count:1;             // Afile says update counter
   uint8_t counted:1;           // Card was successfully logged
   uint8_t log:1;               // Afile says update log
   uint8_t logged:1;            // Card was successfully logged
   uint8_t unlocked:1;          // Door was unlocked
   uint8_t propped:1;           // Door was propped
   uint8_t unlockok:1;          // Door would be unlocked
   uint8_t enterok:1;           // Afile checks say door can be entered
   uint8_t propok:1;            // Afile checks say door can be proped
   uint8_t disarmok:1;          // Afile checks say system can be disarmed
   uint8_t disarmed:1;          // Disarm was done
   uint8_t armed:1;             // Arm was done
   uint8_t strongarmed:1;       // Strong arm was done
   uint8_t armok:1;             // Afile checks say system can be armed
   uint8_t strongarmok:1;       // Afile checks say system can be force armed
   uint8_t remote:1;            // Normal working overridden for remote NFC
   uint8_t recheck:1;           // Afile change
} fob_t;
