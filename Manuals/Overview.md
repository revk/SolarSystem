# Solar System

## System overview

The system makes use of small modules which provide key functions including inputs (connecting to switches, PIRs, etc), secure card/fob readers, outputs (connecting to alarm bells, lights, etc), door control, and keypads, and possibly other devices.

These all operate on a closed, encrypted, peer to peer, WIFi mesh. One of the devices connects to a conventional WiFi to provide the system as a whole with a connection to configuration, control and monitoring. This could be a cloud based system or a local on-site server.

The system provides tracking of inputs, states, and outputs to function as an alarm system, as well as access control system.

An MQTT connection can also be made to a local (insecure) broker - this allows logging of various states and events so as to allow local monitoring and allow the system to inject MQTT messages - for example to turn off lights when alarm set.

## Configuration, control, and monitoring

The system is configured via web pages on a control system. There are a number of key concepts about the way the system works, which are well worth understanding - these are not quite the same as you will find in other alarm systems.

### Organisation / site

The management system can operate multiple separate organisations, and each organisation can have multiple sites. Access control allows users to be assigned various roles in an organisation. So if you have a login to the control system you may not be able to access all of the controls and logs depending on how your administrator has configured things.

An organisation has one or more sites that share some management users and settings.

The door control fobs/cards can hold multiple separate credentials for different organisations and sites.

### Areas

The system has a concept of *areas*. These normally represent physical areas, such as a room or a building. They can overlap, or (more commonly) be one area encompassing a whole building but with sub areas for rooms or floors, etc. The system maintains inputs, states, and outputs for each area independently, and almost all configuration involves defining which areas apply to a specific function. For example, one input could be simultaneously a presence for one area, but simply a warning for another. Areas are given names, but all internal reporting uses an area ID which is a letter `A` to `Z`. Some additional special area codes may be added in due course.

Areas can be linked, e.g. you could have multiple sub offices in a building and when all are armed the lobby area is armed automatically. This linking can also be configured to send MQTT messages, e.g. you may want a lobby light to go out when the alarm is set.

### Access control

The access controller systems are designed to operate a door - with an exit button one side, and a lock control with door open input (and possibly lock engaged input), and an NFC reader on the outside. Many other configurations are possible, and a wide variety of locks are supported.

The minimum working of a door is just an *exit* button and the lock control relay, which unlocks the door for a period when the button is pressed or a valid fob is used.  However there are a number of other input times, including tracking the door is open, the lock is disengaged, or a manual operation such a key has been used. The only output is a solid state relay which works the lock, but there is also a model with two such relays.  The timing for locking, unlocking, etc, are all configurable. The system can report if the door is left ajar or propped open based on timing settings as well. The door can even have a door bell input.

#### Lock/Deadlock

The door works on the principle of a *lock* and a *deadlock*, which can have separate inputs and outputs. The *lock* is simply that the door is locked, e.g. during the day. The *deadlock* is that the door is *armed* for when nobody is in the building. This means you can have a door which is locked all the time using the *lock* output, or have a door which is open all day by using the *deadlock* output. It is even possible to have two separate actual locks if you want.

#### Arm/Disarm

The door is *armed* (well, more accurately one of the *areas* the door is assigned to are *armed*) to set the *deadlock* mode. This is usually done by holding a fob/card for three seconds. The door is disarmed by just using a valid fob. The settings for the fob define if it is allowed to *disarm* things or just allow access only when the system is not armed.

A door can be, in more than one area. This means that a user must have permission to open the door in all areas covered by the door, and if areas are alarmed, to disarm the areas covered by the door that are currently armed. Normally using a fob disarms the alarm and opens the door in one operation. Arming is normally by holding a fob for 3 (configurable) seconds.

The exit button normally only works if the door is not armed, but can be set to auto disarm the door. Holding the exit button can be configured to arm the door - this makes sense if the door is working using a *deadlock* mode rather than setting an alarm for the person inside holding the button.

#### General input/output

Whilst the inputs and outputs are normally tied to functions for working the door they can also be tied to various states. For example, the door lock control relay could also be linked to the `FIRE` state for one or more areas meaning the output is forced active (unlocked) if there is a fire alarm. Similarly the inputs will usually be linked to input states, mainly the *door open* would link to the `ACCESS` state for corresponding areas.

There are logical inputs too such as *NFCFault*, which can be linked to states. There are also physical tamper switches (normally linked to `TAMPER` states).

