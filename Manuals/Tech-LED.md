# Device LED

Some ESP modules have an LED. Some are a simple single colour (normally amber), but most have an RGB LED.

## Single colour

The single colour has two modes.

- Slow (0.3/0.3s) if all is well, ie mesh connected (and if root, MQTT connected)
- Fast (0.1/0.1s) if not

## RGB LED

The devices with an RGB LED will still blink that LED, slow or fast, as above, but the colour is selected depending on the system state. The last of these that is active in areas covered by `area.led` define the LED colour. If none match then the LED is green. If `area.led` is not set then any area applies.

|State|Meaning - lats matching from this list|
|-----|-------|
|`engineer`|Green and Yellow alternating|
|`doorbell`|White|
|`warning`|Yellow|
|`fault`|Magenta|
|`access`|Blue|
|`tamper`|Red|
|`fire`|Red and Yellow alternating|
|`panic`|Red and Magenta alternating|
|`armed`|Cyan|
|`alarm`|Red and Blue alternating|
