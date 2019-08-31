// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"
const char *keypad_fault = NULL;
const char *keypad_tamper = NULL;

#include "galaxybus.h"

#define port_mask(p) ((p)&127)

#define settings  \
  p(keypadtx) \
  p(keypadrx) \
  p(keypadde) \
  p(keypadre) \
  p(keypadclk) \
  u8(keypadtimer,0) \
  u8h(keypadaddress,10)	\
  b(keypaddebug)	\
  b(keypadtamper)	\
  u8(keypadpre,50)	\
  u8(keypadpost,40)	\
  u8(keypadgap,10)	\

#define commands  \
  f(07,display,32,0) \
  f(19,keyclick,1,5) \
  f(0C,sounder,2,0) \
  f(0D,backlight,1,1) \
  f(07a,cursor,2,0) \
  f(07b,blink,1,0) \

#define u8(n,d) uint8_t n;
#define u8h(n,d) uint8_t n;
#define b(n) uint8_t n;
#define p(n) uint8_t n;
settings
#undef u8
#undef u8h
#undef b
#undef p
#define f(id,name,len,def) static uint8_t name[len]={def};uint8_t send##id=false;uint8_t name##_len=0;
   commands
#undef  f
static volatile uint8_t force;

const char *
keypad_command (const char *tag, unsigned int len, const unsigned char *value)
{
   if (!strcmp (tag, "connect") || !strcmp (tag, "disconnect") || !strcmp (tag, "change"))
      force = 1;
#define f(i,n,l,d) if(!strcasecmp(tag,#n)&&len<=l){memcpy(n,value,len);n##_len=len;if(len<l)memset(n+len,0,l-len);send##i=1;return "";}
   commands
#undef f
      return NULL;
}

