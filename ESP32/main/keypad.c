// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"

#include "galaxybus.h"
#include <driver/gpio.h>
#include "alarm.h"
#include "keypad.h"

struct
{
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
   uint8_t keybit:1;            // Key confirm toggle bit
   uint8_t displaybit:1;        // Display update toggle bit
   uint8_t wascursor:1;         // Cursor was set
} ui;

static volatile uint8_t force;
static galaxybus_t *g = NULL;

static void displayprint (const char *fmt, ...);

const char *
keypad_command (const char *tag, jo_t j)
{
   char val[100];
   int len = 0;
   if (j && (len = jo_strncpy (j, val, sizeof (val))) < 0)
      val[len = 0] = 0;
   if (!strcmp (tag, "connect") || !strcmp (tag, "disconnect") || !strcmp (tag, "change"))
      force = 1;
   if (!strcmp (tag, "shutdown"))
      galaxybus_end (g);
   return NULL;
}

enum
{ IDLE, FAILMSG, OKMSG, MESSAGE, PIN };

int messages = 0;
char **message = NULL;

static void
displayprint (const char *fmt, ...)
{
   static int8_t s[2] = { 0 };  // scroll
   char *out = NULL;
   va_list ap;
   va_start (ap, fmt);
   vasprintf (&out, fmt, ap);
   va_end (ap);
   int8_t len[2] = { 0 };
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
         time_t now = time (0);
         if (now > 1000000000)
         {
            struct tm tm;
            localtime_r (&now, &tm);
            char t[50];
            snprintf (t, sizeof (t), "%04d-%02d-%02d %02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                      tm.tm_min);
            for (char *p = t; *p; p++)
               if (*p == '0')
                  *p = 'O';     // The crossed zeros look crap
            memcpy (ui.display + x, t, 16);
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
   free (out);
   s[0]++;
   s[1]++;
   static char last[32] = { 0 };
   if (memcmp (ui.display, last, 32))
   {
      ui.senddisplay = 1;
      memcpy (last, ui.display, 32);
   }
   if (ui.cursor)
      ui.sendcursor = 1;
   ui.cursor = 0;
}

void
keypad_ui (char key)
{                               // Update display for UI
   static uint32_t timeout = 0;
   uint32_t now = uptime ();
   static uint8_t state = IDLE,
      shh = 0,
      backoff = 0;
   static int8_t pos = 0;
   uint8_t bl = 0;              // Back light
   uint8_t bk = 0;              // Blink
   uint8_t kc = 1;              // Key clicks
   void fail (const char *m, uint8_t bad)
   {
      if (state == FAILMSG)
         return;
      displayprint ("%s", m);
      if (!bad)
      {
         backoff = 1;
         state = OKMSG;
      } else
      {
         if (backoff < 128)
            backoff *= 2;
         state = FAILMSG;
      }
      pos = 0;
      timeout = now + backoff;
      ui.sendrefresh = 1;
   }
   {
      const char *reason = NULL;
      int left = revk_shutting_down (&reason);
      if (left)
      {
         fail (reason, 0);
         int pc = revk_ota_progress ();
         if (pc > 0 && pc < 100)
            displayprint ("%s\nLoading %d%%", reason, pc);
         else
            displayprint ("Reboot in %d\n%s", left, reason);
         state = FAILMSG;
         ui.sendrefresh = 1;
         timeout = now + left + 1;
      }
   }
   if (state != FAILMSG && key == '!')
      fail (revk_version, 0);
   if ((!key && now > timeout) || (key && state == OKMSG))
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
      jo_t e = jo_make (NULL);
      jo_string (e, "reason", "Keypad ESC");
      alarm_disarm (areakeystrong, &e);
      fail ("Cancelling", 0);
   } else if (key == 'X' && areakeyarm && !(state_armed & areakeyarm) && (control_arm & areakeyarm))
   {                            // Arm cancel as not yet armed
      jo_t e = jo_make (NULL);
      jo_string (e, "reason", "Keypad ESC");
      alarm_disarm (areakeyarm, &e);
      fail ("Cancelling", 0);
   }
   if (key)
      shh = 1;
   switch (state)
   {                            // Pre display
   case IDLE:
      if (messages
          && (key == 'E' || (!(areakeyarm && (areakeyarm & ~state_armed) && !(areakeystrong && (areakeystrong & ~state_armed))))))
      {                         // Go direct to messages - if no A/B function, or if ENT pressed
         state = MESSAGE;
         pos = 0;
         break;
      }
      if (key == 'A' && areakeyarm && (areakeyarm & ~state_armed) && !keypadpinarm)
      {                         // Arm set
         jo_t e = jo_make (NULL);
         jo_string (e, "reason", "Keypad A");
         alarm_arm (areakeyarm, &e);
         fail ("Arming", 0);
         break;
      }
      if (key == 'B' && areakeystrong && (areakeystrong & ~state_armed) && !keypadpinarm)
      {                         // Strong arm set
         jo_t e = jo_make (NULL);
         jo_string (e, "reason", "Keypad B");
         alarm_strong (areakeystrong, &e);
         fail ("Arming forced", 0);
         break;
      }
      break;
   case OKMSG:
   case FAILMSG:
      if (timeout < now)
         state = IDLE;
      else
      {
         if (key)
            timeout++;
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
            if (pos < sizeof (code) - 1)
               code[pos++] = key;
            timeout = now + 10;
         } else
         {
            code[pos] = 0;      // Terminate input code
            if (key == 'E')
            {                   // ENT
               if (*keypadpin && areakeydisarm && !strcmp (code, keypadpin))
               {
                  jo_t e = jo_make (NULL);
                  jo_string (e, "reason", "Keypad PIN entry");
                  alarm_disarm (areakeydisarm, &e);
                  fail ("Disarming", 0);
               } else if (!strcmp (code, "*"))
               {
                  if (areakeyarm && (areakeyarm & ~state_armed) && !keypadpinarm)
                  {             // Alternative arming, e.g. if in messages state
                     jo_t e = jo_make (NULL);
                     jo_string (e, "reason", "Keypad *");
                     alarm_arm (areakeyarm, &e);
                     fail ("Arming", 0);
                  }
               } else if (!strcmp (code, "#"))
               {
                  if (areakeystrong && (areakeystrong & ~state_armed) && !keypadpinarm)
                  {             // Alternative arming, e.g. if in message state
                     jo_t e = jo_make (NULL);
                     jo_string (e, "reason", "Keypad #");
                     alarm_strong (areakeystrong, &e);
                     fail ("Arming forced", 0);
                  }
               } else
               {
                  jo_t j = jo_object_alloc ();
                  jo_stringf (j, "reason", !*keypadpin ? "No PIN set" : !areakeydisarm ? "No PIN disarm" : "Wrong PIN");
                  if (debug)
                  {
                     jo_string (j, "pin", keypadpin);
                     jo_string (j, "entered", code);
                  }
                  alarm_event ("wrongpin", &j, iotkeypad);
                  fail ("** Wrong PIN! **\n[Attempt logged]", 1);
               }
            } else if (key == 'A' && areakeyarm)
            {                   // Arm with PIN
               if (*keypadpin && !strcmp (code, keypadpin))
               {
                  jo_t e = jo_make (NULL);
                  jo_string (e, "reason", "Keypad PIN/A");
                  alarm_arm (areakeyarm, &e);
                  fail ("Arming", 0);
               } else
                  fail ("** Wrong PIN! **\n[Attempt logged]", 1);
            } else if (key == 'B' && areakeystrong)
            {                   // Strongarm with PIN
               if (*keypadpin && !strcmp (code, keypadpin))
               {
                  jo_t e = jo_make (NULL);
                  jo_string (e, "reason", "Keypad PIN/B");
                  alarm_strong (areakeystrong, &e);
                  fail ("Arming forced", 0);
               } else
                  fail ("** Wrong PIN! **\n[Attempt logged]", 1);
            } else if (key == 'X')
            {
               state = IDLE;
               pos = 0;
               timeout = 0;
            }
         }
      }
      break;
   }
   static area_t lastdisarmed = 0;
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
      } else if ((area = ((~state_armed) & areakeypad)) != lastdisarmed)
      { // Show disarmed happened
         lastdisarmed = area;
         on = 10;
         off = 1;
         idle = "Disarmed";
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
      if (state == FAILMSG)
      {
         kc = 0;
         bl = 1;
         if (*ui.display == '*')
            on = off = 3;
      }
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
         char set[sizeof (area_t) * 8 + 1] = "";
         displayprint ("%s %s\n", idle, area_list (set, area));
      }
      break;
   case FAILMSG:
      break;
   case MESSAGE:
      {
         if (pos >= messages)
            pos = 0;
         displayprint ("%s", message[pos]);
         bl = 1;
         timeout = now + 30;
      }
      break;
   case PIN:
      {
         displayprint ("PIN Entry:\n%.*s", pos, "****************");
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
   if (ui.silent != 1 - kc)
   {
      ui.silent = 1 - kc;
      ui.sendkeyclick = 1;
   }
   if (!timeout)
      timeout = now + 1;        // default
}

