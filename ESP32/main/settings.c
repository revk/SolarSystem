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
#include "sdkconfig.h"
#include "settings.h"
revk_settings_bits_t revk_settings_bits={0};
#define	str(s)	#s
#define	quote(s)	str(s)
revk_settings_t const revk_settings[]={
#ifdef	CONFIG_REVK_SETTINGS_PASSWORD
 {.type=REVK_SETTINGS_STRING,.name="password",.comment="Settings password<br>(not sent securely so use with care)",.len=8,.ptr=&password,.malloc=1,.revk=1,.hide=1,.secret=1},
#endif
#ifdef  CONFIG_MDNS_MAX_INTERFACES
 {.type=REVK_SETTINGS_STRING,.name="hostname",.comment="Hostname[.local]<br>(used in DHCP and MQTT)",.len=8,.ptr=&hostname,.malloc=1,.revk=1,.hide=1},
#else
 {.type=REVK_SETTINGS_STRING,.name="hostname",.comment="Host name<br>(used in DHCP and MQTT)",.len=8,.ptr=&hostname,.malloc=1,.revk=1,.hide=1},
#endif
 {.type=REVK_SETTINGS_STRING,.name="appname",.comment="Application name",.len=7,.dq=1,.def=quote(CONFIG_REVK_APPNAME),.ptr=&appname,.malloc=1,.revk=1,.hide=1},
 {.type=REVK_SETTINGS_STRING,.name="otahost",.comment="OTA hostname",.group=1,.len=7,.dot=3,.dq=1,.def=quote(CONFIG_REVK_OTAHOST),.ptr=&otahost,.malloc=1,.revk=1,.live=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="otadays",.comment="OTA auto load (days)",.group=1,.len=7,.dot=3,.dq=1,.def=quote(CONFIG_REVK_OTADAYS),.ptr=&otadays,.size=sizeof(uint8_t),.revk=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="otastart",.comment="OTA check after startup (min seconds)",.group=1,.len=8,.dot=3,.def="600",.ptr=&otastart,.size=sizeof(uint16_t),.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="otaauto",.comment="OTA auto upgrade",.group=1,.len=7,.dot=3,.def="1",.bit=REVK_SETTINGS_BITFIELD_otaauto,.revk=1,.hide=1,.live=1},
#ifdef	CONFIG_REVK_WEB_BETA
 {.type=REVK_SETTINGS_BIT,.name="otabeta",.comment="OTA from beta release",.group=1,.len=7,.dot=3,.bit=REVK_SETTINGS_BITFIELD_otabeta,.revk=1,.hide=1,.live=1},
#endif
 {.type=REVK_SETTINGS_BLOB,.name="otacert",.comment="OTA cert of otahost",.group=1,.len=7,.dot=3,.dq=1,.def=quote(CONFIG_REVK_OTACERT),.ptr=&otacert,.malloc=1,.revk=1,.base64=1},
 {.type=REVK_SETTINGS_BIT,.name="dark",.comment="Default LED off",.len=4,.bit=REVK_SETTINGS_BITFIELD_dark,.revk=1,.live=1},
 {.type=REVK_SETTINGS_STRING,.name="ntphost",.comment="NTP host",.len=7,.dq=1,.def=quote(CONFIG_REVK_NTPHOST),.ptr=&ntphost,.malloc=1,.revk=1},
 {.type=REVK_SETTINGS_STRING,.name="tz",.comment="Timezone (<a href='https://gist.github.com/alwynallan/24d96091655391107939' target=_blank>info</a>)",.len=2,.dq=1,.def=quote(CONFIG_REVK_TZ),.ptr=&tz,.malloc=1,.revk=1,.hide=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="watchdogtime",.comment="Watchdog (seconds)",.len=12,.dq=1,.def=quote(CONFIG_REVK_WATCHDOG),.ptr=&watchdogtime,.size=sizeof(uint32_t),.revk=1},
#ifdef  CONFIG_IDF_TARGET_ESP32S3
 {.type=REVK_SETTINGS_UNSIGNED,.name="usbuptime",.comment="USB  turns off after this many seconds",.len=9,.dq=1,.def=quote(CONFIG_REVK_USBUPTIME),.ptr=&usbuptime,.size=sizeof(uint16_t),.revk=1},
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="factorygpio",.comment="Factory reset GPIO (press 3 times)",.len=11,.dq=1,.def=quote(CONFIG_REVK_RESET),.ptr=&factorygpio,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.revk=1},
#else
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="factorygpio",.comment="Factory reset GPIO (press 3 times)",.len=11,.ptr=&factorygpio,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.revk=1},
#endif
 {.type=REVK_SETTINGS_STRING,.name="topicgroup",.comment="MQTT Alternative hostname accepted for commands",.group=2,.len=10,.dot=5,.ptr=&topicgroup,.malloc=1,.revk=1,.array=2},
 {.type=REVK_SETTINGS_STRING,.name="topiccommand",.comment="MQTT Topic for commands",.group=2,.len=12,.dot=5,.def="command",.ptr=&topiccommand,.malloc=1,.revk=1,.old="prefixcommand"},
 {.type=REVK_SETTINGS_STRING,.name="topicsetting",.comment="MQTT Topic for settings",.group=2,.len=12,.dot=5,.def="setting",.ptr=&topicsetting,.malloc=1,.revk=1,.old="prefixsetting"},
 {.type=REVK_SETTINGS_STRING,.name="topicstate",.comment="MQTT Topic for state",.group=2,.len=10,.dot=5,.def="state",.ptr=&topicstate,.malloc=1,.revk=1,.old="prefixstate"},
 {.type=REVK_SETTINGS_STRING,.name="topicevent",.comment="MQTT Topic for event",.group=2,.len=10,.dot=5,.def="event",.ptr=&topicevent,.malloc=1,.revk=1,.old="prefixevent"},
 {.type=REVK_SETTINGS_STRING,.name="topicinfo",.comment="MQTT Topic for info",.group=2,.len=9,.dot=5,.def="info",.ptr=&topicinfo,.malloc=1,.revk=1,.old="prefixinfo"},
 {.type=REVK_SETTINGS_STRING,.name="topicerror",.comment="MQTT Topic for error",.group=2,.len=10,.dot=5,.def="error",.ptr=&topicerror,.malloc=1,.revk=1,.old="prefixerror"},
 {.type=REVK_SETTINGS_STRING,.name="topicha",.comment="MQTT Topic for homeassistant",.group=2,.len=7,.dot=5,.def="homeassistant",.ptr=&topicha,.malloc=1,.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="prefixapp",.comment="MQTT use appname/ in front of hostname in topic",.group=3,.len=9,.dot=6,.dq=1,.def=quote(CONFIG_REVK_PREFIXAPP),.bit=REVK_SETTINGS_BITFIELD_prefixapp,.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="prefixhost",.comment="MQTT use (appname/)hostname/topic instead of topic/(appname/)hostname",.group=3,.len=10,.dot=6,.dq=1,.def=quote(CONFIG_REVK_PREFIXHOST),.bit=REVK_SETTINGS_BITFIELD_prefixhost,.revk=1},
