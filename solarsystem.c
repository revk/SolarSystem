// Back end management / control for SolarSystem modules

#define _GNU_SOURCE             /* See feature_test_macros(7) */
typedef unsigned int uint32_t;
#include "config.h"
#ifdef BUILD_ESP32_USING_CMAKE
#include "ESP32/build/config/sdkconfig.h"
#else
#include "ESP32/build/include/sdkconfig.h"
#endif
#include "ESP32/main/areas.h"
#include <stdio.h>
#include <string.h>
#include <popt.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <syslog.h>
#include <openssl/evp.h>
#include <libemail.h>
#include "SQLlib/sqllib.h"
#include "AJL/ajlcurl.h"
#include "ssmqtt.h"
#include "sscert.h"
#include "ssdatabase.h"
#include "ssafile.h"
#include "fobcommand.h"

#define MAX_MIX 5

const char *cakey = "",
   *cacert = "";
const char *mqttkey = "",
   *mqttcert = "";

extern int sqldebug;
int dump = 0;                   // dump level debug
int mqttdump = 0;               // mqtt logging

CURL *curl = NULL;

int poke = 1;                   // Poke devices

static void addset (j_t j, const char *tag, const char *val, const char *always);
static void addsitedata (SQL * sqlp, j_t j, SQL_RES * site, const char *deviceid, const char *parentid, char outofservice);

#define AES_STRING_LEN	(KEY_DATA_LEN*2+1)      // Type/Ver/Key in hex with terminator

void
aidver (SQL * sqlp, const char *aid)
{                               // Update AID version list
   sql_s_t q = { 0 };
   sql_sprintf (&q, "UPDATE `aid` SET ");
   SQL_RES *a = sql_safe_query_store_f (sqlp, "SELECT * FROM `aid` WHERE `aid`=%#s", aid);
   sql_fetch_row (a);
   SQL_RES *r =
      sql_safe_query_store_f (sqlp, "SELECT * FROM `%#S`.`AES` WHERE `aid`=%#s AND `fob`='' order BY `created` DESC LIMIT 3",
                              CONFIG_SQL_KEY_DATABASE, aid);
   sql_fetch_row (r);           // queries will be null if off end
   if (strcmp (sql_colz (a, "ver1"), sql_colz (r, "ver")) || strcmp (sql_colz (a, "ver1date"), sql_colz (r, "created")))
      sql_sprintf (&q, "`ver1`=%#s,`ver1date`=%#s,", sql_col (r, "ver"), sql_col (r, "created"));
   sql_fetch_row (r);           // queries will be null if off end
   if (strcmp (sql_colz (a, "ver2"), sql_colz (r, "ver")) || strcmp (sql_colz (a, "ver2date"), sql_colz (r, "created")))
      sql_sprintf (&q, "`ver2`=%#s,`ver2date`=%#s,", sql_col (r, "ver"), sql_col (r, "created"));
   sql_fetch_row (r);           // queries will be null if off end
   if (strcmp (sql_colz (a, "ver3"), sql_colz (r, "ver")) || strcmp (sql_colz (a, "ver3date"), sql_colz (r, "created")))
      sql_sprintf (&q, "`ver3`=%#s,`ver3date`=%#s,", sql_col (r, "ver"), sql_col (r, "created"));
   sql_free_result (r);
   sql_free_result (a);
   if (sql_back_s (&q) == ',')
   {
      sql_sprintf (&q, " WHERE `aid`=%#s", aid);
      sql_safe_query_s (sqlp, &q);
   } else
      sql_free_s (&q);
}

char *
makeaes (SQL * sqlp, char *target, const char *aid, const char *fob)
{                               // Make an AES key (HEX ver and AES, so AES_STRING_LEN byte buffer)
   int try = 10;
   while (try--)
   {
      unsigned char bin[KEY_DATA_LEN] = { };    // Type, Ver, Key.
      randkey (bin);
      if (aid && !fob)
         *bin = 2;              // Encrypted AID key
      j_base16N (sizeof (bin), bin, AES_STRING_LEN, target);
      if (sql_query_f
          (sqlp, "INSERT INTO `%#S`.`AES` SET `aid`=%#s,`fob`=%#s,`type`=%#.2s,`ver`=%#.2s,`key`=%#.32s", CONFIG_SQL_KEY_DATABASE,
           aid ? : "", fob ? : "", target + 0, target + 2, target + 4))
         continue;
      aidver (sqlp, aid);
      sql_safe_query_f (sqlp, "UPDATE `device` SET `poke`=NOW() WHERE `aid`=%#s AND `online` IS NOT NULL", aid);
      if (sql_affected_rows (sqlp))
         poke = 1;
      return target;
   }
   *target = 0;
   return target;
}

char *
getaes (SQL * sqlp, char *target, const char *aid, const char *fob)
{                               // Get AES key (HEX ver and AES, so AES_STRING_LEN byte buffer)
   // TODO encrypt flag as extra byte on end
   SQL_RES *res =
      sql_safe_query_store_f (sqlp, "SELECT * FROM `%#S`.`AES` WHERE `aid`=%#s AND `fob`=%#s ORDER BY `created` DESC LIMIT 1",
                              CONFIG_SQL_KEY_DATABASE, aid ? : "", fob ? : "");
   if (sql_fetch_row (res))
   {
      snprintf (target, AES_STRING_LEN, "%s%s%s", sql_col (res, "type"), sql_col (res, "ver"), sql_col (res, "key"));
      return target;
   }
   return makeaes (sqlp, target, aid, fob);
}

void
send_message (SQL_RES * res, const char *ud, const char *n)
{                               // SMS
   const char *u = sql_colz (res, "smsuser");
   const char *p = sql_colz (res, "smspass");
   if (*u && *p && *n)
   {
      const char *f = sql_colz (res, "smsfrom");
      j_t s = j_create ();
      j_store_string (s, "username", u);
      j_store_string (s, "password", p);
      j_store_string (s, "da", n);
      if (*f)
         j_store_string (s, "oa", f);
      j_store_string (s, "ud", ud);
      if (!fork ())
      {
         j_t r = j_create ();
         if (sqldebug)
            j_err (j_write_pretty (s, stderr));
         j_curl_send (curl, s, r, NULL, "https://sms.aa.net.uk");
         if (sqldebug)
            j_err (j_write_pretty (r, stderr));
         _exit (0);
      }
      j_delete (&s);
   }
}

void
notify (SQL * sqlp, SQL_RES * res, const char *target, j_t j)
{                               // Generic notify (passed site res)
   if (!target || !j || j_isnull (j))
      return;
   int site = atoi (sql_colz (res, "site"));
   j_store_string (j, "site", sql_colz (res, "sitename"));
   char *event = strdupa (j_get (j, "event") ? : "event");
   *event = toupper (*event);
   char *ud = NULL;             // SMS content
   void makeud (void)
   {
      size_t l = 0;
      FILE *f = open_memstream (&ud, &l);
      fprintf (f, "%s\n", event);
      void areas (const char *tag)
      {
         char found = 0;
         const char *areas = j_get (j, tag);
         if (areas)
            while (*areas)
            {
               if (!found++)
                  fprintf (f, "%s: ", tag);
               else
                  fprintf (f, ", ");
               SQL_RES *a = sql_safe_query_store_f (sqlp, "SELECT * FROM `area` WHERE `site`=%d AND `tag`=%#c", site, *areas);
               if (sql_fetch_row (a))
                  fprintf (f, "%s", sql_colz (a, "areaname"));
               else
                  fprintf (f, "%c", *areas);
               sql_free_result (a);
               areas++;
            }
         if (found)
            fprintf (f, "\n");
      }
#define extras c(,areas)c(,also)c(,disarmok)c(,armok)c(,strongok)
#define s(t,x,l) areas(#x);
#define i(t,x,l) areas(#x);
#define c(t,x) areas(#x);
      extras;
#include "ESP32/main/states.m"
      j_t e;
      for (e = j_first (j); e; e = j_next (e))
      {
         const char *tag = j_name (e);
         if (strcmp (tag, "event") && strcmp (tag, "areas") && strcmp (tag, "ts") && strcmp (tag, "also")
#define s(t,x,l) &&strcmp(tag,#x)
#define i(t,x,l) &&strcmp(tag,#x)
#define c(t,x) &&strcmp(tag,#x)
             extras
#include "ESP32/main/states.m"
            )
         {
            fprintf (f, "%s: ", tag);
            if (j_isstring (e) || j_isbool (e))
               fprintf (f, "%s", j_val (e));
            else if (j_isarray (e))
            {
               char found = 0;
               j_t s;
               for (s = j_first (e); s; s = j_next (s))
               {
                  if (found++)
                     fprintf (f, ", ");
                  fprintf (f, "%s", j_val (s));
               }
            }
            fprintf (f, "\n");
         }
      }
      time_t ts = j_time (j_get (j, "ts"));
      if (ts)
      {
         struct tm tm;
         localtime_r (&ts, &tm);
         fprintf (f, "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
                  tm.tm_sec);
      }
      fclose (f);
   }
   char *t = strdupa (target);
   while (*t)
   {
      while (isspace (*t))
         t++;
      if (!*t)
         break;
      char *e = t;
      while (*e && !isspace (*e))
         e++;
      if (*e)
         *e++ = 0;
      if (!strncasecmp (t, "http://", 7) || !strncasecmp (t, "https://", 8))
      {                         // Let's assume a URL to post to
         if (!fork ())
         {
            j_curl_send (curl, j, NULL, sql_col (res, "hookbearer"), "%s", t);
            _exit (0);
         }
      } else if (strchr (t, '@'))
      {
         if (!ud)
            makeud ();
         if (!fork ())
         {
            FILE *f = NULL;
            email_t e = email_new (&f);
            fprintf (f, "%s", ud);
            const char *from = sql_col (res, "emailfrom");
            if (!from || !*from)
               from = "alarm@access.me.uk";
            email_subject (e, "Alarm message:%s", event);
            email_address (e, "To", t, NULL);
            email_address (e, "From", from, sql_col (res, "sitename"));
            FILE *a = NULL;     // JSON attachment
            email_add (e, "event.json", NULL, &a);
            j_err (j_write (j, a));
            const char *er = email_send (e, 0);
            if (er)
               warnx ("Email failed %s: %s", t, er);
            _exit (0);
         }
      }                         // Let's assume it is an email address
      else if (*t == '+' || isdigit (*t))
      {                         // Let's assume it is a number to SMS
         if (!ud)
            makeud ();
         send_message (res, ud, t);
      } else
         warnx ("Unknown notify %s", t);
      t = e;
   }
   free (ud);
}