static void
task (void *pvParameters)
{
   esp_task_wdt_add (NULL);
   g = galaxybus_init (keypadtimer, keypadtx.num, keypadrx.num, keypadde.num,
                       keypadre.set ? keypadre.num : keypadde.num, keypadclk.set ? keypadclk.num : -1, 0);
   if (!g)
   {
      vTaskDelete (NULL);
      return;
   }
   int64_t keypad_next = 0;
   galaxybus_set_timing (g, keypadtxpre, keypadtxpost, keypadrxpre, keypadrxpost);
   galaxybus_start (g);
   while (1)
   {
      esp_task_wdt_reset ();
      usleep (1000);
      int64_t now = esp_timer_get_time ();
      if (now > keypad_next)
      {
         keypad_next = now + 1000000ULL;
         keypad_ui (0);
      }

      static uint8_t txbuf[100],
        txp = 0;
      static uint8_t online = 0;
      static unsigned int galaxybusfault = 0;

      void keystatus (uint8_t key)
      {
         static const char keymap[] = "0123456789BAEX*#";
         if (ui.keyconfirm)
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
            jo_t j = jo_object_alloc ();
            jo_stringf (j, "key", "%.1s", keymap + (key & 0x0F));
            if (key & 0x80)
               jo_bool (j, "held", 1);
            alarm_event ("key", &j, iotkeypad);
         }
         keypad_ui (keymap[key & 0x0F]);        // Process key
      }

      {                         // Wait - note we normally expect a TIMEOUT rather than having to wait, so this is deliberately long
         int try = 0;
         while (!galaxybus_ready (g) && try++ < 1000)
            usleep (1000);
      }

      {                         // Receiving (not ready returns 0 so processed as any other error)
         uint8_t rxbuf[10];
         int rxp = galaxybus_rx (g, sizeof (rxbuf), rxbuf);
#if 0
         if (!online || rxp < 2 || rxbuf[1] == 0xF2)
         {
            jo_t j = jo_object_alloc ();
            jo_int (j, "rxp", rxp);
            if (txp > 0)
               jo_base16 (j, "tx", txbuf, txp);
            if (rxp > 0)
               jo_base16 (j, "rx", rxbuf, rxp);
            if (rxp < 0)
               jo_string (j, "err", galaxybus_err_to_name (rxp));
            if (galaxybusfault)
               jo_int (j, "count", galaxybusfault);
            revk_info_clients ("debug", &j, -1);
         }
#endif
         if (rxp < 2 || rxbuf[1] == 0xF2)
         {
            ESP_LOGI (TAG, "Rx fail %s", galaxybus_err_to_name (rxp));
            if (galaxybusfault++ > 10)
            {
               online = 0;
               logical_gpio |= logical_KeyFault;
            }
            if (online && txp && rxp != -GALAXYBUS_ERR_MISSED)
            {                   // Resend last
               galaxybus_tx (g, txp, txbuf);
               continue;
            }
         } else if (txp)
         {                      // response to message
            uint8_t cmd = txbuf[1];
            galaxybusfault = 0;
            if (cmd == 0x00 && rxbuf[1] == 0xFF && rxp >= 5)
            {                   // Handle response
               if (!online)
               {
                  online = 1;
                  logical_gpio &= ~logical_KeyFault;
                  ui.displaybit = 1;
                  ui.keybit = 0;
                  ui.sendrefresh = 1;
                  keypad_ui ('!');
               }
            } else if (rxbuf[1] == 0xFE)
            {                   // Idle, no tamper, no key
               logical_gpio &= ~logical_KeyTamper;
               keystatus (0x7F);        // No key
            } else if (cmd == 0x06 && rxbuf[1] == 0xF4 && rxp >= 3)
            {                   // Status
               if ((rxbuf[2] & 0x40))
                  logical_gpio |= logical_KeyTamper;
               else
                  logical_gpio &= ~logical_KeyTamper;
               keystatus (rxbuf[2]);
            }
         }
      }

      if (galaxybus_ready (g))
         continue;              // Out of step

      // Tx
      if (force || !online)
      {                         // Update all the shit
         force = 0;
         ui.senddisplay = 1;
         ui.sendcursor = 1;
         ui.sendblink = 1;
         ui.sendbacklight = 1;
         ui.sendkeyclick = 1;
         if (force || !online || !ui.on || !ui.off)
            ui.sendsounder = 1; // Resending this a lot is a bad idea if making an on/off sound as it breaks it
      }
      txp = 0;                  // Make new mesage
      if (!online)
      {                         // Init
         usleep (100000);
         txbuf[++txp] = 0x00;
         txbuf[++txp] = 0x0E;
      } else if (ui.keyconfirm)
      {                         // key confirm
         ui.keyconfirm = 0;
         txbuf[++txp] = 0x0B;
         txbuf[++txp] = ui.keybit ? 2 : 0;
      } else if (ui.idle && (ui.senddisplay || ui.sendcursor || ui.sendblink || ui.sendrefresh))
      {                         // Text
         txbuf[++txp] = 0x07;
         txbuf[++txp] = 0x01 | (ui.blink ? 0x08 : 0x00) | (ui.displaybit ? 0x80 : 0);
         uint8_t *dis = ui.display;
         if (ui.wascursor)
            txbuf[++txp] = 0x07;        // cursor off while we update
         if (ui.sendrefresh)
            txbuf[++txp] = 0x17;        // clear
         txbuf[++txp] = 0x1F;   //  home
         int n;
         for (n = 0; n < 32; n++)
         {
            if (!(n & 0xF))
            {
               txbuf[++txp] = 0x03;     // Cursor
               txbuf[++txp] = (n ? 0x40 : 0);
            }
            if (dis[n] >= ' ')
               txbuf[++txp] = dis[n];
            else
               txbuf[++txp] = ' ';
         }
         if ((ui.sendcursor && ui.cursor) || ui.wascursor)
         {                      // cursor
            txbuf[++txp] = 0x03;
            txbuf[++txp] = ((ui.cursor & 0x10) ? 0x40 : 0) + (ui.cursor & 0x0F);
            if (ui.cursor & 0x80)
               txbuf[++txp] = 0x06;     // Solid block
            else if (ui.cursor & 0x40)
               txbuf[++txp] = 0x10;     // Underline
            ui.wascursor = (ui.cursor ? 1 : 0);
         }
         ui.displaybit = !ui.displaybit;
         ui.sendcursor = ui.sendblink = 0;      // sent
         ui.senddisplay = 0;
         ui.sendrefresh = 0;
      } else if (ui.sendkeyclick)
      {                         // Key keyclicks
         ui.sendkeyclick = 0;
         txbuf[++txp] = 0x19;
         txbuf[++txp] = (ui.silent ? 3 : ui.quiet ? 5 : 1);
         txbuf[++txp] = 0;
      } else if (ui.sendsounder)
      {                         // Beeper
         ui.sendsounder = 0;
         txbuf[++txp] = 0x0C;
         txbuf[++txp] = ((ui.on && ui.off) ? 3 : ui.on ? 1 : 0);
         txbuf[++txp] = ui.on;
         txbuf[++txp] = ui.off;
      } else if (ui.sendbacklight)
      {                         // Light change
         ui.sendbacklight = 0;
         txbuf[++txp] = 0x0D;
         txbuf[++txp] = ui.backlight;
      } else
         txbuf[++txp] = 0x06;   // Normal poll
      // Send
      txbuf[0] = keypadaddress; // ID of display
      txp++;
      int l = galaxybus_tx (g, txp, txbuf);
      if (l < 0)
      {
         online = 0;
         logical_gpio |= logical_KeyFault;
         ESP_LOGI (TAG, "Tx fail %s", galaxybus_err_to_name (l));
      }
   }
   galaxybus_end (g);
   vTaskDelete (NULL);
}