#ifdef	CONFIG_REVK_BLINK_DEF
#ifdef	CONFIG_REVK_BLINK_WS2812_DEF
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="blink",.comment="WS2812 LED",.len=5,.dq=1,.def=quote(CONFIG_REVK_BLINK),.ptr=&blink,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.revk=1},
#else
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="blink",.comment="R, G, B LED array (set all the same for WS2812 LED)",.len=5,.dq=1,.def=quote(CONFIG_REVK_BLINK),.ptr=&blink,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.revk=1,.array=3},
#endif
#endif
#ifdef  CONFIG_REVK_APMODE
#ifdef	CONFIG_REVK_APCONFIG
 {.type=REVK_SETTINGS_UNSIGNED,.name="apport",.comment="TCP port for config web pages on AP",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APPORT),.ptr=&apport,.size=sizeof(uint16_t),.revk=1},
#endif
 {.type=REVK_SETTINGS_UNSIGNED,.name="aptime",.comment="Limit AP to time (seconds)",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APTIME),.ptr=&aptime,.size=sizeof(uint32_t),.revk=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="apwait",.comment="Wait off line before starting AP (seconds)",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APWAIT),.ptr=&apwait,.size=sizeof(uint32_t),.revk=1},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="apgpio",.comment="Start AP on GPIO",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APGPIO),.ptr=&apgpio,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.revk=1},
#endif
#ifdef  CONFIG_REVK_MQTT
 {.type=REVK_SETTINGS_STRING,.name="mqtthost",.comment="MQTT hostname",.group=5,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MQTTHOST),.ptr=&mqtthost,.malloc=1,.revk=1,.array=CONFIG_REVK_MQTT_CLIENTS,.hide=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="mqttport",.comment="MQTT port",.group=5,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MQTTPORT),.ptr=&mqttport,.size=sizeof(uint16_t),.revk=1,.array=CONFIG_REVK_MQTT_CLIENTS},
 {.type=REVK_SETTINGS_STRING,.name="mqttuser",.comment="MQTT username",.group=5,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MQTTUSER),.ptr=&mqttuser,.malloc=1,.revk=1,.array=CONFIG_REVK_MQTT_CLIENTS,.hide=1},
 {.type=REVK_SETTINGS_STRING,.name="mqttpass",.comment="MQTT password",.group=5,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MQTTPASS),.ptr=&mqttpass,.malloc=1,.revk=1,.array=CONFIG_REVK_MQTT_CLIENTS,.secret=1,.hide=1},
 {.type=REVK_SETTINGS_BLOB,.name="mqttcert",.comment="MQTT CA certificate (for mqtts)",.group=5,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MQTTCERT),.ptr=&mqttcert,.malloc=1,.revk=1,.array=CONFIG_REVK_MQTT_CLIENTS,.base64=1},
#endif
 {.type=REVK_SETTINGS_BLOB,.name="clientkey",.comment="Client Key (OTA and MQTT TLS)",.group=6,.len=9,.dot=6,.ptr=&clientkey,.malloc=1,.revk=1,.base64=1},
 {.type=REVK_SETTINGS_BLOB,.name="clientcert",.comment="Client certificate (OTA and MQTT TLS)",.group=6,.len=10,.dot=6,.ptr=&clientcert,.malloc=1,.revk=1,.base64=1},