const char *
upgrade (SQL_RES * res, slot_t id)
{                               // Send upgrade if needed
   const char *device = sql_colz (res, "device");
   const char *upgrade = sql_col (res, "upgrade");
   if (!upgrade || j_time (upgrade) > time (0))
      return NULL;
   const char *build_suffix = sql_col (res, "build_suffix");
   if (build_suffix && *build_suffix)
   {
      j_t j = j_create ();
      j_stringf (j, "/SS%s.bin", build_suffix);
      slot_send (id, "command", device, "upgrade", &j); // Use specific file based on build suffix for target device
   } else
      slot_send (id, "command", device, "upgrade", NULL);       // Use pre-set URL
   return upgrade;
}

static const char *
settings (SQL * sqlp, SQL_RES * res, slot_t id)
{                               // Security and settings
   j_t j = j_create ();
   j_store_string (j, "nodename", sql_colz (res, "devicename"));
   char outofservice = (*sql_colz (res, "outofservice") == 't');
   // Security
   const char *aid = sql_colz (res, "aid");
   char nfc = (*sql_colz (res, "nfc") == 't');
   j_t aids = j_store_array (j, "aes");
   if (*aid && nfc)
   {                            // Security
      // Keys
      SQL_RES *r =
         sql_safe_query_store_f (sqlp, "SELECT * FROM `%#S`.`AES` WHERE `aid`=%#s AND `fob`='' order BY `created` DESC LIMIT 3",
                                 CONFIG_SQL_KEY_DATABASE, aid);
      while (sql_fetch_row (r))
         j_append_stringf (aids, "%s%s%s", sql_colz (r, "type"), sql_colz (r, "ver"), sql_colz (r, "key"));
      sql_free_result (r);
      if (!j_len (aids))
         j_append_string (aids, getaes (sqlp, alloca (AES_STRING_LEN), aid, NULL));     // Make a key
   }
   if (!j_len (aids))
      aid = "";                 // We have not loaded any keys so no point in even trying an AID
   j_store_string (j, "aid", aid);
   j_t blacklist = j_store_array (j, "blacklist");
   if (*aid)
   {
      int organisation = atoi (sql_colz (res, "organisation"));
      SQL_RES *b = sql_safe_query_store_f (sqlp,
                                           "SELECT * FROM `foborganisation` WHERE `organisation`=%d AND `blocked` IS NOT NULL AND `confirmed` IS NULL ORDER BY `blocked` DESC LIMIT 10",
                                           organisation);
      while (sql_fetch_row (b))
         j_append_string (blacklist, sql_colz (b, "fob"));
      sql_free_result (b);
   }
   // Other settings
   if (*CONFIG_OTA_HOSTNAME)
      j_store_string (j, "otahost", CONFIG_OTA_HOSTNAME);
   int site = atoi (sql_colz (res, "site"));
   {                            // site
      SQL_RES *s = sql_safe_query_store_f (sqlp, "SELECT * FROM `site` WHERE `site`=%d", site);
      if (sql_fetch_row (s))
      {
         addsitedata (sqlp, j, s, sql_colz (res, "device"), sql_col (res, "via"), outofservice);
         if (!outofservice)
         {
            j_t iot = j_store_object (j, "iot");
            if (*sql_colz (s, "iothost"))
            {                   // Only if IOT host
               const char *v;
               if ((v = sql_colz (s, "iottopic")) && *v)
                  j_store_string (iot, "topic", v);
               if (*sql_colz (res, "iotstatedoor") == 't')
                  j_store_true (iot, "statedoor");
               if (*sql_colz (res, "iotstateinput") == 't')
                  j_store_true (iot, "stateinput");
               if (*sql_colz (res, "iotstateoutput") == 't')
                  j_store_true (iot, "stateoutput");
               if (*sql_colz (s, "iotstatesystem") == 't')
                  j_store_true (iot, "statesystem");
               if (*sql_colz (res, "ioteventfob") == 't')
                  j_store_true (iot, "eventfob");
               if (*sql_colz (s, "ioteventarm") == 't')
                  j_store_true (iot, "eventarm");
               if (*sql_colz (res, "iotkeypad") == 't')
                  j_store_true (iot, "keypad");
               if (*sql_colz (res, "iotgps") == 't')
                  j_store_true (iot, "gps");
            }
         }
      }
      sql_free_result (s);
   }
   if (!outofservice)
   {
      j_t door = j_store_object (j, "door");
      if (*sql_colz (res, "door") == 't')
      {                         // Door specific
         j_store_int (door, "auto", 5);
         if (*sql_colz (res, "doorexitarm") == 't')
            j_store_boolean (door, "exitarm", 1);
         if (*sql_colz (res, "doorexitdisarm") == 't')
            j_store_boolean (door, "exitdisarm", 1);
         if (*sql_colz (res, "doordebug") == 't')
            j_store_boolean (door, "debug", 1);
         if (*sql_colz (res, "doorcatch") == 't')
            j_store_boolean (door, "catch", 1);
         int v;                 // We don't send 0, so using default, but -ve means we send explicit 0
         if ((v = atoi (sql_colz (res, "doorunlock"))))
            j_store_int (door, "unlock", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "doorlock"))))
            j_store_int (door, "lock", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "dooropen"))))
            j_store_int (door, "open", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "doorclose"))))
            j_store_int (door, "close", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "doorprop"))))
            j_store_int (door, "prop", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "doorexit"))))
            j_store_int (door, "exit", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "doorpoll"))))
            j_store_int (door, "poll", v < 0 ? 0 : v);
         if ((v = atoi (sql_colz (res, "doordebounce"))))
            j_store_int (door, "debounce", v < 0 ? 0 : v);
         const char *t;
         if ((t = sql_colz (res, "dooriotunlock")) && *t)
            j_store_string (door, "iotunlock", t);
         if ((t = sql_colz (res, "dooriotdead")) && *t)
            j_store_string (door, "iotdead", t);
         if ((t = sql_colz (res, "dooriotundead")) && *t)
            j_store_string (door, "iotundead", t);
      }
      j_t area = j_store_object (j, "area");
      addset (area, "enter", sql_colz (res, "areaenter"), NULL);
      addset (area, "arm", sql_colz (res, "areaarm"), NULL);
      addset (area, "strong", sql_colz (res, "areastrong"), NULL);
      addset (area, "disarm", sql_colz (res, "areadisarm"), NULL);
      addset (area, "led", sql_colz (res, "arealed"), NULL);
      addset (area, "deadlock", sql_colz (res, "areadeadlock"), NULL);
      int pcb = atoi (sql_colz (res, "pcb"));
      if (pcb)
      {                         // Main PCB settings
         SQL_RES *p = sql_safe_query_store_f (sqlp, "SELECT * FROM `pcb` WHERE `pcb`=%d", pcb);
         if (sql_fetch_row (p))
         {
            const char *v;
            if ((v = sql_colz (res, "timer1")) && *v && strlen (v) == 8)
               j_store_int (j, "timer1", atoi (v) * 100 + atoi (v + 3));
            j_t o = j_store_object (j, "keypad");
#define set(h,c) {const char *v=sql_colz(p,#h#c);if(v&&strcmp(v,"-"))j_store_literal(o,#c,v);}
            set (keypad, tx);
            set (keypad, rx);
            set (keypad, re);
            set (keypad, de);
            if (j_len (o))
            {                   // Keypad specific
               if ((v = (outofservice ? "Out of service" : sql_colz (res, "keypadidle"))) && *v)
                  j_store_string (o, "idle", v);
               if ((v = sql_colz (res, "keypadpin")) && *v)
                  j_store_string (o, "pin", v);
               addset (area, "keypad", sql_colz (res, "areakeypad"), NULL);
               addset (area, "keyarm", sql_colz (res, "areakeyarm"), NULL);
               addset (area, "keydisarm", sql_colz (res, "areakeydisarm"), NULL);
            }
            o = j_store_object (j, "nfc");
            set (nfc, tx);
            set (nfc, rx);
            set (nfc, power);
            set (nfc, red);
            set (nfc, amber);
            set (nfc, green);
            set (nfc, card);
            o = j_store_object (j, "gps");
            set (gps, tx);
            set (gps, rx);
            set (gps, tick);
#undef set
            j_t blink = j_store_array (j, "blink");
#define led(n) {const char *v=sql_colz(p,#n);if(!*v||!strcmp(v,"-"))j_append_string(blink,""); else j_append_literal(blink,v);}
            if (strcmp (sql_colz (p, "leda"), "-"))
            {
               led (leda);
            } else
            {
               led (ledr);
               led (ledg);
               led (ledb);
            }
#undef led
         }
         sql_free_result (p);
         j_t input = j_store_array (j, "in");
         j_t output = j_store_array (j, "out");
         j_t power = j_store_array (j, "power");
         SQL_RES *g = sql_safe_query_store_f (sqlp,
                                              "SELECT * FROM `devicegpio` LEFT JOIN `gpio` USING (`gpio`) WHERE `device`=%#s AND `pcb`=%d ORDER BY `initname`",
                                              sql_col (res, "device"), atoi (sql_colz (res, "pcb")));
         while (sql_fetch_row (g))
         {
            const char *type = sql_colz (g, "type");
            j_t gpio = NULL;
            if (*type == 'P')
               gpio = j_append_null (power);
            else if (*type == 'I')
               gpio = j_append_null (input);
            else if (*type == 'O')
               gpio = j_append_null (output);
            if (gpio)
            {
               const char *extra = NULL;
               const char *pin = sql_colz (g, "pin");
               int invert = (*sql_colz (g, "invert") == 't');
               if (*pin == '-')
               {
                  pin++;
                  invert = 1 - invert;
                  if (*type == 'I')
                     extra = "P";       // Pull down on input
               }
               if (*pin)
               {
                  j_store_literalf (gpio, "gpio", "%s%s", (invert ? "-" : ""), pin);
                  if (*type != 'P')
                  {
                     const char *name = sql_colz (g, "name");
                     if (!*name)
                        name = sql_colz (g, "initname");
                     if (*name)
                        j_store_string (gpio, "name", name);
                  }
                  if (*type == 'I')
                  {
                     int hold = atoi (sql_colz (g, "hold"));
                     if (hold)
                        j_store_int (gpio, "hold", hold);
                  }
                  if (*type == 'O')
                  {
                     int pulse = atoi (sql_colz (g, "pulse"));
                     if (pulse)
                        j_store_int (gpio, "pulse", pulse);
                  }
                  if (*type != 'P')
                  {
                     addset (gpio, "func", sql_col (g, "func"), extra);
#define i(t,n,c) addset(gpio,#n,sql_col(g,#n),NULL);
#define c(t,n) addset(gpio,#n,sql_col(g,#n),NULL);
#define s(t,n,c) addset(gpio,#n,sql_col(g,#n),NULL);
#include "ESP32/main/states.m"
                  }
               }
            }
         }
         // We send {} not null as (a) smaller, and (b) ensures content is zapped
         for (int i = 0; i < j_len (output); i++)
            if (j_isnull (j_index (output, i)))
               j_object (j_index (output, i));
         for (int i = 0; i < j_len (input); i++)
            if (j_isnull (j_index (input, i)))
               j_object (j_index (input, i));
         for (int i = 0; i < j_len (power); i++)
            if (j_isnull (j_index (power, i)))
               j_object (j_index (power, i));
         sql_free_result (g);
      }
   }
   if (!j_isnull (j))
      slot_send (id, "setting", sql_colz (res, "device"), NULL, &j);
   j_delete (&j);
   return NULL;
}

