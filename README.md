# SolarSystem

Modular access control and alarm system.

## Open source

Please bear in mind that this is an open source system with no warranty. The system can be used for access control, and/or alarm system, but you need to access its suitability yourself, and consider any implications with regard to your insurers. The system does not come under any sort of approved alarm system.

## Building

Git clone this `--recursive` to get all the submodules, and it should build with just `make` for the host components. Similarly `make set` in the `ESP32` directory for the ESP32 components, which actually runs the normal `idf.py` to build. `make menuconfig` can be used to fine tune the settings, but the defaults should be mostly sane. `make flash` should work to program.

## Manuals

- Overview https://github.com/revk/SolarSystem/blob/master/Manuals/Overview.md
- Server https://github.com/revk/SolarSystem/blob/master/Manuals/Server.md

### Technical

- API https://github.com/revk/SolarSystem/blob/master/Manuals/Tech-API.md
- Fob https://github.com/revk/SolarSystem/blob/master/Manuals/Tech-Fob.md

### Hardware modules

## ESP32 code for modules

Modules work as a mesh over WiFi, and one connects to a back end cloud management system. PCB designs are included (KiCad) as are 3D printable case designs.

Example of access control PCB:

![Access2](https://user-images.githubusercontent.com/996983/183847654-823f6ce3-3b02-4f11-89c0-45e1dcc967e2.png)

## Back end management cloud system

Cloud back end for management of devices without the need for on-site controller. Runs on linux with mariadb and apache.

An instance of the cloud management system is run on https://access.me.uk/ by Andrews & Arnold Ltd, and provisioned DESFire cards can be ordered printed to your design.

Some assembled boards can be ordered https://www.aa.net.uk/etc/circuit-boards/

Copyright © 2019-22 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