void
keypad_boot (void)
{
   if (keypadtx.set && keypadrx.set && keypadde.set && keypadre.set)
   {
      message = malloc (MAX_LEAF_DISPLAY * sizeof (*message));
      memset (message, 0, MAX_LEAF_DISPLAY * sizeof (*message));
      const char *err = port_check (keypadtx.num, TAG, 0);
      if (!err && keypadtx.num != keypadrx.num)
         err = port_check (keypadrx.num, TAG, 1);
      if (!err)
         err = port_check (keypadde.num, TAG, 0);
      if (!err && keypadde.num != keypadre.num)
         err = port_check (keypadre.num, TAG, 0);
      if (err)
      {
         keypadtx.set = keypadrx.set = keypadde.set = keypadre.set = 0;
         logical_gpio |= logical_KeyFault;      // Not all valid
      }
   } else if (keypadtx.num || keypadrx.num || keypadde.num)
      logical_gpio |= logical_KeyFault; // Not all set
}

void
keypad_start (void)
{
   if (keypadtx.set && keypadrx.set && keypadde.set)
      revk_task (TAG, task, NULL, 3);
}

void
keypad_display_update (jo_t j)
{
   if (!message)
      return;
   if (jo_next (j) != JO_ARRAY)
   {
      messages = 0;
      return;
   }
   int count = 0;
   while (jo_next (j) == JO_STRING && count < MAX_LEAF_DISPLAY)
   {
      int l = jo_strlen (j);
      char *t = malloc (l + 1);
      jo_strncpy (j, t, l + 1);
      char *o = message[count];
      message[count] = t;
      free (o);
      count++;
   }
   messages = count;
   while (count < MAX_LEAF_DISPLAY)
   {
      char *o = message[count];
      message[count] = NULL;
      free (o);
      count++;
   }
}
