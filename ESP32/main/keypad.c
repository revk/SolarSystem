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
  u8(keypadtxpre,50)	\
  u8(keypadtxpost,50)	\
  u8(keypadrxpre,50)	\
  u8(keypadrxpost,20)	\
  sl(keypadidle)	\
  sl(keypadpin)	\

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
   uint8_t sendrefresh:1;       // Display refresh
   uint8_t senddisplay:1;       // Send display
   uint8_t sendcursor:1;        // Send cursor
   uint8_t sendkeyclick:1;      // Send quiet/silent
   uint8_t sendsounder:1;       // Send on/off
   uint8_t sendbacklight:1;     // Send backlight
   uint8_t sendblink:1;         // Send blink
   // Internal
   uint8_t idle:1;              // Key idle
   uint8_t keyconfirm:1;        // Key confirmation
   uint8_t keyconfirming:1;     // Key confirmation - waiting reply
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

enum { IDLE, FAILMSG, MESSAGE, PIN };

int messages = 0;
char **message = NULL;

static void displayprint(const char *fmt, ...)
{
   static int8_t s[2] = { };    // scroll
   char *out = NULL;
   va_list ap;
   va_start(ap, fmt);
   vasprintf(&out, fmt, ap);
   va_end(ap);
   int8_t len[2] = { };
   char *v = out,
       *l2;
   while (*v && *v != '\n')
      v++;
   while (v > out && v[-1] == ' ')
      v--;
   l2 = v;
   len[0] = v - out;
   if (*l2)
      l2++;
   v = l2;
   while (*v && *v != '\n')
      v++;
   while (v > out && v[-1] == ' ')
      v--;
   len[1] = v - l2;
   if (s[0] > len[0] - 16)
      s[0] = 0;
   if (s[1] > len[1] - 16)
      s[1] = 0;
   v = out + s[0];
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
         v += s[1];
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
   s[0]++;
   s[1]++;
   static char last[32] = { };
   if (memcmp(ui.display, last, 32))
   {
      ui.senddisplay = 1;
      memcpy(last, ui.display, 32);
   }
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
   uint8_t bl = 0;              // Back light
   uint8_t bk = 0;              // Blink
   void fail(const char *m, int delay) {
      displayprint("%s", m);
      state = FAILMSG;
      pos = 0;
      timeout = now + delay;
      ui.sendrefresh = 1;
   }
   if (!key && now > timeout)
   {
      timeout = 0;
      state = IDLE;
   }
   if (((key >= '0' && key <= '9') || key == '*' || key == '#') && state != PIN && state != FAILMSG)
   {
      state = PIN;
      pos = 0;
   }
   // ESC key to cancel arming regardless of state
   if (key == 'X' && areakeystrong && !(state_armed & areakeystrong) && (control_strong & areakeystrong))
   {                            // Strongarm cancel as not yet armed (very small window of time on this one)
      jo_t e = jo_make(NULL);
      jo_string(e, "reason", "Keypad ESC");
      alarm_disarm(areakeystrong, &e);
      fail("Cancelling", 2);
   } else if (key == 'X' && areakeyarm && !(state_armed & areakeyarm) && (control_arm & areakeyarm))
   {                            // Arm cancel as not yet armed
      jo_t e = jo_make(NULL);
      jo_string(e, "reason", "Keypad ESC");
      alarm_disarm(areakeyarm, &e);
      fail("Cancelling", 2);
   }
   if (key)
      shh = 1;
   switch (state)
   {                            // Pre display
   case IDLE:
      if (messages && (key == 'E' || (!(areakeyarm && (areakeyarm & ~state_armed) && !(areakeystrong && (areakeystrong & ~state_armed))))))
      {                         // Go direct to messages - if no A/B function, or if ENT pressed
         state = MESSAGE;
         pos = 0;
         break;
      }
      if (key == 'A' && areakeyarm && (areakeyarm & ~state_armed))
      {                         // Arm set
         jo_t e = jo_make(NULL);
         jo_string(e, "reason", "Keypad A");
         alarm_arm(areakeyarm, &e);
         fail("Arming", 2);
         break;
      }
      if (key == 'B' && areakeystrong && (areakeystrong & ~state_armed))
      {                         // Arm set
         jo_t e = jo_make(NULL);
         jo_string(e, "reason", "Keypad B");
         alarm_strong(areakeystrong, &e);
         fail("Arming forced", 2);
         break;
      }
      break;
   case FAILMSG:
      if (timeout < now)
         state = IDLE;
      else
      {
         if (key)
            timeout++;
         bl = 1;
      }
      break;
   case MESSAGE:
      if (key == 'X')
      {                         // Leave messages
         state = IDLE;
         break;
      }
      if (key == 'A')
      {                         // next
         if (pos >= messages - 1)
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
         state = IDLE;          // No more messages
      break;
   case PIN:
      {
         static char code[17];
         if ((key >= '0' && key <= '9') || key == '*' || key == '#')
         {                      // PIN for full 12 keys
            if (pos < sizeof(code) - 1)
               code[pos++] = key;
            timeout = now + 10;
         } else if (key == 'B' && pos)
            pos--;              // Delete
         else if (key == 'E')
         {                      // ENT
            code[pos] = 0;      // Terminate input code
            if (*keypadpin && areakeydisarm && !strcmp(code, keypadpin))
            {
               jo_t e = jo_make(NULL);
               jo_string(e, "reason", "Keypad PIN entry");
               alarm_disarm(areakeydisarm, &e);
               fail("Disarming", 2);
            } else if (!strcmp(code, "*"))
            {
               if (areakeyarm && (areakeyarm & ~state_armed))
               {                // Alternative arming, e.g. if in messages state
                  jo_t e = jo_make(NULL);
                  jo_string(e, "reason", "Keypad *");
                  alarm_arm(areakeyarm, &e);
                  fail("Arming", 2);
               }
            } else if (!strcmp(code, "#"))
            {
               if (areakeystrong && (areakeystrong & ~state_armed))
               {                // Alternative arming, e.g. if in message state
                  jo_t e = jo_make(NULL);
                  jo_string(e, "reason", "Keypad #");
                  alarm_strong(areakeystrong, &e);
                  fail("Arming forced", 2);
               }
            } else
            {
               jo_t j = jo_object_alloc();
               jo_stringf(j, "reason", !*keypadpin ? "No PIN set" : !areakeydisarm ? "No PIN disarm" : "Wrong PIN");
               if (debug)
               {
                  jo_string(j, "pin", keypadpin);
                  jo_string(j, "entered", code);
               }
               alarm_event("wrongpin", &j, iotkeypad);
               fail("Wrong PIN!\n[Attempt logged]", 10);
            }
         } else if (key == 'X')
         {
            state = IDLE;
            pos = 0;
            timeout = 0;
         }
      }
      break;
   }
   const char *idle = keypadidle;
   area_t area = 0;
   if (!*idle)
      idle = revk_id;
   {                            // Beep, idle, and backlight
      uint8_t on = 0,
          off = 0;
      if ((area = (state_fire & areakeypad)))
      {
         on = 2;
         off = 1;
         idle = "FIRE!";
         bl = 1;
      } else if ((area = (state_alarm & areakeypad)))
      {
         on = 10;
         off = 1;
         idle = "ALARM!";
         bl = 1;
      } else if ((area = (state_prealarm & areakeypad)))
      {
         on = 5;
         off = 1;
         idle = "Alarm!";
      } else if ((area = (state_prearm & areakeypad)))
      {
         on = off = 1;
         if (!messages)
            off = 20;           // Not a problem (yet)
         idle = "Arming";
      } else if (now & 1)
      {
         if ((area = (state_armed & areakeypad)))
            idle = "Armed";
         else if (messages)
            idle = "Msgs: Press ENT";
         else if ((area = (state_alarmed & areakeypad)))
            idle = "Check alarms";
         else if ((area = (state_tampered & areakeypad)))
            idle = "Check tampers";
         else if ((area = (state_faulted & areakeypad)))
            idle = "Check faults";
      }
      if (!on && !off)
         shh = 0;
      if (shh)
         on = off = 0;
      if (ui.on != on || ui.off != off)
      {
         ui.sendsounder = 1;
         ui.on = on;
         ui.off = off;
      }
   }
   if (state_alarmed & areakeypad)
      bk = 1;
   switch (state)
   {
   case IDLE:                  // Idle display
      {
         char set[sizeof(area_t) * 8 + 1] = "";
         displayprint("%s %s\n", idle, area_list(set, area));
      }
      break;
   case FAILMSG:
      break;
   case MESSAGE:
      {
         if (pos >= messages)
            pos = 0;
         displayprint("%s", message[pos]);
         bl = 1;
         timeout = now + 30;
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
         bl = 1;
      }
      break;
   }
   if (ui.backlight != bl)
   {
      ui.sendbacklight = 1;
      ui.backlight = bl;
   }
   if (ui.blink != bk)
   {
      ui.sendblink = 1;
      ui.blink = bk;
   }
   if (!timeout)
      timeout = now + 1;        // default
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
      static unsigned int galaxybusfault = 0;
      static int64_t rxwait = 0;

      void keystatus(uint8_t key) {
         static const char keymap[] = "0123456789BAEX*#";
         if (ui.keyconfirm || ui.keyconfirming)
            return;             // Pending confirmation
         if (key == 0x7F)
         {
            ui.idle = 1;
            return;             // Idle
         }
         ui.idle = 0;
         ui.keyconfirm = 1;
         ui.keybit = !ui.keybit;        // Send confirmation
         if (debug)
         {                      // Debug logging
            jo_t j = jo_object_alloc();
            jo_stringf(j, "key", "%.1s", keymap + (key & 0x0F));
            if (key & 0x80)
               jo_bool(j, "held", 1);
            alarm_event("key", &j, iotkeypad);
         }
         keypad_ui(keymap[key & 0x0F]); // Process key
      }

      if (galaxybus_ready(g))
      {                         // Receiving
         ui.keyconfirming = 0;
         rxwait = 0;
         int p = galaxybus_rx(g, sizeof(buf), buf);
         if (p < 2)
         {
            ESP_LOGI(TAG, "Rx fail %s", galaxybus_err_to_name(p));
            if (galaxybusfault++ > 10)
            {
               online = 0;
               logical_gpio |= logical_KeyFault;
            }
            usleep(100000);
         } else
         {
            galaxybusfault = 0;
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
               keystatus(0x7F); // No key
            } else if (cmd == 0x06 && buf[1] == 0xF4 && p >= 3)
            {                   // Status
               if ((buf[2] & 0x40))
                  logical_gpio |= logical_KeyTamper;
               else
                  logical_gpio &= ~logical_KeyTamper;
               keystatus(buf[2]);
            }
         }
      }

      if (rxwait > now)
         continue;              // Awaiting reply
      if (rxwait)
      {                         // Timeout on reply
         if (galaxybusfault++ > 10)
         {
            online = 0;
            logical_gpio |= logical_KeyFault;
            rxwait = now + 1000000LL;
         } else
            rxwait = now + 250000LL;
         if (ui.keyconfirming)
            ui.keyconfirm = 1;  // try again
      } else
         rxwait = now + 100000LL;
      // Tx
      if (force || galaxybusfault > 5 || !online)
      {                         // Update all the shit
         force = 0;
         ui.senddisplay = 1;
         ui.sendcursor = 1;
         ui.sendblink = 1;
         if (!online)
            ui.sendrefresh = 1;
         ui.sendbacklight = 1;
         ui.sendkeyclick = 1;
         if (force || !online || !ui.on || !ui.off)
            ui.sendsounder = 1; // Resending this a lot is a bad idea if making an on/off sound as it breaks it
      }
      p = 0;
      if (!online)
      {                         // Init
         buf[++p] = 0x00;
         buf[++p] = 0x0E;
      } else if (ui.keyconfirm)
      {                         // key confirm
         ui.keyconfirm = 0;
         ui.keyconfirming = 1;
         buf[++p] = 0x0B;
         buf[++p] = ui.keybit ? 2 : 0;
      } else if (ui.idle && (ui.senddisplay || ui.sendcursor || ui.sendblink || ui.resenddisplay || ui.sendrefresh))
      {                         // Text
         buf[++p] = 0x07;
         buf[++p] = 0x01 | (ui.blink ? 0x08 : 0x00) | (ui.displaybit ? 0x80 : 0);
         uint8_t *dis = ui.display;
         if (ui.wascursor)
            buf[++p] = 0x07;    // cursor off while we update
         if (ui.sendrefresh)
            buf[++p] = 0x17;    // clear
         else
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
               if (dis[n] >= ' ')
                  buf[++p] = dis[n];
               else
                  buf[++p] = ' ';
            }
         }
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
         if (ui.sendrefresh)
         {
            ui.sendrefresh = 0;
            ui.senddisplay = 1;
         } else
         {
            ui.displaybit = !ui.displaybit;
            if (ui.senddisplay)
               ui.resenddisplay = 1;    // always send twice
            else
               ui.sendcursor = ui.sendblink = ui.resenddisplay = 0;     // sent
            ui.senddisplay = 0;
         }
      } else if (ui.sendkeyclick)
      {                         // Key keyclicks
         ui.sendkeyclick = 0;
         buf[++p] = 0x19;
         buf[++p] = (ui.silent ? 3 : ui.quiet ? 5 : 1);
         buf[++p] = 0;
      } else if (ui.sendsounder)
      {                         // Beeper
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
         ESP_LOGI(TAG, "Tx fail %s", galaxybus_err_to_name(l));
         usleep(100000);
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