### Fobs

Fobs, or cards, have access permissions, which define a set of areas where they are allowed to enter, arm or disarm, and other functions. They can also have day of week and time of day restrictions. The access details are securely encoded in DESFire key fobs so they work without the system needing to refer to a central database in real time. Even so they also have timeouts to ensure that a lost fob is not useful.

A fob can also be individually blacklisted - with a limited blacklist list loaded in to the flash of the door controllers on a site so that the fob is blacklisted even when the management server is not on-line.

#### Access controls

The fobs are actually linked to named access controls which define a settings and permissions, e.g. *Staff*, *Cleaner*, *Visitor*, etc.

### Input states

The various input states are intended to reflect reality, as it changes. Whilst the actual inputs may work in various ways they ultimately boil down to OFF or ON, even if that means comparing a voltage or a light level, etc.

Each of these inputs can be configured to indicate that an input state (in each area) is active, so a physical contact switch on a window may represent `TAMPER` on area `B` for example. A physical input can represent multiple types of input in multiple areas all at once.

These are then aggregated per area, so if any input is ON indicating `PRESENCE` in area `B`, then area `B PRESENCE` state is ON for the whole site. Only when all such inputs are OFF will the overall state for area `B PRESENCE` go to OFF.

Some inputs have special meaning, e.g. on door control systems (e.g. exit button), but even these can also be assigned as alarm system inputs (e.g. door open can represent `ACCESS` as well as being used as part of the door entry system).

There are a number of basic input types. A change of input state is always propagated through the system and logged before changing back, even if a simple brief change like a `DOORBELL` push and release. The system cycles every few seconds ensuring all inputs are aggregated.

It also possible for an input to have a means to detect tamper or fault (e.g. specific voltages, or failure of some hardware to respond), though that is not used at present.

There are physical tamper switches, but also logical inputs for various detected situations, such as *NFCFault*, which can be configured to trigger input states.

There is also a simple time of day *Timer* logical input on some devices which can be used as an input to trigger something at a time (typically arming at end of day).

