// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Keypad / display
// ESP-01 based with RS485 board fits in Galaxy keypad/display module

#include "keypad.h"
const char* keypad_fault = NULL;
const char* keypad_tamper = NULL;

#define PINS ((1<<1) | (1<<3))  // Tx and Rx

#define PRETX 50000  // Pre tx RTS - should overlap with end of drive from keypad
#define POSTTX 4000  // Post tx RTS - should overlap with start of drive from keypoad
#define PRERX 10000  // Time to allow for rx
#define KEYPADBAUD  9600
#define KEYPADBITS  10  // 8N1

#define KEYPADBODGE // Don't rely on poll being called fast enough (TODO handle with interrupts?)

#include <ESPRevK.h>

#define RTS 2

#define app_commands  \
  f(07,display,32,0) \
  f(19,keyclick,1,5) \
  f(0C,sounder,2,0) \
  f(0D,backlight,1,1) \
  f(07a,cursor,2,0) \
  f(07b,blink,1,0) \

#define app_settings  \
  s(keypad);   \
  n(keypaddebug); \

#define f(id,name,len,def) static byte name[len]={def};boolean send##id=false;byte name##_len=0;
  app_commands
#undef  f

#define s(n) const char *n=NULL
#define n(n) int n=0;
  app_settings
#undef n
#undef s

  static int keypadbeepoverride = 0;

  const char* keypad_setting(const char *tag, const byte *value, size_t len)
  { // Called for commands retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
#define n(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=value?atoi((char*)value):0;return t;}}while(0)
    app_settings
#undef n
#undef s
    return NULL; // Done
  }

  boolean keypad_command(const char*tag, const byte *message, size_t len)
  { // Called for incoming MQTT messages
    if (!keypad)return false; // Not running keypad
#define f(i,n,l,d) if(!strcasecmp_P(tag,PSTR(#n))&&len<=l){memcpy(n,message,len);n##_len=len;if(len<l)memset(n+len,0,l-len);send##i=true;return true;}
    app_commands
#undef f
    if (!strcasecmp_P(tag, PSTR( "output8")))
    {
      keypadbeepoverride = ((len && *message == '1') ? 1 : 0);
      send0C = true;
      return true;
    }
    return false;
  }

  boolean keypad_setup(ESPRevK &revk)
  {
    if (!keypad)return false; // Not running keypad
    debugf("GPIO pin available %X for Keypad", gpiomap);
    if ((gpiomap & PINS) != PINS)
    {
      keypad_fault = PSTR("Keypad pins (Tx/Rx) not available");
      keypad = NULL;
      return false;
    }
    gpiomap &= ~PINS;
    debugf("GPIO remaining %X", gpiomap);
#ifndef REVKDEBUG
    Serial.begin(KEYPADBAUD);	// Galaxy uses 9600 8N2
#endif
    digitalWrite(RTS, LOW);
    pinMode(RTS, OUTPUT);
    debug("Keypad OK");
    return true;
  }

  boolean keypad_loop(ESPRevK &revk, boolean force)
  {
    if (!keypad)return false; // Not running keypad
    long now = micros();

    static byte buf[100], p = 0;
    static long txdone = 0;
    static long rxdone = 0;
    static byte cmd = 0;
    static boolean online = false;
    static boolean send0B = false;
    static boolean toggle0B = false;
    static boolean toggle07 = false;
    static boolean send07c = false; // second send
    static byte lastkey = 0x7F;
    static boolean sounderack = false;

    // Poll
    if (force || !online)
    { // Update all the shit
      send07 = true;
      send07a = true;
      send07b = true;
      send0B = true;
      send0C = true;
      send0D = true;
      send19 = true;
      sounderack = false;
    }
    if (txdone)
    { // Sending
      if ((int)(txdone - now) < 0)
      { // Sending done
        if (p)
        {
          Serial.write(buf, p);
          txdone = ((now + p * KEYPADBITS * 1000000 / KEYPADBAUD + POSTTX) ? : 1);
          p = 0; // ready for rx
#ifdef KEYPADBODGE
          delay(p);
          Serial.flush(); // Delay in situ for reliability
          delay(1);
          digitalWrite(RTS, LOW);
#endif
          return true;
        }
        txdone = 0;

        rxdone = ((now + PRERX) ? : 1); // Timeout for first byte rx, for not responding
      }
      return true;
    }

    if (rxdone && Serial.available())
    { // Rx byte
      buf[p] = Serial.read();
      if (p < sizeof(buf)) p++;
      if ((p == 3 && (buf[1] == 0xF2 || buf[1] == 0xFE)) || (p == 4 && buf[1] == 0xF4))
        digitalWrite(RTS, HIGH); // Overlap drive at end of message
      rxdone = ((now + 2 * KEYPADBITS * 1000000 / KEYPADBAUD) ? : 1); // Timeout for next byte rx
      return true;
    }

    if (rxdone)
    { // Receiving
      if ((int)(rxdone - now) < 0)
      { // Receiving done
        rxdone = 0;
        if (keypaddebug && (!online || p < 2 || buf[1] != 0xFE))
          revk.info(F("Rx"), F("%d: %02X %02X %02X %02X"), p, buf[0], buf[1], buf[2], buf[3]);
        if (p)
        {
          static const char *keymap = PSTR("0123456789BAEX*#");
          unsigned int c = 0xAA, n;
          for (n = 0; n < p - 1; n++)
            c += buf[n];
          while (c > 0xFF)
            c = (c >> 8) + (c & 0xFF);
          if (p < 2 || buf[n] != c || buf[0] != 0x11 || buf[1] == 0xF2)
          {

            keypad_fault = PSTR("Bad response from keyboard");
            online = false;
          }
          else
          {
            keypad_fault = NULL;
            static long keyhold = 0;
            if (cmd == 0x00 && buf[1] == 0xFF && p > 5)
            { // Set up response
              if (!online)
              {
                online = true;
                toggle0B = true;
                toggle07 = true;
              }
            } else if (buf[1] == 0xFE)
            { // Idle, no tamper, no key
              keypad_tamper = NULL;
              if (!send0B)
              {
                if (lastkey & 0x80)
                {
                  if ((int)(keyhold - now) < 0)
                  {
                    revk.event(F("gone"), F("%.1S"), keymap + (lastkey & 0x0F));
                    lastkey = 0x7F;
                  }
                } else
                  lastkey = 0x7F;
              }
            } else if (cmd == 0x06 && buf[1] == 0xF4 && p > 3)
            { // Status
              if (*keypad == 'T' && (buf[2] & 0x40))
                keypad_tamper = PSTR("Keypad");
              else
                keypad_tamper = NULL;
              if (!send0B)
              { // Key
                if (buf[2] == 0x7F)
                { // No key
                  if (lastkey & 0x80)
                  {
                    if ((int)(keyhold - now) < 0)
                    {
                      revk.event(F("gone"), F("%.1S"), keymap + (lastkey & 0x0F));
                      lastkey = 0x7F;
                    }
                  } else
                    lastkey = 0x7F;
                } else
                { // key
                  send0B = true;
                  if ((lastkey & 0x80) && buf[2] != lastkey)
                    revk.event(F("gone"), F("%.1S"), keymap + (lastkey & 0x0F));
                  if (!(buf[2] & 0x80) || buf[2] != lastkey)
                    revk.event((buf[2] & 0x80) ? F("hold") : F("key"), F("%.1S"), keymap + (buf[2] & 0x0F));
                  if (buf[2] & 0x80)
                    keyhold = now + 2000000;
                  if (insafemode)
                  { // Special case for safe mode (off line)
                    if (buf[2] == 0x0D)
                    { // ESC in safe mode, shut up
                      sounderack = true;
                      send0C = true;
                    }
                    if (buf[2] == 0x8D)
                      revk.restart(); // ESC held in safe mode
                  }
                  lastkey = buf[2];
                }
              }
            }
          }
        }
        else
        {
          keypad_fault = PSTR("No response from keypad");
          online = false;
        }
      }
      return true;
    }

    p = 0;
    if (!online)
    { // Init
      buf[++p] = 0x00;
      buf[++p] = 0x0E;
    } else    if (send0B)
    { // key confirm
      send0B = false;
      buf[++p] = 0x0B;
      buf[++p] = toggle0B ? 2 : 0;
      toggle0B = !toggle0B;
    } else if (lastkey >= 0x7F && (send07 || send07a || send07b || send07c))
    { // Text
      buf[++p] = 0x07;
      buf[++p] = 0x01 | ((blink[0] & 1) ? 0x08 : 0x00) | (toggle07 ? 0x80 : 0);
      byte len = display_len;
      byte temp[33];
      byte *dis = display;
      if (!revk.mqttconnected)
      { // Off line
        len = snprintf_P((char*)temp, sizeof(temp), PSTR("%-16.16s%-9.9s %6.6s"), revk.get_mqtthost(), revk.get_wifissid(), revk.chipid);
        dis = temp;
      }
      if (cursor_len)
        buf[++p] = 0x07; // cursor off while we update
      if (len)
      {
        buf[++p] = 0x1F; //  home
        int n;
        for (n = 0; n < 32; n++)
        {
          if (!(n & 0xF))
          {
            buf[++p] = 0x03; // Cursor
            buf[++p] = (n ? 0x40 : 0);
          }
          if (n < len)buf[++p] = dis[n];
          else buf[++p] = ' ';
        }
      }
      else
        buf[++p] = 0x17; // clear
      if (send07a || cursor_len)
      { // cursor
        buf[++p] = 0x03;
        buf[++p] =  ((cursor[0] & 0x10) ? 0x40 : 0) + (cursor[0] & 0x0F);
        if (cursor[0] & 0x80)
          buf[++p] = 0x06;       // Solid block
        else if (cursor[0] & 0x40)
          buf[++p] = 0x10;       // Underline
      }
      toggle07 = !toggle07;
      if (send07)
        send07c = true; // always send twice
      else
        send07a = send07b = send07c = false; // sent
      send07 = false;
    } else if (send19)
    { // Key keyclicks
      send19 = false;
      buf[++p] = 0x19;
      if (insafemode)
        buf[++p] = 0x01; // Sound normal
      else
        buf[++p] = (keyclick[0] & 0x7); // 0x03 (silent), 0x05 (quiet), or 0x01 (normal)
      buf[++p] = 0;
    } else if (send0C)
    { // Beeper
      send0C = false;
      byte *s = sounder;
      byte len = sounder_len;
      if (insafemode)
      {
        if (sounderack)
          len = 0; // quiet
        else
        {
          const byte beepy[] = {1, 1};
          s = (byte*)beepy;
          len = 2;
        }
      } else if (keypadbeepoverride)
      {
        const byte beepy[] = {1, 0};
        s = (byte*)beepy;
        len = 2;
      }
      buf[++p] = 0x0C;
      buf[++p] = (len ? s[1] ? 3 : 1 : 0);
      buf[++p] = (s[0] & 0x3F); // Time on
      buf[++p] = (s[1] & 0x3F); // Time off
    } else if (send0D)
    { // Light change
      send0D = false;
      buf[++p] = 0x0D;
      if (!revk.mqttconnected)
        buf[++p] = 1;
      else
        buf[++p] = (backlight[0] & 1);
    } else
      buf[++p] = 0x06; // Normal poll
    // Send
    buf[0] = 0x10; // ID of display
    p++;
    { // Checksum
      unsigned int c = 0xAA, n;
      for (n = 0; n < p; n++)
        c += buf[n];
      while (c > 0xFF)
        c = (c >> 8) + (c & 0xFF);
      buf[p++] = c;
    }
    if (keypaddebug && buf[1] != 0x06)
      revk.info(F("Tx"), F("%d: %02X %02X %02X %02X"), p, buf[0], buf[1], buf[2], buf[3]);
    cmd = buf[1];
    digitalWrite(RTS, HIGH);
    txdone = ((now + PRETX) ? : 1); // 8N1 at 9600
    return true;
  }
