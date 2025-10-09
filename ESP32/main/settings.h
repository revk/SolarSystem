// Settings
// Generated from:-
// components/ESP32-RevK/settings.def
// main/settings.def
// main/settings_nfc.def
// main/settings_door.def
// main/settings_gps.def
// main/settings_output.def
// main/settings_input.def
// main/settings_keypad.def
// main/settings_alarm.def

#include <stdint.h>
#include <stddef.h>
typedef struct revk_settings_s revk_settings_t;
struct revk_settings_s {
 void *ptr;
 const char name[16];
 const char *def;
 const char *flags;
 const char *old;
 const char *comment;
 uint16_t size;
 uint8_t group;
 uint8_t bit;
 uint8_t dot:4;
 uint8_t len:4;
 uint8_t type:3;
 uint8_t decimal:5;
 uint8_t digits:5;
 uint8_t array:7;
 uint8_t malloc:1;
 uint8_t revk:1;
 uint8_t live:1;
 uint8_t hide:1;
 uint8_t fix:1;
 uint8_t set:1;
 uint8_t hex:1;
 uint8_t base32:1;
 uint8_t base64:1;
 uint8_t secret:1;
 uint8_t dq:1;
 uint8_t rtc:1;
 uint8_t gpio:1;
};
typedef struct revk_settings_blob_s revk_settings_blob_t;
struct revk_settings_blob_s {
 uint16_t len;
 uint8_t data[];
};
typedef struct revk_gpio_s revk_gpio_t;
struct revk_gpio_s {
 uint16_t num:10;
 uint16_t strong:1;
 uint16_t weak:1;
 uint16_t pulldown:1;
 uint16_t nopull:1;
 uint16_t invert:1;
 uint16_t set:1;
};
enum {
#ifdef	CONFIG_REVK_SETTINGS_PASSWORD
#endif
#ifdef  CONFIG_MDNS_MAX_INTERFACES
#else
#endif
 REVK_SETTINGS_BITFIELD_otaauto,
#ifdef	CONFIG_REVK_WEB_BETA
 REVK_SETTINGS_BITFIELD_otabeta,
#endif
 REVK_SETTINGS_BITFIELD_dark,
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#else
#endif
 REVK_SETTINGS_BITFIELD_prefixapp,
 REVK_SETTINGS_BITFIELD_prefixhost,
#ifdef	CONFIG_REVK_BLINK_DEF
#ifdef	CONFIG_REVK_BLINK_WS2812_DEF
#else
#endif
#endif
#ifdef  CONFIG_REVK_APMODE
#ifdef	CONFIG_REVK_APCONFIG
#endif
#endif
#ifdef  CONFIG_REVK_MQTT
#endif
#if     defined(CONFIG_REVK_WIFI) || defined(CONFIG_REVK_MESH)
 REVK_SETTINGS_BITFIELD_wifips,
 REVK_SETTINGS_BITFIELD_wifimaxps,
#endif
#ifndef	CONFIG_REVK_MESH
 REVK_SETTINGS_BITFIELD_aplr,
 REVK_SETTINGS_BITFIELD_aphide,
#endif
#ifdef	CONFIG_REVK_MESH
 REVK_SETTINGS_BITFIELD_meshlr,
 REVK_SETTINGS_BITFIELD_meshroot,
#endif
#define	AREAS	"ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ"
 REVK_SETTINGS_BITFIELD_debug,
 REVK_SETTINGS_BITFIELD_iotstatedoor,
 REVK_SETTINGS_BITFIELD_iotstateinput,
 REVK_SETTINGS_BITFIELD_iotstateoutput,
 REVK_SETTINGS_BITFIELD_iotstatesystem,
 REVK_SETTINGS_BITFIELD_ioteventfob,
 REVK_SETTINGS_BITFIELD_ioteventarm,
 REVK_SETTINGS_BITFIELD_iotkeypad,
 REVK_SETTINGS_BITFIELD_iotgps,
 REVK_SETTINGS_BITFIELD_doordebug,
 REVK_SETTINGS_BITFIELD_doorexitarm,
 REVK_SETTINGS_BITFIELD_doorexitdisarm,
 REVK_SETTINGS_BITFIELD_doorcatch,
#define	MAXOUTPUT	10
#define	OUTPUT_FUNCS	"LDBE----"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release (this needs to be L>>1)
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error
#define	MAXINPUT	16
#define	INPUT_FUNCS	"EOLDMP--"
#define	INPUT_FUNC_E	0x80		// Exit pressed
#define	INPUT_FUNC_O	0x40		// Open door
#define	INPUT_FUNC_L	0x20		// Lock unlocked
#define	INPUT_FUNC_D	0x10		// Deadlock unlocked (This needs to be L>>1)
#define	INPUT_FUNC_M	0x08		// Manual active
#define	INPUT_FUNC_P	0x04		// Pull down (not really a function, but a rarely needed input flag)
 REVK_SETTINGS_BITFIELD_keypadpinarm,
#define	MAX_MIX		5
};
typedef struct revk_settings_bits_s revk_settings_bits_t;
struct revk_settings_bits_s {
#ifdef	CONFIG_REVK_SETTINGS_PASSWORD
#endif
#ifdef  CONFIG_MDNS_MAX_INTERFACES
#else
#endif
 uint8_t otaauto:1;	// OTA auto upgrade
#ifdef	CONFIG_REVK_WEB_BETA
 uint8_t otabeta:1;	// OTA from beta release
#endif
 uint8_t dark:1;	// Default LED off
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#else
#endif
 uint8_t prefixapp:1;	// MQTT use appname/ in front of hostname in topic
 uint8_t prefixhost:1;	// MQTT use (appname/)hostname/topic instead of topic/(appname/)hostname
#ifdef	CONFIG_REVK_BLINK_DEF
#ifdef	CONFIG_REVK_BLINK_WS2812_DEF
#else
#endif
#endif
#ifdef  CONFIG_REVK_APMODE
#ifdef	CONFIG_REVK_APCONFIG
#endif
#endif
#ifdef  CONFIG_REVK_MQTT
#endif
#if     defined(CONFIG_REVK_WIFI) || defined(CONFIG_REVK_MESH)
 uint8_t wifips:1;	// WiFi power save
 uint8_t wifimaxps:1;	// WiFi power save (max)
#endif
#ifndef	CONFIG_REVK_MESH
 uint8_t aplr:1;	// AP LR mode
 uint8_t aphide:1;	// AP hide SSID
#endif
#ifdef	CONFIG_REVK_MESH
 uint8_t meshlr:1;	// Mesh use LR mode
 uint8_t meshroot:1;	// This is preferred mesh root
#endif
#define	AREAS	"ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ"
 uint8_t debug:1;
 uint8_t iotstatedoor:1;
 uint8_t iotstateinput:1;
 uint8_t iotstateoutput:1;
 uint8_t iotstatesystem:1;
 uint8_t ioteventfob:1;
 uint8_t ioteventarm:1;
 uint8_t iotkeypad:1;
 uint8_t iotgps:1;
 uint8_t doordebug:1;
 uint8_t doorexitarm:1;
 uint8_t doorexitdisarm:1;
 uint8_t doorcatch:1;
#define	MAXOUTPUT	10
#define	OUTPUT_FUNCS	"LDBE----"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release (this needs to be L>>1)
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error
#define	MAXINPUT	16
#define	INPUT_FUNCS	"EOLDMP--"
#define	INPUT_FUNC_E	0x80		// Exit pressed
#define	INPUT_FUNC_O	0x40		// Open door
#define	INPUT_FUNC_L	0x20		// Lock unlocked
#define	INPUT_FUNC_D	0x10		// Deadlock unlocked (This needs to be L>>1)
#define	INPUT_FUNC_M	0x08		// Manual active
#define	INPUT_FUNC_P	0x04		// Pull down (not really a function, but a rarely needed input flag)
 uint8_t keypadpinarm:1;
#define	MAX_MIX		5
};
#ifdef	CONFIG_REVK_SETTINGS_PASSWORD
extern char* password;	// Settings password<br>(not sent securely so use with care)
#endif
#ifdef  CONFIG_MDNS_MAX_INTERFACES
extern char* hostname;	// Hostname[.local]<br>(used in DHCP and MQTT)
#else
extern char* hostname;	// Host name<br>(used in DHCP and MQTT)
#endif
extern char* appname;	// Application name
extern char* otahost;	// OTA hostname
extern uint8_t otadays;	// OTA auto load (days)
extern uint16_t otastart;	// OTA check after startup (min seconds)
#define	otaauto	revk_settings_bits.otaauto
#ifdef	CONFIG_REVK_WEB_BETA
#define	otabeta	revk_settings_bits.otabeta
#endif
extern revk_settings_blob_t* otacert;	// OTA cert of otahost
#define	dark	revk_settings_bits.dark
extern char* ntphost;	// NTP host
extern char* tz;	// Timezone (<a href='https://gist.github.com/alwynallan/24d96091655391107939' target=_blank>info</a>)
extern uint32_t watchdogtime;	// Watchdog (seconds)
#ifdef  CONFIG_IDF_TARGET_ESP32S3
extern uint16_t usbuptime;	// USB  turns off after this many seconds
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
extern revk_gpio_t factorygpio;	// Factory reset GPIO (press 3 times)
#else
extern revk_gpio_t factorygpio;	// Factory reset GPIO (press 3 times)
#endif
extern char* topicgroup[2];	// MQTT Alternative hostname accepted for commands
extern char* topiccommand;	// MQTT Topic for commands
extern char* topicsetting;	// MQTT Topic for settings
extern char* topicstate;	// MQTT Topic for state
extern char* topicevent;	// MQTT Topic for event
extern char* topicinfo;	// MQTT Topic for info
extern char* topicerror;	// MQTT Topic for error
extern char* topicha;	// MQTT Topic for homeassistant
#define	prefixapp	revk_settings_bits.prefixapp
#define	prefixhost	revk_settings_bits.prefixhost
#ifdef	CONFIG_REVK_BLINK_DEF
#ifdef	CONFIG_REVK_BLINK_WS2812_DEF
extern revk_gpio_t blink;	// WS2812 LED
#else
extern revk_gpio_t blink[3];	// R, G, B LED array (set all the same for WS2812 LED)
#endif
#endif
#ifdef  CONFIG_REVK_APMODE
#ifdef	CONFIG_REVK_APCONFIG
extern uint16_t apport;	// TCP port for config web pages on AP
#endif
extern uint32_t aptime;	// Limit AP to time (seconds)
extern uint32_t apwait;	// Wait off line before starting AP (seconds)
extern revk_gpio_t apgpio;	// Start AP on GPIO
#endif
#ifdef  CONFIG_REVK_MQTT
extern char* mqtthost[CONFIG_REVK_MQTT_CLIENTS];	// MQTT hostname
extern uint16_t mqttport[CONFIG_REVK_MQTT_CLIENTS];	// MQTT port
extern char* mqttuser[CONFIG_REVK_MQTT_CLIENTS];	// MQTT username
extern char* mqttpass[CONFIG_REVK_MQTT_CLIENTS];	// MQTT password
extern revk_settings_blob_t* mqttcert[CONFIG_REVK_MQTT_CLIENTS];	// MQTT CA certificate (for mqtts)
#endif
extern revk_settings_blob_t* clientkey;	// Client Key (OTA and MQTT TLS)
extern revk_settings_blob_t* clientcert;	// Client certificate (OTA and MQTT TLS)
#if     defined(CONFIG_REVK_WIFI) || defined(CONFIG_REVK_MESH)
extern uint16_t wifireset;	// Restart if WiFi off for this long (seconds)
extern char* wifissid;	// WiFI SSID (name)
extern char* wifipass;	// WiFi password
extern char* wifiip;	// WiFi Fixed IP
extern char* wifigw;	// WiFi Fixed gateway
extern char* wifidns[3];	// WiFi fixed DNS
extern uint8_t wifibssid[6];	// WiFI BSSID
extern uint8_t wifichan;	// WiFI channel
extern uint16_t wifiuptime;	// WiFI turns off after this many seconds
#define	wifips	revk_settings_bits.wifips
#define	wifimaxps	revk_settings_bits.wifimaxps
#endif
#ifndef	CONFIG_REVK_MESH
extern char* apssid;	// AP mode SSID (name)
extern char* appass;	// AP mode password
extern uint8_t apmax;	// AP max clients
extern char* apip;	// AP mode block
#define	aplr	revk_settings_bits.aplr
#define	aphide	revk_settings_bits.aphide
#endif
#ifdef	CONFIG_REVK_MESH
extern char* nodename;	// Mesh node name
extern uint16_t meshreset;	// Reset if mesh off for this long (seconds)
extern uint8_t meshid[6];	// Mesh ID (hex)
extern uint8_t meshkey[16];	// Mesh key
extern uint16_t meshwidth;	// Mesh width
extern uint16_t meshdepth;	// Mesh depth
extern uint16_t meshmax;	// Mesh max devices
extern char* meshpass;	// Mesh AP password
#define	meshlr	revk_settings_bits.meshlr
#define	meshroot	revk_settings_bits.meshroot
#endif
#define	AREAS	"ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ"
extern revk_gpio_t tamper;
#define	debug	revk_settings_bits.debug
#define	iotstatedoor	revk_settings_bits.iotstatedoor
#define	iotstateinput	revk_settings_bits.iotstateinput
#define	iotstateoutput	revk_settings_bits.iotstateoutput
#define	iotstatesystem	revk_settings_bits.iotstatesystem
#define	ioteventfob	revk_settings_bits.ioteventfob
#define	ioteventarm	revk_settings_bits.ioteventarm
#define	iotkeypad	revk_settings_bits.iotkeypad
#define	iotgps	revk_settings_bits.iotgps
extern char* iottopic;
extern uint8_t nfcred;
extern uint8_t nfcamber;
extern uint8_t nfcgreen;
extern uint8_t nfccard;
extern revk_gpio_t nfctx;
extern revk_gpio_t nfcrx;
extern revk_gpio_t nfcpower;
extern uint16_t nfcpoll;
extern uint16_t nfcholdpoll;
extern uint8_t nfchold;
extern uint8_t nfclonghold;
extern uint16_t nfcledpoll;
extern uint16_t nfciopoll;
extern uint8_t nfcuart;
extern uint8_t nfcbaud;
extern char* nfcmqttbell;
extern uint8_t aes[3][18];
extern uint8_t aid[3];
extern char* ledIDLE;
extern uint8_t doorauto;
extern uint32_t doorunlock;
extern uint32_t doorlock;
extern uint32_t dooropen;
extern uint32_t doorclose;
extern uint32_t doorprop;
extern uint32_t doorexit;
extern uint32_t doorpoll;
extern uint32_t doordebounce;
#define	doordebug	revk_settings_bits.doordebug
#define	doorexitarm	revk_settings_bits.doorexitarm
#define	doorexitdisarm	revk_settings_bits.doorexitdisarm
#define	doorcatch	revk_settings_bits.doorcatch
extern char* fallback[10];
extern char* blacklist[10];
extern char* dooriotopen;
extern char* dooriotdead;
extern char* dooriotundead;
extern char* dooriotlock;
extern char* dooriotunlock;
extern revk_gpio_t gpstx;
extern revk_gpio_t gpsrx;
extern revk_gpio_t gpstick;
extern uint8_t gpsuart;
#define	MAXOUTPUT	10
#define	OUTPUT_FUNCS	"LDBE----"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release (this needs to be L>>1)
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error
extern revk_gpio_t blink[3];
extern uint8_t rgbs;
extern revk_gpio_t outgpio[MAXOUTPUT];
extern uint8_t outfunc[MAXOUTPUT];
extern int16_t outpulse[MAXOUTPUT];
extern uint8_t outrgb[MAXOUTPUT];
extern char* outname[MAXOUTPUT];
extern uint32_t outengineer[MAXOUTPUT];
extern uint32_t outfaulted[MAXOUTPUT];
extern uint32_t outtampered[MAXOUTPUT];
extern uint32_t outalarmed[MAXOUTPUT];
extern uint32_t outarmed[MAXOUTPUT];
extern uint32_t outprearm[MAXOUTPUT];
extern uint32_t outprealarm[MAXOUTPUT];
extern uint32_t outalarm[MAXOUTPUT];
extern uint32_t outdoorbell[MAXOUTPUT];
extern uint32_t outwarning[MAXOUTPUT];
extern uint32_t outfault[MAXOUTPUT];
extern uint32_t outaccess[MAXOUTPUT];
extern uint32_t outpresence[MAXOUTPUT];
extern uint32_t outtamper[MAXOUTPUT];
extern uint32_t outfire[MAXOUTPUT];
extern uint32_t outpanic[MAXOUTPUT];
extern revk_gpio_t powergpio[MAXOUTPUT];
extern uint8_t powerrgb[MAXOUTPUT];
#define	MAXINPUT	16
#define	INPUT_FUNCS	"EOLDMP--"
#define	INPUT_FUNC_E	0x80		// Exit pressed
#define	INPUT_FUNC_O	0x40		// Open door
#define	INPUT_FUNC_L	0x20		// Lock unlocked
#define	INPUT_FUNC_D	0x10		// Deadlock unlocked (This needs to be L>>1)
#define	INPUT_FUNC_M	0x08		// Manual active
#define	INPUT_FUNC_P	0x04		// Pull down (not really a function, but a rarely needed input flag)
extern uint8_t inpoll;
extern revk_gpio_t ingpio[MAXINPUT];
extern uint8_t inrgb[MAXINPUT];
extern uint8_t infunc[MAXINPUT];
extern uint8_t inhold[MAXINPUT];
extern char* inname[MAXINPUT];
extern uint32_t indoorbell[MAXINPUT];
extern uint32_t inwarning[MAXINPUT];
extern uint32_t infault[MAXINPUT];
extern uint32_t inaccess[MAXINPUT];
extern uint32_t inpresence[MAXINPUT];
extern uint32_t intamper[MAXINPUT];
extern uint32_t infire[MAXINPUT];
extern uint32_t inpanic[MAXINPUT];
extern uint32_t inarm[MAXINPUT];
extern uint32_t instrong[MAXINPUT];
extern uint32_t indisarm[MAXINPUT];
extern revk_gpio_t keypadtx;
extern revk_gpio_t keypadrx;
extern revk_gpio_t keypadde;
extern revk_gpio_t keypadre;
extern revk_gpio_t keypadclk;
extern uint8_t keypadtimer;
extern uint8_t keypadaddress;
extern uint8_t keypadtxpre;
extern uint8_t keypadtxpost;
extern uint8_t keypadrxpre;
extern uint8_t keypadrxpost;
extern char* keypadidle;
extern char* keypadpin;
#define	keypadpinarm	revk_settings_bits.keypadpinarm
#define	MAX_MIX		5
extern uint32_t arealed;
extern uint32_t areaenter;
extern uint32_t areaarm;
extern uint32_t areastrong;
extern uint32_t areadisarm;
extern uint32_t areadeadlock;
extern uint32_t areakeypad;
extern uint32_t areakeydisarm;
extern uint32_t areakeystrong;
extern uint32_t areakeyarm;
extern uint32_t engineer;
extern uint32_t armed;
extern uint32_t smsarm;
extern uint32_t smsdisarm;
extern uint32_t smscancel;
extern uint32_t smsarmfail;
extern uint32_t smsalarm;
extern uint32_t smspanic;
extern uint32_t smsfire;
extern uint32_t mixand[MAX_MIX];
extern uint32_t mixset[MAX_MIX];
extern uint16_t armcancel;
extern uint16_t armdelay;
extern uint16_t alarmdelay;
extern uint16_t alarmhold;
extern uint8_t meshexpect;
extern uint8_t meshcycle;
extern uint8_t meshwarmup;
extern uint8_t meshflap;
extern uint8_t meshdied;
extern uint16_t mqttdied;
extern char* smsnumber;
extern uint16_t timer1;
extern revk_settings_bits_t revk_settings_bits;
enum {
 REVK_SETTINGS_SIGNED,
 REVK_SETTINGS_UNSIGNED,
 REVK_SETTINGS_BIT,
 REVK_SETTINGS_BLOB,
 REVK_SETTINGS_STRING,
 REVK_SETTINGS_OCTET,
};
#ifdef	CONFIG_REVK_SETTINGS_PASSWORD
#define REVK_SETTINGS_PASSWORD
#endif
#ifdef  CONFIG_MDNS_MAX_INTERFACES
#define REVK_SETTINGS_HOSTNAME
#else
#define REVK_SETTINGS_HOSTNAME
#endif
#define REVK_SETTINGS_APPNAME
#define REVK_SETTINGS_OTAHOST
#define REVK_SETTINGS_OTADAYS
#define REVK_SETTINGS_OTASTART
#define REVK_SETTINGS_OTAAUTO
#ifdef	CONFIG_REVK_WEB_BETA
#define REVK_SETTINGS_OTABETA
#endif
#define REVK_SETTINGS_OTACERT
#define REVK_SETTINGS_DARK
#define REVK_SETTINGS_NTPHOST
#define REVK_SETTINGS_TZ
#define REVK_SETTINGS_WATCHDOGTIME
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#define REVK_SETTINGS_USBUPTIME
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#define REVK_SETTINGS_FACTORYGPIO
#else
#define REVK_SETTINGS_FACTORYGPIO
#endif
#define REVK_SETTINGS_TOPICGROUP
#define REVK_SETTINGS_TOPICCOMMAND
#define REVK_SETTINGS_TOPICSETTING
#define REVK_SETTINGS_TOPICSTATE
#define REVK_SETTINGS_TOPICEVENT
#define REVK_SETTINGS_TOPICINFO
#define REVK_SETTINGS_TOPICERROR
#define REVK_SETTINGS_TOPICHA
#define REVK_SETTINGS_PREFIXAPP
#define REVK_SETTINGS_PREFIXHOST
#ifdef	CONFIG_REVK_BLINK_DEF
#ifdef	CONFIG_REVK_BLINK_WS2812_DEF
#define REVK_SETTINGS_BLINK
#else
#define REVK_SETTINGS_BLINK
#endif
#endif
#ifdef  CONFIG_REVK_APMODE
#ifdef	CONFIG_REVK_APCONFIG
#define REVK_SETTINGS_APPORT
#endif
#define REVK_SETTINGS_APTIME
#define REVK_SETTINGS_APWAIT
#define REVK_SETTINGS_APGPIO
#endif
#ifdef  CONFIG_REVK_MQTT
#define REVK_SETTINGS_MQTTHOST
#define REVK_SETTINGS_MQTTPORT
#define REVK_SETTINGS_MQTTUSER
#define REVK_SETTINGS_MQTTPASS
#define REVK_SETTINGS_MQTTCERT
#endif
#define REVK_SETTINGS_CLIENTKEY
#define REVK_SETTINGS_CLIENTCERT
#if     defined(CONFIG_REVK_WIFI) || defined(CONFIG_REVK_MESH)
#define REVK_SETTINGS_WIFIRESET
#define REVK_SETTINGS_WIFISSID
#define REVK_SETTINGS_WIFIPASS
#define REVK_SETTINGS_WIFIIP
#define REVK_SETTINGS_WIFIGW
#define REVK_SETTINGS_WIFIDNS
#define REVK_SETTINGS_WIFIBSSID
#define REVK_SETTINGS_WIFICHAN
#define REVK_SETTINGS_WIFIUPTIME
#define REVK_SETTINGS_WIFIPS
#define REVK_SETTINGS_WIFIMAXPS
#endif
#ifndef	CONFIG_REVK_MESH
#define REVK_SETTINGS_APSSID
#define REVK_SETTINGS_APPASS
#define REVK_SETTINGS_APMAX
#define REVK_SETTINGS_APIP
#define REVK_SETTINGS_APLR
#define REVK_SETTINGS_APHIDE
#endif
#ifdef	CONFIG_REVK_MESH
#define REVK_SETTINGS_NODENAME
#define REVK_SETTINGS_MESHRESET
#define REVK_SETTINGS_MESHID
#define REVK_SETTINGS_MESHKEY
#define REVK_SETTINGS_MESHWIDTH
#define REVK_SETTINGS_MESHDEPTH
#define REVK_SETTINGS_MESHMAX
#define REVK_SETTINGS_MESHPASS
#define REVK_SETTINGS_MESHLR
#define REVK_SETTINGS_MESHROOT
#endif
#define	AREAS	"ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ"
#define REVK_SETTINGS_TAMPER
#define REVK_SETTINGS_DEBUG
#define REVK_SETTINGS_IOTSTATEDOOR
#define REVK_SETTINGS_IOTSTATEINPUT
#define REVK_SETTINGS_IOTSTATEOUTPUT
#define REVK_SETTINGS_IOTSTATESYSTEM
#define REVK_SETTINGS_IOTEVENTFOB
#define REVK_SETTINGS_IOTEVENTARM
#define REVK_SETTINGS_IOTKEYPAD
#define REVK_SETTINGS_IOTGPS
#define REVK_SETTINGS_IOTTOPIC
#define REVK_SETTINGS_NFCRED
#define REVK_SETTINGS_NFCAMBER
#define REVK_SETTINGS_NFCGREEN
#define REVK_SETTINGS_NFCCARD
#define REVK_SETTINGS_NFCTX
#define REVK_SETTINGS_NFCRX
#define REVK_SETTINGS_NFCPOWER
#define REVK_SETTINGS_NFCPOLL
#define REVK_SETTINGS_NFCHOLDPOLL
#define REVK_SETTINGS_NFCHOLD
#define REVK_SETTINGS_NFCLONGHOLD
#define REVK_SETTINGS_NFCLEDPOLL
#define REVK_SETTINGS_NFCIOPOLL
#define REVK_SETTINGS_NFCUART
#define REVK_SETTINGS_NFCBAUD
#define REVK_SETTINGS_NFCMQTTBELL
#define REVK_SETTINGS_AES
#define REVK_SETTINGS_AID
#define REVK_SETTINGS_LEDIDLE
#define REVK_SETTINGS_DOORAUTO
#define REVK_SETTINGS_DOORUNLOCK
#define REVK_SETTINGS_DOORLOCK
#define REVK_SETTINGS_DOOROPEN
#define REVK_SETTINGS_DOORCLOSE
#define REVK_SETTINGS_DOORPROP
#define REVK_SETTINGS_DOOREXIT
#define REVK_SETTINGS_DOORPOLL
#define REVK_SETTINGS_DOORDEBOUNCE
#define REVK_SETTINGS_DOORDEBUG
#define REVK_SETTINGS_DOOREXITARM
#define REVK_SETTINGS_DOOREXITDISARM
#define REVK_SETTINGS_DOORCATCH
#define REVK_SETTINGS_FALLBACK
#define REVK_SETTINGS_BLACKLIST
#define REVK_SETTINGS_DOORIOTOPEN
#define REVK_SETTINGS_DOORIOTDEAD
#define REVK_SETTINGS_DOORIOTUNDEAD
#define REVK_SETTINGS_DOORIOTLOCK
#define REVK_SETTINGS_DOORIOTUNLOCK
#define REVK_SETTINGS_GPSTX
#define REVK_SETTINGS_GPSRX
#define REVK_SETTINGS_GPSTICK
#define REVK_SETTINGS_GPSUART
#define	MAXOUTPUT	10
#define	OUTPUT_FUNCS	"LDBE----"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release (this needs to be L>>1)
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error
#define REVK_SETTINGS_BLINK
#define REVK_SETTINGS_RGBS
#define REVK_SETTINGS_OUTGPIO
#define REVK_SETTINGS_OUTFUNC
#define REVK_SETTINGS_OUTPULSE
#define REVK_SETTINGS_OUTRGB
#define REVK_SETTINGS_OUTNAME
#define REVK_SETTINGS_OUTENGINEER
#define REVK_SETTINGS_OUTFAULTED
#define REVK_SETTINGS_OUTTAMPERED
#define REVK_SETTINGS_OUTALARMED
#define REVK_SETTINGS_OUTARMED
#define REVK_SETTINGS_OUTPREARM
#define REVK_SETTINGS_OUTPREALARM
#define REVK_SETTINGS_OUTALARM
#define REVK_SETTINGS_OUTDOORBELL
#define REVK_SETTINGS_OUTWARNING
#define REVK_SETTINGS_OUTFAULT
#define REVK_SETTINGS_OUTACCESS
#define REVK_SETTINGS_OUTPRESENCE
#define REVK_SETTINGS_OUTTAMPER
#define REVK_SETTINGS_OUTFIRE
#define REVK_SETTINGS_OUTPANIC
#define REVK_SETTINGS_POWERGPIO
#define REVK_SETTINGS_POWERRGB
#define	MAXINPUT	16
#define	INPUT_FUNCS	"EOLDMP--"
#define	INPUT_FUNC_E	0x80		// Exit pressed
#define	INPUT_FUNC_O	0x40		// Open door
#define	INPUT_FUNC_L	0x20		// Lock unlocked
#define	INPUT_FUNC_D	0x10		// Deadlock unlocked (This needs to be L>>1)
#define	INPUT_FUNC_M	0x08		// Manual active
#define	INPUT_FUNC_P	0x04		// Pull down (not really a function, but a rarely needed input flag)
#define REVK_SETTINGS_INPOLL
#define REVK_SETTINGS_INGPIO
#define REVK_SETTINGS_INRGB
#define REVK_SETTINGS_INFUNC
#define REVK_SETTINGS_INHOLD
#define REVK_SETTINGS_INNAME
#define REVK_SETTINGS_INDOORBELL
#define REVK_SETTINGS_INWARNING
#define REVK_SETTINGS_INFAULT
#define REVK_SETTINGS_INACCESS
#define REVK_SETTINGS_INPRESENCE
#define REVK_SETTINGS_INTAMPER
#define REVK_SETTINGS_INFIRE
#define REVK_SETTINGS_INPANIC
#define REVK_SETTINGS_INARM
#define REVK_SETTINGS_INSTRONG
#define REVK_SETTINGS_INDISARM
#define REVK_SETTINGS_KEYPADTX
#define REVK_SETTINGS_KEYPADRX
#define REVK_SETTINGS_KEYPADDE
#define REVK_SETTINGS_KEYPADRE
#define REVK_SETTINGS_KEYPADCLK
#define REVK_SETTINGS_KEYPADTIMER
#define REVK_SETTINGS_KEYPADADDRESS
#define REVK_SETTINGS_KEYPADTXPRE
#define REVK_SETTINGS_KEYPADTXPOST
#define REVK_SETTINGS_KEYPADRXPRE
#define REVK_SETTINGS_KEYPADRXPOST
#define REVK_SETTINGS_KEYPADIDLE
#define REVK_SETTINGS_KEYPADPIN
#define REVK_SETTINGS_KEYPADPINARM
#define	MAX_MIX		5
#define REVK_SETTINGS_AREALED
#define REVK_SETTINGS_AREAENTER
#define REVK_SETTINGS_AREAARM
#define REVK_SETTINGS_AREASTRONG
#define REVK_SETTINGS_AREADISARM
#define REVK_SETTINGS_AREADEADLOCK
#define REVK_SETTINGS_AREAKEYPAD
#define REVK_SETTINGS_AREAKEYDISARM
#define REVK_SETTINGS_AREAKEYSTRONG
#define REVK_SETTINGS_AREAKEYARM
#define REVK_SETTINGS_ENGINEER
#define REVK_SETTINGS_ARMED
#define REVK_SETTINGS_SMSARM
#define REVK_SETTINGS_SMSDISARM
#define REVK_SETTINGS_SMSCANCEL
#define REVK_SETTINGS_SMSARMFAIL
#define REVK_SETTINGS_SMSALARM
#define REVK_SETTINGS_SMSPANIC
#define REVK_SETTINGS_SMSFIRE
#define REVK_SETTINGS_MIXAND
#define REVK_SETTINGS_MIXSET
#define REVK_SETTINGS_ARMCANCEL
#define REVK_SETTINGS_ARMDELAY
#define REVK_SETTINGS_ALARMDELAY
#define REVK_SETTINGS_ALARMHOLD
#define REVK_SETTINGS_MESHEXPECT
#define REVK_SETTINGS_MESHCYCLE
#define REVK_SETTINGS_MESHWARMUP
#define REVK_SETTINGS_MESHFLAP
#define REVK_SETTINGS_MESHDIED
#define REVK_SETTINGS_MQTTDIED
#define REVK_SETTINGS_SMSNUMBER
#define REVK_SETTINGS_TIMER1
#define	REVK_SETTINGS_HAS_OLD
#define	REVK_SETTINGS_HAS_COMMENT
#define	REVK_SETTINGS_HAS_GPIO
#define	REVK_SETTINGS_HAS_NUMERIC
#define	REVK_SETTINGS_HAS_SIGNED
#define	REVK_SETTINGS_HAS_UNSIGNED
#define	REVK_SETTINGS_HAS_BIT
#define	REVK_SETTINGS_HAS_BLOB
#define	REVK_SETTINGS_HAS_STRING
#define	REVK_SETTINGS_HAS_OCTET
typedef uint8_t revk_setting_bits_t[27];
typedef uint8_t revk_setting_group_t[3];
extern const char revk_settings_secret[];
