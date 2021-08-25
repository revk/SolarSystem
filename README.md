# SolarSystem

Modular access control and alarm system.

## ESP32 code for modules

Modules work as a mesh over WiFi, and one connects to a back end cloud management system. PCB designs are included (KiCad) as are 3D printable case designs.

Example of access control PCB.
![Access](https://user-images.githubusercontent.com/996983/119697498-26a1e900-be48-11eb-8498-32d888ed1305.png)

## Back end management cloud system

Cloud back end for management of devices without the need for on-site controller. Runs on linux with mariadb and apache.

An instance of the cloud management system is run on https://access.me.uk/ by Andrews & Arnold Ltd, and provisioned DESFire cards can be ordered printed to your design.

Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
