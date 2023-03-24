# Autonomous Door Control

***This is a low level technical document providing information to allow you to use the access control modules without the Solar System control.***

The SolarSystem normally provides full door control sequencing and monitoring, i.e. it knows to engage the lock when the door closes, and spot a door ajar state if the lock engaged input does not indicate the lock has in fact engaged in the appropriate time, etc.

The door can be in one of a set of states, which depend on the state of inputs and outputs and timers. Typically each state also has an LED sequence shown on the NFC reader.

Normally the door works in autonomously as part of a mesh system, but it can be configured to work as a simple slave device over MQTT allowing an external control system to operate the doors and make decisions on access.

The door control is determined by the setting `doorauto`:-

|Setting|Meaning|
|-------|-------|
|`0`|No monitoring or control of door inputs/outputs - this is used one devices there are not a door controller.|
|`1`|Lowest level state tracking and reporting of door/lock state. No automatic operation of exit button or access cards. It is expected that the control software tells it when to open doors.|
|`2`|Tracking and control, with exit button operation to allow the door to open (if not in `DEADLOCKED`) state. This allows fast response for exit buttons, but ensures security by checking all cards/fobs with the control system.|
|`3`|As `2`, but checks DESFire fob access controls and reports them. Does not open doors for fob.|
|`4`|As `3`, but opens doors if card is acceptable.|
|`5`|As `4`, but also disarms/arms (deadlock) if acceptable. This is how door are normally configured when part of a mesh.|

## Inputs / outputs

The operation replies on having input and output settings defined. It is valid for some of these not to be defined. Obviously some of these need to be configured to invert the operation to match the way of working that is expected. If using the door with other inputs and outputs, they need to be configured to avoid these.

The inputs and outputs are tied to door controls using settings `inputfunc` or `outputfunc`. These are letter coded.

|Code|Meaning for input|
|----|-------|
|`E`|**Exit**: This input is a door exit push button.|
|`O`|**Open**: This input indicates the door is open.|
|`L`|**Unlocked**: This input indicates the lock is in an unlocked state.|
|`D`|**Undeadlocked**: This input indicates the lock is in an undeadlocked state.|
|`M`|**Manual**: This is an input indicating the next door open is via some sort of manual override.|
|`P`|**Pulldown**: This flags that the input needs to be pulled down rather than the normal pull up.|

|Code|Meaning for output|
|----|-------|
|`L`|**Unlock**: Output to disable the lock.|
|`D`|**Undeadlock**: Output to disable the deadlock.|
|`B`|**Beep**: Output to make a beep.|
|`E`|**Error**: Output to indicate an error.|

## Lock states

The system allows for two locks, a *lock* and *deadlock*. The main *lock* is disengaged and engaged whenever the door is used. The *deadlock* is for when the alarm is set. Typically the deadlock may not exist, or could simply be an output to tell an alarm system to be set. In some cases the main lock may not be used, allowing access without fobs, but deadlock operates an electric lock to secure the door when the alarm is set.

Each look has a state which is tracked. The state is based on observing two signals, an *unlock* output, and an *unlocked* input, which exist for each lock. The output controls the actual lock, and the input indicates the lock is engaged or not. Remember that inputs and outputs can be inverted so as to operate as unlock and unlocked as expected here.

- If there is not an input nor output defined, then the state tracks the output that would be set, as `LOCKED` or `UNLOCKED`.
- If there is only an input defined, we set the state to `LOCKED` or `UNLOCKED` based on the input.
- If the output state changes, then this indicates we are trying to *lock* or *unlock*. As such a timer is started and the state set to `LOCKING` or `UNLOCKING` accordingly. Note that while the door is *open*, the timer does not run when `LOCKING` (i.e. is continually restarted) as it is assumed the lock cannot engage in that state.
- If the input state changes whilst the timer is running, and changes to a state that matches the output state, then the timer ends early.
- If the timer ends (including as above): If the input is not defined, or it matches the output we change state to `LOCKED` or `UNLOCKED` accordingly. Otherwise we change state to `LOCKFAIL` or `UNLOCKFAIL` accordingly.
- If the input state changes and the timer is not running, then we change state to `FAULT` or `FORCED` accordingly if the input does not match the output, otherwise `LOCKED` or `UNLOCKED`.
- The lock state is then used to determine the door state, and state of faults and tampers.

|State|Meaning|
|-----|-------|
|`LOCKED`|Locked|
|`UNLOCKED`|Unlocked|
|`LOCKING`|Locking, the lock timer is running|
|`UNLOCKING`|Unlocking, the unlock timer is running|
|`LOCKFAIL`|The lock did not engage at the end of the lock timer|
|`UNLOCKFAIL`|The lock did not disengage at the end of the lock timer|
|`FAULT`|The lock changed to a locked state when we did not ask it to|
|`FORCED`|The lock changed to an unlocked state when we did not ask it to|

## Door states

The door can be in one of the following states, reported over MQTT.

