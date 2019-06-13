// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// RFID Card reader for Solar System
// ESP-12F based for use with PN532

// Wiring for ESP-12F
// PN532 connnections (in addition to GND/3V3)
// GPIO13 MOSI
// GPIO12 MISO
// GPIO14 SCK (CLK)
// GPIO16 SDA (SS)

#include <ESPRevK.h>
#include "PN532_SPI.h"
#include "PN532_HSU.h"
#include "PN532RevK.h"
#include "RS485.h"

RS485 bus(0x11, false);

extern void Output_set(int, boolean);
extern boolean Input_get(int);

PN532_SPI pn532spi(SPI, ss);
PN532_HSU pn532hsu(Serial);
PN532RevK NFC(pn532spi);
boolean nfcok = false;
const char* NFC_fault = NULL;
const char* NFC_tamper = NULL;
boolean held = false;
char ledpattern[10];

#define app_settings  \
  s(nfc);   \
  s(nfccommit); \
  v(nfcred,1); \
  v(nfcgreen,0); \
  v(nfctamper,3); \
  v(nfcmax,-1); \

#define s(n) const char *n=NULL
#define v(n,d) int8_t n=d
  app_settings
#undef s
#undef v

#define readertimeout 100

  const char* NFC_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
#define v(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
    app_settings
