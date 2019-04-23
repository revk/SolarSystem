* Min Reader

This is a design for an RFID based reader that can be used with Solar System. It works using 2.4GHz WiFi to communicate. Please see security advice in the general documenttion.

* Defaults

This connects to a WiFi SSID of "IoT' with password "security", and connects to an (unsecure) MQTT server called "mqtt.iot" on port 1883. From there you can send commands to change settings. You will see it appear on MQTT sending stat/Reader/XXXXXX and a build date. XXXXXX is the hostname (device ID).

* Settings

Settings are sent using a topic of setting/Reader/XXXXXX/settingname, e.g. setting/Reader/XXXXXX/wifissid.

- wifissid	The SSID
- wifipass	The WPA2 password

Note that you can also set wifissid2, wifipass2, wifissid3, wifipass3, but if these are set then only non-hidden SSID are selected from the set of up to three settings. If using a hidden SSID then only set the above single entry.

- otahost	The OTA host (normally expects to be Let's Encrypt https)
- otasha1	Can be set to 20 byte SHA1 hash of certificate (instead of Let's Encrypt based)

- mqtthost	The MQTT host
- mqttuser	The MQTT username (no point unless using TLS)
- mqttpass	The MQTT password (no point unless using TLS)
- mqttsha1	The 20 byte SHA1 hash of the certificate (and hence uses TLS on port 8883)

* Data

The status is sent as stat/Reader/XXXXXX/...

- id		A card has been applied, sends the 8 character hex ID
- held		A card has been held for 3 seconds, sends the 8 character hex ID
- gone		A previously held card has now been removed

Typically you get id, or id+held+gone if held for more than 3 seconds.

* PCB

A PCB design is included in SVG as millable tracks, separate layer for milling, edge cutting, and drilling with 0.5mm drill bit (mills to hole size).
Connect to :-

- MRFC522	https://www.amazon.co.uk/gp/product/B07FC9ZFGC/
- 12V reg	https://www.amazon.co.uk/gp/product/B07DJZKMYM/
- Screw term	https://www.amazon.co.uk/gp/product/B01MT4LC0F/
- header pins	https://www.amazon.co.uk/gp/product/B06XR8CV8P/
- ESP-12F	https://www.amazon.co.uk/gp/product/B07DBC7KWN/

* Case

A 3D print case design is incldued in OpenSCAD format.

