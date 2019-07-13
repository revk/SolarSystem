    /*
       DESFire card issue for access control

       Copyright (C) 2019  RevK and Andrews & Arnold Ltd

       This program is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       This program is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with this program.  If not, see <http://www.gnu.org/licenses/>.
     */

// This can be used in one of two main ways...
// If you have access to the config, specify the config file, this provides the MQTT details, AID, and AES
// If you do not, then specify AID and MQTT details
// You need to say which reader you are using as well
// If a reader is set in config with nfc="M" then it is assumed to be the default "management" reader


#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <popt.h>
#include <err.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <mosquitto.h>
#include <openssl/evp.h>
#include <desfireaes.h>
#include <axl.h>

int debug = 0;

int
main (int argc, const char *argv[])
{
   const char *mqtthost = NULL;
   const char *mqttuser = NULL;
   const char *mqttpass = NULL;
   const char *mqttcert = NULL;
   const char *setname = NULL;
   char *hexaid = NULL;
   const char *hexaes = NULL;
   const char *hexreader = NULL;
   const char *config = NULL;
   const char *username = NULL;
   int doformat = 0;
   int mqttport = 0;
   int comms = 0;
   int logs = 50;
   int random = 0;
   int lock = 0;
   int forceallow = 0;
   {                            // POPT
      poptContext optCon;       // context for parsing command-line options
      const struct poptOption optionsTable[] = {
         {"mqtt-host", 'h', POPT_ARG_STRING, &mqtthost, 0, "MQTT host",
          "hostname"},
         {"mqtt-user", 'U', POPT_ARG_STRING, &mqttuser, 0, "MQTT user",
          "username"},
         {"mqtt-pass", 'P', POPT_ARG_STRING, &mqttpass, 0, "MQTT pass",
          "password"},
         {"mqtt-cert", 'C', POPT_ARG_STRING, &mqttcert, 0, "MQTT CA",
          "filename"},
         {"mqtt-port", 'p', POPT_ARG_INT, &mqttport, 0, "MQTT port", "port"},
         {"user", 'u', POPT_ARG_STRING, &username, 0, "Username", "username"},
         {"name", 'n', POPT_ARG_STRING, &setname, 0, "Users name", "Full name"},
         {"config", 'c', POPT_ARG_STRING, &config, 0, "Config", "filename"},
         {"aid", 'a', POPT_ARG_STRING, &hexaid, 0, "AID", "XXXXXX"},
         {"aes", 'A', POPT_ARG_STRING, &hexaes, 0, "AES",
          "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
         {"reader", 'r', POPT_ARG_STRING, &hexreader, 0, "Reader", "XXXXXX"},
         {"format", 0, POPT_ARG_NONE, &doformat, 0, "Format card", 0},
         {"logs", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &logs, 0,
          "Log records", "N"},
         {"comms", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &comms, 0,
          "File comms", "0/1/3"},
         {"random", 0, POPT_ARG_NONE, &random, 0, "Random UID (when formatting)",
          0},
         {"lock", 0, POPT_ARG_NONE, &lock, 0, "Lock against further formatting (when formatting)", 0},
         {"allow", 0, POPT_ARG_NONE, &forceallow, 0, "Force door allow list setting", 0},
         {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug", 0},
         POPT_AUTOHELP {}
      };

      optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
      poptSetOtherOptionHelp (optCon, "[aid/config] [reader] [username]");

      int c;
      if ((c = poptGetNextOpt (optCon)) < -1)
         errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

      while (1)
      {
         const char *v = poptPeekArg (optCon);
         if (!v)
            break;
         if (!hexaid && df_hex (4, NULL, v) == 3)
            hexaid = (char *) poptGetArg (optCon);
         else if (!hexreader && df_hex (4, NULL, v) == 3)
            hexreader = poptGetArg (optCon);
         else if (!hexaes && df_hex (17, NULL, v) == 16)
            hexaes = poptGetArg (optCon);
         else if (!config)
            config = poptGetArg (optCon);
         else
            break;
      }

      if (poptPeekArg (optCon) && !username)
         username = poptGetArg (optCon);
      if (poptPeekArg (optCon))
      {
         poptPrintUsage (optCon, stderr, 0);
         return -1;
      }
      poptFreeContext (optCon);
   }

   xml_t c = NULL;
   if (config)
   {                            // Settings from config
      c = xml_tree_read_file (config);
      if (!c)
         errx (1, "Cannot read %s", config);
      {                         // Go to same directory
         char *dir = strdupa (config);
         char *f = strrchr (dir, '/');
         if (f)
         {
            *f++ = 0;
            chdir (dir);
         }
      }
      const char *v;
      int found = 0;
      if ((v = xml_get (c, "system@aid")) && (!hexaid || !strcasecmp (v, hexaid)))
      {                         // Main AID
         if (!hexaid)
            hexaid = (char *) v;
         v = xml_get (c, "system@aes");
         if (hexaes && v && strcasecmp (v, hexaes))
            errx (1, "Incorrect AES");
         hexaes = v;
         found = 1;
      }
      xml_t d = NULL;
      while ((d = xml_element_next_by_name (c, d, "device")))
         if ((v = xml_get (d, "@nfc")))
         {
            if (!xml_get (d, "@input1") && !xml_get (d, "@output1") && !hexreader)
               hexreader = xml_get (d, "@id");  // Assume management reader as no input or output
            if (!found && hexaid)
            {
               v = xml_get (d, "@aid");
               if (v && !strcasecmp (v, hexaid))
               {                // Match
                  v = xml_get (d, "@aes");
                  if (hexaes && v && strcasecmp (v, hexaes))
                     errx (1, "Incorrect AES");
                  hexaes = v;
                  if (!hexreader)
                     hexreader = xml_get (d, "@id");
                  found = 1;
               }
            }
         }
      if (!found)
         warnx ("Specified AID not found in config");
      if (!mqtthost)
         mqtthost = xml_get (c, "system@mqtt-host");
      if (!mqttuser)
         mqttuser = xml_get (c, "system@mqtt-user");
      if (!mqttpass)
         mqttpass = xml_get (c, "system@mqtt-pass");
      if (!mqttcert)
         mqttcert = xml_get (c, "system@mqtt-ca");
   }
   if (!hexaid)
      errx (1, "Specify AID or config file");
   unsigned char aid[3];
   if (df_hex (sizeof (aid), aid, hexaid) != sizeof (aid))
      errx (1, "AID is hex XXXXXX (%s)", hexaid);
   if (asprintf (&hexaid, "%02X%02X%02X", aid[0], aid[1], aid[2]) < 0)
      errx (1, "malloc");
   if (!hexaes)
      errx (1, "Specify AES or config file");
   unsigned char aes[16];
   if (df_hex (sizeof (aes), aes, hexaes) != sizeof (aes))
      errx (1, "AES is hex XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX (%s)", hexaes);
   if (!hexreader)
      errx (1, "Specify Reader");
   unsigned char reader[3];
   if (df_hex (sizeof (reader), reader, hexreader) != sizeof (reader))
      errx (1, "Reader is hex XXXXXX (%s)", hexreader);

   xml_t user = NULL;
   if (username && config)
   {
      xml_t u = NULL;
      while ((u = xml_element_next_by_name (c, u, "user")))
         if (!strcasecmp (xml_get (u, "@name") ? : "", username))
            break;
      if (!u)
         errx (1, "Cannot fine user %s", username);
      if (!setname)
         setname = xml_get (u, "@full-name");
      user = u;
   }
   char *fob = NULL;

   // Socket for responses
   int sp[2];
   if (socketpair (AF_LOCAL, SOCK_DGRAM, 0, sp) < 0)
      err (1, "socketpair");

   // MQTT messages
   void mqtt_message (struct mosquitto *mqtt, void *obj, const struct mosquitto_message *msg)
   {
      mqtt = mqtt;
      obj = obj;
      char *topic = strdupa (msg->topic);
      char *m = strrchr (topic, '/');
      if (!m)
         return;
      m++;
      if (!strcasecmp (m, "gone"))
      {
         send (sp[0], NULL, 0, 0);      // Indicate card gone
         return;
      }
      if (!strcasecmp (m, "id") || !strcasecmp (m, "access"))
      {
         asprintf (&fob, "%.*s", msg->payloadlen, (char *) msg->payload);
         printf ("Card found %s\n", fob);
         int n;
         unsigned int id = 0,
            v;
         for (n = 0; n < msg->payloadlen; n += 2)
         {
            sscanf ((char *) msg->payload + n, "%02X", &v);
            id ^= (v << (((n / 2) & 3) * 8));
         }
         id &= ~0x80000001;
         printf ("Fob ID as Max %08u\n", id % 100000000);
         send (sp[0], NULL, 0, 0);      // Indicate card gone
         return;
      }
      if (!strcasecmp (m, "nfc"))
      {
         int n;
         if (debug)
         {
            fprintf (stderr, "Rx        ");
            for (n = 0; n < msg->payloadlen; n++)
               fprintf (stderr, " %02X", ((unsigned char *) msg->payload)[n]);
            fprintf (stderr, "\n");
         }
         send (sp[0], msg->payload, msg->payloadlen, 0);
      }
   }

   // Connect MQTT
   mosquitto_lib_init ();
   struct mosquitto *mqtt = mosquitto_new ("cardissue", true, NULL);
   mosquitto_message_callback_set (mqtt, mqtt_message);
   if (mqttcert)
      mosquitto_tls_set (mqtt, mqttcert, NULL, NULL, NULL, NULL);
   if (mqttuser)
      mosquitto_username_pw_set (mqtt, mqttuser, mqttpass);
   if (mosquitto_connect_async (mqtt, mqtthost ? : "localhost", mqttport ? : mqttcert ? 8883 : 1883, 60))
      errx (1, "MQTT connect failed %s:%d", mqtthost, mqttport);
   char *topic;
   if (asprintf (&topic, "event/SS/%s/+", hexreader) < 0)
      errx (1, "malloc");
   if (mosquitto_subscribe (mqtt, NULL, topic, 0))
      errx (1, "Sub failed");
   free (topic);
   if (asprintf (&topic, "info/SS/%s/+", hexreader) < 0)
      errx (1, "malloc");
   if (mosquitto_subscribe (mqtt, NULL, topic, 0))
      errx (1, "Sub failed");
   free (topic);
   mosquitto_loop_start (mqtt);

   void led (const char *led)
   {
      char *topic;
      if (asprintf (&topic, "command/SS/%s/led", hexreader) < 0)
         errx (1, "malloc");
      mosquitto_publish (mqtt, NULL, topic, strlen (led), led, 1, 0);
      free (topic);
   }
   volatile int done = -1;
   // DESFire setup
   int recvt (int fd, void *data, size_t len, int timeout)
   {
      fd_set fset;
      FD_ZERO (&fset);
      FD_SET (fd, &fset);
      struct timeval t = { timeout, 0 };
      int r = select (fd + 1, &fset, NULL, &fset, &t);
      if (r < 0)
      {
         led ("R-");
         if (debug)
            fprintf (stderr, "Rx fail %d\n", r);
         return r;
      }
      if (!r)
      {
         led ("R-");
         if (debug)
            fprintf (stderr, "Rx timeout\n");
         return -1;
      }
      int l = recv (fd, data, len, 0);
      if (l <= 0 || (*(unsigned char *) data && *(unsigned char *) data != 0xAF))
      {
         if (debug)
         {
            if (!l && done >= 0)
               fprintf (stderr, "Card gone\n");
            else if (l < 0)
               fprintf (stderr, "Rx len %d\n", l);
            else if (l > 0)
               fprintf (stderr, "Card status %02X\n", *(unsigned char *) data);
         }
         led (done ? "-" : "R-");
      }
      return l;
   }
   // DESFire dx function
   int dx (void *obj, unsigned int len, unsigned char *data, unsigned int max)
   {
      obj = obj;
      char *topic;
      if (asprintf (&topic, "command/SS/%s/nfc", hexreader) < 0)
         errx (1, "malloc");
      if (debug)
      {
         unsigned int n;
         fprintf (stderr, "Tx        ");
         for (n = 0; n < len; n++)
            fprintf (stderr, " %02X", data[n]);
         fprintf (stderr, "\n");
      }
      mosquitto_publish (mqtt, NULL, topic, len, data, 1, 0);
      free (topic);
      return recvt (sp[1], data, max, 10);
   }
   const char *e;
   df_t d;
   if ((e = df_init (&d, NULL, &dx)))
      errx (1, "Init failed: %s", e);
   done = 0;

   unsigned char buf[10240];
   led ("R");
   printf ("Waiting for card at reader %s\n", hexreader);
   if (recvt (sp[1], buf, sizeof (buf), 30) != 0)
      errx (1, "Giving up");
   led ("G-");

   if ((e = df_select_application (&d, NULL)))
      errx (1, "Failed to select application: %s", e);

   unsigned char ver[28];
   if ((e = df_get_version (&d, ver)))
      errx (1, "Version: %s", e);
   printf ("Ver:");
   unsigned int n;
   for (n = 0; n < sizeof (ver); n++)
      printf (" %02X", ver[n]);
   printf ("\n");

   // Check if card needs formatting
   if (!doformat)
   {
      unsigned char version;
      if ((e = df_get_key_version (&d, 0, &version)))
         errx (1, "Version: %s", e);
      if (!version)
         doformat = 1;          // Needs formatting
   }
   // Format card
   if (doformat)
   {
      printf ("Formatting card\n");
      if ((e = df_format (&d, NULL)))
         errx (1, "Format: %s", e);
      if ((e = df_authenticate (&d, 0, NULL)))
         errx (1, "Master auth: %s", e);
      if ((e = df_change_key_settings (&d, 0x09)))
         errx (1, "Change key settings: %s", e);
      if ((e = df_set_configuration (&d, (random ? 2 : 0) | (lock ? 1 : 0))))
         errx (1, "Set config: %s", e);
   } else if ((e = df_authenticate (&d, 0, NULL)))
      errx (1, "Master auth: %s", e);

   // Get ID
   unsigned char uid[7];
   if ((e = df_get_uid (&d, uid)))
      errx (1, "Getting UID: %s", e);

   if (fob)
      free (fob);
   asprintf (&fob, "%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
   printf ("UID %s\n", fob);
   {                            // Max reader ID
      int n;
      unsigned int id = 0;
      for (n = 0; n < 7; n++)
         id ^= (uid[n] << (((n / 2) & 3) * 8));
      id |= 0x80000001;
      printf ("Fob ID as Max %08u (secure)\n", id % 100000000);
   }

   if (chdir (hexaid) && (mkdir (hexaid, 0700) || chdir (hexaid)))
      err (1, "Cannot make directory");

   unsigned char cardkey[16] = { 0 };
   {                            // Do we have a key
      int f = open (fob, O_RDONLY);
      if (f >= 0)
      {
         if (read (f, cardkey, 16) != 16)
            warn ("Failed to read card key");
         close (f);
      } else
      {
         f = open ("/dev/urandom", O_RDONLY);
         if (f < 0)
            err (1, "random");
         if (read (f, cardkey, 16) != 16)
            err (1, "random");
         close (f);
         f = open (fob, O_CREAT | O_WRONLY, 0600);
         if (f < 0)
            err (1, "Cannot make UID key file");
         if (write (f, cardkey, 16) != 16)
            err (1, "Cannot write UID key file");
         close (f);
      }
   }

   // Check if AID exists
   if ((e = df_get_application_ids (&d, &n, sizeof (buf), buf)))
      errx (1, "Application list: %s", e);
   while (n && memcmp (buf + n * 3 - 3, aid, 3))
      n--;
   if (!n)
   {                            // Create application
      printf ("Creating application %s\n", hexaid);
      if ((e = df_create_application (&d, aid, DF_SET_DEFAULT, 2)))
         errx (1, "Create application: %s", e);
      if ((e = df_select_application (&d, aid)))
         errx (1, "Select application: %s", e);
      if ((e = df_authenticate (&d, 0, NULL)))
         errx (1, "Authenticate: %s", e);
      if ((e = df_change_key (&d, 0, 0x01, NULL, cardkey)))
         errx (1, "Change key: %s", e);
      if ((e = df_authenticate (&d, 0, cardkey)))
         errx (1, "Authenticate: %s", e);
      if ((e = df_change_key (&d, 1, 0x01, NULL, aes)))
         errx (1, "Change key: %s", e);
      if ((e = df_change_key_settings (&d, 0xEB)))
         errx (1, "Change key settings: %s", e);
   } else
   {
      if ((e = df_select_application (&d, aid)))
         errx (1, "Select application: %s", e);
      if ((e = df_authenticate (&d, 1, aes)))
         errx (1, "Authenticate AID: %s", e);   // Checking this works
      if ((e = df_authenticate (&d, 0, cardkey)))
         errx (1, "Authenticate Card: %s", e);  // So we have access
   }

   // Get information
   unsigned long long fids;
   if ((e = df_get_file_ids (&d, &fids)))
      errx (1, "File IDs: %s", e);
   if (fids & (1 << 0))
   {                            // Name file
      char type;
      unsigned char comms;
      unsigned int size;
      if ((e = df_get_file_settings (&d, 0, &type, &comms, NULL, &size, NULL, NULL, NULL, NULL, NULL)))
         errx (1, "File settings: %s", e);
      if (type != 'D' && type != 'B')
         printf ("Name file is wrong type (%c)\n", type);
      else if (size > sizeof (buf))
         printf ("Name file is silly size (%d)\n", size);
      {                         // Get name
         if ((e = df_read_data (&d, 0, comms, 0, size, buf)))
            errx (1, "Read data: %s", e);
         printf ("Name: %.*s\n", size, buf);
         if (setname && strcmp (setname, (char *) buf))
         {
            printf ("Name wrong, deleting name\n");
            if ((e = df_delete_file (&d, 0)))
               errx (1, "Delete file: %s", e);
            fids &= ~(1 << 0);  // deleted
         } else if (config && !username && *buf)
         {
            xml_t u = NULL;
            while ((u = xml_element_next_by_name (c, u, "user")))
               if (!strcasecmp (xml_get (u, "@full-name") ? : "", (char *) buf))
                  break;
            if (u)
            {                   // Found
               if (!setname)
                  setname = xml_get (u, "@full-name");
               user = u;
            }
         }
      }
   }
   if (!(fids & (1 << 0)) && setname && *setname)
   {                            // Set the name
      printf ("Setting name: %s\n", setname);
      if ((e = df_create_file (&d, 0, 'D', comms, 0x1000, strlen (setname), 0, 0, 0, 0, 0)))
         errx (1, "Create file: %s", e);
      if ((e = df_write_data (&d, 0, 'D', comms, 0, strlen (setname), setname)))
         errx (1, "Write file: %s", e);
   }

   if (fids & (1 << 1))
   {                            // Log file
      char type;
      unsigned char comms;
      unsigned int size,
        recs;
      if ((e = df_get_file_settings (&d, 1, &type, &comms, NULL, &size, NULL, NULL, &recs, NULL, NULL)))
         errx (1, "File settings: %s", e);
      if (type != 'C')
         printf ("Log file wrong type (%c)\n", type);
      else if (size != 10)
         printf ("Log file wrong format (%d)\n", size);
      else if (!recs)
         printf ("Log file empty\n");
      else
      {
         if ((e = df_read_records (&d, 1, comms, 0, recs, 10, buf)))
            errx (1, "Read data: %s", e);
         while (recs--)
         {
            char r[7],
             *v;
            sprintf (r, "%02X%02X%02X", buf[recs * 10 + 0], buf[recs * 10 + 1], buf[recs * 10 + 2]);
            xml_t d = NULL;
            if (c)
               while ((d = xml_element_next_by_name (c, d, "device")))
                  if ((v = xml_get (d, "@id")) && !strcasecmp (v, r))
                     break;
            printf ("Log %s on %02X%02X-%02X-%02X %02X:%02X:%02X %s\n", r, buf[recs * 10 + 3], buf[recs * 10 + 4],
                    buf[recs * 10 + 5], buf[recs * 10 + 6], buf[recs * 10 + 7], buf[recs * 10 + 8], buf[recs * 10 + 9],
                    d ? xml_get (d, "@name") ? : "" : "");
         }
      }
   } else
   {
      printf ("Creating log file\n");
      if ((e = df_create_file (&d, 1, 'C', comms, 0x0100, 10, 0, 0, logs, 0, 0)))
         errx (1, "Create file: %s", e);
   }

   if (fids & (1 << 2))
   {                            // Counter file
      char type;
      unsigned char comms;
      if ((e = df_get_file_settings (&d, 2, &type, &comms, NULL, NULL, NULL, NULL, NULL, NULL, NULL)))
         errx (1, "File settings: %s", e);
      if (type != 'V')
         printf ("Counter file wrong type (%c)\n", type);
      else
      {
         unsigned int value;
         if ((e = df_get_value (&d, 2, comms, &value)))
            errx (1, "Read value: %s", e);
         printf ("Count: %d\n", value);
      }
   } else
   {
      printf ("Creating counter file\n");
      if ((e = df_create_file (&d, 2, 'V', comms, 0x0010, 0, 0, 0x7FFFFFFF, 0, 0, 0)))
         errx (1, "Create file: %s", e);
   }

   if (user)
   {                            // Access file
      char *allow = NULL,
         *deny = NULL,
         *afile = NULL;
      size_t allowlen = 0,
         denylen = 0,
         afilelen = 0;
      FILE *allowf = open_memstream (&allow, &allowlen);
      FILE *denyf = open_memstream (&deny, &denylen);
      FILE *afilef = open_memstream (&afile, &afilelen);
      // Check doors
      const char *open = xml_get (user, "@open") ? : "*";
      xml_t door = NULL;
      while ((door = xml_element_next_by_name (c, door, "door")))
      {
         const char *devname = xml_get (door, "@max");
         if (!devname)
            devname = xml_get (door, "@min");
         if (!devname)
            devname = xml_get (door, "@i_fob");
         if (!devname)
            break;
         xml_t device = NULL;
         while ((device = xml_element_next_by_name (c, device, "device")))
            if (!strcasecmp (xml_get (device, "@id") ? : "", devname) || !strcasecmp (xml_get (device, "@name") ? : "", devname))
               break;
         if (!device)
            break;
         const char *devid = xml_get (device, "@id");
         if (!devid)
            break;
         unsigned char id[3];
         if (df_hex (3, id, devid) != 3)
            break;
         const char *lock = xml_get (door, "@lock");
         if (lock && *open != '*')
         {                      // Check if allowed
            const char *c;
            for (c = open; *c && !strchr (lock, *c); c++);
            if (!*c)
               lock = NULL;     // Not allowed
         }
         if (*open == '*' || lock)
         {
            if (debug)
               fprintf (stderr, "Allow %s\n", devid);
            fwrite (id, 3, 1, allowf);
         } else
         {
            if (debug)
               fprintf (stderr, "Deny  %s\n", devid);
            fwrite (id, 3, 1, denyf);
         }
      }
      fclose (allowf);
      fclose (denyf);
      if (!allowlen)
         warnx ("User is not allowed to use any doors");
      if (allowlen < denylen)
         forceallow = 1;        // Allow list is shorter
      if (!allowlen || denylen)
      {
         char *devs = allow;
         int devslen = allowlen;
         if (!forceallow)
         {                      // Set allowed
            devs = deny;
            devslen = denylen;
         }
         while (devslen)
         {
            int l = devslen;
            if (l > 14)
               l = 14;
            char tag = (forceallow ? 0xA0 : 0xB0) + l;
            fwrite (&tag, 1, 1, afilef);
            fwrite (devs, l, 1, afilef);
            devslen -= l;
            devs += l;
         }
      }
      // Times
      const char *t = xml_get (user, "@time-from");
      if (t && *t)
      {
         unsigned char times[14];
         int l = df_hex (sizeof (times), times, t);
         if (l == 2 || l == 4 || l == 14)
         {
            char tag = 0xF0 + l;
            fwrite (&tag, 1, 1, afilef);
            fwrite (times, l, 1, afilef);
         }
      }
      t = xml_get (user, "@time-to");
      if (t && *t)
      {
         unsigned char times[14];
         int l = df_hex (sizeof (times), times, t);
         if (l == 2 || l == 4 || l == 14)
         {
            char tag = 0x20 + l;
            fwrite (&tag, 1, 1, afilef);
            fwrite (times, l, 1, afilef);
         }
      }
      // TODO expiry

      fclose (afilef);
      {
         size_t p;
         printf ("Access");
         for (p = 0; p < afilelen; p++)
            printf (" %02X", afile[p]);
         printf ("\n");
      }

      if (fids & (1 << 3))
      {                         // Access file
         char type;
         unsigned char comms;
         unsigned int size = 0;
         if ((e = df_get_file_settings (&d, 3, &type, &comms, NULL, &size, NULL, NULL, NULL, NULL, NULL)))
            errx (1, "File settings: %s", e);
         if (type != 'D')
            printf ("Access file wrong type (%c)\n", type);
         else
         {                      // Check content
            if (size != afilelen)
            {
               printf ("Access file wrong size (%d!=%d)\n", size, afilelen);
               if ((e = df_delete_file (&d, 3)))
                  errx (1, "Delete file: %s", e);
               fids &= ~(1 << 3);
            } else
            {
               if ((e = df_read_data (&d, 3, comms, 0, afilelen, buf)))
                  errx (1, "File read: %s", e);
               if (memcmp (buf, afile, afilelen))
               {
                  printf ("Access file wrong content.\n");
                  if ((e = df_delete_file (&d, 3)))
                     errx (1, "Delete file: %s", e);
                  fids &= ~(1 << 3);
               }
            }
         }
      }
      if (!(fids & (1 << 3)))
      {                         // Create access file
         printf ("Creating access file\n");
         if ((e = df_create_file (&d, 3, 'D', comms, 0x1000, afilelen, 0, 0, 0, 0, 0)))
            errx (1, "Create file: %s", e);
         if ((e = df_write_data (&d, 3, 'D', comms, 0, afilelen, afile)))
            errx (1, "Write file: %s", e);
      }

      if (allow)
         free (allow);
      if (deny)
         free (deny);
      if (afile)
         free (afile);
   }

   unsigned int mem;
   if ((e = df_free_memory (&d, &mem)))
      errx (1, "Read mem: %s", e);
   printf ("Free memory: %u\n", mem);

   if (config)
   {
      int l = strlen (fob);
      xml_t u = NULL;
      while ((u = xml_element_next_by_name (c, u, "user")))
      {
         const char *f = xml_get (u, "@fob");
         if (f)
         {
            while (*f)
            {
               if (!strncasecmp (f, fob, l) && (!f[l] || isspace (f[l])))
                  break;
               while (*f && !isspace (*f))
                  f++;
               while (*f && isspace (*f))
                  f++;
            }
            if (*f)
               break;
         }
      }
      if (u)
      {                         // Found
         const char *n = xml_get (u, "@name");
         if (n)
         {
            if (!username)
               printf ("Fob is for user %s\n", n);
            else if (strcasecmp (xml_get (u, "@name") ? : "", username))
               errx (1, "Fob is already on user %s", n);
         }
      } else if (username)
      {                         // Find user and add fob
         while ((u = xml_element_next_by_name (c, u, "user")))
            if (!strcasecmp (xml_get (u, "@name") ? : "", username))
               break;
         {
            const char *f = xml_get (u, "@fob");
            if (f && !*f)
               f = NULL;
            xml_addf (u, "@fob", "%s%s%s", fob, fob ? " " : "", f ? : "");
         }
         // Write new config
         FILE *f = fopen (config, "w");
         if (!f)
            err (1, "Can't open %s", config);
         xml_write (f, c);
         fclose (f);
      }
   }

   printf ("Remove card\n");
   led ("G");

   done = 1;
   if (recvt (sp[1], buf, sizeof (buf), 30) != 0)
      errx (1, "Giving up");

   if (c)
      xml_tree_delete (c);
   mosquitto_disconnect (mqtt);
   mosquitto_loop_stop (mqtt, false);
   mosquitto_lib_cleanup ();
   return 0;
}
