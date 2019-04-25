* Keypad

MQTT commands

- upgrade	OTA upgrade
- restart	Restart
- display	Up to 32 characters (2x16)
- cursor	1 byte, 0x80=solid, 0x40=underline, 0x10=second line, last 4 bits is position
- beep		2 bytes, time on, time off
- keyclick	1 byte (3=silent, 5=quiet, 1=normal, 0=no beeps)
- backlight	1 byte 0/1
- blink		1 byte 0/1

MQTT status
- fault		0/1
- tamper	0/1
- key		Key press (1 character)
- hold		Key hold (1 character)