|State|Meaning|
|-----|-------|
|`DOORBELL`|This is simply a user state - it would usually be triggered on the doorbell input on an NFC reader. It can then be used to drive an output somewhere that is perhaps an actual bell.|
|`WARNING`|A warning state is the lowest level of input, and is not considered critical. An example may be that a smoke cloak is low on fluid, etc. It is self clearing - i.e. when the warning is no longer valid the warning state ends.|
|`FAULT`|A fault is where some mis-operation is detected but not likely to be tampering. Faults can be ignored when arming an area. Faults latch as `FAULTED` which are cleared next time the area is armed.|
|`ACCESS`|This indicates that an access is open, e.g. a window, or door. I.e. there is no confirmation that someone is present, but that the building is not secure. In some cases this could be ignored and allow arming of an area. Note that a change of state of an `ACCESS` will normally be considered to be a short `PRESENCE` which would trigger an alarm - so if arming with a window open, closing the window counts as `PRESENCE` and means the alarm would go off.|
|`PRESENCE`|This indicates someone present. It might be RADAR, PIR, or something else, but is used where it has detected a possible intruder. Areas are not armed whilst there is `PRESENCE` (arming waits a while for it to clear). A `PRESENCE` triggers an `ALARM` in an armed area.|
|`TAMPER`|A tamper is where some specific tamper is detected. This is often a side effect of other systems, e.g. an input using resistors may detect a tamper on the input. Some devices have explicit tamper inputs. It can also be used directly on an input for something that indicates damage, such as a window break glass detector. A tamper should not be ignored. A change of `TAMPER` causes `PRESENCE` which triggers and `ALARM`. Tampers latch as `TAMPERED` which are cleared next time the area is armed.|
|`FIRE`|This indicates a fire alarm input is active - usually a connection to a fire alarm panel. Fire alarms do not depend on a zone being `ARMED`. A fire input is self clearing. Typically `FIRE` is attached to door unlock outputs to override normal door controls. The system is not intended to be used as a fire alarm system, just to monitor a fire alarm system and take additional actions - doors should have break glass exit controls for safety.|
|`PANIC`|This is typically a manual input indicating a panic. Panic alarms do not depend on a zone being `ARMED`. A panic input is self clearing. This could be a panic button for a silent alarm perhaps, or could be an assistance input from a disabled toilet, for example. Obviously you can configure this to cause an alarm as needed.|
|`ENGINEER`|This is a system wide setting which stops `TAMPER` from triggering an alarm. It can also be used an output, for example,` to a bell box to stop the bell making a noise.|

### Arm/Strong arm

Arming / disarming is done per area and can be several areas at once. This can be done from the web control pages.

Arming (normally holding a fob for 3 seconds) will wait for `TAMPER`, `ACCESS` or `PRESENCE` to clear before arming, and if they take to long then arming fails. If arming fails it fails for all the areas that you are trying to arm - i.e. it does not arm some areas and leave others unarmed. Strong arm is an option to force setting.

Strong arming (normally holding a fob for 10 seconds) will not wait for `TAMPER` or `ACCESS` to clear. This allows you to arm the system anyway. If the `TAMPER` or `ACCESS` stay then that is not a problem, but if any such input changes state that triggers a `PRESENCE` which sets off the alarm. This is really a bodge to allow arming when something is broken, such as a window sensor, so as to allow arming rather than leaving the system unarmed. Be careful to consider who may be given permissions to do this - normally, for example, if it just an open internal door, the person arming should go and close the doors and try again. One use is where this did not happen and so an administrator does a strong arm remotely.

Disarming is done as a consequence of a valid use of a fob, allowed to disarm all the armed areas that are keeping a door in *deadlock*.

Note that these can be triggered directly on an input, and can be triggered on a timer event, so as to allow auto arming at a particular time of day.

### Input/output timers

Inputs are cycled every few seconds - during which the input is latched, so even a brief input will count and be aggregated for the site for the cycle. This means an input will always last a few seconds no matter how short it was in reality.

There are however some timers than can be configured:

- Arm cancel - if arming is not possible within a certain time the arming is cancelled. Normally a `TAMPER`, `ACCESS` or `PRESENCE` will hold off arming. This allows for exit during arming.
- Pre alarm - when an alarm condition happens it triggers a pre-alarm for a time. If disarm is done during the pre alarm then the alarm is not activated. Even if the alarm trigger goes away, a pre-alarm will trigger an alarm at the end of the timer if not disarmed first. This is to allow for entry before disarming.
- Alarm clear - once the trigger for an alarm stops, the alarm state continues for a time.
- Output timers - an output can have a timer, so, for example, a bell output may be limited to avoid annoyance.

### System states

In addition to the input states, which set, and clear, based on actual inputs, or events, there are states which can be set internally or derived from inputs or other states or timeouts.

|State|Meaning|
|-----|-------|
|`PREARM`|Aggregate of arm requests that are not set in `ARM` yet. i.e. waiting for `PRESENCE` to clear.|
|`ARMED`|This is the state where we are armed. In this state `PRESENCE`, triggers the alarm process for the area (starting with `PREALARM`). Note that change of `TAMPER` or `ACCESS` causes `PRESENCE` and hence an alarm.|
|`PREALARM`|This is a state where we intend to `ALARM`, but are holding off for a configurable period. Disarming during this time is not an alarm.|
|`ALARM`|We have an active alarm condition in an area. This is cleared once the `PRESENCE` has stopped for a configurable time.|
|`ALARMED`|We have had a previous active `ALARM` condition in an area. This persists even after `ALARM` times out. This is cleared next time the area is armed.|
|`FAULTED`|We have had a `FAULT` at some point.  This is cleared next time the area is armed.|
|`TAMPERED`|We have had a `TAMPER` at some point.  This is cleared next time the area is armed.|

### Outputs

Devices can have output which are OFF or ON. These are driven based on having a specific input or state (as above) for specific area(a) associated with the output. If any of the states and areas specified are ON then the output is ON.

A typical example is a bell box output for the bell linked to `ALARM` state for one or more areas. One may, for example link a strobe to `ALARMED`, a light to `ARMED`, and an engineer hold off to `ENGINEER`.

Whilst systems such as door control have outputs (well, one output, to unlock the door) controlled by the door control, it is also possible to link these, so the door opens when there is `FIRE`, for example.

An output can have a maximum time limit, typically for bell output.

### Keypad

A keypad provides keys and a display, and can be configured to operate in one or more areas. This then causes the display to show when states are active such, as well as messages such as why arming is being delayed, etc.

- It is possible to arm and strong arm areas from a keypad
- A keypad specific PIN can be entered to disarm
- Active conditions are shown and can be scrolled through using arrow keys

### Stand alone fob readers

Whilst NFC readers are normally part of door entry units, they can be connected to simple I/O modules and used as a logging point. These can be used to arm/disarm areas, the same as a door control for an area but without the door. They can be used just to log, e.g. clock in/out, or security guard check points. They can also be used by an administrator to configure fobs on a desk, for example.

The system includes web hooks for fob events (and various other types of event) which can feed these fob access reports to an external system for time recording, etc.

### Event hooks

The control pages detect (when on line) various events, and each event can be set up to send an SMS, an email, or a webhook (https/JSON post).

- Offline event
- Fob event
- Fob deny event
- Alarm event
- Fire event
- Panic event
- Warning event
- Tamper event
- Fault event
- Trigger event
- Inhibit event
- Arm event
- Strong arm event
- Disarm event
- Arm fail event
- Open event
- Forced event
- Not open event
- Not closed event
- Propped event
- Wrong PIN even

Sending an SMS required A&A SMS account set up on the control system.

### SMS from controller

The design allows for an SMS for some events direct from a door controller. This includes, for example, SMS to a number configured on a fob when arm or arm fail.

At present this is sent via the web control pages, so only works on line, but plans are that this will allow direct SMS via an on site mobile module at some point.

### MQTT/IoT

Secure MQTT is used for command and control to the management system from the root node connected to WiFi.

However, the root node can also connect to a secondary MQTT server for IoT, e.g. tasmota connected switches for lights, buttons, etc.

Many events can then be configured to send MQTT messages on this secondary connection, e.g. when an area is armed, all the lights are turned off.

There is also reporting of various states and debugging which can be enabled.

### Mesh WiFi on site

The system configures the devices to form a mesh WiFi on site with only one connecting to the site WiFi. This allows devices to communicate without the need for a working separate AP or Internet connection. The Mesh WiFi can also use a non standard *long range* rate and operates over a wide area by forming a mesh of devices based on signal strength.

Additional devices could be added with no alarm system function simply to extend the range of the mesh over a larger site.

The MQTT connection to each device is relayed via the current root device, which can change depending on circumstances.

## Hardware

Each module stores its configuration internally in flash, and can operate without any central control system - just using the peer to peer mesh WiFi. Even without the peer to peer Mesh, each door control can operate autonomously to allow access based on the permissions on the fob used. The control system is used to manage the system, managing configuration and settings, and monitoring status, reporting, and logging. The Internet link is currently needed for SMS sending.

The hardware is mainly custom ESP32 controller boards. The main modules are :-

- Door controller - has connections for exit button, door closed, lock, and NFC reader
- NFC reader module - normally connects to door controller but can be connected to generic I/O where used in ways that are not a door control.
- Generic I/O device - connects to simple switch inputs, or ADCs, etc.
- Keypad control devices - fits inside a galaxy keypad to provide keypad control functions
- Bell box I/O - connects to bell box for tamper, strobe, light, engineer mode, and bell. This can also include a GPS to set the system time even when no internet access.
- It is also possible to use otehr ESP32 devices, and has been tested on Shelly Plus 1, and Shelly DC i4 which make cheap and useful extensions of the system.

At present inputs are simply a switch, but in future modules may support a variety of inputs, not just switches, such as resistor based inputs with fault and tamper, light level sensors, motion sensors, proximity sensors, etc.

Because the system can connect to a local MQTT broker, outputs using IoT devices such as Shelly Plus 1 can be used to control lights, etc. However, as a Shelly Plus 1 can run the system code, it can work an output directly and even run off mains.

Additional modules may be designed in the future:-

- Direct ethernet connection for internet connection rather than needing local WiFi.
- Direct mobile module for SMS

If no local WiFI, at present, a simple mobile WiFi router can be used.

### Adding modules

The system allows working with a back end server, this can be one provided by A&A [access.me.uk](https://access.me.uk) or your own. In any case, if you buy or make a new ESP32 module for the system it will need setting up.

Initiall the module tries to connect to an SSID of `SolarSystem` with a passphrase of `security`. Once connected it connects to the server and appears as a new *pending* device. This can take a few minutes. If the device is on a known and trusted IP, it can be provisioned on to a system (organisation/site) by an administrator.

As this is a fixed SSID/passphrase you need a WiFi with internet access (or access to your server) in place. This is not always convenient, but simply making a mobile hotspot with the required SSID and passphrase will suffice.

Once provisioned to a site, the site WiFi settings will be applied to get the device on line.