static void
task (void *pvParameters)
{
   galaxybus_t *g = pvParameters;
   galaxybus_start (g);
   while (1)
   {
      usleep (1000);            // TODO
      int64_t now = esp_timer_get_time ();

      static uint8_t buf[100],
        p = 0;
      static uint8_t cmd = 0;
      static uint8_t online = 0;
      static uint8_t send0B = 0;
      static uint8_t toggle0B = 0;
      static uint8_t toggle07 = 0;
      static uint8_t send07c = 0;
      static uint8_t lastkey = 0x7F;
      static uint8_t sounderack = 0;
      static unsigned int galaxybusfault = 0;
      static unsigned int rxwait = 0;

      if (galaxybus_ready (g))
      {                         // Receiving
         rxwait = 0;
         int p = galaxybus_rx (g, sizeof (buf), buf);
         if (keypaddebug && (!online || p < 2 || buf[1] != 0xFE))
            revk_info ("Rx", "%d: %02X %02X %02X %02X %s", p, buf[0], buf[1], buf[2], buf[3],
                       p < 0 ? galaxybus_err_to_name (p) : "");
         static const char keymap[] = "0123456789BAEX*#";
         if (p < 2)
         {
            rxwait = now + 3000000;
            if (galaxybusfault++ > 5)
            {
               status (keypad_fault = galaxybus_err_to_name (p));
               online = 0;
            }
         } else
         {
            galaxybusfault = 0;
            status (keypad_fault = NULL);
            static long keyhold = 0;
            if (cmd == 0x00 && buf[1] == 0xFF && p >= 5)
            {                   // Set up response
               if (!online)
               {
                  online = 1;
                  //toggle0B = 1;
                  toggle07 = 1;
               }
            } else if (buf[1] == 0xF2)
               force = 1;       // Error?
            else if (buf[1] == 0xFE)
            {                   // Idle, no tamper, no key
               status (keypad_tamper = NULL);
               if (!send0B)
               {
                  if (lastkey & 0x80)
                  {
                     if (keyhold < now)
                     {
                        revk_event ("gone", "%.1S", keymap + (lastkey & 0x0F));
                        lastkey = 0x7F;
                     }
                  } else
                     lastkey = 0x7F;
               }
            } else if (cmd == 0x06 && buf[1] == 0xF4 && p >= 3)
            {                   // Status
               if (keypadtamper && (buf[2] & 0x40))
                  status (keypad_tamper = "Case open");
               else
                  status (keypad_tamper = NULL);
               if (!send0B)
               {                // Key
                  toggle0B = !toggle0B;
                  if (buf[2] == 0x7F)
                  {             // No key
                     if (lastkey & 0x80)
                     {
                        if (keyhold < now)
                        {
                           revk_event ("gone", "%.1S", keymap + (lastkey & 0x0F));
                           lastkey = 0x7F;
                        }
                     } else
                        lastkey = 0x7F;
                  } else
                  {             // key
                     send0B = 1;
                     if ((lastkey & 0x80) && buf[2] != lastkey)
                        revk_event ("gone", "%.1S", keymap + (lastkey & 0x0F));
                     if (!(buf[2] & 0x80) || buf[2] != lastkey)
                        revk_event ((buf[2] & 0x80) ? "hold" : "key", "%.1S", keymap + (buf[2] & 0x0F));
                     if (buf[2] & 0x80)
                        keyhold = now + 2000000;
                     if (revk_offline ())
                     {          // Special case for safe mode (off line)
                        if (buf[2] == 0x0D)
                        {       // ESC in safe mode, shut up
                           sounderack = 1;
                           send0C = 1;
                        }
                        if (buf[2] == 0x8D)
                           revk_restart ("ESC", 0);     // ESC held in safe mode
                     }
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
            status (keypad_fault = "No response");
            online = 0;
         }
         rxwait = now + 3000000;
      } else
         rxwait = now + 250000;

      // Tx
      if (force || galaxybusfault || !online)
      {                         // Update all the shit
         force = 0;
         send07 = 1;
         send07a = 1;
         send07b = 1;
         send0B = 1;
         send0C = 1;
         send0D = 1;
         send19 = 1;
         sounderack = 0;
      }
      p = 0;
      if (!online)
      {                         // Init
         buf[++p] = 0x00;
         buf[++p] = 0x0E;
      } else if (send0B)
      {                         // key confirm
         send0B = 0;
         buf[++p] = 0x0B;
         buf[++p] = toggle0B ? 2 : 0;
      } else if (lastkey >= 0x7F && (send07 || send07a || send07b || send07c))
      {                         // Text
         buf[++p] = 0x07;
         buf[++p] = 0x01 | ((blink[0] & 1) ? 0x08 : 0x00) | (toggle07 ? 0x80 : 0);
         uint8_t len = display_len;
         uint8_t temp[33];
         uint8_t *dis = display;
         if (revk_offline ())
         {                      // Off line
            len = snprintf ((char *) temp, sizeof (temp), "%-16.16s%-9.9s %6.6s", revk_mqtt (), revk_wifi (), revk_id);
            dis = temp;
         }
         if (cursor_len)
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
         if (send07a || cursor_len)
         {                      // cursor
            buf[++p] = 0x03;
            buf[++p] = ((cursor[0] & 0x10) ? 0x40 : 0) + (cursor[0] & 0x0F);
            if (cursor[0] & 0x80)
               buf[++p] = 0x06; // Solid block
            else if (cursor[0] & 0x40)
               buf[++p] = 0x10; // Underline
         }
         toggle07 = !toggle07;
         if (send07)
            send07c = 1;        // always send twice
         else
            send07a = send07b = send07c = 0;    // sent
         send07 = 0;
      } else if (send19)
      {                         // Key keyclicks
         send19 = 0;
         buf[++p] = 0x19;
         if (revk_offline ())
            buf[++p] = 0x01;    // Sound normal
         else
            buf[++p] = (keyclick[0] & 0x7);     // 0x03 (silent), 0x05 (quiet), or 0x01 (normal)
         buf[++p] = 0;
      } else if (send0C)
      {                         // Beeper
         send0C = 0;
         uint8_t *s = sounder;
         uint8_t len = sounder_len;
         if (revk_offline () > 10)
         {
            if (sounderack)
               len = 0;         // quiet
            else
            {
               const uint8_t beepy[] = { 1, 1 };
               s = (uint8_t *) beepy;
               len = 2;
            }
         }
#if 0                           // TODO use output_get and configure which output
         else if (keypadbeepoverride)
         {
            const uint8_t beepy[] = { 1, 0 };
            s = (uint8_t *) beepy;
            len = 2;
         }
#endif
         buf[++p] = 0x0C;
         buf[++p] = (len ? s[1] ? 3 : 1 : 0);
         buf[++p] = (s[0] & 0x3F);      // Time on
         buf[++p] = (s[1] & 0x3F);      // Time off
      } else if (send0D)
      {                         // Light change
         send0D = 0;
         buf[++p] = 0x0D;
         if (revk_offline ())
            buf[++p] = 1;
         else
            buf[++p] = (backlight[0] & 1);
      } else
         buf[++p] = 0x06;       // Normal poll
      // Send
      buf[0] = keypadaddress;   // ID of display
      p++;
      cmd = buf[1];
      int l = galaxybus_tx (g, p, buf);
      if (keypaddebug && (buf[1] != 0x06 || l < 0))
         revk_info ("Tx", "%d: %02X %02X %02X %02X %s", p, buf[0], buf[1], buf[2], buf[3], l < 0 ? galaxybus_err_to_name (l) : "");
      if (l < 0)
         rxwait = now + 3000000;
   }
}

void
keypad_init (void)
{
#define u8(n,d) revk_register(#n,0,sizeof(n),&n,#d,0);
#define u8h(n,d) revk_register(#n,0,sizeof(n),&n,#d,SETTING_HEX);
#define b(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_BOOLEAN|SETTING_LIVE);
#define p(n) revk_register(#n,0,sizeof(n),&n,NULL,SETTING_SET);
   settings
#undef u8
#undef u8h
#undef b
#undef p
      if (keypadtx && keypadrx && keypadde)
   {
      const char *err = port_check (port_mask (keypadtx), TAG, 0);
      if (!err && keypadtx != keypadrx)
         port_check (port_mask (keypadrx), TAG, 1);
      if (!err)
         port_check (port_mask (keypadde), TAG, 0);
      if (!err && keypadre)
         port_check (port_mask (keypadre), TAG, 0);
      if (err && keypadde != keypadre)
         status (keypad_fault = err);
      galaxybus_t *g = galaxybus_init (keypadtimer, port_mask (keypadtx), port_mask (keypadrx), port_mask (keypadde),
                                       keypadre ? port_mask (keypadre) : -1,
                                       keypadclk ? port_mask (keypadclk) : -1,
                                       0);
      if (!g)
         status (keypad_fault = "Init failed");
      else
      {
         galaxybus_set_timing (g, keypadpre, keypadpost, keypadgap);
         revk_task (TAG, task, g);
      }
   } else if (keypadtx || keypadrx || keypadde)
      status (keypad_fault = "Set keypadtx, keypadrx and keypadde");
}