|State|Meaning|
|-----|-------|
|`DEADLOCKED`|This is the normal state when the alarm is set, it means the door is closed and the main *lock* and *deadlock* are engaged. i.e. alarm set. In this mode the system will not autonomously allow exit of secure cards to open the door unless mode `4` fully autonomous and the fob allows unlocking.|
|`LOCKED`|This is the normal, idle, door closed state where the door is closed and the main lock is engaged, but the deadlock is not engaged. i.e. alarm not set.|
|`SHUT`|The door is closed but not locked and won't be locked, because there is no *unlock* output. This is a state you see when a door is only actually locked on a *deadlock* output.|
|`UNLOCKING`|This is a state after `LOCKED`, or `DEADLOCKED`, where the lock or locks are disengaged, and we are waiting for the lock engaged input(s) to confirm the lock is no longer engaged.|
|`UNLOCKED`|This is a state after `UNLOCKING` where the lock is disengaged, and confirmed as such, but the door is not yet open.|
|`OPEN`|This is a state where the door is open, and should be. A timeout runs, at the end of which we change to `NOTCLOSED` state.|
|`PROPPED`|This is a state where the door is open, and has been authorised to be propped open.|
|`CLOSED`|This is a state the same as unlocked after the door was open. A timeout runs, at the end of which we engage lock(s).|
|`LOCKING`|This is a state where the door is now closed, and the lock(s) are engaging. This also applies when changing from `LOCKED` to `DEADLOCKED`.|
|`NOTCLOSED`|This is a state where the door is open, and has been for too long without authorisation. This follows from OPEN state. This can be changed to PROPPED.|
|`AJAR`|The door is closed, but the lock has not engaged, suggesting it is not fully closed and could be opened.|

These states are based on a door open input, and the lock states.

If the door is open: The door can be in `OPEN`, `NOTCLOSED`, or `PROPPED` state.

- If not already in one of those states, the state changes to `OPEN` and a timer it set. If setting `OPEN` and any lock is in `LOCKING` state, we unlock that lock. i.e. opening whilst locking is allowed.
- If the timer expires in `OPEN` state the state changes to `NOTCLOSED`.
- If `OPEN` or `NOTCLOSED` and the door prop is authorised then the state changes to `PROPPED`.
- If the door is closed: We test door states until one matches.
- `SHUT` is a special stae for doors that only lock on *deadlock* control when the door is closed.

## Fault states

- Either lock is in a `UNLOCKFAIL`, or `FAULT` state.
- Exit button or ranger input are stuck active.

## Tamper states

- Either lock is in `FORCED` state.
- Either lock is in `LOCKED` state and the door is open, i.e. door forced.

## Settings

In addition to the main door setting, there are some timings that can be set.

|Setting|Meaning|
|-------|-------|
|`doorunlock`|Time (ms) from lock disengage to lock engaged input confirming disengaged. This is used to indicate there is a problem unlocking (fault condition) if the lock engaged input is defined, or simply used as the time to allow in `UNLOCKING` state if not.|
|`doorlock`|Time (ms) from lock engaged to lock engaged input confirming engaged. This is used to indicate there is a problem locking (AJAR state) if the lock engaged input is defined, or simply used as the time to allow in `LOCKING` state if not.|
|`doorprop`|Time (ms) to allow door open before considered being `NOTCLOSED` state.|
|`dooropen`|Time (ms) to allow door to remain in `UNLOCKED` state after `UNLOCKING` state before door is locked as not having been opened.|
|`doorclose`|Time (ms) to allow door to remain in `CLOSED` state after closing the door before it is locked.|
|`doorexit`|Time (ms) to allow exit button press, beyond which its is assumed to be stuck somehow.|
|`doorcycle`|Time (ms) to cycle lock and beep in event of error (`AJAR`, `NOTCLOSED`, or forced)|
|`doorbeep`|If set to `1`, then beep whilst waiting for door to open|
|`nfcpoll`|Time (ms) for polling nfc|
|`nfcholdpoll`|Time (ms) for polling when NFC held|
|`nfchold`|Number of hold polls before held, 0 means no held.|
|`nfclonghold`|Number of hold polls before longheld, 0 means no longheld|

## Commands

The inputs report state changes as normal, and the door state changes are reported as a state message, but there are commands which can be sent to impact the door state:-

|Command|Meaning|
|-------|-------|
|`lock`|This aims to change state to `LOCKED`. If the door is `CLOSED` or `UNLOCKED`, it immediately changes to `LOCKING` state. This can also be used in `DEADLOCKED` state to move out of `DEADLOCKED` state, i.e. alarm unset.|
|`unlock`|This is the same as working the exit button, but works in `DEADLOCKED` state too. It disengages locks moving to `UNLOCKING` and `CLOSED` state to allow the door to be opened.|
|`prop`|If the door is in `OPEN` or `NOTCLOSED` state it changes to `PROPPED` state.|
|`access`|Sends new access file, and then re-evaluates the card (opening door, or locking if necessary).|

These commands can operate with no payload, simply carrying out the action.

However, they can also include a payload so as to update the access file one the card. The payload is a JSON object with `"id"` containing the card ID (hex) and `"afile"` containing the access file (hex). Once the access file is updated the card is re-evaluated with the new access file.

The access file is also cached, so that if there is then a second fob access from the same fob in a short time the `"afile`" data is directly stored from the cache.

## NFC remote

The `nfcremote` command can be sent with no data to start a remote mode. Once a card has connected further `nfc` commands can be sent with a payload that is a JSON string containing hex of a raw DESFire command. And `nfc` info response carries the reply in the same way. The `nfcdone` command can be sent when complete, or the remote mode stops when card is gone. In the event of an error, an `nfcerror` info response contains the error string.
