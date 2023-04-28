# Configuration tips

Configuration is managed via the web control pages, e.g. https://access.me.uk but it also relates a lot to the physical installation and wiring.

## Wiring

Each module is set based on a *PCB* template which defines what GPIO pins it uses and sets some sensible defaults.

Most of the modules have general purpose GPIO pins which mean they can be used and inputs or outputs. Documentation for each module explains the typical usage. In some cases a GPIO can be only used as an output (e.g. the door lock release relay on access modules).
You will need to decide how you will wire things up - if as per recommendations or differently as you may need. For example, the access system is typically used with an exit button, door open contact, and lock dis-engaged input. But if you want to use more inputs (such as a warning when a break glass is used, etc) you will need use some of the other GPIO pins.

For each pin you can decide if it is input, or output, or a fixed “power” output (i.e. GND or 3.3V output). You can say which way up the input or output works, e.g. typically an input on a button is active low (connected to GND) as inputs normally have an internal pull up.

Configuring the system you will need to match the configuration of each module to the way you wire it up.

## WiFi

The system uses WiFi at 2.4GHz.

It is recommended that you operate the WiFi on a fixed channel, even if you have multiple APs. If not, then you could find some nodes use one channel and some another, splitting the site in two. You can also fix the channel on the site config if you wish, but the system will look for the AP on other channels if not found on the channel you have set.

You may also fix the preferred root node - typically one nearest an access point. This can simply make things quicker. If the root node fails, then a new root node is picked so the system can get on line anyway and operate with a missing node.

The system is quite robust, and uses low levels of data, but you should try and pick a channel that has low interference from other sources.

## Upgrading

You can upgrade nodes from the control system, and it is recommended that you keep nodes up to date. The administrator may advise you when upgrades are needed.

Upgrades can take some time, but if all nodes are within range of APs then one trick is make all nodes out of service. This makes them all work as a root node connecting to the AP. You can then upgrade all nodes. This is quicker as they all get the upgrade directly rather than relayed via the internal mesh network. When all done, set all back in service.

Note that doors will still work autonomously while out of service, but alarm systems are not coordinated on the site. Do not leave a site out of service if you want the alarm to work!

## Areas

At the top level you need to decide on areas that will exist in your system. You can link areas, e.g. if you have sub offices that need arming separately, you may want a lobby that is armed when all sub offices are armed. It is best to decide the areas first. The configuration of devices and GPIO pins all relate to areas in use.

## Special inputs and outputs

Some inputs and outputs are special, relating to the operation of the door controller. However, you can (and probably should) also consider these linked to areas as well. For example :-

- Door open input set for `ACCESS`, as it means the area is not secure.
- Lock disengaged set for `ACCESS`, as it means the area is not secure.
- Lock unlock output set for `FIRE`, so doors unlock in the event of a fire.
- Exit button set for `PRESENCE`, as pressing it means someone is there to have pressed the button.
- Special inputs like *NFCFault* set to `FAULT` and the tamper inputs set to `TAMPER`
- Timer inputs are useful to set to arm at a late in the day time, for example

## Types of input

The main inputs for an alarm are `ACCESS` and `PRESENCE`. The alarm is triggered by `PRESENCE`, but you cannot normally set the alarm until all `ACCESS` or `TAMPER` inputs are cleared. Also, if an `ACCESS` (or `TAMPER`) input changes state (e.g. someone opens or closes a window) that trips a `PRESENCE` for a few seconds internally (and so triggers and alarm).

As part of the admin configuration, each module is set up with a number of inputs. Most inputs are a GPIO pin, i.e. a numbered pin on the ESP32 processor which are available as a connector of some sort. Some are internally connected, e.g. a tamper switch on a door access board. There are also GPIO pins on the NFC reader which are connected to a tamper or bell input, and whilst these are not ESP32 GPIO pins, they are mapped to special (logical) GPIO inputs so that they all work in the same way. There are also some special cases, such as *NFCFault* which are treated as inputs, but actually detected differently such as a loss of communications to the NFC reader. For most practical purposes these can all be treated in the same way.

It is useful to set a meaningful name on the inputs, as this is used on the monitoring and the display on the keypad. E.g. `“Tamper”` not `“Pin 5”` on a generic module connected to a PIN, etc.

The inputs relate to specific input states, as per https://github.com/revk/SolarSystem/blob/master/Manuals/Overview.md

## Type of output

The main output used on the system is the door relay and the bell box. The door relay is simple, as it opens the door when needed as a special output, but may want `FIRE` set as well to override. You can do tricks like have a door bell input on a special area as a button input by someone’s desk, and set `DOORBELL` output on the lock on the door, thus allowing a button to open the door. Basically `DOORBELL` is a general purpose self clearing control/input.

As part of the admin configuration, each module is set up with a number of inputs and outputs. In some cases for a simple GPIO pin you can decide if the pin is an input or output, and then which areas case the output to be active. You can also set an output GPIO as a fixed power output (low or high).

Outputs can be set to be active for any area and any of the input or internal states, or any of the derived states as below. If any of those defined in any area defined are active the output is made active. The states are listed on https://github.com/revk/SolarSystem/blob/master/Manuals/Overview.md

