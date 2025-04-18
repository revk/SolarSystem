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
 uint8_t gpio:1;
 uint8_t rtc:1;
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
 REVK_SETTINGS_BITFIELD_otaauto,
 REVK_SETTINGS_BITFIELD_otabeta,
 REVK_SETTINGS_BITFIELD_prefixapp,
 REVK_SETTINGS_BITFIELD_prefixhost,
#ifdef	CONFIG_REVK_BLINK_DEF
#endif
 REVK_SETTINGS_BITFIELD_dark,
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#else
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
 uint8_t otaauto:1;	// OTA auto upgrade
 uint8_t otabeta:1;	// OTA from beta release
 uint8_t prefixapp:1;	// MQTT use appname/ in front of hostname in topic
 uint8_t prefixhost:1;	// MQTT use (appname/)hostname/topic instead of topic/(appname/)hostname
#ifdef	CONFIG_REVK_BLINK_DEF
#endif
 uint8_t dark:1;	// Default LED off
#ifdef  CONFIG_IDF_TARGET_ESP32S3
#else
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
extern char* password;	// Settings password (this is not sent securely so use with care on local networks you control)
#endif
extern char* hostname;	// Host name
extern char* appname;	// Application name
extern char* otahost;	// OTA hostname
extern uint8_t otadays;	// OTA auto load (days)
extern uint16_t otastart;	// OTA check after startup (min seconds)
#define	otaauto	revk_settings_bits.otaauto
#define	otabeta	revk_settings_bits.otabeta
extern revk_settings_blob_t* otacert;	// OTA cert of otahost
extern char* ntphost;	// NTP host
extern char* tz;	// Timezone (<a href='https://gist.github.com/alwynallan/24d96091655391107939' target=_blank>info</a>)
extern uint32_t watchdogtime;	// Watchdog (seconds)
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
extern revk_gpio_t blink[3];	// R, G, B LED array (set all the same for WS2812 LED)
#endif
#define	dark	revk_settings_bits.dark
#ifdef  CONFIG_IDF_TARGET_ESP32S3
extern revk_gpio_t factorygpio;	// Factory reset GPIO (press 3 times)
#else
extern revk_gpio_t factorygpio;	// Factory reset GPIO (press 3 times)
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
extern revk_settings_blob_t* mqttcert[CONFIG_REVK_MQTT_CLIENTS];	// MQTT CA certificate
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
typedef uint8_t revk_setting_bits_t[26];
typedef uint8_t revk_setting_group_t[3];
extern const char revk_settings_secret[];
