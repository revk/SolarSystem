// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Keypad / display
// ESP-01 based with RS485 board fits in Galaxy keypad/display module

#include "keypad.h"
const char* keypad_fault = false;

#define PINS ((1<<1) | (1<<3))  // Tx and Rx

#include <ESP8266RevK.h>

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

  const char* keypad_setting(const char *tag, const byte *value, size_t len)
  { // Called for commands retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
#define n(n) do{const char *t=PSTR(#n);if(!strcmp_P(tag,t)){n=value?atoi((char*)value):0;return t;}}while(0)
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
    return false;
  }

  boolean keypad_setup(ESP8266RevK &revk)
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
    Serial.begin(9600);	// Galaxy uses 9600 8N2
#endif
    digitalWrite(RTS, LOW);
    pinMode(RTS, OUTPUT);
    debug("Keypad OK");
    return true;
  }

  boolean keypad_loop(ESP8266RevK &revk, boolean force)
  {
    if (!keypad)return false; // Not running keypad
    long now = millis();
    if (force)
    { // Update all the shit
      send07 = true;
      send0C = true;
      send0D = true;
      send19 = true;
    }
    static long next = 0;
    if ((int)(next - now) < 0)
    {
      static boolean send0B = false;
      static boolean toggle0B = false;
      static boolean toggle07 = false;
      static boolean online = false;
      static boolean tamper = false;
      static boolean send07c = false; // second send
      next = ((now + 1000) ? : 1); // Default if nothing responding
      byte buf[100], p = 0;
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
      } else if (send07 || send07a || send07b || send07c)
      { // Text
        if (send07)
          send07c = true; // always send twice
        else
          send07a = send07b = send07c = false; // sent
        send07 = false;
        buf[++p] = 0x07;
        buf[++p] = 0x01 | ((blink[0] & 1) ? 0x08 : 0x00) | (toggle07 ? 0x80 : 0);
        byte len = display_len;
        byte *dis = display;
        if (!revk.mqttconnected)
        { // Off line
          dis = (byte*)"OFFLINE!";
          len = 8;
        }
        if (len)
        {
          if (cursor[0])
          {
            buf[++p] = 0x07; // cursor off while we update
            send07a = true; // send cursor when done
          }
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
        if (send07a)
        { // cursor
          if (cursor_len)
          {
            buf[++p] = 0x03;
            buf[++p] =  ((cursor[0] & 0x10) ? 0x40 : 0) + (cursor[0] & 0x0F);
            if (cursor[0] & 0x80)
              buf[++p] = 0x06;       // Solid block
            else if (cursor[0] & 0x40)
              buf[++p] = 0x10;       // Underline
          }
          else buf[++p] = 0x07; // cursor off
        }
        toggle07 = !toggle07;
      } else if (send19)
      { // Key keyclicks
        send19 = false;
        buf[++p] = 0x19;
        if (!revk.mqttconnected)
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
          static const byte beepy[] = {1, 1};
          s = (byte*)beepy;
          len = 2;
        }
        buf[++p] = 0x0C;
        buf[++p] = len ? s[1] ? 3 : 1 : 0;
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
      byte cmd = buf[1];
      digitalWrite(RTS, HIGH);
      delay(1);
      Serial.write(buf, p);
      Serial.flush();
      digitalWrite(RTS, LOW);
      Serial.setTimeout(10);
      p = Serial.readBytes(buf, 1);
      if (p)
      {
        static const char *keymap = PSTR("0123456789BA\n\e*#");
        Serial.setTimeout(2);
        p += Serial.readBytes(buf + p, sizeof(buf) - p);
        if (keypaddebug && buf[1] != 0xFE)
          revk.info(F("Rx"), F("%d: %02X %02X %02X %02X"), p, buf[0], buf[1], buf[2], buf[3]);
        unsigned int c = 0xAA, n;
        for (n = 0; n < p - 1; n++)
          c += buf[n];
        while (c > 0xFF)
          c = (c >> 8) + (c & 0xFF);
        if (p < 2 || buf[n] != c || buf[0] != 0x11 || buf[1] == 0xF2)
          keypad_fault = PSTR("Bad response from keyboard");
        else
        {
          keypad_fault = NULL;
          static byte lastkey = 0;
          static long keyhold = 0;
          if (cmd == 0x00 && buf[1] == 0xFF && p > 5)
          { // Set up response
            if (!online)
            {
              online = true;
              toggle0B = false;
              toggle07 = true;
              send07 = true;
              send07a = true;
              send07b = true;
              send0B = true;
              send0C = true;
              send0D = true;
              send19 = true;
            }
          } else if (buf[1] == 0xFE)
          { // Idle, no tamper
            if (tamper)revk.state(F("tamper"), F("0"));
            tamper = false;
            if (!send0B && (lastkey & 0x80) && (int)(keyhold - now) < 0)
            {
              revk.event(F("gone"), F("%.1S"), keymap + (lastkey & 0x0F));
              lastkey = 0x7F;
            }
          } else if (cmd == 0x06 && buf[1] == 0xF4 && p > 3)
          { // Status
            if (buf[2] & 0x40)
            {
              if (!tamper)revk.state(F("tamper"), F("1"));
              tamper = true;
            } else
            {
              if (tamper)revk.state(F("tamper"), F("0"));
              tamper = false;
            }
            if (!send0B)
            { // Key
              if (buf[2] == 0x7F)
              { // No key
                if ((lastkey & 0x80) && (int)(keyhold - now) < 0)
                  revk.event(F("gone"), F("%.1S"), keymap + (lastkey & 0x0F));
              } else
              { // key
                send0B = true;
                if ((lastkey & 0x80) && buf[2] != lastkey)
                  revk.event(F("gone"), F("%.1S"), keymap + (lastkey & 0x0F));
                if (!(buf[2] & 0x80) || buf[2] != lastkey)
                  revk.event(buf[2] & 0x80 ? F("hold") : F("key"), F("%.1S"), keymap + (buf[2] & 0x0F));
                if (buf[2] & 0x80)keyhold = now + 2000;
              }
              lastkey = buf[2];
            }
          }
          next = ((millis() + 20) ? : 1);
        }
      } else
      {
        keypad_fault = PSTR("No response from keypad");
        online = false;
      }
    }
    return true;
  }