#if     defined(CONFIG_REVK_WIFI) || defined(CONFIG_REVK_MESH)
 {.type=REVK_SETTINGS_UNSIGNED,.name="wifireset",.comment="Restart if WiFi off for this long (seconds)",.group=7,.len=9,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIRESET),.ptr=&wifireset,.size=sizeof(uint16_t),.revk=1},
 {.type=REVK_SETTINGS_STRING,.name="wifissid",.comment="WiFI SSID (name)",.group=7,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFISSID),.ptr=&wifissid,.malloc=1,.revk=1,.hide=1},
 {.type=REVK_SETTINGS_STRING,.name="wifipass",.comment="WiFi password",.group=7,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIPASS),.ptr=&wifipass,.malloc=1,.revk=1,.hide=1,.secret=1},
 {.type=REVK_SETTINGS_STRING,.name="wifiip",.comment="WiFi Fixed IP",.group=7,.len=6,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIIP),.ptr=&wifiip,.malloc=1,.revk=1},
 {.type=REVK_SETTINGS_STRING,.name="wifigw",.comment="WiFi Fixed gateway",.group=7,.len=6,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIGW),.ptr=&wifigw,.malloc=1,.revk=1},
 {.type=REVK_SETTINGS_STRING,.name="wifidns",.comment="WiFi fixed DNS",.group=7,.len=7,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIDNS),.ptr=&wifidns,.malloc=1,.revk=1,.array=3},
 {.type=REVK_SETTINGS_OCTET,.name="wifibssid",.comment="WiFI BSSID",.group=7,.len=9,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIBSSID),.ptr=&wifibssid,.size=sizeof(uint8_t[6]),.revk=1,.hex=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="wifichan",.comment="WiFI channel",.group=7,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFICHAN),.ptr=&wifichan,.size=sizeof(uint8_t),.revk=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="wifiuptime",.comment="WiFI turns off after this many seconds",.group=7,.len=10,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIUPTIME),.ptr=&wifiuptime,.size=sizeof(uint16_t),.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="wifips",.comment="WiFi power save",.group=7,.len=6,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIPS),.bit=REVK_SETTINGS_BITFIELD_wifips,.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="wifimaxps",.comment="WiFi power save (max)",.group=7,.len=9,.dot=4,.dq=1,.def=quote(CONFIG_REVK_WIFIMAXPS),.bit=REVK_SETTINGS_BITFIELD_wifimaxps,.revk=1},
#endif
#ifndef	CONFIG_REVK_MESH
 {.type=REVK_SETTINGS_STRING,.name="apssid",.comment="AP mode SSID (name)",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APSSID),.ptr=&apssid,.malloc=1,.revk=1},
 {.type=REVK_SETTINGS_STRING,.name="appass",.comment="AP mode password",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APPASS),.ptr=&appass,.malloc=1,.revk=1,.secret=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="apmax",.comment="AP max clients",.group=4,.len=5,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APMAX),.ptr=&apmax,.size=sizeof(uint8_t),.revk=1,.hide=1},
 {.type=REVK_SETTINGS_STRING,.name="apip",.comment="AP mode block",.group=4,.len=4,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APIP),.ptr=&apip,.malloc=1,.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="aplr",.comment="AP LR mode",.group=4,.len=4,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APLR),.bit=REVK_SETTINGS_BITFIELD_aplr,.revk=1},
 {.type=REVK_SETTINGS_BIT,.name="aphide",.comment="AP hide SSID",.group=4,.len=6,.dot=2,.dq=1,.def=quote(CONFIG_REVK_APHIDE),.bit=REVK_SETTINGS_BITFIELD_aphide,.revk=1},
#endif
#ifdef	CONFIG_REVK_MESH
 {.type=REVK_SETTINGS_STRING,.name="nodename",.comment="Mesh node name",.len=8,.ptr=&nodename,.malloc=1,.revk=1,.hide=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshreset",.comment="Reset if mesh off for this long (seconds)",.group=8,.len=9,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHRESET),.ptr=&meshreset,.size=sizeof(uint16_t),.revk=1,.hide=1},
 {.type=REVK_SETTINGS_OCTET,.name="meshid",.comment="Mesh ID (hex)",.group=8,.len=6,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHID),.ptr=&meshid,.size=sizeof(uint8_t[6]),.revk=1,.hex=1,.hide=1},
 {.type=REVK_SETTINGS_OCTET,.name="meshkey",.comment="Mesh key",.group=8,.len=7,.dot=4,.ptr=&meshkey,.size=sizeof(uint8_t[16]),.revk=1,.secret=1,.hex=1,.hide=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshwidth",.comment="Mesh width",.group=8,.len=9,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHWIDTH),.ptr=&meshwidth,.size=sizeof(uint16_t),.revk=1,.hide=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshdepth",.comment="Mesh depth",.group=8,.len=9,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHDEPTH),.ptr=&meshdepth,.size=sizeof(uint16_t),.revk=1,.hide=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshmax",.comment="Mesh max devices",.group=8,.len=7,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHMAX),.ptr=&meshmax,.size=sizeof(uint16_t),.revk=1,.hide=1},
 {.type=REVK_SETTINGS_STRING,.name="meshpass",.comment="Mesh AP password",.group=8,.len=8,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHPASS),.ptr=&meshpass,.malloc=1,.revk=1,.secret=1,.hide=1},
 {.type=REVK_SETTINGS_BIT,.name="meshlr",.comment="Mesh use LR mode",.group=8,.len=6,.dot=4,.dq=1,.def=quote(CONFIG_REVK_MESHLR),.bit=REVK_SETTINGS_BITFIELD_meshlr,.revk=1,.hide=1},
 {.type=REVK_SETTINGS_BIT,.name="meshroot",.comment="This is preferred mesh root",.group=8,.len=8,.dot=4,.bit=REVK_SETTINGS_BITFIELD_meshroot,.revk=1,.hide=1},
