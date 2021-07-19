// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"
const char *keypad_fault = NULL;
const char *keypad_tamper = NULL;

#include "galaxybus.h"
#include <driver/gpio.h>

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
#define sl(n) char *n;
settings
#undef u8
#undef u8h
#undef b
#undef p
#undef sl
#define f(id,name,len,def) static uint8_t name[len]={def};uint8_t send##id=false;uint8_t name##_len=0;
    commands
#undef  f
static volatile uint8_t force;

const char *keypad_command(const char *tag, jo_t j)
{
   char val[100];
   int len = 0;
   if (j && (len = jo_strncpy(j, val, sizeof(val))) < 0)
      val[len = 0] = 0;
   if (!strcmp(tag, "connect") || !strcmp(tag, "disconnect") || !strcmp(tag, "change"))
      force = 1;
#define f(i,n,l,d) if(!strcasecmp(tag,#n)&&len<=l){memcpy(n,val,len);n##_len=len;if(len<l)memset(n+len,0,l-len);send##i=1;return "";}
   commands
#undef f
       return NULL;
}

int64_t keypad_ui(char key)
{                               // Update display for UI
   ESP_LOGI(TAG, "UI %c", key);
   // TODO keypad display format...
   // TODO keypad beeping

   if (!key)
   {                            // To idle
      struct tm t;
      time_t now = time(0);
      localtime_r(&now, &t);
      char temp[50];
      snprintf(temp, sizeof(temp), "%-16.16s%04d-%02d-%02d %02d:%02d", *keypadidle ? keypadidle : revk_id, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
      memcpy(display, temp, display_len = 32);
      send07 = 1;
      return esp_timer_get_time() + 1000000LL * (60 - t.tm_sec);        // Next minute
   }
   return esp_timer_get_time() + 1000000LL;
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
         keypad_next = keypad_ui(0);

      static uint8_t buf[100],
       p = 0;
      static uint8_t cmd = 0;
      static uint8_t online = 0;
      static uint8_t send0B = 0;
      static uint8_t toggle0B = 0;
      static uint8_t toggle07 = 0;
      static uint8_t send07c = 0;
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
            ESP_LOGI(TAG, "Rx fail %s", galaxybus_err_to_name(p));
            if (galaxybusfault++ > 5)
            {
               status(keypad_fault = galaxybus_err_to_name(p));
               online = 0;
            }
            usleep(100000);
         } else
         {
            galaxybusfault = 0;
            status(keypad_fault = NULL);
            static int64_t keyhold = 0;
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
               status(keypad_tamper = NULL);
               if (!send0B)
               {
                  if (lastkey & 0x80)
                  {
                     if (keyhold < now)
                     {
                        jo_t j = jo_object_alloc();
                        jo_stringf(j, "key", "%.1s", keymap + (lastkey & 0x0F));
                        revk_event_clients("gone", &j, debug | (iotkeypad << 1));
                        lastkey = 0x7F;
                     }
                  } else
                     lastkey = 0x7F;
               }
            } else if (cmd == 0x06 && buf[1] == 0xF4 && p >= 3)
            {                   // Status
               if (keypadtamper && (buf[2] & 0x40))
                  status(keypad_tamper = "Case open");
               else
                  status(keypad_tamper = NULL);
               if (!send0B)
               {                // Key
                  if (buf[2] == 0x7F)
                  {             // No key
                     if (lastkey & 0x80)
                     {
                        if (keyhold < now)
                        {
                           jo_t j = jo_object_alloc();
                           jo_stringf(j, "key", "%.1s", keymap + (lastkey & 0x0F));
                           revk_event_clients("gone", &j, debug | (iotkeypad << 1));
                           lastkey = 0x7F;
                        }
                     } else
                        lastkey = 0x7F;
                  } else
                  {             // key
                     toggle0B = !toggle0B;
                     send0B = 1;
                     if ((lastkey & 0x80) && buf[2] != lastkey)
                     {
                        jo_t j = jo_object_alloc();
                        jo_stringf(j, "key", "%.1s", keymap + (lastkey & 0x0F));
                        revk_event_clients("gone", &j, debug | (iotkeypad << 1));
                     }
                     if (!(buf[2] & 0x80) || buf[2] != lastkey)
                     {
                        jo_t j = jo_object_alloc();
                        jo_stringf(j, "key", "%.1s", keymap + (buf[2] & 0x0F));
                        revk_event_clients((buf[2] & 0x80) ? "hold" : "key", &j, debug | (iotkeypad << 1));
                        if (!(buf[2] & 0x80))
                           keypad_next = keypad_ui(keymap[buf[2] & 0x0F]);
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
            status(keypad_fault = "No response");
            online = 0;
         }
         rxwait = now + 3000000LL;
      } else
         rxwait = now + 250000LL;

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
         uint8_t *dis = display;
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
         buf[++p] = (keyclick[0] & 0x7);        // 0x03 (silent), 0x05 (quiet), or 0x01 (normal)
         buf[++p] = 0;
      } else if (send0C)
      {                         // Beeper
         send0C = 0;
         uint8_t *s = sounder;
         uint8_t len = sounder_len;
         buf[++p] = 0x0C;
         buf[++p] = (len ? s[1] ? 3 : 1 : 0);
         buf[++p] = (s[0] & 0x3F);      // Time on
         buf[++p] = (s[1] & 0x3F);      // Time off
      } else if (send0D)
      {                         // Light change
         send0D = 0;
         buf[++p] = 0x0D;
         buf[++p] = (backlight[0] & 1);
      } else
         buf[++p] = 0x06;       // Normal poll
      // Send
      buf[0] = keypadaddress;   // ID of display
      p++;
      cmd = buf[1];
      int l = galaxybus_tx(g, p, buf);
      if (l < 0)
      {
         ESP_LOGI(TAG, "Tx fail %s", galaxybus_err_to_name(l));
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
      const char *err = port_check(port_mask(keypadtx), TAG, 0);
      if (!err && keypadtx != keypadrx)
         err = port_check(port_mask(keypadrx), TAG, 1);
      if (!err)
         err = port_check(port_mask(keypadde), TAG, 0);
      if (!err && keypadde != keypadre)
         err = port_check(port_mask(keypadre), TAG, 0);
      gpio_set_pull_mode(port_mask(keypadrx), GPIO_PULLUP_ONLY);
      status(keypad_fault = err);
      // Done early because it beeps a lot!
      revk_task(TAG, task, NULL);
   } else if (keypadtx || keypadrx || keypadde)
      status(keypad_fault = "Set keypadtx, keypadrx, keypadde and keypadre");
}

void keypad_start(void)
{
}
