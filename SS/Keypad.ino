// Solar System
// (c) Andrews & Arnold Ltd, Adrian Kennard, see LICENSE file (GPL)

// Keypad / display
// ESP-01 based with RS485 board fits in Galaxy keypad/display module

const char* Keypad_fault = NULL;
const char* Keypad_tamper = NULL;

#include <ESPRevK.h>
#include <RS485.h>

RS485 rs485(0x11, false);

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

  const char* Keypad_setting(const char *tag, const byte *value, size_t len)
  { // Called for commands retrieved from EEPROM
#define s(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=(const char *)value;return t;}}while(0)
#define n(n) do{const char *t=PSTR(#n);if(!strcasecmp_P(tag,t)){n=value?atoi((char*)value):0;return t;}}while(0)
    app_settings
#undef n
#undef s
    return NULL; // Done
  }

  boolean Keypad_command(const char*tag, const byte *message, size_t len)
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

  boolean Keypad_setup(ESPRevK &revk)
  {
    if (!keypad)return false; // Not running keypad
    if (bustx < 0 || busrx < 0 || busde < 0)
    {
      Keypad_fault = PSTR("Define bustx/busrs/busde pins");
      keypad = NULL;
      return false;
    }
    unsigned int pins = ((1 << bustx) | (1 << busrx) | (1 << busde));
    debugf("GPIO pin available %X for Keypad", gpiomap);
    if ((gpiomap & pins) != pins)
    {
      Keypad_fault = PSTR("Pins not available");
      keypad = NULL;
      return false;
    }
    gpiomap &= ~pins;
    debugf("GPIO remaining %X", gpiomap);
    rs485.SetPins(busde, bustx, busrx);
    rs485.Start();
    debug("Keypad OK");
    return true;
  }

  boolean Keypad_loop(ESPRevK &revk, boolean force)
  {
    if (!keypad)return false; // Not running keypad
    long now = millis();

    static byte buf[100], p = 0;
    static byte cmd = 0;
    static boolean online = false;
    static boolean send0B = false;
    static boolean toggle0B = false;
    static boolean toggle07 = false;
    static boolean send07c = false; // second send
    static byte lastkey = 0x7F;
    static boolean sounderack = false;
    static int rs485fault = 0;

    static unsigned int rxwait = 0;
    if (rs485.Available())
    { // Receiving
      rxwait = 0;
      int p = rs485.Rx(sizeof(buf), buf);
      if (keypaddebug && (!online || p < 2 || buf[1] != 0xFE))
        revk.info(F("Rx"), F("%d: %02X %02X %02X %02X"), p, buf[0], buf[1], buf[2], buf[3]);
      static const char *keymap = PSTR("0123456789BAEX*#");
      if (p < 2)
      {
        if (rs485fault++ > 2)
        {
          if (p != RS485MISSED)
            Keypad_fault = PSTR("RS485 Rx missed");
          else if (p == RS485STARTBIT)
            Keypad_fault = PSTR("RS485 Start bit error");
          else if (p == RS485STOPBIT)
            Keypad_fault = PSTR("RS485 Stop bit error");
          else if (p == RS485CHECKSUM)
            Keypad_fault = PSTR("RS485 Checksum error");
          else if (p == RS485TOOBIG)
            Keypad_fault = PSTR("RS485 Too big error");
          else
            Keypad_fault = PSTR("Bad response");
          online = false;
        }
      }
      else
      {
        rs485fault = 0;
        Keypad_fault = NULL;
        static long keyhold = 0;
        if (cmd == 0x00 && buf[1] == 0xFF && p >= 5)
        { // Set up response
          if (!online)
          {
            online = true;
            toggle0B = true;
            toggle07 = true;
          }
        } else if (buf[1] == 0xFE)
        { // Idle, no tamper, no key
          Keypad_tamper = NULL;
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
        } else if (cmd == 0x06 && buf[1] == 0xF4 && p >= 3)
        { // Status
          if (*keypad == 'T' && (buf[2] & 0x40))
            Keypad_tamper = PSTR("Open");
          else
            Keypad_tamper = NULL;
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
                keyhold = now + 2000;
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

    if (rxwait && ((int)rxwait - now) > 0)
      return true; // Waiting

    if (rxwait)
    {
      if (rs485fault++ > 2)
      {
        Keypad_fault = PSTR("No response");
        online = false;
      }
    }

    // Tx
    if (force || rs485fault || !online)
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
    rxwait = ((now + 1000) ? : 1);
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
    if (keypaddebug && buf[1] != 0x06)
      revk.info(F("Tx"), F("%d: %02X %02X %02X %02X"), p, buf[0], buf[1], buf[2], buf[3]);
    cmd = buf[1];
    rs485.Tx(p, buf);
    return true;
  }
