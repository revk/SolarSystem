# Door and fob operation

This describes some of the basic working of fobs/cards on the door access.

## Areas

It is important to remember that the system as a whole operates on areas (`A` to `Z`). If working with a single door, or simple installation you may find you only ever use area `A`, however, all of the operations are defined to work on sets of areas.

## Deadlock

The door control settings have an area set to define if the door is deadlocked. If any of the areas in this set are armed, the door is considered *deadlocked*. In a deadlocked state you cannot open the door, but you may be able to disarm it and open it in one operation as detailed below.

The door controller can also have a deadlock output which may work an actual electric lock mechanism.

## Fob disarm

When a fob is presented it could disarm some areas.

- The fob has time controls, and if outside these then no action is taken, else…
- The fob has a set of areas it can disarm.
- The door has a set of areas it can allow disarm.
- The fob can disarm areas that are in both of the above sets (i.e. AND operation on sets).

However, the fob will only disarm any areas if it is also allowed entry (once disarmed) as detailed below.

## Fob entry

When a fob is presented it could open the door.

- The fob has time controls, and if outside these then no action is taken, else…
- If the door is deadlocked, then the fobs disarm capabilities are considered, if they would not cause the door to be un-deadlocked, then no further action is taken, else…
- The fob has a set of areas it is allowed to enter.
- The door has a set of areas in which it is considered to be for the purpose of entry.
- If the fob’s set of enter areas cover all of the door’s set of entry areas then entry is allowed.
- If entry is allowed then any disarming that the fob can do is performed first.

## Fob arm

When a fob is held for a period (normally 3 seconds) then arming is considered.

- The fob has time controls, and if outside these then no action is taken, unless…
- The fob has a setting allowing latearm, if this is set then the time controls are disregarded. If not, and not in time controls, then no further action is taken.
- The fob has a set of areas it can arm.
- The door has a set of areas it can arm.
- The areas that are in both of these sets are then armed (i.e. AND operation on sets).

Normal arming means waiting for `ACCESS`, `PRESENCE`, and `TAMPER` to clear first, otherwise the arming will fail. It would not partially arm in such cases.

## Strong arm

When a fob is held for a longer period (normally 10 seconds) then strong arming is considered.

- The fob has time controls, and if outside these then no action is taken, unless…
- The fob has a setting allowing latearm, if this is set then the time controls are disregarded. If not, and not in time controls, then no further action is taken.
- The fob has a set of areas it can strongarm.
- The door has a set of areas it can strongarm.
- The areas that are in both of these sets are then armed (i.e. AND operation on sets).

Normal arming means waiting only for `PRESENCE` to clear first, otherwise the arming will fail. It would not partially arm in such cases. When armed with `ACCESS` or `TAMPER`, and change of state of those inputs causes a `PRESENCE` and hence an alarm.

## Exit button

The exit button allows the door to be unlocked if it is not deadlocked.

However the exit button can be set to allow disarming of the door, and hence allowing it to be unlocked if the door is deadlocked. In this case the set of areas for which the door will deadlock and disarm will be disarmed when the exit button is used.

### Exit button arm

The exit button can be held for a period (normally 3 seconds). By default this logs as a stuck button, but there is a setting to enable arming using the exit button instead. If this is set then all of the areas covered by the deadlock and arming are armed.

Using the exit button in this way will first (on initial press) work to disarm and/or open the door as normal, and then arm the door. Arming also causes the door to lock.

## Status reporting

A button event is reported on use of the exit button.

A fob event is reported on use of the fob. The fob event has a number of useful fields. Most Boolean fields are only present if true.