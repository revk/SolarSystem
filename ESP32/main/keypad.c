// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"

#include "galaxybus.h"
#include <driver/gpio.h>
#include "alarm.h"
#include "keypad.h"

#define port_mask(p) ((p)&127)

#define settings  \
  p(keypadtx) \
  p(keypadrx) \
  p(keypadde) \
  p(keypadre) \
  p(keypadclk) \
  u8(keypadtimer,0) \
  u8h(keypadaddress,10)	\
  b(keypadtamper)	\
  u8(keypadtxpre,50)	\
  u8(keypadtxpost,40)	\
  u8(keypadrxpre,50)	\
  u8(keypadrxpost,10)	\
  sl(keypadidle)	\

// TODO keypad tamper in web UI?
struct {
   uint8_t display[32];
   uint8_t cursor;              // low 4 bits is x, 0x10 is second row, 0x40 is underline, 0x80 is block
   uint8_t on:6;                // Sounder on
   uint8_t off:6;               // Sounder off
   uint8_t quiet:1;             // Key click
   uint8_t silent:1;            // Key click
   uint8_t backlight:1;         // Backlight
   uint8_t blink:1;             // Blink LED
   // Send
   uint8_t senddisplay:1;       // Send display
   uint8_t sendcursor:1;        // Send cursor
   uint8_t sendkeyclick:1;      // Send quiet/silent
   uint8_t sendsounder:1;       // Send on/off
   uint8_t sendbacklight:1;     // Send backlight
   uint8_t sendblink:1;         // Send blink
   // Internal
   uint8_t keyconfirm:1;        // Key confirmation
   uint8_t keybit:1;            // Key confirm toggle bit
   uint8_t displaybit:1;        // Display update toggle bit
   uint8_t wascursor:1;         // Cursor was set
   uint8_t resenddisplay;       // Resend display
} ui;

#define u8(n,d) uint8_t n;
#define u8h(n,d) uint8_t n;
#define b(n) uint8_t n;
#define p(n) uint8_t n;
#define sl(n) char *n;
settings
#undef u8
#undef u8h
#undef b
#undef p
#undef sl
static volatile uint8_t force;

static void displayprint(const char *fmt, ...);

const char *keypad_command(const char *tag, jo_t j)
{
   char val[100];
   int len = 0;
   if (j && (len = jo_strncpy(j, val, sizeof(val))) < 0)
      val[len = 0] = 0;
   if (!strcmp(tag, "connect") || !strcmp(tag, "disconnect") || !strcmp(tag, "change"))
      force = 1;
   return NULL;
}

enum { IDLE, MESSAGE, PIN };

int messages = 0;
char **message = NULL;

