#ifndef __PN532RevK_H__
#define __PN532RevK_H__

#include <PN532.h>     // Elechouse library

class PN532RevK : public PN532 {
  public:
    PN532RevK(PN532Interface &interface): PN532(interface) {
      _interface = &interface;
    };
    uint8_t getGeneralStatus(int timeout = 0); // Returns number of cards
    uint8_t diagnose6(int timeout = 0); // Test 6 is card presence detection (0=OK/present)
  private:
    PN532Interface *_interface;
};

#endif