So, for example, you will probably want the bell box “bell” connection set to `ALARM`, and perhaps its strobe, if it has one, to `ALARMED`.

## Tamper and fault

You should take care to configure `TAMPER` and `FAULT` inputs. Note that a tamper is a case where we believe someone is messing with the devices - such as taking a bell box off a wall or unscrewing the cover of a PIR. A fault is where something has broken in some way but not specifically detecting someone interfering with a module. This is a subtle distinction and you may want to configure some fault cases as tamper as well, as tamper changes cause `PRESENCE` and hence an alarm, but faults do not. Some examples…

- A bell box has direct `TAMPER` and `FAULT` connections (depending on the bell box). These are normal GPIO inputs and you set the tamper and fault areas.
- A PIR will typically have a tamper, and so fitting a generic module in a PIR you will usually have `PRESENCE` and `TAMPER` connections to configure.
- A keypad has a tamper switch which appears as a general purpose input. This is not actually on a real GPIO, but appears as a special GPIO *KeyTamper* as a logical input.
- A door access controller has an on-board tamper button which is set as a normal GPIO input and so configured with `TAMPER` areas.
- The NFC reader has a tamper input which appears as a special GPIO on *NFCTamper*. It also has *NFCBell* as an input for `DOORBELL` connection, if used.
- The NFC reader and keypad also have special GPIO inputs *NFCFault* and *KeyFault* which should be configured as `FAULT` areas. These cover cases where communications fails. You may consider the *NFCFault* should perhaps also be a `TAMPER` setting as most likely someone had pulled the reader off the wall.

## Some timers

The site level also has a few timers. These are worth considering. E.g. you can delay alarm for a short time to allow disarming, but this delay also means that more information is sent in an alarm event - with all of the triggers during the pre-alarm time.

## Doors & fobs

The settings for doors are quite complex. Apart from the simple aspect of which inputs and outputs are used, the doors work with fobs and allow access and also allow arming and disarming areas.

A separate document covers this in a lot more detail, but some key points:-

- The door has a logical *lock* and *deadlock*. Typically there is only one actual lock, but depending on the use case this may be the main *lock* which is locked every time the door closes, or the *deadlock* which is locked only where the system is armed. The *deadlock* area setting controls when the door is deadlocked (if any of the areas specified are armed).
- The *enter* settings are required permissions to allow entry, and so the user must have all of these before entry is considered. This does mean that if the *enter* settings are not set then anyone is allowed to enter, so beware.
You can, however, only enter if the door is not deadlocked, or you can disarm any areas that are causing the door to be deadlocked. Bear in mind, even if you can only disarm one area, additional areas may automatically disarm as a consequence and so allow entry.
 - This means you can have doors where someone is allowed access as long as the system is not armed, but not allowed when it is.
You can have more *arm* settings than *disarm* if you want, both on the door and the fob. This can be useful in some cases where someone with limited access to disarm is the last person leaving and so needs to arm everything.
- It is possible to set the exit button to disarm, and also allow holding it to arm. This disarms, and arms the *deadlock* areas. This can be useful to deadlock doors, but is usually not useful for setting a system alarm as you are generally inside when setting.

## Monitoring

The system is monitored by the control system, but there are two levels of monitoring.

**It is important to remember that monitoring is not infallible.**

## SMS

The top level has the notion of SMS monitoring. This allows an SMS for specific events in specific areas, and you set at the site level and a number to SMS. This covers arming, arming fail, disarming, alarm, panic and fire.

At present the SMS is only sent from the control system, but the plan is that the system will be able to do this by itself - either a new mobile module or configuration via a module router. This means that the SMS alerting for these events can work even when not on-line to the control system.

The system allows two sets of credentials for SMS service from A&A. The idea is that you can set different alert tones. To use the secondary SMS server, prefix the mobile number with a `*`. This is ideal for Alarm, or Armfail which are rare to make a louder noise.

## Email

You can have events emailed, just include the email address in the hook.

The config allows you to set the sender address.

## Mastodon

You can have events sent by direct message on Mastodon to any fediverse account. Simply include the target handle in the hook.

The config requires a mastodon server and a bearer (`access_token`) - you will need to use suitable tools to obtain a valid access token for sending toots.

## Hooks

The control system can also generate a number of alerts, which can be SMS, email, or a web hook (https ideally). These cover many more types of event as listed on the site management page, including things like nodes (or the whole site) going off line. For each type of event, you can list one or more hooks.

These only work when the system is on-line (apart from the off line hook, which tells you the system is not on line for more than a few minutes).

Note, to send any SMS from the system you need an A&A SMS account, and there are charges for SMS.

## JSON

The underlying events use JSON payloads. For use in a hook, an `"event"` field is added for the event type, and a `"site"` field added for the site name.

For a web hook the JSON is then posted to the URL. No response is expected (e.g. `204`).

For email and text the JSON is expanded as text, and notably any area codes (e.g. `A` to `Z`) are replaced with the area names to make it easier to understand.

For email, the JSON is also included as an attachment to the email.

## Logging

All of the events are included in a log (whilst the system is on-line). The logs can be viewed per site, per device generating the event, and for fob events - per fob. Note that some events from the root device which could change.