static void displayprint(const char *fmt, ...)
{
   char *out = NULL;
   va_list ap;
   va_start(ap, fmt);
   vasprintf(&out, fmt, ap);
   va_end(ap);
   char *v = out;
   int x = 0;
   while (*v && *v != '\n')
   {
      if (x < 16)
         ui.display[x++] = *v;
      v++;
   }
   while (x < 16)
      ui.display[x++] = ' ';
   if (*v)
   {                            // New line
      v++;                      // Line 2
      if (!*v)
      {                         // No second line - put date/time if set - print a space if not wanted or just one line with no \n
         time_t now = time(0);
         if (now > 1000000000)
         {
            struct tm tm;
            localtime_r(&now, &tm);
            char t[50];
            snprintf(t, sizeof(t), "%04d-%02d-%02d %02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
            for (char *p = t; *p; p++)
               if (*p == '0')
                  *p = 'O';     // The crossed zeros look crap
            memcpy(ui.display + x, t, 16);
            x += 16;
         }
      } else
      {                         // Second line
         while (*v && *v != '\n')
         {
            if (x < 32)
               ui.display[x++] = *v;
            v++;
         }
      }
   }
   while (x < 32)
      ui.display[x++] = ' ';
   free(out);
   ui.senddisplay = 1;
   if (ui.cursor)
      ui.sendcursor = 1;
   ui.cursor = 0;
}

void keypad_ui(char key)
{                               // Update display for UI
   static uint32_t timeout = 0;
   uint32_t now = uptime();
   static uint8_t state = IDLE,
       shh = 0;
   static int8_t pos = 0;
   if (key == 'X' || (!key && now > timeout))
   {
      timeout = 0;
      state = IDLE;
   }
   if (((key >= '0' && key <= '9') || key == '*' || key == '#') && state != PIN)
   {
      state = PIN;
      pos = 0;
      shh = 0;
   }
   void fail(const char *m) {
      displayprint("%s", m);
      state = IDLE;
      pos = 0;
      timeout = now + 5;
   }
   switch (state)
   {                            // Pre display
   case IDLE:
      if (now > timeout)
      {
         if (messages)
            state = MESSAGE;
         pos = 0;
      }
      break;
   case MESSAGE:
      if (key)
      {
         shh = 1;
         timeout = now + 10;
      }
      if (key == 'A')
      {                         // next
         if (pos >= messages)
            pos = 0;
         else
            pos++;
      }
      if (key == 'B')
      {                         // prev
         if (pos)
            pos--;
         else
            pos = messages - 1;
      }
      if (pos >= messages)
         state = IDLE;
      break;
   case PIN:
      {
         static char code[16];
         if (key)
         {
            timeout = now + 10;
            shh = 1;
         }
         if ((key >= '0' && key <= '9') || key == '*' || key == '#')
         {                      // PIN for full 12 keys
            if (pos < sizeof(code))
               code[pos++] = key;
         } else if (key == 'B' && pos)
            pos--;              // Delete
         else if (key == 'E')
         {                      // ENT
            // TODO check code?
            fail("Wrong PIN");
         }
      }
      break;
   }
   {                            // Backlight
      uint8_t bl = 0;
      if (state != IDLE)
         bl = 1;
      if (ui.backlight != bl)
      {
         ui.sendbacklight = 1;
         ESP_LOGD(TAG, "Backlight %d", bl);
      }
      ui.backlight = bl;
   }
   {                            // Beep
      uint8_t on = 0,
          off = 0;
      if (!shh)
      {
         if (state_alarm & areakeypad)
         {
            on = 10;
            off = 1;
         } else if (state_prearm & areakeypad)
         {
            on = off = 1;
         } else if (state_tamper & areakeypad)
         {
            on = 1;
            off = 63;
         }
      }
      if (ui.on != on || ui.off != off)
      {
         ui.sendsounder = 1;
         ui.on = on;
         ui.off = off;
      }
   }
   {                            // LED blink
      uint8_t bl = 0;
      if (state_alarmed & areakeypad)
         bl = 1;
      if (ui.blink != bl)
      {
         ui.sendblink = 1;
         ESP_LOGD(TAG, "LED Blink %d", bl);
      }
      ui.blink = bl;
   }
   switch (state)
   {
   case IDLE:                  // Idle display
      if (now > timeout)
         displayprint("%-16s\n", *keypadidle ? keypadidle : revk_id);
      break;
   case MESSAGE:
      {
         if (pos >= messages)
            pos = 0;
         displayprint("%s", message[pos]);
      }
      break;
   case PIN:
      {
         displayprint("PIN Entry:\n%.*s", pos, "****************");
         if (pos < 16)
         {
            ui.cursor = pos + 0x10 + 0x40;      // Line 1 underscore at pos
            ui.sendcursor = 1;
         }
      }
      break;
   }
   // Default one second
}

static void task(void *pvParameters)
{
   galaxybus_t *g = galaxybus_init(keypadtimer, port_mask(keypadtx), port_mask(keypadrx), port_mask(keypadde),
                                   keypadre ? port_mask(keypadre) : -1,
                                   keypadclk ? port_mask(keypadclk) : -1,
                                   0);
   if (!g)
   {
      vTaskDelete(NULL);
      return;
   }
   esp_task_wdt_add(NULL);
   int64_t keypad_next = 0;
   galaxybus_set_timing(g, keypadtxpre, keypadtxpost, keypadrxpre, keypadrxpost);
   galaxybus_start(g);
   while (1)
   {
      esp_task_wdt_reset();
      usleep(1000);
      int64_t now = esp_timer_get_time();
      if (now > keypad_next)
      {
         keypad_next = now + 1000000ULL;
         keypad_ui(0);
      }

      static uint8_t buf[100],
       p = 0;
      static uint8_t cmd = 0;
      static uint8_t online = 0;
      static uint8_t lastkey = 0x7F;
      static unsigned int galaxybusfault = 0;
      static int64_t rxwait = 0;

      if (galaxybus_ready(g))
      {                         // Receiving
         rxwait = 0;
         int p = galaxybus_rx(g, sizeof(buf), buf);
         static const char keymap[] = "0123456789BAEX*#";
         if (p < 2)
         {
            ESP_LOGD(TAG, "Rx fail %s", galaxybus_err_to_name(p));
            if (galaxybusfault++ > 5)
            {
               online = 0;
               logical_gpio |= logical_KeyFault;
            }
            usleep(100000);
         } else
         {
            galaxybusfault = 0;
            static int64_t keyhold = 0;
            if (cmd == 0x00 && buf[1] == 0xFF && p >= 5)
            {                   // Set up response
               if (!online)
               {
                  online = 1;
                  logical_gpio &= ~logical_KeyFault;
                  ui.displaybit = 1;
               }
            } else if (buf[1] == 0xF2)
               force = 1;       // Error?
            else if (buf[1] == 0xFE)
            {                   // Idle, no tamper, no key
               logical_gpio &= ~logical_KeyTamper;
               if (!ui.keyconfirm)
               {
                  if (lastkey & 0x80)
                  {
                     if (keyhold < now)
                     {
                        if (debug)
                        {
                           jo_t j = jo_object_alloc();
                           jo_stringf(j, "key", "%.1s", keymap + (lastkey & 0x0F));
                           alarm_event("gone", &j, iotkeypad);
                        }
                        lastkey = 0x7F;
                     }
                  } else
                     lastkey = 0x7F;
               }
            } else if (cmd == 0x06 && buf[1] == 0xF4 && p >= 3)
            {                   // Status
               if (keypadtamper && (buf[2] & 0x40))
                  logical_gpio |= logical_KeyTamper;
               else
                  logical_gpio &= ~logical_KeyTamper;
               if (!ui.keyconfirm)
               {                // Key
                  if (buf[2] == 0x7F)
                  {             // No key
                     if (lastkey & 0x80)
                     {
                        if (keyhold < now)
                        {
                           if (debug)
                           {
                              jo_t j = jo_object_alloc();
                              jo_stringf(j, "key", "%.1s", keymap + (lastkey & 0x0F));
                              alarm_event("gone", &j, iotkeypad);
                           }
                           lastkey = 0x7F;
                        }
                     } else
                        lastkey = 0x7F;
                  } else
                  {             // key
                     ui.keybit = !ui.keybit;
                     ui.keyconfirm = 1;
                     if ((lastkey & 0x80) && buf[2] != lastkey)
                     {
                        if (debug)
                        {
                           jo_t j = jo_object_alloc();
                           jo_stringf(j, "key", "%.1s", keymap + (lastkey & 0x0F));
                           alarm_event("gone", &j, iotkeypad);
                        }
                     }
                     if (!(buf[2] & 0x80) || buf[2] != lastkey)
                     {
                        if (debug)
                        {
                           jo_t j = jo_object_alloc();
                           jo_stringf(j, "key", "%.1s", keymap + (buf[2] & 0x0F));
                           alarm_event((buf[2] & 0x80) ? "hold" : "key", &j, iotkeypad);
                        }
                        if (!(buf[2] & 0x80))
                           keypad_ui(keymap[buf[2] & 0x0F]);
                     }
                     if (buf[2] & 0x80)
                        keyhold = now + 2000000LL;
                     lastkey = buf[2];
                  }
               }
            }
         }
      }

      if (rxwait > now)
         continue;              // Awaiting reply
      if (rxwait)
      {
         if (galaxybusfault++ > 5)
         {
            online = 0;
            logical_gpio |= logical_KeyFault;
         }
         rxwait = now + 3000000LL;
      } else
         rxwait = now + 250000LL;
      // Tx
      if (force || galaxybusfault || !online)
      {                         // Update all the shit
         force = 0;
         ui.senddisplay = 1;
         ui.sendcursor = 1;
         ui.sendblink = 1;
         ui.keyconfirm = 1;
         ui.sendsounder = 1;
         ui.sendbacklight = 1;
         ui.sendkeyclick = 1;
      }
      p = 0;
      if (!online)
      {                         // Init
         buf[++p] = 0x00;
         buf[++p] = 0x0E;
      } else if (ui.keyconfirm)
      {                         // key confirm
         ui.keyconfirm = 0;
         buf[++p] = 0x0B;
         buf[++p] = ui.keybit ? 2 : 0;
      } else if (lastkey >= 0x7F && (ui.senddisplay || ui.sendcursor || ui.sendblink || ui.resenddisplay))
      {                         // Text
         buf[++p] = 0x07;
         buf[++p] = 0x01 | (ui.blink ? 0x08 : 0x00) | (ui.displaybit ? 0x80 : 0);
         uint8_t len = 32;
         uint8_t *dis = ui.display;
         if (ui.wascursor)
            buf[++p] = 0x07;    // cursor off while we update
         if (len)
         {
            buf[++p] = 0x1F;    //  home
            int n;
            for (n = 0; n < 32; n++)
            {
               if (!(n & 0xF))
               {
                  buf[++p] = 0x03;      // Cursor
                  buf[++p] = (n ? 0x40 : 0);
               }
               if (n < len)
                  buf[++p] = dis[n];
               else
                  buf[++p] = ' ';
            }
         } else
            buf[++p] = 0x17;    // clear
         if ((ui.sendcursor && ui.cursor) || ui.wascursor)
         {                      // cursor
            buf[++p] = 0x03;
            buf[++p] = ((ui.cursor & 0x10) ? 0x40 : 0) + (ui.cursor & 0x0F);
            if (ui.cursor & 0x80)
               buf[++p] = 0x06; // Solid block
            else if (ui.cursor & 0x40)
               buf[++p] = 0x10; // Underline
            ui.wascursor = (ui.cursor ? 1 : 0);
         }
         ui.displaybit = !ui.displaybit;
         if (ui.senddisplay)
            ui.resenddisplay = 1;       // always send twice
         else
            ui.sendcursor = ui.sendblink = ui.resenddisplay = 0;        // sent
         ui.senddisplay = 0;
      } else if (ui.sendkeyclick)
      {                         // Key keyclicks
         ui.sendkeyclick = 0;
         buf[++p] = 0x19;
         buf[++p] = (ui.silent ? 3 : ui.quiet ? 5 : 1);
         buf[++p] = 0;
      } else if (ui.sendsounder)
      {                         // Beeper
         ESP_LOGI(TAG, "Sounder %d %d", ui.on, ui.off);
         ui.sendsounder = 0;
         buf[++p] = 0x0C;
         buf[++p] = ((ui.on && ui.off) ? 3 : ui.on ? 1 : 0);
         buf[++p] = ui.on;
         buf[++p] = ui.off;
      } else if (ui.sendbacklight)
      {                         // Light change
         ui.sendbacklight = 0;
         buf[++p] = 0x0D;
         buf[++p] = ui.backlight;
      } else
         buf[++p] = 0x06;       // Normal poll
      // Send
      buf[0] = keypadaddress;   // ID of display
      p++;
      cmd = buf[1];
      int l = galaxybus_tx(g, p, buf);
      if (l < 0)
      {
         online = 0;
         logical_gpio |= logical_KeyFault;
         ESP_LOGD(TAG, "Tx fail %s", galaxybus_err_to_name(l));
         usleep(500000);
         rxwait = 0;
      }
   }
}

void keypad_boot(void)
{
   revk_register("keypad", 0, sizeof(keypadtx), &keypadtx, NULL, SETTING_SET | SETTING_SECRET); // Parent
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8h(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_HEX);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN|SETTING_LIVE);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
#define sl(n) revk_register(#n,0,0,&n,NULL,SETTING_LIVE);
   settings;
#undef u8
#undef u8h
#undef b
#undef p
#undef sl
   if (keypadtx && keypadrx && keypadde && keypadre)
   {
      message = malloc(MAX_LEAF_DISPLAY * sizeof(*message));
      memset(message, 0, MAX_LEAF_DISPLAY * sizeof(*message));
      const char *err = port_check(port_mask(keypadtx), TAG, 0);
      if (!err && keypadtx != keypadrx)
         err = port_check(port_mask(keypadrx), TAG, 1);
      if (!err)
         err = port_check(port_mask(keypadde), TAG, 0);
      if (!err && keypadde != keypadre)
         err = port_check(port_mask(keypadre), TAG, 0);
      gpio_set_pull_mode(port_mask(keypadrx), GPIO_PULLUP_ONLY);
      logical_gpio |= logical_KeyFault;
      // Done early because it beeps a lot!
      revk_task(TAG, task, NULL);
   } else if (keypadtx || keypadrx || keypadde)
      logical_gpio |= logical_KeyFault;
}

void keypad_start(void)
{
}

void keypad_display_update(jo_t j)
{
   if (!message)
      return;
   if (jo_next(j) != JO_ARRAY)
   {
      messages = 0;
      return;
   }
   int count = 0;
   while (jo_next(j) == JO_STRING && count < MAX_LEAF_DISPLAY)
   {
      int l = jo_strlen(j);
      char *t = malloc(l + 1);
      jo_strncpy(j, t, l + 1);
      char *o = message[count];
      message[count] = t;
      free(o);
      count++;
   }
   messages = count;
   while (count < MAX_LEAF_DISPLAY)
   {
      char *o = message[count];
      message[count] = NULL;
      free(o);
      count++;
   }
}