#undef s
#undef v
    return NULL; // Done
  }

  boolean NFC_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!nfcok)
      return false; // Not configured
    if (!strcasecmp_P(tag, PSTR("AID")) && len == 3)
    {
      NFC.set_aid(message);
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("AES")) && len == 16)
    {
      NFC.set_aes(message);
      return true;
    }
    if (!strcasecmp_P(tag, PSTR("NFC")) && len)
    {
      held = true; // Stops normal held, but sends gone so reader app knows card removed
      byte res[100], rlen = sizeof(res);
      byte bad = NFC.data(len, (byte*)message, rlen, res);
      if (!bad && rlen)
        revk.info(F("nfc"), rlen - 1, res + 1);
      else revk.error(F("nfc"), F("failed %02X (%d bytes sent %02X %02X %02X...)"), bad, len, message[0], message[1], message[2]);
      return true;
    }
    if (!strcasecmp_P(tag, "led") && len < sizeof(ledpattern))
    { // Sequence of LED colours (R/G/-) to repeat
      if (len)
        memcpy((void*)ledpattern, (void*) message, len);
      if (len < sizeof(ledpattern))
        ledpattern[len] = 0;
      return true;
    }
    return false;
  }

  boolean NFC_setup(ESPRevK&revk)
  {
    if (!nfc)return false; // Not configured
    if (*nfc != 'H' && ss < 0)
    {
      NFC_fault = PSTR("Define SS for SPI");
      nfc = NULL;
      return false;
    }
    unsigned int pins = ((1 << 12) | (1 << 13) | (1 << 14) | (1 << ss)); // SPI
    if (*nfc == 'H')pins = ((1 << 1) | (1 << 3)); // HSU
    if (nfcmax >= 0)
    {
      if (bustx < 0 || busrx < 0 || busde < 0)
      {
        Keypad_fault = PSTR("Define bustx/busrs/busde pins");
        keypad = NULL;
        return false;
      }
      pins |= ((1 << bustx) | (1 << busrx) | (1 << busde));
    }
    debugf("GPIO pin available %X for PN532", gpiomap);
    if ((gpiomap & pins) != pins)
    {
      NFC_fault = PSTR("SPI not available");
      nfc = NULL;
      return false;
    }
    gpiomap &= ~pins;
    debugf("GPIO remaining %X", gpiomap);
    if (*nfc == 'H')
    { // HSU mode rather than SPI
#ifdef REVKDEBUG
      NFC_fault = PSTR("Cannot do serial debug and serial PN532");
      nfc = NULL;
      return false;
#endif
      NFC.set_interface(pn532hsu);
    }
    byte outputs = 0;
    if (nfcred >= 0)outputs |= (1 << nfcred);
    if (nfcgreen >= 0)outputs |= (1 << nfcgreen);
    if (!NFC.begin(outputs))
    { // no reader
      NFC_fault = PSTR("PN532 failed");
      nfc = NULL;
      return false;
    }
    debug("PN532 OK");
    nfcok = true;
    *ledpattern = 0;
    return true;
  }

  char tid[100]; // ID

  boolean NFC_loop(ESPRevK&revk, boolean force)
  {
    if (!nfcok)return false; // Not configured
    long now = (millis() ? : 1); // Allowing for wrap, and using 0 to mean not set
    static long lednext = 0;
    static byte ledlast = 0xFF;
    static byte ledpos = 0;
    if ((int)(lednext - now) <= 0)
    {
      lednext += 100;
      ledpos++;
      if (ledpos >= sizeof(ledpattern) || !ledpattern[ledpos])ledpos = 0;
      byte newled = 0;
      // We are assuming exactly two LEDs, one at a time (back to back) on P30 and P31
      if (nfcred >= 0 && ledpattern[ledpos] == 'R')newled = (1 << nfcred);
      if (nfcgreen >= 0 && ledpattern[ledpos] == 'G')newled = (1 << nfcgreen);
      if (newled != ledlast)
        NFC.led(ledlast = newled);
    }
    static long tampercheck = 0;
    if ((int)(tampercheck - now) <= 0)
    {
      tampercheck = now + 250;
      int p3 = NFC.p3();
      if (p3 < 0)
        NFC_fault = PSTR("PN532");
      else
      { // INT1 connected via switch to VCC, so expected high
        if (NFC_fault)
        {
          NFC.begin();
          ledlast = 0xFF;
        }
        NFC_fault = NULL;
        if (nfctamper < 0 || (p3 & (1 << nfctamper)))
          NFC_tamper = NULL;
        else
          NFC_tamper = PSTR("PN532");
      }
    }
    static long cardcheck = 0;
    if ((int)(cardcheck - now) < 0)
    {
      cardcheck = now + readerpoll;
      static long found = 0;
      String id, err;
      if (found)
      {
        // TODO MIFARE Classic 4 byte ID don't show as in field, and constantly re-read ID, grrr.
        if (!NFC.inField(readertimeout) || (!NFC.secure && NFC.getID(id, err) && !strcmp(id.c_str(), tid)))
        { // still here
          if (!held && (int)(now - found) > holdtime)
          {
#ifdef USE_OUTPUT
            if (fallback && !strcmp(fallback, tid) && (!NFC.aidset || NFC.secure))
              output_safe_set(true);
#endif
            revk.event(F("held"), F("%s"), tid); // Previous card gone
            held = 1;
          }
        } else
        { // gone
          if (held)
          {
#ifdef USE_OUTPUT
            if (fallback && !strcmp(fallback, tid))
              output_safe_set(false);
#endif
            revk.event(F("gone"), F("%s"), tid); // Previous card gone
            held = false;
          }
          found = 0;
        }
      } else {
        if (NFC.getID(id, err))
        {
          found = (now ? : 1);
          strncpy(tid, id.c_str(), sizeof(tid));
          if (nfccommit && NFC.secure)
          { // Log and commit first, and ensure commit worked, before sending ID - this is noticably slower, so optional and not default
            if (NFC.desfire_log(err) >= 0)
              revk.event(F("id"), F("%s"), tid);
          } else
          { // Send ID first, then log to card, quicker, but could mean an access is not logged on the card if removed quickly enough
            revk.event(F("id"), F("%s"), tid);
            if (NFC.secure)
              NFC.desfire_log(err);
          }
        }
        if (*err.c_str())
        {
          revk.error(F("id"), F("%s"), err.c_str());
          found = 0;
        }
      }
    }
    if (nfcmax >= 0)
    { // Pretending to be a max reader
      if (bus.Available())
      {
        byte buf[RS485MAX];
        int l = bus.Rx(sizeof(buf), buf);
        if (l >= 2)
        {
          if (buf[1] == 0x0E)
          { // Init response
            // TODO
          } else
          { // Status response
            // TODO
          }
        }
      }
    }
    return true;
  }
