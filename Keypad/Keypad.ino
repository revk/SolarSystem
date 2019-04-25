// Keypad / display
// ESP-01 based with RS485 board
// Connect to Galaxy keypad

#include <ESP8266RevK.h>

ESP8266RevK revk(__FILE__, __DATE__ " " __TIME__);

#define RTS 2

static boolean send07 = false;
static boolean send0B = false;
static boolean send0C = false;
static boolean send0D = false;
static boolean send19 = false;
static boolean blink = false;
static byte display[2][16] = {"Hello", "World"};

boolean app_setting(const char *setting, const byte *value, size_t len)
{ // Called for settings retrieved from EEPROM
  return false; // Done
}

boolean app_cmnd(const char*suffix, const byte *message, size_t len)
{ // Called for incoming MQTT messages
  return false;
}

void setup()
{
#ifndef REVKDEBUG
  Serial.begin(9600);	// Galaxy uses 9600 8N2
#endif
  digitalWrite(RTS, LOW);
  pinMode(RTS, OUTPUT);
  
}

void loop()
{
  revk.loop();
  long now = millis();
  static long next = 0;
  if ((int)(next - now) < 0)
  {
    next = ((now + 1000) ? : 1); // Default if nothing responding
    static boolean toggle0B = false;
    static boolean toggle07 = false;
    static boolean online = false;
    static boolean tamper = false;
    static boolean fault = false;
    byte buf[100], p = 0, id = 0x10;
    if (!online)
    { // Init
      buf[++p] = 0x00;
      buf[++p] = 0x0E;
    } else    if (send0B)
    { // key confirm
      send0B = false;
      buf[++p] = 0x0B;
      buf[++p] = toggle0B ? 2 : 0;
      toggle0B = ~toggle0B;
    } else if (send07)
    { // Text
      send07 = false;
      buf[++p] = 0x07;
      buf[++p] = 0x01 | (blink ? 0x08 : 0x00) | (toggle07 ? 0x80 : 0);
      // TODO simplistic
      buf[++p] = 0x1F; // clear / home
      int y, x;
      for (y = 0; y < 2; y++)
      {
        buf[++p] = 0x03; // Cursor
        buf[++p] = (y ? 0x40 : 0);
        for (x = 0; x < 16; x++)buf[++p] = (display[y][x] ? : ' ');
      }
    } else if (send19)
    { // Settings
      send19 = false;
      buf[++p] = 0x06; // TODO poll for now
    } else if (send0C)
    { // Beep change
      send0C = false;
      buf[++p] = 0x06; // TODO poll for now
    } else if (send0D)
    { // Light change
      send0D = false;
      buf[++p] = 0x06; // TODO poll for now
    } else
    { // Poll
      buf[++p] = 0x06;
    }
    // Send
    buf[0] = id;
    p++;
    { // Checksum
      unsigned int c = 0xAA, n;
      for (n = 0; n < p; n++)
        c += buf[n];
      while (c > 0xFF)
        c = (c >> 8) + (c & 0xFF);
      buf[p++] = c;
    }
#ifdef DEBUG
    revk.stat("Tx", "%d: %02X %02X %02X %02X", p, buf[0], buf[1], buf[2], buf[3]);
#endif
    digitalWrite(RTS, HIGH);
    delay(1);
    Serial.write(buf, p);
    Serial.flush();
    digitalWrite(RTS, LOW);
    Serial.setTimeout(10);
    p = Serial.readBytes(buf, 1);
    if (p)
    {
      Serial.setTimeout(2);
      p += Serial.readBytes(buf + p, sizeof(buf) - p);
      unsigned int c = 0xAA, n;
      for (n = 0; n < p - 1; n++)
        c += buf[n];
      while (c > 0xFF)
        c = (c >> 8) + (c & 0xFF);
      if (p < 2 || buf[n] != c || buf[0] != 0x11 || buf[1] == 0xF2)
      {
        if (!fault)
        {
          revk.stat("fault", "1");
          fault = true;
        }
        next = ((now + 1000) ? : 1);
      }
      else
      {
        if (fault)
        {
          revk.stat("fault", "0");
          fault = false;
        }
        if (!online)
        {
          revk.stat("present", "1");
          online = true;
          toggle0B = false;
          toggle07 = true;
          send07 = true;
          send0B = true;
          send0C = true;
          send0D = true;
          send19 = true;
        }
        if (buf[1] == 0xFE)
        { // Idle, no tamper
          if (tamper)revk.stat("tamper", "0");
          tamper = false;
        } else if (buf[1] == 0xF4 && p > 3)
        { // Status
          if (buf[2] & 0x40)
          {
            if (!tamper)revk.stat("tamper", "1");
            tamper = true;
          } else
          {
            if (tamper)revk.stat("tamper", "0");
            tamper = false;
          }
          if (buf[2] != 0x7F)
          { // Key
            send0B = true;
            // Note mapping and key held bit...
            // TODO why duplicates?
            revk.stat("key", "%02X", buf[2]);
          }
        }
      }
#ifdef DEBUG
      revk.stat("Rx", "%d: %02X %02X %02X %02X", p, buf[0], buf[1], buf[2], buf[3]);
#endif
      next = ((millis() + 50) ? : 1);
    } else
    {
      if (online)
        revk.stat("present", "0");
      online = false;
    }
    delay(1);
  }
}