static void
addset (j_t j, const char *tag, const char *val, const char *always)
{
   if (!always && (!val || !*val))
      return;
   if (!val)
      val = "";
   char v[65],                  // Allow for big sets, and certainly maximum size area
    *p = v,
      *e = v + sizeof (v) - 1;
   while (*val && p < e)
   {
      if (*val != ',')
         *p++ = *val;
      val++;
   }
   if (always)
      while (*always && p < e)
         *p++ = *always++;
   *p = 0;
   j_store_string (j, tag, v);
}

static void
addsitedata (SQL * sqlp, j_t j, SQL_RES * site, const char *deviceid, const char *parentid, char outofservice)
{
   const char *v;
   if (!parentid || !*parentid || !strcmp (parentid, deviceid))
      parentid = NULL;          // Not sensible
   // Mix
   j_t mix = j_store_array (j, "mix");
   for (int s = 0; s < MAX_MIX; s++)
   {
      char temp[30];
      sprintf (temp, "mixand%d", s + 1);
      if ((v = sql_colz (site, temp)) && *v)
      {
         j_t j = j_append_object (mix);
         addset (j, "and", v, 0);
         sprintf (temp, "mixset%d", s + 1);
         if ((v = sql_colz (site, temp)) && *v)
            addset (j, "set", v, 0);
      }
   }
   // Standard wifi settings
   v = sql_colz (site, "iothost");
   j_store_string (j, "mqtthost2", *v ? v : NULL);
   addset (j, "engineer", sql_colz (site, "engineer"), "");
   j_store_int (j, "armcancel", atoi (sql_colz (site, "armcancel")) ? : 60);    // Force a default
   j_store_int (j, "armdelay", atoi (sql_colz (site, "armdelay")));
   j_store_int (j, "alarmdelay", atoi (sql_colz (site, "alarmdelay")));
   j_store_int (j, "alarmhold", atoi (sql_colz (site, "alarmhold")));
   j_store_boolean (j, "debug", *sql_colz (site, "debug") == 't');
   j_t wifi = j_store_object (j, "wifi");
   if ((v = sql_colz (site, "wifissid")) && *v)
      j_store_string (wifi, "ssid", v);
   if ((v = sql_colz (site, "wifipass")) && *v)
      j_store_string (wifi, "pass", v);
   if (!outofservice && (v = sql_colz (site, "wifichan")) && atoi (v))
      j_store_int (wifi, "chan", atoi (v));
   if (!outofservice && (v = sql_colz (site, "wifibssid")) && strlen (v) == 12)
      j_store_string (wifi, "bssid", v);
   j_t sms = j_store_object (j, "sms");
   addset (sms, "arm", sql_col (site, "smsarm"), 0);
   addset (sms, "disarm", sql_col (site, "smsdisarm"), 0);
   addset (sms, "cancel", sql_col (site, "smscancel"), 0);
   addset (sms, "armfail", sql_col (site, "smsarmfail"), 0);
   addset (sms, "alarm", sql_col (site, "smsalarm"), 0);
   addset (sms, "panic", sql_col (site, "smspanic"), 0);
   addset (sms, "fire", sql_col (site, "smsfire"), 0);
   if (j_len (sms) && (v = sql_colz (site, "smsnumber")) && *v)
      j_store_string (sms, "number", v);
#ifdef	CONFIG_REVK_MESH
   j_t mesh = j_store_object (j, "mesh");
   j_store_int (mesh, "cycle", 2);
   if (outofservice || *sql_colz (site, "nomesh") == 't')
   {                            // Not making a mesh, so set a mesh of 1
      j_store_string (mesh, "id", deviceid);
      j_store_string (mesh, "pass", deviceid);
      j_store_int (mesh, "max", 1);
   } else
   {
      v = sql_colz (site, "meshid");
      if (!*v)
      {                         // Make a mesh ID
         int tries = 100;
         while (tries--)
         {
            unsigned char mac[6];
            char smac[13];
            randblock (mac, sizeof (mac));
            mac[0] &= 0xFE;     // Non broadcast
            mac[0] |= 0x02;     // Local
            j_base16N (sizeof (mac), mac, sizeof (smac), smac);
            if (!sql_query_f (sqlp, "UPDATE `site` SET `meshid`=%#s WHERE `site`=%#s", smac, sql_col (site, "site")))
            {
               v = strdupa (smac);
               break;
            }
         }
      }
      if (*v)
         j_store_string (mesh, "id", v);
      v = sql_colz (site, "meshkey");
      if (!*v)
      {                         // Make a mesh ID
         int tries = 100;
         while (tries--)
         {
            unsigned char key[16];
            char skey[33];
            randblock (key, sizeof (key));
            j_base16N (sizeof (key), key, sizeof (skey), skey);
            if (!sql_query_f (sqlp, "UPDATE `site` SET `meshkey`=%#s WHERE `site`=%#s", skey, sql_col (site, "site")))
            {
               v = strdupa (skey);
               break;
            }
         }
      }
      if (*v)
         j_store_string (mesh, "key", v);
      v = sql_colz (site, "meshpass");
      if (!*v)
      {                         // Make mesh passphrase
         int tries = 100;
         while (tries--)
         {
            unsigned char pass[24];
            char spass[33];
            randblock (pass, sizeof (pass));
            j_base64N (sizeof (pass), pass, sizeof (spass), spass);
            if (!sql_query_f (sqlp, "UPDATE `site` SET `meshpass`=%#s WHERE `site`=%#s", spass, sql_col (site, "site")))
            {
               v = strdupa (spass);
               break;
            }
         }
      }
      if (*v)
         j_store_string (mesh, "pass", v);
      SQL_RES *res = sql_safe_query_store_f (sqlp,
                                             "SELECT SUM(if(`outofservice`='false',1,0)) AS `N`,COUNT(*) AS `T` FROM `device` WHERE `site`=%#s",
                                             sql_col (site, "site"));
      if (sql_fetch_row (res))
      {
         j_store_int (mesh, "max", atoi (sql_colz (res, "T")));
         j_store_int (mesh, "expect", atoi (sql_colz (res, "N")));
      }
      sql_free_result (res);
      if (*sql_colz (site, "meshlr") == 't')
         j_store_true (mesh, "lr");
      if (!strcmp (sql_colz (site, "root"), deviceid))
         j_store_true (mesh, "root");
   }
#endif
}

