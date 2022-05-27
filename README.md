# SolarSystem

Modular access control and alarm system.

# Building

Git clone this `--recursive` to get all the submodules, and it should build with just `make` for the host components. Similarly `make` in the `ESP32` directory for the ESP32 components, which actually runs the normal `idf.py` to build. `make menuconfig` can be used to fine tune the settings, but the defaults should be mostly sane. `make flash` should work to program.

## ESP32 code for modules

Modules work as a mesh over WiFi, and one connects to a back end cloud management system. PCB designs are included (KiCad) as are 3D printable case designs.

Example of access control PCB:

![Access2](https://user-images.githubusercontent.com/996983/170712038-56918a26-39f6-4e2f-869d-362451e7e87e.png)

## Back end management cloud system

Cloud back end for management of devices without the need for on-site controller. Runs on linux with mariadb and apache.

An instance of the cloud management system is run on https://access.me.uk/ by Andrews & Arnold Ltd, and provisioned DESFire cards can be ordered printed to your design.

Copyright © 2019-22 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