#endif
#define	AREAS	"ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ"
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="tamper",.len=6,.ptr=&tamper,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_BIT,.name="debug",.len=5,.bit=REVK_SETTINGS_BITFIELD_debug,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="iotstatedoor",.group=9,.len=12,.dot=3,.bit=REVK_SETTINGS_BITFIELD_iotstatedoor,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="iotstateinput",.group=9,.len=13,.dot=3,.bit=REVK_SETTINGS_BITFIELD_iotstateinput,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="iotstateoutput",.group=9,.len=14,.dot=3,.bit=REVK_SETTINGS_BITFIELD_iotstateoutput,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="iotstatesystem",.group=9,.len=14,.dot=3,.bit=REVK_SETTINGS_BITFIELD_iotstatesystem,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="ioteventfob",.group=9,.len=11,.dot=3,.bit=REVK_SETTINGS_BITFIELD_ioteventfob,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="ioteventarm",.group=9,.len=11,.dot=3,.bit=REVK_SETTINGS_BITFIELD_ioteventarm,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="iotkeypad",.group=9,.len=9,.dot=3,.bit=REVK_SETTINGS_BITFIELD_iotkeypad,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="iotgps",.group=9,.len=6,.dot=3,.bit=REVK_SETTINGS_BITFIELD_iotgps,.live=1},
 {.type=REVK_SETTINGS_STRING,.name="iottopic",.group=9,.len=8,.dot=3,.ptr=&iottopic,.malloc=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcred",.group=10,.len=6,.dot=3,.ptr=&nfcred,.size=sizeof(uint8_t),.flags="-"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcamber",.group=10,.len=8,.dot=3,.ptr=&nfcamber,.size=sizeof(uint8_t),.flags="-"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcgreen",.group=10,.len=8,.dot=3,.ptr=&nfcgreen,.size=sizeof(uint8_t),.flags="-"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfccard",.group=10,.len=7,.dot=3,.ptr=&nfccard,.size=sizeof(uint8_t),.flags="-"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcidle",.group=10,.len=7,.dot=3,.ptr=&nfcidle,.size=sizeof(uint8_t),.live=1},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="nfctx",.group=10,.len=5,.dot=3,.ptr=&nfctx,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="nfcrx",.group=10,.len=5,.dot=3,.ptr=&nfcrx,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="nfcpower",.group=10,.len=8,.dot=3,.ptr=&nfcpower,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcpoll",.group=10,.len=7,.dot=3,.def="50",.ptr=&nfcpoll,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcholdpoll",.group=10,.len=11,.dot=3,.def="500",.ptr=&nfcholdpoll,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfchold",.group=10,.len=7,.dot=3,.def="6",.ptr=&nfchold,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfclonghold",.group=10,.len=11,.dot=3,.def="20",.ptr=&nfclonghold,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcledpoll",.group=10,.len=10,.dot=3,.def="100",.ptr=&nfcledpoll,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfciopoll",.group=10,.len=9,.dot=3,.def="200",.ptr=&nfciopoll,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcuart",.group=10,.len=7,.dot=3,.def="1",.ptr=&nfcuart,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="nfcbaud",.group=10,.len=7,.dot=3,.dq=1,.def=quote(CONFIG_NFC_BAUD_CODE),.ptr=&nfcbaud,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_STRING,.name="nfcmqttbell",.group=10,.len=11,.dot=3,.ptr=&nfcmqttbell,.malloc=1},
 {.type=REVK_SETTINGS_OCTET,.name="aes",.len=3,.ptr=&aes,.size=sizeof(uint8_t[18]),.hex=1,.array=3,.secret=1},
 {.type=REVK_SETTINGS_OCTET,.name="aid",.len=3,.ptr=&aid,.size=sizeof(uint8_t[3]),.hex=1},
 {.type=REVK_SETTINGS_STRING,.name="ledIDLE",.group=11,.len=7,.dot=3,.def="3R3-",.ptr=&ledIDLE,.malloc=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorauto",.group=12,.len=8,.dot=4,.ptr=&doorauto,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorunlock",.group=12,.len=10,.dot=4,.def="1000",.ptr=&doorunlock,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorlock",.group=12,.len=8,.dot=4,.def="3000",.ptr=&doorlock,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="dooropen",.group=12,.len=8,.dot=4,.def="5000",.ptr=&dooropen,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorclose",.group=12,.len=9,.dot=4,.def="500",.ptr=&doorclose,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorprop",.group=12,.len=8,.dot=4,.def="60000",.ptr=&doorprop,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorexit",.group=12,.len=8,.dot=4,.def="3000",.ptr=&doorexit,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doorpoll",.group=12,.len=8,.dot=4,.def="100",.ptr=&doorpoll,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="doordebounce",.group=12,.len=12,.dot=4,.def="200",.ptr=&doordebounce,.size=sizeof(uint32_t)},
 {.type=REVK_SETTINGS_BIT,.name="doordebug",.group=12,.len=9,.dot=4,.bit=REVK_SETTINGS_BITFIELD_doordebug},
 {.type=REVK_SETTINGS_BIT,.name="doorexitarm",.group=12,.len=11,.dot=4,.bit=REVK_SETTINGS_BITFIELD_doorexitarm},
 {.type=REVK_SETTINGS_BIT,.name="doorexitdisarm",.group=12,.len=14,.dot=4,.bit=REVK_SETTINGS_BITFIELD_doorexitdisarm},
 {.type=REVK_SETTINGS_BIT,.name="doorcatch",.group=12,.len=9,.dot=4,.bit=REVK_SETTINGS_BITFIELD_doorcatch},
 {.type=REVK_SETTINGS_STRING,.name="fallback",.len=8,.ptr=&fallback,.malloc=1,.array=10},
 {.type=REVK_SETTINGS_STRING,.name="blacklist",.len=9,.ptr=&blacklist,.malloc=1,.array=10},
 {.type=REVK_SETTINGS_STRING,.name="dooriotopen",.group=12,.len=11,.dot=4,.ptr=&dooriotopen,.malloc=1},
 {.type=REVK_SETTINGS_STRING,.name="dooriotdead",.group=12,.len=11,.dot=4,.ptr=&dooriotdead,.malloc=1},
 {.type=REVK_SETTINGS_STRING,.name="dooriotundead",.group=12,.len=13,.dot=4,.ptr=&dooriotundead,.malloc=1},
 {.type=REVK_SETTINGS_STRING,.name="dooriotlock",.group=12,.len=11,.dot=4,.ptr=&dooriotlock,.malloc=1},
 {.type=REVK_SETTINGS_STRING,.name="dooriotunlock",.group=12,.len=13,.dot=4,.ptr=&dooriotunlock,.malloc=1},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="gpstx",.group=13,.len=5,.dot=3,.ptr=&gpstx,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="gpsrx",.group=13,.len=5,.dot=3,.ptr=&gpsrx,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="gpstick",.group=13,.len=7,.dot=3,.ptr=&gpstick,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="gpsuart",.group=13,.len=7,.dot=3,.def="2",.ptr=&gpsuart,.size=sizeof(uint8_t)},
