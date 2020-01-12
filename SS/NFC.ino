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
byte outputs = 0;

#define app_settings  \
  s(nfc);   \
  s(nfccommit); \
  v(nfcred,1); \
  v(nfcgreen,0); \
  v(nfctamper,3); \
  v(nfcpoll,50); \
  h(nfcbus,0xFF); \
  l(aes); \
  l(aid); \

#define s(n) const char *n=NULL
#define v(n,d) int n=d
#define h(n,d) byte n=d;
#define l(n) const byte *n=NULL; int n##_len=0;
  app_settings
#undef l
#undef h
#undef s
#undef v

#define readertimeout 100

  void NFC_led(int len, const char *led)
  {
    if (len > sizeof(ledpattern))len = sizeof(ledpattern);
    if (len)
      memcpy((void*)ledpattern, led, len);
    if (len < sizeof(ledpattern))
      ledpattern[len] = 0;
  }

  const char* NFC_setting(const char *tag, const byte *value, size_t len)
  { // Called for settings retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
#define v(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(value?atoi((char*)value):d);return t;}}while(0)
#define h(n,d) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){if(len==2)n=(((value[0]&15)+(value[0]>='A'?9:0))<<4)+((value[1]&15)+(value[1]>='A'?9:0));else n=d; return t;}}while(0)
#define l(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=value;n##_len=len;return t;}}while(0)
    app_settings
#undef l
#undef h
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
    if (!strcasecmp_P(tag, PSTR("NFC")))
    {
      held = true; // Stops normal held and door checks, but sends gone so reader app knows card removed
      if (len)
      { // Send data
        byte res[100], rlen = sizeof(res);
        byte bad = NFC.data(len, (byte*)message, rlen, res, 2000);
        if (!bad && rlen)
          revk.info(F("nfc"), rlen - 1, res + 1);
        else revk.error(F("nfc"), F("failed %02X (%d bytes sent %02X %02X %02X...)"), bad, len, message[0], message[1], message[2]);
      }
      return true;
    }
    if (!strcasecmp_P(tag, "led"))
    { // Sequence of LED colours (R/G/-) to repeat
      NFC_led(len, (const char*)message);
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
    if (nfcbus != 0xFF)
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
    if (nfcred >= 0)outputs |= (1 << nfcred);
    if (nfcgreen >= 0)outputs |= (1 << nfcgreen);
    if (!NFC.begin(outputs))
    { // no reader
      NFC_fault = PSTR("PN532 failed");
      nfc = NULL;
      return false;
    }
    if (nfcbus != 0xFF)
    {
      bus.SetPins(busde, bustx, busrx);
      bus.SetAddress(nfcbus, true);
      bus.Start();
    }
    debug("PN532 OK");
    nfcok = true;
    *ledpattern = 0;
    if (aid_len == 3)NFC.set_aid(aid);
    if (aes_len == 16)NFC.set_aes(aes);
    return true;
  }

  char tid[100]; // ID
  byte bid[10]; // Binary ID

  boolean NFC_loop(ESPRevK&revk, boolean force)
  {
    if (!nfcok)return false; // Not configure
    int32_t waitres = NFC.waiting();
    if (waitres && !NFC.available() && waitres < 100) return true; // Waiting response
    static long lednext = 0;
    static byte ledlast = 0xFF;
    static byte ledpos = 0;
    long now = (millis() ? : 1); // Allowing for wrap, and using 0 to mean not set
    if (!waitres && nfcpoll < 1000)
    { // Normal working, setting poll to >=1s means test/debug so only do the polling not the led / tamper
      if ((int)(lednext - now) <= 0)
      { // LED
        // Note this is simply a pattern or R, G, or - now, at 100ms
        // Plan to allow digits to control timing in multiples of 100ms, and also allow R+G, etc, combinations, and maybe R, A, G LEDs.
        lednext += 100;
        ledpos++;
        if (ledpos >= sizeof(ledpattern) || !ledpattern[ledpos] || !*ledpattern)ledpos = 0;
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
        if (p3 < 0 && !NFC.begin(outputs))
          NFC_fault = PSTR("PN532");
        else
        { // INT1 connected via switch to VCC, so expected high
          if (NFC_fault && NFC.begin(outputs))
          { // Reset
            ledlast = 0xFF;
            NFC_fault = NULL;
          }
          if (nfctamper < 0 || (p3 & (1 << nfctamper)))
            NFC_tamper = NULL;
          else
            NFC_tamper = PSTR("PN532");
        }
      }
    }
    static byte ids = 0;
    static long found = 0;
    static long cardcheck = 0;
    static const char *noaccess = NULL;
    if ((int)(cardcheck - now) < 0 || waitres)
    {
      cardcheck = now + nfcpoll;
      String id, err;
      if (found)
      {
        // TODO MIFARE Classic 4 byte ID don't show as in field, and constantly re-read ID, grrr.
        if (!NFC.inField(readertimeout) || (!NFC.secure && (ids = NFC.getID(id, err, 100, bid)) && !strcmp(id.c_str(), tid)))
        { // still here
          if (!held && (int)(now - found) > holdtime)
          {
            if (door >= 5 && !noaccess)Door_deadlock(NULL); // Deadlock mode
            revk.event(F("held"), F("%s"), tid); // Previous card gone
            held = true;
          }
        } else
        { // gone
          if (held)
          {
            revk.event(F("gone"), F("%s"), tid); // Previous card gone
            held = false;
          }
          found = 0;
        }
      } else if (!waitres)
        NFC.ILPT(); // Do an InlistPassive Target
      else
      {
        if ((ids = NFC.getID(id, err, 100, bid)))
        { // Got ID
          found = (now ? : 1);
          if (ids > 1)
            strncpy_P(tid, PSTR("Multiple"), sizeof(tid)); // Multiple ID
          else
            strncpy(tid, id.c_str(), sizeof(tid));
          if (held)
          { // Preset for NFC reader remote
            revk.event( F("card"), F("%s"), tid); // Report access
            err = String("");
            held = false; // Normal
          }
          else
          {
            noaccess = Door_fob(tid, err); // Check door control
            if (nfccommit && NFC.secure && !*err.c_str())NFC.desfire_log(err); // Log before action
            if (!noaccess && NFC.nfcstatus == 0x01)noaccess = PSTR("NFC timeout");
            if (NFC.nfcstatus)
            {
              NFC.led(ledlast = (nfcred >= 0 && !(ledlast & (1 << nfcred)) ? (1 << nfcred) : 0)); // Blink red
              lednext = now + 200;
              revk.event(F("nfcfail"), strcmp_P("", noaccess) ? F("%s %02X %S") : F("%s %02X"), tid, NFC.nfcstatus, noaccess); // NFC access was not clean
            }
            else if (!noaccess)
            { // Autonomous door control
              NFC.led(ledlast = (nfcgreen >= 0 && !(ledlast & (1 << nfcgreen)) ? (1 << nfcgreen) : 0)); // Blink green
              lednext = now + 200;
              Door_unlock(NULL); // Door system was happy with fob, let 'em in
              revk.event( F("access"), F("%s"), tid); // Report access
              // TODO logging access when off line?
            } else
            { // Report to control as access not allowed

              NFC.led(ledlast = (nfcred >= 0 && !(ledlast & (1 << nfcred)) ? (1 << nfcred) : 0)); // Blink red
              lednext = now + 200;
              if (strcmp_P("", noaccess))
                revk.event(F("noaccess"), F("%s %02X %02X %S"), tid, NFC.nfcstatus, NFC.desfirestatus, noaccess); // ID and reason why not autonomous
              else
              {
                if (*NFC.atr && NFC.atr[1] == 0x75)revk.event(F("id"), F("%s DESFire"), tid); // ID
                else if (*NFC.atr && NFC.atr[1] == 0x78)revk.event(F("id"), F("%s ISO"), tid); // ID
                else if (*NFC.atr)revk.event(F("id"), F("%s ATR%02X"), tid, NFC.atr[1]); // ID
                else revk.event(F("id"), F("%s"), tid); // ID
                //if (*NFC.atr) revk.event(F("atr"), F("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"), NFC.atr[0], NFC.atr[1], NFC.atr[2], NFC.atr[3], NFC.atr[4], NFC.atr[5], NFC.atr[6], NFC.atr[7], NFC.atr[8], NFC.atr[9], NFC.atr[10], NFC.atr[11], NFC.atr[12], NFC.atr[13], NFC.atr[14], NFC.atr[15], NFC.atr[16], NFC.atr[17], NFC.atr[18], NFC.atr[19]); // ID
              }
            }
            if (!nfccommit && NFC.secure && !*err.c_str())NFC.desfire_log(err); // Log after action
          }
          if (*err.c_str())
          { // Report any error
            revk.error(F("id"), F("%02X %02X %s"), NFC.nfcstatus, NFC.desfirestatus, err.c_str());
            found = 0;
          }
        }
      }
    }
    if (nfcbus != 0xFF)
    { // Pretending to be a max reader
      if (bus.Available())
      {
        byte buf[RS485MAX];
        int l = bus.Rx(sizeof(buf), buf);
        //revk.event(F("Rx"), F("%d %02X %02X %02X"), l, buf[0], buf[1], buf[2]);
        if (l >= 2)
        {
          if (buf[1] == 0x0E)
          { // Init response (what a max sends)
            byte buf[5];
            buf[0] = 0x11;
            buf[1] = 0xFF;
            buf[2] = 0x00;
            buf[3] = 0x00;
            buf[4] = 0xB1;
            rs485.Tx(5, buf);
          } else
          { // Status response
            if (buf[1] == 0x0C && l >= 3)
            { // Output
              static byte last = 0;
              if ((last ^ buf[2]) & 0x02)Output_set(1, buf[2] & 0x02);
              if ((last ^ buf[2]) & 0x04)Output_set(2, buf[2] & 0x04);
              last = buf[2];
            }
            else if (buf[1] == 0x07 && l >= 3)
            { // LED (may need some work)
              char *p = ledpattern;
              if (buf[2] & (1 << 5))*p++ = '-';
              if (buf[2] & (1 << 4))*p++ = 'R';
              if (buf[2] & (1 << 3))*p++ = 'R';
              if (buf[2] & (1 << 2))*p++ = 'R';
              if (buf[2] & (1 << 1))*p++ = 'R';
              if (buf[2] & (1 << 0))*p++ = 'G';
              *p = 0;
            }
            // Status response
            buf[0] = 0x11;
            buf[1] = (NFC_tamper ? 0xFC : 0xF4);
            buf[2] = (Input_get(1) ? 0x20 : 0) + (Input_get(2) ? 0x10 : 0) + (Input_get(3) ? 0x40 : 0); // Exit, Close, and an extra one for lock engaged
            //revk.event(F("Tx"), F("%02X %02X %02X"), buf[0], buf[1], buf[2]);
            if (found)
            { // Make an ID (8 digit BCD coded)
              unsigned int p, v = 0;;
              for (p = 0; p < 7; p++)v ^= (bid[p] << ((p & 3) * 8));
              v &= ~0x80000001;
              if (NFC.secure)v |= 0x80000001;
              buf[3] = ((v / 10000000) % 10) * 16 + ((v / 1000000) % 10);
              buf[4] = ((v / 100000) % 10) * 16 + ((v / 10000) % 10);
              buf[5] = ((v / 1000) % 10) * 16 + ((v / 100) % 10);
              buf[6] = ((v / 10) % 10) * 16 + ((v / 1) % 10);
              rs485.Tx(7, buf);
            } else rs485.Tx(3, buf);
          }
        }
      }
    }
    return true;
  }
