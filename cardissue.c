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


#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <popt.h>
#include <err.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
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
  const char *hexaid = NULL;
  const char *hexaes = NULL;
  const char *hexreader = NULL;
  const char *config = NULL;
  int doformat = 0;
  int mqttport = 1883;
  int comms = 0;
  int logs = 50;
  {				// POPT
    poptContext optCon;		// context for parsing command-line options
    const struct poptOption optionsTable[] = {
      {"mqtt-host", 'h', POPT_ARG_STRING, &mqtthost, 0, "MQTT host", "hostname"},
      {"mqtt-user", 'U', POPT_ARG_STRING, &mqttuser, 0, "MQTT user", "username"},
      {"mqtt-pass", 'P', POPT_ARG_STRING, &mqttpass, 0, "MQTT pass", "password"},
      {"mqtt-cert", 'C', POPT_ARG_STRING, &mqttcert, 0, "MQTT CA", "filename"},
      {"mqtt-port", 'p', POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &mqttport, 0, "MQTT port", "port"},
      {"name", 'n', POPT_ARG_STRING, &setname, 0, "Users name", "Full name"},
      {"config", 'c', POPT_ARG_STRING, &config, 0, "Config", "filename"},
      {"aid", 'a', POPT_ARG_STRING, &hexaid, 0, "AID", "XXXXXX"},
      {"aes", 'A', POPT_ARG_STRING, &hexaes, 0, "AES", "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"},
      {"reader", 'r', POPT_ARG_STRING, &hexreader, 0, "Reader", "XXXXXX"},
      {"format", 0, POPT_ARG_NONE, &doformat, 0, "Format card", 0},
      {"logs", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &logs, 0, "Log records", "N"},
      {"comms", 0, POPT_ARG_INT | POPT_ARGFLAG_SHOW_DEFAULT, &comms, 0, "File comms", "0/1/3"},
      {"debug", 'v', POPT_ARG_NONE, &debug, 0, "Debug", 0},
      POPT_AUTOHELP {}
    };

    optCon = poptGetContext (NULL, argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp (optCon, "[aid/config] [reader]");

    int c;
    if ((c = poptGetNextOpt (optCon)) < -1)
      errx (1, "%s: %s\n", poptBadOption (optCon, POPT_BADOPTION_NOALIAS), poptStrerror (c));

    if (poptPeekArg (optCon) && !config && !hexaid)
      {
	const char *v = poptGetArg (optCon);
	if (strlen (v) == 6 && df_hex (4, NULL, v) == 3)
	  hexaid = v;
	else
	  config = v;
      }
    if (poptPeekArg (optCon) && !hexreader)
      hexreader = poptGetArg (optCon);

    if (poptPeekArg (optCon))
      {
	poptPrintUsage (optCon, stderr, 0);
	return -1;
      }
    poptFreeContext (optCon);
  }

  xml_t c = NULL;
  if (config)
    {				// Settings from config
      c = xml_tree_read_file (config);
      if (!c)
	errx (1, "Cannot read %s", config);
      const char *v;
      if (hexaid)
	{			// Is this an AID in the config?
	  if ((v = xml_get (c, "system@aid")) && !strcasecmp (v, hexaid))
	    {			// Main AID
	      v = xml_get (c, "system@aes");
	      if (hexaes && v && strcasecmp (v, hexaes))
		errx (1, "Incorrect AES");
	      hexaes = v;
	    }
	  else
	    {			// Check device specific aid
	      xml_t d = NULL;
	      while ((d = xml_element_next_by_name (c, d, "device")))
		if (xml_get (d, "@nfc"))
		  {
		    v = xml_get (d, "@aid");
		    if (v && !strcasecmp (v, hexaid))
		      {		// Match
			v = xml_get (d, "@aes");
			if (hexaes && v && strcasecmp (v, hexaes))
			  errx (1, "Incorrect AES");
			hexaes = v;
			if (!hexreader)
			  hexreader = xml_get (d, "@id");
			break;
		      }
		  }
	      if (!d)
		errx (1, "Specified AID not found in config");
	    }
	}
      else
	{
	  hexaid = xml_get (c, "system@aid");
	  if (!hexaes)
	    hexaes = xml_get (c, "system@aes");
	}
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
    errx (1, "AID is hex XXXXXX");
  // TODO saved AES

  if (!hexaes)
    errx (1, "Specify AES or config file");
  unsigned char aes[16];
  if (df_hex (sizeof (aes), aes, hexaes) != sizeof (aes))
    errx (1, "AES is hex XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
  if (!hexreader)
    errx (1, "Specify Reader");
  unsigned char reader[3];
  if (df_hex (sizeof (reader), reader, hexreader) != sizeof (reader))
    errx (1, "Reader is hex XXXXXX");


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
	send (sp[0], NULL, 0, 0);	// Indicate card gone
	return;
      }
    if (!strcasecmp (m, "id"))
      {
	if (debug)
	  fprintf (stderr, "ID %.*s\n", msg->payloadlen, (char *) msg->payload);
	send (sp[0], NULL, 0, 0);	// Indicate card gone
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
  if (mosquitto_connect_async (mqtt, mqtthost ? : "localhost", mqttport, 60))
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
	if (debug)
	  fprintf (stderr, "Rx fail %d\n", r);
	return r;
      }
    if (!r)
      {
	if (debug)
	  fprintf (stderr, "Rx timeout\n");
	return -1;
      }
    return recv (fd, data, len, 0);
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

  unsigned char buf[1024];
  printf ("Waiting for card at reader %s\n", hexreader);
  if (recvt (sp[1], buf, sizeof (buf), 30) != 0)
    errx (1, "Giving up");

  printf ("Card found\n");

  if ((e = df_select_application (&d, NULL)))
    errx (1, "Failed to select application: %s", e);

  // Check if card needs formatting
  if (!doformat)
    {
      unsigned char version;
      if ((e = df_get_key_version (&d, 0, &version)))
	errx (1, "Version: %s", e);
      if (!version)
	doformat = 1;		// Needs formatting
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
      if ((e = df_set_configuration (&d, 0x02)))
	errx (1, "Set config: %s", e);
    }
  else if ((e = df_authenticate (&d, 0, NULL)))
    errx (1, "Master auth: %s", e);

  // Get ID
  unsigned char uid[7];
  if ((e = df_get_uid (&d, uid)))
    errx (1, "Getting UID: %s", e);

  char hexuid[15];
  sprintf (hexuid, "%02X%02X%02X%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);
  printf ("UID %s\n", hexuid);

  // TODO per card key
  unsigned char cardkey[16] = { 0 };

  // Check if AID exists
  unsigned int n;
  if ((e = df_get_application_ids (&d, &n, sizeof (buf), buf)))
    errx (1, "Application list: %s", e);
  while (n && memcmp (buf + n * 3 - 3, aid, 3))
    n--;
  if (!n)
    {				// Create application
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
      if ((e = df_change_key_settings (&d, 0x09)))
	errx (1, "Change key settings: %s", e);
      if ((e = df_change_key (&d, 1, 0x01, NULL, aes)))
	errx (1, "Change key: %s", e);
    }
  else
    {
      if ((e = df_select_application (&d, aid)))
	errx (1, "Select application: %s", e);
      if ((e = df_authenticate (&d, 1, aes)))
	errx (1, "Authenticate AID: %s", e);	// Checking this works
      if ((e = df_authenticate (&d, 0, cardkey)))
	errx (1, "Authenticate Card: %s", e);	// So we have access
    }

  // Get information
  unsigned long long fids;
  if ((e = df_get_file_ids (&d, &fids)))
    errx (1, "File IDs: %s", e);
  if (fids & (1 << 0))
    {				// Name file
      char type;
      unsigned char comms;
      unsigned int size;
      if ((e = df_get_file_settings (&d, 0, &type, &comms, NULL, &size, NULL, NULL, NULL, NULL, NULL)))
	errx (1, "File settings: %s", e);
      if (type != 'D' && type != 'B')
	printf ("Name file is wrong type (%c)\n", type);
      else if (size > sizeof (buf))
	printf ("Name file is silly size (%d)\n", size);
      {				// Get name
	if ((e = df_read_data (&d, 0, comms, 0, size, buf)))
	  errx (1, "Read data: %s", e);
	printf ("Name: %.*s\n", size, buf);
      }
    }
  else if (setname)
    {				// Set the name
      printf ("Setting name: %s\n", setname);
      if ((e = df_create_file (&d, 0, 'D', comms, 0x1000, strlen (setname), 0, 0, 0, 0, 0)))
	errx (1, "Create file: %s", e);
      if ((e = df_write_data (&d, 0, 'D', comms, 0, strlen (setname), setname)))
	errx (1, "Write file: %s", e);
    }

  if (fids & (1 << 1))
    {				// Log file
      char type;
      unsigned char comms;
      unsigned int size, recs;
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
	    printf ("Log %02X%02X%02X on %02X%02X-%02X-%02X %02X:%02X:%02X\n",
		    buf[recs * 10 + 0], buf[recs * 10 + 1], buf[recs * 10 + 2],
		    buf[recs * 10 + 3], buf[recs * 10 + 4], buf[recs * 10 + 5], buf[recs * 10 + 6],
		    buf[recs * 10 + 7], buf[recs * 10 + 8], buf[recs * 10 + 9]);
	}
    }
  else
    {
      printf ("Creating log file\n");
      if ((e = df_create_file (&d, 1, 'C', comms, 0x0100, 10, 0, 0, 0, logs, 0)))
	errx (1, "Create file: %s", e);
    }

  if (fids & (1 << 2))
    {				// Counter file
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
    }
  else
    {
      printf ("Creating counter file\n");
      if ((e = df_create_file (&d, 2, 'V', comms, 0x0010, 0, 0, 0x7FFFFFFF, 0, 0, 0)))
	errx (1, "Create file: %s", e);
    }

  if (c)
    xml_tree_delete (c);
  mosquitto_lib_cleanup ();
  return 0;
}