#define	MAXOUTPUT	10
#define	OUTPUT_FUNCS	"LDBE----"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release (this needs to be L>>1)
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="blink",.len=5,.ptr=&blink,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.array=3},
 {.type=REVK_SETTINGS_UNSIGNED,.name="rgbs",.len=4,.ptr=&rgbs,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="outgpio",.group=14,.len=7,.dot=3,.ptr=&outgpio,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.array=MAXOUTPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outfunc",.group=14,.len=7,.dot=3,.ptr=&outfunc,.size=sizeof(uint8_t),.array=MAXOUTPUT,.flags=OUTPUT_FUNCS},
 {.type=REVK_SETTINGS_SIGNED,.name="outpulse",.group=14,.len=8,.dot=3,.ptr=&outpulse,.size=sizeof(int16_t),.array=MAXOUTPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outrgb",.group=14,.len=6,.dot=3,.ptr=&outrgb,.size=sizeof(uint8_t),.array=MAXOUTPUT},
 {.type=REVK_SETTINGS_STRING,.name="outname",.group=14,.len=7,.dot=3,.ptr=&outname,.malloc=1,.array=MAXOUTPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outengineer",.group=14,.len=11,.dot=3,.ptr=&outengineer,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outfaulted",.group=14,.len=10,.dot=3,.ptr=&outfaulted,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outtampered",.group=14,.len=11,.dot=3,.ptr=&outtampered,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outalarmed",.group=14,.len=10,.dot=3,.ptr=&outalarmed,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outarmed",.group=14,.len=8,.dot=3,.ptr=&outarmed,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outprearm",.group=14,.len=9,.dot=3,.ptr=&outprearm,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outprealarm",.group=14,.len=11,.dot=3,.ptr=&outprealarm,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outalarm",.group=14,.len=8,.dot=3,.ptr=&outalarm,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outdoorbell",.group=14,.len=11,.dot=3,.ptr=&outdoorbell,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outwarning",.group=14,.len=10,.dot=3,.ptr=&outwarning,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outfault",.group=14,.len=8,.dot=3,.ptr=&outfault,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outaccess",.group=14,.len=9,.dot=3,.ptr=&outaccess,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outpresence",.group=14,.len=11,.dot=3,.ptr=&outpresence,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outtamper",.group=14,.len=9,.dot=3,.ptr=&outtamper,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outfire",.group=14,.len=7,.dot=3,.ptr=&outfire,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="outpanic",.group=14,.len=8,.dot=3,.ptr=&outpanic,.size=sizeof(uint32_t),.array=MAXOUTPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="powergpio",.group=15,.len=9,.dot=5,.ptr=&powergpio,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.array=MAXOUTPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="powerrgb",.group=15,.len=8,.dot=5,.ptr=&powerrgb,.size=sizeof(uint8_t),.array=MAXOUTPUT},
#define	MAXINPUT	16
#define	INPUT_FUNCS	"EOLDMP--"
#define	INPUT_FUNC_E	0x80		// Exit pressed
#define	INPUT_FUNC_O	0x40		// Open door
#define	INPUT_FUNC_L	0x20		// Lock unlocked
#define	INPUT_FUNC_D	0x10		// Deadlock unlocked (This needs to be L>>1)
#define	INPUT_FUNC_M	0x08		// Manual active
#define	INPUT_FUNC_P	0x04		// Pull down (not really a function, but a rarely needed input flag)
 {.type=REVK_SETTINGS_UNSIGNED,.name="inpoll",.group=16,.len=6,.dot=2,.def="10",.ptr=&inpoll,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="ingpio",.group=16,.len=6,.dot=2,.ptr=&ingpio,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕",.array=MAXINPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inrgb",.group=16,.len=5,.dot=2,.ptr=&inrgb,.size=sizeof(uint8_t),.array=MAXINPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="infunc",.group=16,.len=6,.dot=2,.ptr=&infunc,.size=sizeof(uint8_t),.array=MAXINPUT,.flags=INPUT_FUNCS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inhold",.group=16,.len=6,.dot=2,.ptr=&inhold,.size=sizeof(uint8_t),.array=MAXINPUT},
 {.type=REVK_SETTINGS_STRING,.name="inname",.group=16,.len=6,.dot=2,.ptr=&inname,.malloc=1,.array=MAXINPUT},
 {.type=REVK_SETTINGS_UNSIGNED,.name="indoorbell",.group=16,.len=10,.dot=2,.ptr=&indoorbell,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inwarning",.group=16,.len=9,.dot=2,.ptr=&inwarning,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="infault",.group=16,.len=7,.dot=2,.ptr=&infault,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inaccess",.group=16,.len=8,.dot=2,.ptr=&inaccess,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inpresence",.group=16,.len=10,.dot=2,.ptr=&inpresence,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="intamper",.group=16,.len=8,.dot=2,.ptr=&intamper,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="infire",.group=16,.len=6,.dot=2,.ptr=&infire,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inpanic",.group=16,.len=7,.dot=2,.ptr=&inpanic,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="inarm",.group=16,.len=5,.dot=2,.ptr=&inarm,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="instrong",.group=16,.len=8,.dot=2,.ptr=&instrong,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="indisarm",.group=16,.len=8,.dot=2,.ptr=&indisarm,.size=sizeof(uint32_t),.array=MAXINPUT,.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="keypadtx",.group=17,.len=8,.dot=6,.ptr=&keypadtx,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="keypadrx",.group=17,.len=8,.dot=6,.ptr=&keypadrx,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="keypadde",.group=17,.len=8,.dot=6,.ptr=&keypadde,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="keypadre",.group=17,.len=8,.dot=6,.ptr=&keypadre,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.gpio=1,.name="keypadclk",.group=17,.len=9,.dot=6,.ptr=&keypadclk,.size=sizeof(revk_gpio_t),.fix=1,.set=1,.flags="- ~↓↕⇕"},
 {.type=REVK_SETTINGS_UNSIGNED,.name="keypadtimer",.group=17,.len=11,.dot=6,.ptr=&keypadtimer,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="keypadaddress",.group=17,.len=13,.dot=6,.def="10",.ptr=&keypadaddress,.size=sizeof(uint8_t),.hex=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="keypadtxpre",.group=17,.len=11,.dot=6,.def="75",.ptr=&keypadtxpre,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="keypadtxpost",.group=17,.len=12,.dot=6,.def="50",.ptr=&keypadtxpost,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="keypadrxpre",.group=17,.len=11,.dot=6,.def="100",.ptr=&keypadrxpre,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="keypadrxpost",.group=17,.len=12,.dot=6,.def="20",.ptr=&keypadrxpost,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_STRING,.name="keypadidle",.group=17,.len=10,.dot=6,.ptr=&keypadidle,.malloc=1,.live=1},
 {.type=REVK_SETTINGS_STRING,.name="keypadpin",.group=17,.len=9,.dot=6,.ptr=&keypadpin,.malloc=1,.live=1},
 {.type=REVK_SETTINGS_BIT,.name="keypadpinarm",.group=17,.len=12,.dot=6,.bit=REVK_SETTINGS_BITFIELD_keypadpinarm,.live=1},
#define	MAX_MIX		5
 {.type=REVK_SETTINGS_UNSIGNED,.name="arealed",.group=18,.len=7,.dot=4,.ptr=&arealed,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areaenter",.group=18,.len=9,.dot=4,.ptr=&areaenter,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areaarm",.group=18,.len=7,.dot=4,.ptr=&areaarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areastrong",.group=18,.len=10,.dot=4,.ptr=&areastrong,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areadisarm",.group=18,.len=10,.dot=4,.ptr=&areadisarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areadeadlock",.group=18,.len=12,.dot=4,.ptr=&areadeadlock,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areakeypad",.group=18,.len=10,.dot=4,.ptr=&areakeypad,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areakeydisarm",.group=18,.len=13,.dot=4,.ptr=&areakeydisarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areakeystrong",.group=18,.len=13,.dot=4,.ptr=&areakeystrong,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="areakeyarm",.group=18,.len=10,.dot=4,.ptr=&areakeyarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="engineer",.len=8,.ptr=&engineer,.size=sizeof(uint32_t),.flags=AREAS,.live=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="armed",.len=5,.ptr=&armed,.size=sizeof(uint32_t),.flags=AREAS,.live=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smsarm",.group=19,.len=6,.dot=3,.ptr=&smsarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smsdisarm",.group=19,.len=9,.dot=3,.ptr=&smsdisarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smscancel",.group=19,.len=9,.dot=3,.ptr=&smscancel,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smsarmfail",.group=19,.len=10,.dot=3,.ptr=&smsarmfail,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smsalarm",.group=19,.len=8,.dot=3,.ptr=&smsalarm,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smspanic",.group=19,.len=8,.dot=3,.ptr=&smspanic,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="smsfire",.group=19,.len=7,.dot=3,.ptr=&smsfire,.size=sizeof(uint32_t),.flags=AREAS},
 {.type=REVK_SETTINGS_UNSIGNED,.name="mixand",.group=20,.len=6,.dot=3,.ptr=&mixand,.size=sizeof(uint32_t),.flags=AREAS,.array=MAX_MIX},
 {.type=REVK_SETTINGS_UNSIGNED,.name="mixset",.group=20,.len=6,.dot=3,.ptr=&mixset,.size=sizeof(uint32_t),.flags=AREAS,.array=MAX_MIX},
 {.type=REVK_SETTINGS_UNSIGNED,.name="armcancel",.group=21,.len=9,.dot=3,.ptr=&armcancel,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="armdelay",.group=21,.len=8,.dot=3,.ptr=&armdelay,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="alarmdelay",.group=22,.len=10,.dot=5,.ptr=&alarmdelay,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="alarmhold",.group=22,.len=9,.dot=5,.ptr=&alarmhold,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshexpect",.group=8,.len=10,.dot=4,.ptr=&meshexpect,.size=sizeof(uint8_t),.live=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshcycle",.group=8,.len=9,.dot=4,.def="3",.ptr=&meshcycle,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshwarmup",.group=8,.len=10,.dot=4,.def="60",.ptr=&meshwarmup,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshflap",.group=8,.len=8,.dot=4,.def="10",.ptr=&meshflap,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="meshdied",.group=8,.len=8,.dot=4,.def="240",.ptr=&meshdied,.size=sizeof(uint8_t)},
 {.type=REVK_SETTINGS_UNSIGNED,.name="mqttdied",.group=5,.len=8,.dot=4,.def="600",.ptr=&mqttdied,.size=sizeof(uint16_t)},
 {.type=REVK_SETTINGS_STRING,.name="smsnumber",.group=19,.len=9,.dot=3,.ptr=&smsnumber,.malloc=1},
 {.type=REVK_SETTINGS_UNSIGNED,.name="timer1",.len=6,.ptr=&timer1,.size=sizeof(uint16_t)},
{0}};
#undef quote
#undef str
#ifdef	CONFIG_REVK_SETTINGS_PASSWORD
char* password=NULL;
#endif
#ifdef  CONFIG_MDNS_MAX_INTERFACES
char* hostname=NULL;
#else
char* hostname=NULL;
#endif
char* appname=NULL;
char* otahost=NULL;
uint8_t otadays=0;
uint16_t otastart=0;
#ifdef	CONFIG_REVK_WEB_BETA
#endif
revk_settings_blob_t* otacert=NULL;
char* ntphost=NULL;
char* tz=NULL;
uint32_t watchdogtime=0;
#ifdef  CONFIG_IDF_TARGET_ESP32S3
uint16_t usbuptime=0;
#endif
#ifdef  CONFIG_IDF_TARGET_ESP32S3
revk_gpio_t factorygpio={0};
#else
revk_gpio_t factorygpio={0};
#endif
char* topicgroup[2]={0};
char* topiccommand=NULL;
char* topicsetting=NULL;
char* topicstate=NULL;
char* topicevent=NULL;
char* topicinfo=NULL;
char* topicerror=NULL;
char* topicha=NULL;
#ifdef	CONFIG_REVK_BLINK_DEF
#ifdef	CONFIG_REVK_BLINK_WS2812_DEF
revk_gpio_t blink={0};
#else
revk_gpio_t blink[3]={0};
#endif
#endif
#ifdef  CONFIG_REVK_APMODE
#ifdef	CONFIG_REVK_APCONFIG
uint16_t apport=0;
#endif
uint32_t aptime=0;
uint32_t apwait=0;
revk_gpio_t apgpio={0};
#endif
#ifdef  CONFIG_REVK_MQTT
char* mqtthost[CONFIG_REVK_MQTT_CLIENTS]={0};
uint16_t mqttport[CONFIG_REVK_MQTT_CLIENTS]={0};
char* mqttuser[CONFIG_REVK_MQTT_CLIENTS]={0};
char* mqttpass[CONFIG_REVK_MQTT_CLIENTS]={0};
revk_settings_blob_t* mqttcert[CONFIG_REVK_MQTT_CLIENTS]={0};
#endif
revk_settings_blob_t* clientkey=NULL;
revk_settings_blob_t* clientcert=NULL;
#if     defined(CONFIG_REVK_WIFI) || defined(CONFIG_REVK_MESH)
uint16_t wifireset=0;
char* wifissid=NULL;
char* wifipass=NULL;
char* wifiip=NULL;
char* wifigw=NULL;
char* wifidns[3]={0};
uint8_t wifibssid[6]={0};
uint8_t wifichan=0;
uint16_t wifiuptime=0;
#endif
#ifndef	CONFIG_REVK_MESH
char* apssid=NULL;
char* appass=NULL;
uint8_t apmax=0;
char* apip=NULL;
#endif
#ifdef	CONFIG_REVK_MESH
char* nodename=NULL;
uint16_t meshreset=0;
uint8_t meshid[6]={0};
uint8_t meshkey[16]={0};
uint16_t meshwidth=0;
uint16_t meshdepth=0;
uint16_t meshmax=0;
char* meshpass=NULL;
#endif
#define	AREAS	"ABCDEFGHIJKLMNOPQRSTUVWXYZZZZZZZ"
revk_gpio_t tamper={0};
char* iottopic=NULL;
uint8_t nfcred=0;
uint8_t nfcamber=0;
uint8_t nfcgreen=0;
uint8_t nfccard=0;
uint8_t nfcidle=0;
revk_gpio_t nfctx={0};
revk_gpio_t nfcrx={0};
revk_gpio_t nfcpower={0};
uint16_t nfcpoll=0;
uint16_t nfcholdpoll=0;
uint8_t nfchold=0;
uint8_t nfclonghold=0;
uint16_t nfcledpoll=0;
uint16_t nfciopoll=0;
uint8_t nfcuart=0;
uint8_t nfcbaud=0;
char* nfcmqttbell=NULL;
uint8_t aes[3][18]={0};
uint8_t aid[3]={0};
char* ledIDLE=NULL;
uint8_t doorauto=0;
uint32_t doorunlock=0;
uint32_t doorlock=0;
uint32_t dooropen=0;
uint32_t doorclose=0;
uint32_t doorprop=0;
uint32_t doorexit=0;
uint32_t doorpoll=0;
uint32_t doordebounce=0;
char* fallback[10]={0};
char* blacklist[10]={0};
char* dooriotopen=NULL;
char* dooriotdead=NULL;
char* dooriotundead=NULL;
char* dooriotlock=NULL;
char* dooriotunlock=NULL;
revk_gpio_t gpstx={0};
revk_gpio_t gpsrx={0};
revk_gpio_t gpstick={0};
uint8_t gpsuart=0;
#define	MAXOUTPUT	10
#define	OUTPUT_FUNCS	"LDBE----"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release (this needs to be L>>1)
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error
revk_gpio_t blink[3]={0};
uint8_t rgbs=0;
revk_gpio_t outgpio[MAXOUTPUT]={0};
uint8_t outfunc[MAXOUTPUT]={0};
int16_t outpulse[MAXOUTPUT]={0};
uint8_t outrgb[MAXOUTPUT]={0};
char* outname[MAXOUTPUT]={0};
uint32_t outengineer[MAXOUTPUT]={0};
uint32_t outfaulted[MAXOUTPUT]={0};
uint32_t outtampered[MAXOUTPUT]={0};
uint32_t outalarmed[MAXOUTPUT]={0};
uint32_t outarmed[MAXOUTPUT]={0};
uint32_t outprearm[MAXOUTPUT]={0};
uint32_t outprealarm[MAXOUTPUT]={0};
uint32_t outalarm[MAXOUTPUT]={0};
uint32_t outdoorbell[MAXOUTPUT]={0};
uint32_t outwarning[MAXOUTPUT]={0};
uint32_t outfault[MAXOUTPUT]={0};
uint32_t outaccess[MAXOUTPUT]={0};
uint32_t outpresence[MAXOUTPUT]={0};
uint32_t outtamper[MAXOUTPUT]={0};
uint32_t outfire[MAXOUTPUT]={0};
uint32_t outpanic[MAXOUTPUT]={0};
revk_gpio_t powergpio[MAXOUTPUT]={0};
uint8_t powerrgb[MAXOUTPUT]={0};
#define	MAXINPUT	16
#define	INPUT_FUNCS	"EOLDMP--"
#define	INPUT_FUNC_E	0x80		// Exit pressed
#define	INPUT_FUNC_O	0x40		// Open door
#define	INPUT_FUNC_L	0x20		// Lock unlocked
#define	INPUT_FUNC_D	0x10		// Deadlock unlocked (This needs to be L>>1)
#define	INPUT_FUNC_M	0x08		// Manual active
#define	INPUT_FUNC_P	0x04		// Pull down (not really a function, but a rarely needed input flag)
uint8_t inpoll=0;
revk_gpio_t ingpio[MAXINPUT]={0};
uint8_t inrgb[MAXINPUT]={0};
uint8_t infunc[MAXINPUT]={0};
uint8_t inhold[MAXINPUT]={0};
char* inname[MAXINPUT]={0};
uint32_t indoorbell[MAXINPUT]={0};
uint32_t inwarning[MAXINPUT]={0};
uint32_t infault[MAXINPUT]={0};
uint32_t inaccess[MAXINPUT]={0};
uint32_t inpresence[MAXINPUT]={0};
uint32_t intamper[MAXINPUT]={0};
uint32_t infire[MAXINPUT]={0};
uint32_t inpanic[MAXINPUT]={0};
uint32_t inarm[MAXINPUT]={0};
uint32_t instrong[MAXINPUT]={0};
uint32_t indisarm[MAXINPUT]={0};
revk_gpio_t keypadtx={0};
revk_gpio_t keypadrx={0};
revk_gpio_t keypadde={0};
revk_gpio_t keypadre={0};
revk_gpio_t keypadclk={0};
uint8_t keypadtimer=0;
uint8_t keypadaddress=0;
uint8_t keypadtxpre=0;
uint8_t keypadtxpost=0;
uint8_t keypadrxpre=0;
uint8_t keypadrxpost=0;
char* keypadidle=NULL;
char* keypadpin=NULL;
#define	MAX_MIX		5
uint32_t arealed=0;
uint32_t areaenter=0;
uint32_t areaarm=0;
uint32_t areastrong=0;
uint32_t areadisarm=0;
uint32_t areadeadlock=0;
uint32_t areakeypad=0;
uint32_t areakeydisarm=0;
uint32_t areakeystrong=0;
uint32_t areakeyarm=0;
uint32_t engineer=0;
uint32_t armed=0;
uint32_t smsarm=0;
uint32_t smsdisarm=0;
uint32_t smscancel=0;
uint32_t smsarmfail=0;
uint32_t smsalarm=0;
uint32_t smspanic=0;
uint32_t smsfire=0;
uint32_t mixand[MAX_MIX]={0};
uint32_t mixset[MAX_MIX]={0};
uint16_t armcancel=0;
uint16_t armdelay=0;
uint16_t alarmdelay=0;
uint16_t alarmhold=0;
uint8_t meshexpect=0;
uint8_t meshcycle=0;
uint8_t meshwarmup=0;
uint8_t meshflap=0;
uint8_t meshdied=0;
uint16_t mqttdied=0;
char* smsnumber=NULL;
uint16_t timer1=0;
const char revk_settings_secret[]="✶✶✶✶✶✶✶✶";