void
bogus (slot_t id)
{                               // This is bogus auth
   j_t m = j_create ();
   j_store_string (m, "clientkey", "");
   j_store_string (m, "clientcert", "");
   slot_send (id, "setting", NULL, NULL, &m);
}

void
daily (SQL * sqlp)
{                               // Daily tasks and clean up
   time_t now = time (0);
   sql_safe_query (sqlp, "DELETE FROM `session` WHERE `expires`<NOW()");        // Old sessions
   sql_safe_query (sqlp, "DELETE FROM `event` WHERE `logged`<DATE_SUB(NOW(),INTERVAL 1 MONTH)");        // Old event logs
   SQL_RES *a = sql_safe_query_store (sqlp, "SELECT * FROM `aid` LEFT JOIN `site` USING (`site`)");
   while (sql_fetch_row (a))
   {
      const char *aid = sql_colz (a, "aid");
      time_t ver1date = sql_time (sql_colz (a, "ver1date"));
      int rollover = atoi (sql_colz (a, "rollover"));
      if (rollover && ver1date && (ver1date / 86400) * 86400 + rollover * 86400 <= now)
      {
         makeaes (sqlp, alloca (AES_STRING_LEN), aid, NULL);    // New key
         continue;
      }
      const char *ver1 = sql_col (a, "ver1");
      int changed = 0;
      SQL_RES *r =
         sql_safe_query_store_f (sqlp, "SELECT * FROM `%#S`.`AES` WHERE `aid`=%#s AND `fob`=''", CONFIG_SQL_KEY_DATABASE, aid);
      if (!sql_num_rows (r))
         makeaes (sqlp, alloca (AES_STRING_LEN), aid, NULL);    // No keys exist, make the current key
      else if (!ver1 || !ver1date)
         aidver (sqlp, aid);    // ver1/ver1date is not set, so set it
      else
         while (sql_fetch_row (r))
         {                      // Check for keys we can delete
            const char *ver = sql_colz (r, "ver");
            if (strcmp (ver, ver1))
            {
               SQL_RES *f =
                  sql_safe_query_store_f (sqlp, "SELECT COUNT(*) AS `N` FROM `fobaid` WHERE `aid`=%#s AND `ver`=%#s", aid, ver);
               if (!sql_fetch_row (f) || !atoi (sql_colz (f, "N")))
               {                // No fobs using this version - remove it
                  sql_safe_query_f (sqlp, "DELETE FROM `%#S`.`AES` WHERE `aid`=%#s AND `fob`='' AND `ver`=%#s",
                                    CONFIG_SQL_KEY_DATABASE, aid, ver);
                  changed++;
               }
               sql_free_result (f);
            }
         }
      sql_free_result (r);
      if (changed)
      {
         aidver (sqlp, aid);
         sql_safe_query_f (sqlp, "UPDATE `device` SET `poke`=NOW() WHERE `aid`=%#s AND `online` IS NOT NULL", aid);
         if (sql_affected_rows (sqlp))
            poke = 1;
      }
   }
   sql_free_result (a);
}

void
doupgrade (SQL * sqlp, int site)
{                               // Poking upgrades that may need doing - pick one per site, as site is one at a time
   SQL_RES *device = sql_safe_query_store_f (sqlp,
                                             "SELECT * FROM `device` WHERE `site`=%d AND `upgrade`<=NOW() AND `id` IS NOT NULL ORDER BY if(`outofservice`='true',0,if(`via` IS NOT NULL,1,2)),`devicename` LIMIT 1",
                                             site);
   while (sql_fetch_row (device))
   {
      slot_t id = strtoull (sql_colz (device, "id"), NULL, 10);
      upgrade (device, id);
   }
   sql_free_result (device);
}

void
dooffline (SQL * sqlp, int site)
{                               // Check offline sites
   SQL_RES *res = sql_safe_query_store_f (sqlp, "SELECT * FROM `site` WHERE `site`=%d", site);
   if (sql_fetch_row (res))
   {
      j_t j = j_create (),
         l = NULL;
      SQL_RES *device = sql_safe_query_store_f (sqlp,
                                                "SELECT * FROM `device` WHERE `site`=%d AND `offlinereport` IS NULL AND `online` IS NULL AND `outofservice`='false'",
                                                site);
      while (sql_fetch_row (device))
      {
         if (!l)
         {
            j_store_string (j, "event", "offline");
            struct tm tm;
            time_t now = time (0);
            localtime_r (&now, &tm);
            j_store_stringf (j, "ts", "%04d-%02d-%02d %02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
                             tm.tm_min);
            l = j_store_array (j, "devices");
         }
         j_append_string (l, sql_colz (device, "devicename"));
         sql_safe_query_f (sqlp, "UPDATE `device` SET `offlinereport`=NOW() WHERE `device`=%#s", sql_colz (device, "device"));
      }
      sql_free_result (device);
      if (l)
      {
         notify (sqlp, res, sql_col (res, "hookoffline"), j);
         j_delete (&j);
      }
   }
   sql_free_result (res);
}

void
dopoke (SQL * sqlp)
{                               // Poking that may need doing
   SQL_RES *res = sql_safe_query_store (sqlp, "SELECT * FROM `device` WHERE `poke`<=NOW() AND `id` IS NOT NULL ORDER BY `poke`");
   while (sql_fetch_row (res))
   {
      slot_t id = strtoull (sql_colz (res, "id"), NULL, 10);
      sql_safe_query_f (sqlp, "UPDATE `device` SET `poke`=NULL WHERE `device`=%#s", sql_col (res, "device"));
      settings (sqlp, res, id);
   }
   sql_free_result (res);
}

const char *
forkcommand (j_t * jp, slot_t device, slot_t local)
{
   j_t j = *jp;
   *jp = 0;
   int relaysock;
   slot_t id = slot_create (&relaysock, "local");
   if (!id)
      return "Link failed";
   slot_link (device, id);
   j_store_int (j, "id", id);
   if (device)
      j_store_int (j, "device", device);
   if (local)
      j_store_int (j, "local", local);
   j_store_int (j, "socket", relaysock);
   pthread_t t;
   if (pthread_create (&t, NULL, fobcommand, j))
      err (1, "Cannot create fob adopt thread");
   pthread_detach (t);
   return NULL;
}

const char *
doapi (SQL * sqlp, slot_t local, j_t meta, j_t j)
{
   local = local;
   SQL_RES *res;
   int organisation = atoi (j_get (meta, "organisation") ? : "");
   int user = atoi (j_get (meta, "api") ? : "");
   if (!organisation)
      return "No organisation";
   if (!user)
      return "No user";
   res =
      sql_safe_query_store_f (sqlp, "SELECT * FROM `userorganisation` WHERE `user`=%d AND `organisation`=%d", user, organisation);
   if (!sql_fetch_row (res))
   {
      sql_free_result (res);
      return "No user found";
   }
   int canapi = (*sql_colz (res, "canapi") == 't');
   int apiexpires = (*sql_colz (res, "apiexpires") == 't');
   int apiarm = (*sql_colz (res, "apiarm") == 't');
   int apistrong = (*sql_colz (res, "apistrong") == 't');
   int apidisarm = (*sql_colz (res, "apidisarm") == 't');
   sql_free_result (res);
   if (!canapi)
      return "No API access";
   const char *api = j_get (j, "command");
   if (!api)
      return "No API command";
   if (!strcmp (api, "expires"))
   {
      if (!apiexpires)
         return "Not allowed";
      const char *fob = j_get (j, "fob");
      if (!fob)
         return "No fob";
      time_t expires = j_time (j_get (j, "expires"));
      if (expires)
         sql_safe_query_f (sqlp, "UPDATE `foborganisation` SET `expires`=%#T WHERE `fob`=%#s AND `organisation`=%d", expires, fob,
                           organisation);
      else
         sql_safe_query_f (sqlp, "UPDATE `foborganisation` SET `expires`=NULL WHERE `fob`=%#s AND `organisation`=%d", fob,
                           organisation);
      return NULL;
   }

   if (!strcmp (api, "arm") || !strcmp (api, "strong") || !strcmp (api, "disarm"))
   {
      if ((!strcmp (api, "arm") && !apiarm) || (!strcmp (api, "strong") && !apistrong) || (!strcmp (api, "disarm") && !apidisarm))
         return "Not allowed";

      int site = atoi (j_get (j, "site") ? : "");
      if (!site)
         return "No site";
      res = sql_safe_query_store_f (sqlp, "SELECT * FROM `site` WHERE `site`=%d", site);
      int siteorg = 0;
      if (sql_fetch_row (res))
      {
         siteorg = atoi (sql_colz (res, "organisation"));
      }
      sql_free_result (res);
      if (!siteorg || siteorg != organisation)
         return "Invalid site";

      j_t areas = j_find (j, "areas");
      if (!areas)
         return "No areas";

      slot_t id = 0;
      const char *deviceid = NULL;
      res =
         sql_safe_query_store_f (sqlp,
                                 "SELECT `id`,`device` FROM `device` WHERE `site`=%d AND `id` IS NOT NULL AND `via` IS NULL AND `outofservice`='false' LIMIT 1",
                                 site);
      if (sql_fetch_row (res))
      {
         id = strtoll (sql_colz (res, "id") ? : "", NULL, 10);
         deviceid = strdupa (sql_colz (res, "device"));
      }
      sql_free_result (res);
      if (!id)
         return "Device not on line";

      if (!strcmp (api, "arm"))
         slot_send (id, "command", deviceid, "arm", &areas);
      else if (!strcmp (api, "strong"))
         slot_send (id, "command", deviceid, "strong", &areas);
      else if (!strcmp (api, "disarm"))
         slot_send (id, "command", deviceid, "disarm", &areas);

      return NULL;
   }
   return "Unknown API";
}

