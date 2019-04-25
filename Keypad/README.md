* Keypad

A WiFi driver for a galaxy keypad/display for use with Solar System
Designed to run on an ESP-01 connected to an RS485 level shifter (using GPIO0 for RTS)

* Settings

Settings are sent using a topic of setting/Reader/XXXXXX/settingname, e.g. setting/Reader/XXXXXX/wifissid.

- wifissid      The SSID
- wifipass      The WPA2 password

Note that you can also set wifissid2, wifipass2, wifissid3, wifipass3, but if these are set then only non-hidden SSID are selected from the set of up to three settings. If using a hidden SSID then only set the above single entry.

- otahost       The OTA host (normally expects to be Let's Encrypt https)
- otasha1       Can be set to 20 byte SHA1 hash of certificate (instead of Let's Encrypt based)

- mqtthost      The MQTT host
- mqttuser      The MQTT username (no point unless using TLS)
- mqttpass      The MQTT password (no point unless using TLS)
- mqttsha1      The 20 byte SHA1 hash of the certificate (and hence uses TLS on port 8883)

Settings are applied when you next restart. If you mess up, reflash via serial with erase flash option

- restart       Clean restart
- upgrade       Upgrade from OTA - path is /Reader.ino.nodenc.bin

* Commands

- display	Up to 32 characters (2x16)
- cursor	1 byte, 0x80=solid, 0x40=underline, 0x10=second line, last 4 bits is position
- beep		2 bytes, time on, time off
- keyclick	1 byte (3=silent, 5=quiet, 1=normal, 0=no beeps)
- backlight	1 byte 0/1
- blink		1 byte 0/1

* Status

- fault		0/1
- tamper	0/1
- key		Key press (1 character)
- hold		Key hold (1 character)