int
main (int argc, const char *argv[])
{
   void babysit (int s)
   {
      s = s;
      while (waitpid (-1, 0, WNOHANG) > 0);
      signal (SIGCHLD, &babysit);
   }
   signal (SIGCHLD, &babysit);
#ifdef	SQL_DEBUG
   sqldebug = 1;
#endif
   const char *dir = NULL;
   int nodaemon = 0;
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         {"debug", 'v', POPT_ARG_NONE, &sqldebug, 0, "Debug", NULL},
         {"dump", 'V', POPT_ARG_NONE, &dump, 0, "Debug dump", NULL},
         {"no-daemon", 'D', POPT_ARG_NONE, &nodaemon, 0, "No daemon", NULL},
         {"mqtt", 'm', POPT_ARG_NONE, &mqttdump, 0, "Debug mqtt", NULL},
         {"directory", 'd', POPT_ARG_STRING, &dir, 0, "Directory", "path"},
         POPT_AUTOHELP {}
      };
      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      //poptSetOtherOptionHelp (optCon, "");
      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));
      if (poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
      poptFreeContext (optCon);
   }
   if (dir && chdir (dir))
      err (1, "Failed to chdir to %s", dir);
   if (!sqldebug && !mqttdump && !dump && !nodaemon)
      daemon (1, 1);
   curl = curl_easy_init ();
   if (sqldebug)
      curl_easy_setopt (curl, CURLOPT_VERBOSE, 1L);
   // Get started
   signal (SIGPIPE, SIG_IGN);   // Don't crash on pipe errors
   openlog ("SS", LOG_CONS | LOG_PID, LOG_USER);        // Main logging is to syslog
   {                            // File limits - allow lots of connections at once
      struct rlimit l = {
      };
      if (!getrlimit (RLIMIT_NOFILE, &l))
      {
         l.rlim_cur = l.rlim_max;
         if (setrlimit (RLIMIT_NOFILE, &l))
            syslog (LOG_INFO, "Could not increase files");
      }
      if (!getrlimit (RLIMIT_NPROC, &l))
      {
         l.rlim_cur = l.rlim_max;
         if (setrlimit (RLIMIT_NPROC, &l))
            syslog (LOG_INFO, "Could not increase threads");
      }
   }
   // Load (or make) keys
   const char *msgkey = "",
      *msgcert = "";
   if (!access (CONFIG_KEYS_FILE, R_OK))
   {
      j_t j = j_create ();
      if (!j_read_file (j, CONFIG_KEYS_FILE))
      {
         cakey = strdup (j_get (j, "ca.key") ? : "");
         mqttkey = strdup (j_get (j, "mqtt.key") ? : "");
      }
      j_delete (&j);
   }
   if (!access (CONFIG_MSG_KEY_FILE, R_OK))
   {
      j_t j = j_create ();
      if (!j_read_file (j, CONFIG_MSG_KEY_FILE))
      {
         msgkey = strdup (j_get (j, "msg.key") ? : "");
         cacert = strdup (j_get (j, "ca.cert") ? : "");
         msgcert = strdup (j_get (j, "msg.cert") ? : "");
         mqttcert = strdup (j_get (j, "mqtt.cert") ? : "");
      }
      j_delete (&j);
   }
   if (!*cakey || !*mqttkey || !*msgkey || !*cacert || !*mqttcert || !*msgcert)
   {
      if (!*cakey)
      {
         cakey = makekey ();
         cacert = "";           // Certs no longer valid if key changes
         mqttcert = "";
      }
      if (!*mqttkey)
         mqttkey = makekey ();
      if (!*msgkey)
         msgkey = makekey ();
      if (!*cacert)
         cacert = makecert (cakey, NULL, NULL, "SolarSystem");
      if (!*mqttcert)
         mqttcert = makecert (mqttkey, cakey, cacert, CONFIG_MQTT_HOSTNAME);
      if (!*msgcert)
         msgcert = makecert (msgkey, cakey, cacert, "-LOCAL--MSG-");
      unlink (CONFIG_KEYS_FILE);
      j_t j = j_create ();
      j_object (j);
      j_string (j_path (j, "ca.key"), cakey);
      j_string (j_path (j, "mqtt.key"), mqttkey);
      int f = open (CONFIG_KEYS_FILE, O_CREAT | O_WRONLY, 0400);
      if (f < 0)
         err (1, "Cannot make %s", CONFIG_KEYS_FILE);
      j_err (j_write_fd (j, f));
      close (f);
      j_delete (&j);
      unlink (CONFIG_MSG_KEY_FILE);
      j = j_create ();
      j_object (j);
      j_string (j_path (j, "msg.key"), msgkey);
      j_string (j_path (j, "ca.cert"), cacert);
      j_string (j_path (j, "mqtt.cert"), mqttcert);
      j_string (j_path (j, "msg.cert"), msgcert);
      f = open (CONFIG_MSG_KEY_FILE, O_CREAT | O_WRONLY, 0440);
      if (f < 0)
         err (1, "Cannot make %s", CONFIG_KEYS_FILE);
      j_err (j_write_fd (j, f));
      close (f);
      j_delete (&j);
      warnx ("New " CONFIG_MSG_KEY_FILE " may need group change for www-data");
   }
   // Connect
   sstypes ("types");
   SQL sql;
   sql_cnf_connect (&sql, CONFIG_SQL_SERVER_FILE);
   sskeydatabase (&sql);
   ssdatabase (&sql);           // Selects the database, so do after key database
   syslog (LOG_INFO, "Starting");
   sql_safe_query (&sql, "DELETE FROM `pending`");
   sql_safe_query (&sql,
                   "UPDATE `device` SET `id`=NULL,`via`=NULL,`offlinereason`='System restart',`online`=NULL,`lastonline`=NOW(),`progress`=NULL WHERE `id` IS NOT NULL");
   mqtt_start ();
   // Main loop getting messages (from MQTT or websocket)
   while (1)
   {
      time_t now = time (0);
      {                         // Daily jobs - every day (UTC)
         static int today = 0;
         if (now / 86400 != today)
         {
            today = now / 86400;
            daily (&sql);
         }
      }
      {                         // Every minute
         static int tick = 0;
         if (now / 60 != tick || poke)
         {
            tick = now / 60;
            SQL_RES *res = sql_safe_query_store (&sql,
                                                 "SELECT `site` FROM `device` WHERE `site` IS NOT NULL AND `online` IS NULL AND `offlinereport` IS NULL AND `outofservice`='false' AND `lastonline`<DATE_SUB(NOW(),INTERVAL 11 MINUTE) GROUP BY `site`");
            while (sql_fetch_row (res))
               dooffline (&sql, atoi (sql_colz (res, "site")));
            sql_free_result (res);
            res =
               sql_safe_query_store (&sql,
                                     "SELECT `site` FROM `device` WHERE `site` IS NOT NULL AND `id` IS NOT NULL AND `upgrade` IS NOT NULL AND `upgrade`<NOW() GROUP BY `site`");
            while (sql_fetch_row (res))
               doupgrade (&sql, atoi (sql_colz (res, "site")));
            sql_free_result (res);
         }
      }
      if (poke)
      {
         poke = 0;
         dopoke (&sql);
      }
      j_t j = incoming ();
      if (!j)
         continue;
      if (dump)
         j_err (j_write_pretty (j, stderr));
      SQL_RES *device = NULL;
      j_t meta = j_find (j, "_meta");
      const char *secureid = j_get (meta, "device");
      const char *deviceid = j_get (meta, "target");
      slot_t id = strtoull (j_get (meta, "id") ? : "", NULL, 10);
      SQL_RES *checkdevice (void)
      {                         // Check device is secure and get device SQL
         if (device || !secureid || *secureid == '-' || *secureid == '*')
            return device;
         if (deviceid && *deviceid == '*')
            return NULL;
         device =
            sql_safe_query_store_f (&sql, "SELECT * FROM `device` WHERE `device`=%#s AND `id`=%lld AND `site` IS NOT NULL",
                                    deviceid ? : secureid, id);
         if (!sql_fetch_row (device))
         {
            sql_free_result (device);
            device = NULL;
            secureid = NULL;
            deviceid = NULL;
         }
         return device;
      }
      if (meta)
         meta = j_detach (meta);
      char forked = 0;
      const char *local (slot_t local)
      {                         // Commands sent to us from local system
         const char *v;
         if (j_find (meta, "poke"))
         {
            poke = 1;
            return NULL;
         }
         if ((v = j_get (meta, "rollover")))
         {
            if (!*makeaes (&sql, alloca (AES_STRING_LEN), v, NULL))
               return "Failed";
            return NULL;
         }
         slot_t id = 0;         // Collection id
         {                      // Identify the device we want to talk to...
            SQL_RES *res = NULL;
            if ((v = j_get (meta, "site")) && atoi (v))
               res =
                  sql_safe_query_store_f (&sql,
                                          "SELECT `id`,`device` FROM `device` WHERE `site`=%d AND `id` IS NOT NULL AND `via` IS NULL AND `outofservice`='false' LIMIT 1",
                                          atoi (v));
            else if ((v = j_get (meta, "device")))
               res =
                  sql_safe_query_store_f (&sql, "SELECT `id`,`device` FROM `device` WHERE `device`=%#s AND `site` IS NOT NULL", v);
            else if ((v = j_get (meta, "pending")))
               res = sql_safe_query_store_f (&sql, "SELECT `id`,`pending` AS `device` FROM `pending` WHERE `pending`=%#s", v);
            if (res)
            {                   // Check device on line, find id
               deviceid = v;
               if (sql_fetch_row (res))
               {
                  id = strtoll (sql_colz (res, "id") ? : "", NULL, 10);
                  deviceid = strdupa (sql_colz (res, "device"));
               }
               sql_free_result (res);
               if (!id)
                  return "Device not on line";
            }
         }
         // Note, API is first as called with user supplied data, so meta could have something else as well.
         if (j_find (meta, "api"))
            return doapi (&sql, local, meta, j);
         if (j_find (meta, "provision") && deviceid)
         {                      // JSON is rest of settings to send
            char *key = makekey ();
            char *cert = makecert (key, cakey, cacert, v);
            j_t j = j_create ();
            if (*CONFIG_OTA_HOSTNAME)
               j_store_string (j, "otahost", CONFIG_OTA_HOSTNAME);
            j_store_string (j, "clientcert", cert);
            j_store_string (j, "clientkey", key);
            free (cert);
            free (key);
            const char *fail = slot_send (id, "setting", deviceid, NULL, &j);
            if (fail)
               return fail;
            const char *aid = j_get (meta, "aid");
            if (aid && !fail)
            {
               SQL_RES *s =
                  sql_safe_query_store_f (&sql, "SELECT * FROM `aid` LEFT JOIN `site` USING (`site`) WHERE `aid`=%#s", aid);
               if (sql_fetch_row (s))
               {
                  j = j_create ();
                  addsitedata (&sql, j, s, deviceid, NULL, 0);
                  fail = slot_send (id, "setting", deviceid, NULL, &j);
               }
               sql_free_result (s);
            }
            if (!fail)
               sql_safe_query_f (&sql, "DELETE FROM `pending` WHERE `pending`=%#s", deviceid);
            return fail;
         }
         if ((v = j_get (meta, "deport")))
         {
            j_store_object (j, "client");       // Clear client
            j_t mqtt = j_store_object (j, "mqtt");
            j_store_string (mqtt, "host", v);
            j_store_string (mqtt, "cert", "");  // Cleared as typically in factory default
            const char *fail = slot_send (id, "setting", deviceid, NULL, &j);
            // Set online later to remove from pending lists in UI
            if (!fail)
               sql_safe_query_f (&sql, "DELETE FROM `pending` WHERE `pending`=%#s", deviceid);
            return fail;
         }
         if ((v = j_get (meta, "print")))
         {                      // Card printing - record allocated key
            const char *fob = j_get (j, "_ID");
            const char *ver = j_get (j, "_KEYVER");
            const char *key = j_get (j, "_KEYAES");
            if (!fob || !*fob || !ver || !*ver || !key || !*key)
               return "Bad request";
            SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `%#S`.`AES` WHERE `fob`=%#s", CONFIG_SQL_KEY_DATABASE, fob);
            if (sql_fetch_row (res))
            {
               sql_free_result (res);
               return "Already exists";
            }
            sql_free_result (res);
            sql_safe_query_f (&sql, "INSERT INTO `%#S`.`AES` SET `fob`=%#s,`ver`=%#s,`key`=%#s,`aid`=''", CONFIG_SQL_KEY_DATABASE,
                              fob, ver, key);
            sql_safe_query_f (&sql, "INSERT INTO `fob` SET `fob`=%#s,`provisioned`=NOW()", fob);
            return NULL;
         }
         if ((v = j_get (meta, "prefix")))
         {                      // Send to device
            if (!id)
               return "No id";
            const char *suffix = j_get (meta, "suffix");
            const char *fail = NULL;
            j_t data = j_find (j, "_data");
            if (data)
               slot_send (id, v, deviceid, suffix, &data);
            else if (j_len (j))
               slot_send (id, v, deviceid, suffix, &j);
            else
               slot_send (id, v, deviceid, suffix, NULL);
            return fail;
         }
         if (j_find (meta, "fobidentify"))
         {                      // Identify a fob
            j_t init = j_create ();
            j_store_true (init, "identify");
            j_store_int (init, "device", id);
            j_store_string (init, "deviceid", deviceid);
            forked = 1;
            return forkcommand (&init, id, local);
         }
         if (j_find (meta, "fobprovision") || j_find (meta, "fobadopt") || j_find (meta, "fobformat"))
         {                      // Fob specific
            j_t init = j_create ();
            char key[AES_STRING_LEN] = "";
            const char *aid = j_get (meta, "aid");
            const char *fobid = j_get (meta, "fob");
            const char *fobname = j_get (meta, "fobname");
            int access = atoi (j_get (meta, "access") ? : "");
            int organisation = atoi (j_get (meta, "organisation") ? : "");
            if (j_find (meta, "fobprovision"))
               j_store_true (init, "provision");
            if (j_find (meta, "fobadopt"))
            {
               if (!fobid)
                  return "Fob needed";
               if (!aid)
                  return "SID needed";
               j_store_true (init, "adopt");
            }
            if (j_find (meta, "fobformat"))
            {
               if (!fobid)
                  return "Fob needed";
               j_store_true (init, "hardformat");
            }
            if (organisation)
               j_store_int (init, "organisation", organisation);
            if (access)
               j_store_int (init, "access", access);
            if (fobid)
            {
               j_store_string (init, "fob", fobid);
               j_store_string (init, "masterkey", getaes (&sql, key, NULL, fobid));
            }
            if (fobname)
               j_store_string (init, "fobname", fobname);
            if (aid)
            {
               j_store_string (init, "aid", aid);
               j_store_string (init, "aid0key", getaes (&sql, key, aid, fobid));
               j_store_string (init, "aid1key", getaes (&sql, key, aid, NULL));
            }
            j_store_int (init, "device", id);
            j_store_string (init, "deviceid", deviceid);
            forked = 1;
            return forkcommand (&init, id, local);
         }
         return "Unknown local request";
      }
      const char *loopback (void)
      {                         // From linked
         const char *v;
         const char *fob = j_get (j, "fob");
         const char *fobname = j_get (j, "fobname");
         const char *aid = j_get (j, "aid");
         if (fob)
            sql_safe_query_f (&sql, "INSERT IGNORE INTO `fob` SET `fob`=%#s,`provisioned`=NOW()", fob);
         if ((v = j_get (j, "mem")) && fob)
            sql_safe_query_f (&sql, "UPDATE `fob` SET `mem`=%#s WHERE `fob`=%#s AND (`mem` IS NULL OR `mem`<>%#s)", v, fob, v);
         if ((v = j_get (j, "capacity")) && fob)
            sql_safe_query_f (&sql, "UPDATE `fob` SET `capacity`=%#s WHERE `fob`=%#s AND (`capacity` IS NULL OR `capacity`<>%#s)",
                              v, fob, v);
         if (j_find (meta, "provisioned") && fob && (v = j_get (j, "masterkey")))
         {
            sql_safe_query_f (&sql, "REPLACE INTO `%#S`.`AES` SET `fob`=%#s,`aid`='',`type`=%#.2s,`ver`=%#.2s,`key`=%#s",
                              CONFIG_SQL_KEY_DATABASE, fob, v, v + 2, v + 4);
            if (aid && (v = j_get (j, "aid0key")))
               sql_safe_query_f (&sql, "REPLACE INTO `%#S`.`AES` SET `fob`=%#s,`aid`=%#s,`type`=%#.2s,`ver`=%#.2s,`key`=%#s",
                                 CONFIG_SQL_KEY_DATABASE, fob, aid, v, v + 2, v + 4);
         }
         if (j_find (meta, "adopted") && fob && aid)
         {
            int access = atoi (j_get (j, "access") ? : "");
            if ((v = j_get (j, "aid0key")))
               sql_safe_query_f (&sql, "REPLACE INTO `%#S`.`AES` SET `fob`=%#s,`aid`=%#s,`type`=%#.2s,`ver`=%#.2s,`key`=%#s",
                                 CONFIG_SQL_KEY_DATABASE, fob, aid, v, v + 2, v + 4);
            if ((v = j_get (j, "organisation")))
               sql_safe_query_f (&sql,
                                 "INSERT IGNORE INTO `foborganisation` SET `fob`=%#s,`organisation`=%s,`fobname`=%#s ON DUPLICATE KEY UPDATE `fobname`=%#s",
                                 fob, v, fobname, fobname);
            sql_safe_query_f (&sql,
                              "INSERT IGNORE INTO `fobaid` SET `fob`=%#s,`aid`=%#s,`ver`=%#s,`adopted`=NOW(),`access`=%d ON DUPLICATE KEY UPDATE `access`=%d",
                              fob, aid, j_get (j, "ver"), access, access);
         }
         if (j_find (meta, "formatted") && fob)
            sql_safe_query_f (&sql, "DELETE FROM `fobaid` WHERE `fob`=%#s", fob);
         return NULL;
      }
      const char *process (void)
      {
         if (!meta)
            return "No meta data";
         if (j_find (meta, "loopback"))
            return loopback ();
         j_t t;
         if ((t = j_find (meta, "subscribe")))
         {
            for (j_t s = j_first (t); s; s = j_next (s))
            {
               const char *p = j_val (s);
               if (strncmp (p, "command/", 8))
                  continue;
               while (*p && *p != '/')
                  p++;
               if (*p)
               {
                  p++;
                  while (*p && *p != '/')
                     p++;
                  if (*p)
                  {
                     p++;
                     const char *dev = p;
                     while (*p && *p != '/')
                        p++;
                     if (p - dev == 12)
                     {
                        if (secureid)
                        {
                           int site = 0;
                           SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `device` WHERE `device`=%#s", secureid);
                           if (!sql_fetch_row (res))
                              sql_safe_query_f (&sql, "INSERT INTO `device` SET `device`=%#s,`id`=%lld,`online`=NOW()", secureid,
                                                id);
                           else
                              site = atoi (sql_colz (res, "site"));
                           sql_free_result (res);
                           if (!site)
                              sql_safe_query_f (&sql,
                                                "INSERT INTO `pending` SET `pending`=%#s,`id`=%lld,`online`=NOW() ON DUPLICATE KEY UPDATE `id`=%lld",
                                                secureid, id, id);
                           if (strncmp (secureid, dev, p - dev))
                           {    // Must be same site
                              sql_safe_query_f (&sql,
                                                "UPDATE `device` SET `online`=NOW(),`offlinereason`=NULL,`lastonline`=NOW(),`id`=%lld,`via`=%#s WHERE `device`=%#.*s AND `site`=%d",
                                                id, secureid, p - dev, dev, site);
                           } else
                              sql_safe_query_f (&sql,
                                                "UPDATE `device` SET `online`=NOW(),`offlinereason`=NULL,`lastonline`=NOW(),`id`=%lld,`via`=NULL WHERE `device`=%#.*s",
                                                id, p - dev, dev);
                           device =
                              sql_safe_query_store_f (&sql, "SELECT * FROM `device` WHERE `device`=%#.*s AND `site` IS NOT NULL",
                                                      p - dev, dev);
                           if (sql_fetch_row (device))
                           {
                              if (!strncmp (secureid, dev, p - dev))
                                 upgrade (device, id);  // Priority upgrade for connecting device as fastest
                              settings (&sql, device, id);
                           }
                           poke = 1;
                        } else
                           sql_safe_query_f (&sql,
                                             "INSERT INTO `pending` SET `pending`=%#.*s,`online`=NOW(),`id`=%lld ON DUPLICATE KEY UPDATE `id`=%lld",
                                             p - dev, dev, id, id);
                     }
                  }
               }
            }
            return NULL;
         }
         if ((t = j_find (meta, "unsubscribe")))
         {
            for (j_t s = j_first (t); s; s = j_next (s))
            {
               const char *p = j_val (s);
               if (strncmp (p, "command/", 8))
                  continue;
               while (*p && *p != '/')
                  p++;
               if (*p)
               {
                  p++;
                  while (*p && *p != '/')
                     p++;
                  if (*p)
                  {
                     p++;
                     const char *dev = p;
                     while (*p && *p != '/')
                        p++;
                     if (p - dev == 12)
                     {
                        if (checkdevice ())
                           sql_safe_query_f (&sql,
                                             "UPDATE `device` SET `via`=NULL,`online`=NULL,`lastonline`=NOW(),`progress`=NULL,`id`=NULL,`offlinereport`=NULL,`offlinereason`='Timeout' WHERE `device`=%#.*s AND `id`=%lld",
                                             p - dev, dev, id);
                        else
                           sql_safe_query_f (&sql, "DELETE FROM `pending` WHERE `pending`=%#.*s AND `id`=%lld", p - dev, dev, id);
                     }
                  }
               }
            }
            return NULL;
         }
         if ((t = j_find (meta, "local")))
         {
            id = strtoll (j_val (t) ? : "", NULL, 10);
            const char *reply = local (id);
            if (id)
            {                   // Send response
               if (reply)
               {                // Send reply
                  j_t j = j_create ();
                  j_string (j, reply);
                  slot_send (id, NULL, secureid, NULL, &j);     // reply
               }
               if (!forked)
               {                // Tell command we are closed
                  j_t j = j_create ();
                  slot_send (id, NULL, secureid, NULL, &j);
               }
            }
            return reply;
         }
         if (!id)
            return "No id";
         const char *address = j_get (meta, "address");
         const char *prefix = j_get (meta, "prefix");
         const char *suffix = j_get (meta, "suffix");
         j_t up = j_find (j, "up");
         if (up && prefix && !strcmp (prefix, "state") && !suffix && deviceid)
         {                      // Up message
            if (j_isbool (up) && !j_istrue (up))
            {                   // Down
               sql_safe_query_f (&sql,
                                 "UPDATE `device` SET `via`=NULL,`offlinereport`=NULL,`offlinereason`=%#s,`online`=NULL,`lastonline`=NOW(),`progress`=NULL,`id`=NULL WHERE `device`=%#s AND `id`=%lld",
                                 j_get (j, "reason"), deviceid, id);
               return NULL;
            }
            sql_s_t s = { 0 };
            if (!device
                && (device =
                    sql_safe_query_store_f (&sql, "SELECT * FROM `device` WHERE `device`=%#s AND `site` IS NOT NULL", deviceid))
                && !sql_fetch_row (device))
            {
               sql_free_result (device);
               device = NULL;
            }
            if (secureid && device)
            {
               sql_sprintf (&s, "UPDATE `device` SET ");        // known, update
               if (!sql_col (device, "online"))
                  sql_sprintf (&s, "`offlinereason`=NULL,`poke`=NULL,");
               time_t boot = time (0) - atoi (j_val (up));
               if (sql_time (sql_colz (device, "boot")) < boot)
                  sql_sprintf (&s, "`boot`=%#T,", boot);
            } else              // pending - update pending
            {
               sql_sprintf (&s, "REPLACE INTO `pending` SET ");
               deviceid = j_get (j, "id");
               sql_sprintf (&s, "`id`=%lld,", id);
               sql_sprintf (&s, "`pending`=%#s,", deviceid);
               // makes no sense if (secureid && deviceid && strcmp(secureid, deviceid)) sql_sprintf(&s, "`authenticated`=%#s,", "true");
            }
            if (!secureid || !device || !sql_col (device, "online"))
            {
               if (secureid && strcmp (secureid, deviceid))
                  sql_sprintf (&s, "`via`=%#s,", secureid);
               sql_sprintf (&s, "`online`=NOW(),");     // Can happen if reconnect without unsub/sub (i.e. fast enough)
               if (device && secureid)
                  settings (&sql, device, id);
               poke = 1;
            }
            if (!secureid || !device || (address && strcmp (sql_colz (device, "address"), address)))
               sql_sprintf (&s, "`address`=%#s,", address);
            const char *build_suffix = j_get (j, "build-suffix");
            if (!secureid || !device || (build_suffix && strcmp (sql_colz (device, "build_suffix"), build_suffix)))
               sql_sprintf (&s, "`build_suffix`=%#s,", build_suffix);
            const char *version = j_get (j, "version");
            if (!secureid || !device || (version && strcmp (sql_colz (device, "version"), version)))
            {
               sql_sprintf (&s, "`version`=%#s,", version);
               if (device && secureid)
                  sql_safe_query_f (&sql,
                                    "INSERT INTO `event` SET `logged`=NOW(),`device`=%#s,`suffix`='upgrade',`data`='{\"version\":\"%#S%#S\"}'",
                                    deviceid, version, build_suffix);
            }
            const char *build = j_get (j, "build");
            if (!secureid || !device || (build && strcmp (sql_colz (device, "build"), build)))
               sql_sprintf (&s, "`build`=%#s,", build);
            const char *secureboot = (j_test (j, "secureboot", 0) ? "true" : "false");
            if (!secureid || !device || (secureboot && strcmp (sql_colz (device, "secureboot"), secureboot)))
               sql_sprintf (&s, "`secureboot`=%#s,", secureboot);
            const char *encryptednvs = (j_test (j, "encryptednvs", 0) ? "true" : "false");
            if (!secureid || !device || (encryptednvs && strcmp (sql_colz (device, "encryptednvs"), encryptednvs)))
               sql_sprintf (&s, "`encryptednvs`=%#s,", secureboot);
            int flash = atoi (j_get (j, "flash") ? : "");
            if (flash && (!secureid || !device || (flash != atoi (sql_colz (device, "flash")))))
               sql_sprintf (&s, "`flash`=%d,", flash);
            int chan = atoi (j_get (j, "chan") ? : "");
            if (chan && (!secureid || !device || (chan != atoi (sql_colz (device, "chan")))))
               sql_sprintf (&s, "`chan`=%d,", chan);
            const char *ssid = j_get (j, "ssid");
            if (!secureid || !device || (ssid && strcmp (sql_colz (device, "ssid"), ssid)))
               sql_sprintf (&s, "`ssid`=%#s,", ssid);
            const char *bssid = j_get (j, "bssid");
            if (!secureid || !device || (bssid && strcmp (sql_colz (device, "bssid"), bssid)))
               sql_sprintf (&s, "`bssid`=%#s,", bssid);
            if (sql_back_s (&s) == ',' && deviceid)
            {
               if (device && secureid)
                  sql_sprintf (&s, ",`id`=%lld WHERE `device`=%#s AND (`id` IS NULL OR `id`=%lld)", id, deviceid, id);
               sql_safe_query_s (&sql, &s);
            } else
               sql_free_s (&s);
            return NULL;
         }
         if (prefix && !strcmp (prefix, "state") && suffix && !strcmp (suffix, "system") && checkdevice ()
             && *sql_colz (device, "outofservice") == 'f')
         {
            SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `site` WHERE `site`=%#s", sql_col (device, "site"));
            if (sql_fetch_row (res))
            {
               sql_s_t s = { 0 };
               sql_sprintf (&s, "UPDATE `site` SET ");
               char temp[sizeof (area_t) * 16];
               char *commalist (const char *a)
               {
                  if (!a)
                     return "";
                  char *o = temp;
                  while (*a && o < temp + sizeof (temp) - 1 && strchr (AREAS, *a))
                  {
                     if (o > temp)
                        *o++ = ',';
                     *o++ = *a++;
                  }
                  *o = 0;
                  return temp;
               }
               int n;
               const char *v;
               if ((n = atoi (j_get (j, "nodes") ? : "")) != atoi (sql_col (res, "nodes") ? : "-1"))
                  sql_sprintf (&s, "`nodes`=%d,", n);
               if ((n = atoi (j_get (j, "missing") ? : "")) != atoi (sql_col (res, "missing") ? : "-1"))
                  sql_sprintf (&s, "`missing`=%d,", n);
#define s(t,n,c) if(strcmp(#n,"engineer")){if(strcmp(sql_colz(res,#n),v=commalist(j_get(j,#n))))sql_sprintf(&s,"`%#S`=%#s,",#n,v);}
#include "ESP32/main/states.m"
               if (strcmp (sql_colz (res, "status"), v = (j_get (j, "status") ? : "")))
                  sql_sprintf (&s, "`status`=%#s,", v);
               if (sql_back_s (&s) == ',' && deviceid)
               {
                  sql_sprintf (&s, " WHERE `site`=%#s", sql_col (device, "site"));
                  sql_safe_query_s (&sql, &s);
               } else
                  sql_free_s (&s);
            }
            sql_free_result (res);
            return NULL;
         }
         if (prefix && !strcmp (prefix, "sms") && checkdevice ())
         {
            SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `site` WHERE `site`=%#s", sql_col (device, "site"));
            if (sql_fetch_row (res))
               send_message (res, j_get (j, "message"), j_get (j, "number") ? : sql_colz (res, "smsnumber"));
            sql_free_result (res);
            return NULL;
         }
         if (!prefix)
         {                      // Down (all other messages have a topic)
            if (secureid)
               sql_safe_query_f (&sql,
                                 "UPDATE `device` SET `via`=NULL,`offlinereport`=NULL,`offlinereason`='Closed',`online`=NULL,`lastonline`=NOW(),`progress`=NULL,`id`=NULL,`lastonline`=NOW() WHERE `id`=%lld",
                                 id);
            else                // pending
               sql_safe_query_f (&sql, "DELETE FROM `pending` WHERE `id`=%lld", id);
            return NULL;
         }
         if (prefix && !strcmp (prefix, "event"))
         {
            const char *fob = NULL;
            if (!strcmp (suffix ? : "", "fob"))
            {
               fob = j_get (j, "id");
               SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `fob` WHERE `fob`=%#s", fob);
               if (!sql_fetch_row (res))
                  fob = NULL;
               sql_free_result (res);
            }
            char *data = j_write_str (j);
            sql_safe_query_f (&sql, "INSERT INTO `event` SET `logged`=NOW(),`device`=%#s,`fob`=%#s,`suffix`=%#s,`data`=%#s",
                              deviceid, fob, suffix, data);
            free (data);
            j_store_string (j, "event", suffix);
            if (suffix && *suffix && checkdevice ())
            {                   // Event hooks
               SQL_RES *res = sql_safe_query_store_f (&sql, "SELECT * FROM `site` WHERE `site`=%#s", sql_col (device, "site"));
               if (sql_fetch_row (res))
               {
                  char *tag;
                  if (asprintf (&tag, "hook%s", suffix) < 0)
                     errx (1, "malloc");
                  if ((!j_find (j, "deny") && !j_find (j, "fail")) || j_find (j, "gone"))
                  {             // fail/deny are sent twice - one read and on gone, and we only log these on gone event as otherwise we log fixed issues like expiry
                     const char *hook = sql_col (res, tag);
                     if (hook && *hook)
                        notify (&sql, res, hook, j);
                     if (!strcmp (suffix, "fob") && j_find (j, "deny")
                         && strcmp (j_get (j, "fail") ? : "", "PN532_ERR_STATUS_TIMEOUT") && (hook = sql_col (res, "hookfobdeny"))
                         && *hook)
                        notify (&sql, res, hook, j);    // Failed other than simple timeout (i.e. too quick)
                  }
               }
               sql_free_result (res);
            }
            if (!strcmp (suffix, "fob") && checkdevice ())
            {                   // Fob usage - loads of options
               int organisation = atoi (sql_colz (device, "organisation"));
               const char *aid = sql_colz (device, "aid");
               const char *fobid = j_get (j, "id");
               char held = j_test (j, "held", 0);
               char gone = j_test (j, "gone", 0);
               char secure = j_test (j, "secure", 0);
               char block = j_test (j, "block", 0);
               char remote = j_test (j, "remote", 0);
               char updated = j_test (j, "updated", 0);
               char blacklist = j_test (j, "blacklist", 0);
               if (!held && !gone && !remote)
               {                // Initial fob use
                  SQL_RES *fa = sql_safe_query_store_f (&sql,
                                                        "SELECT * FROM `fobaid` LEFT JOIN `foborganisation` USING (`fob`) LEFT JOIN `access` USING (`access`) WHERE `fob`=%#s AND `aid`=%#s",
                                                        fobid, aid);
                  if (!sql_fetch_row (fa))
                  {
                     sql_free_result (fa);
                     fa = NULL;
                  }
                  if ((block || (updated && blacklist)) && secure)
                  {             // Confirm blocked
                     sql_safe_query_f (&sql,
                                       "UPDATE `foborganisation` SET `confirmed`=NOW() WHERE `organisation`=%d AND `fob`=%#s AND `confirmed` IS NULL",
                                       organisation, fobid);
                     if (sql_affected_rows (&sql))
                     {
                        sql_safe_query_f (&sql, "UPDATE `device` SET `poke`=NOW() WHERE `organisation`=%d", organisation);
                        poke = 1;
                     }
                  }
                  if (secure)
                  {
                     if (fa)
                     {
                        sql_s_t q = { 0 };
                        const char *ts = j_get (j, "ts");
                        time_t tst = j_time (ts);
                        sql_sprintf (&q, "UPDATE `fobaid` SET `lastused`=%#T", tst);
                        const char *firstinday = sql_col (fa, "firstinday");
                        if (!firstinday || strncmp (firstinday, ts, 10))
                           sql_sprintf (&q, ",`firstinday`=%#T", tst);
                        const char *ver = j_get (j, "ver");
                        if (ver && strcmp (ver, sql_colz (fa, "ver")))
                           sql_sprintf (&q, ",`ver`=%#s", ver);
                        if (!sql_col (fa, "adopted"))
                           sql_sprintf (&q, ",`adopted`=%#T", tst);
                        sql_sprintf (&q, " WHERE `fob`=%#s AND `aid`=%#s", fobid, aid);
                        sql_safe_query_s (&sql, &q);
                     }
                     // Check afile
                     const char *crc = j_get (j, "crc");
                     if (crc)
                     {          // Check afile
                        unsigned was = strtoull (crc, NULL, 16);
                        unsigned char afile[256] = { };
                        unsigned new = makeafile (fa, afile);
                        if (was != new)
                        {       // Send afile
                           if (sqldebug)
                              warnx ("CRC mismatch %08X %08X", was, new);
                           j_t a = j_create ();
                           j_store_string (a, "id", fobid);
                           j_store_string (a, "afile", j_base16a (*afile + 1, afile));
                           slot_send (id, "command", deviceid, "access", &a);
                        }
                     }
                  }
                  if (fa)
                     sql_free_result (fa);
               }
            }
         } else if (prefix && !strcmp (prefix, "error"))
         {
         } else if (prefix && !strcmp (prefix, "info"))
         {
            if (suffix && !strcmp (suffix, "upgrade") && checkdevice ())
            {
               if (j_find (j, "complete"))
               {                // Done
                  sql_safe_query_f (&sql,
                                    "UPDATE `device` SET `upgrade`=NULL,`version`=NULL,`build_suffix`=NULL,`progress`=NULL WHERE `device`=%#s",
                                    deviceid);
                  poke = 1;
               } else if (j_find (j, "size"))   // making progress
                  sql_safe_query_f (&sql, "UPDATE `device` SET `progress`=%d WHERE `device`=%#s",
                                    atoi (j_get (j, "progress") ? : ""), deviceid);
            }
         }
         if (j)
         {
            slot_t l = slot_linked (id);
            if (l)
               slot_send (l, prefix, deviceid, suffix, &j);     // Send to linked session
         }
         return NULL;
      }
      const char *fail = process ();
      if (device)
         sql_free_result (device);
      if (fail)
         warnx ("Failed to process message: %s", fail);
      j_delete (&j);
      j_delete (&meta);
   }

   sql_close (&sql);
   curl_easy_cleanup (curl);
   return 0;
}
