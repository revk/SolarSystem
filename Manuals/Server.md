# Web control server

## Cloud server

There is a cloud based server available at [access.me.uk](https://access.me.uk). Contact A&A for more details - this is currently free of charge.

## Running your own server

The system is designed to operate using self contained devices on a site, but with the management and control provided using a back end (e.g. cloud) based server. The on-site devices form a private mesh WiFi network with one node acting as a gateway on a local Internet connected WiFi. This gateway allows the back end server access to all of the on-site devices.

### Environment

The server is intended to be installed on a linux system (tested on Debian) with *mariadb* SQL server, *apache* web server, *gcc* compilation environment with `make`, and *csh* or equiviliant shell available for scripts.

Using `make apt` in the top directory will `apt install` the main required applications.

### Git

Your will need to clone with `--recursive` to ensure you have the submodules. The example files such as `solarsystem.service` assume you have cloned in to `/projects/SolarSystem` so will need editing if using a different directory.

```
git clone --recursive https://github.com/revk/SolarSystem.git
```

### Server build

The server is build in the top level, using `make`. You can also use `make update` to update and buils all submodules.

The system uses `Kconfig` which can be edited using `make menuconfig` - mostly the defaults should work.

The server itself is `solarsystem` but would normally be run using `systemd`, and a `solarsystem.service` example is included for this. You will note the example simply runs `solarsystem` as user `adrian`. You will want to add a suitable user to your system and run as that user.

### ESP32 module code

The ESP32 code is build in the `ESP32` directory. In that directory you can use `make set` to build the full set of ESP32 modules.

You can edit the configuration using `make menuconfig`. Mostly the defaults should work, but you should consider the defaults such as `CONFIG_REVK_MQTTHOST` and `CONFIG_REVK_MQTTCERT` for your server.

You will need the full [ESP IDF environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) to make the ESP modules. The latest stable is recommended.

### Mariadb

Mariadb will need to be set up and two databases created `SS` and `SSKey`. The actual names can be configured in the server build configuration if you wish. You need to set grants for `SS` to be accessible by your server user, and by your apache user (e.g. `www-data`). You must set access to `SSKey` only to be your server user.

You can set explicit credentials for access in `~/.my.cnf` for the users. There is also a configuration options for these config files as part of the server build.

The software will make all of the necessary tables automatically, and software updates will add additional fields and make chanegs and needed.

However, you make like to create the `pcb` and `gpio` tables initially from the file `pcb.sql` as this contains the pinouts for the various boards and other devices commonly used with the system.

### Apache

An example `apache.conf` is included, and should be edited accordinly and put in `/etc/apache2/sites-available` for the site.

The site does not have to be publically accessible, but `https` is recommended, even if that is using local CA, etc.

### MQTT

The ESP32 devices connect to the server over `mqtts` on port 8883, using the server name. But this is not a conventional mqtt server in that it does not pass traffic between devices. A CA is created in `solarsystem.keys` and the `mqtt` key is created under that CA.

## Security

There are a number of aspects of the system that relate to security.

The system as a whole has *users*, *fobs*, *customers*, *sites*, *devices*, and so on. These relate to the permissions for who can do what. Web user permissions are finely controlled.

Each device has a client certificate, this is issued under a system wide CA with a common name of the device MAC address. The process for loading the key in to the device must be tightly controlled with no possibility of snooping - so on a local network only as part of setting up a device to be shipped to a site. The key is never stored apart from in the device itself.

When purchasing boards from A&A, the devices are re-set on `access.me.uk` with a key, but if using your own server you have to set this up yourself.

Each door controller device is set for an application ID (AID). The AID is normally for a whole site, but a site can have more than one AID if needed. A fob can be set up in more than one AID (subject to memory available in the fob). There is a key for the AID that is known to devices using that AID, and the fobs - this is held in a separate database on the server, and a system exists for updates of this key when fobs are used.

Fobs are set with a master key - this is stored in a separate database and must be kept secure. Once this is set it is possible to communicate with the fob securely including remotely loading new applications and keys without those being snooped on. However the initial config of a new blank fob with a master key is inherently insecure and must only be done locally, e.g. on a local network. Once this is done, a fob config can be done via any connected NFC reader. Obviously it is important to know who owns a fob and ensure that only they can update the fob.

When ordering fobs or printed cards from A&A a random master key is set and loaded on to `access.me.uk`. You can order cards without this done as well.

The server has two databases, one for AES keys and one for everything else. The keys can only be accessed by the back end server and not the web interface user.

## System structure

The system is structured with various types of object in database:-

|System|Meaning|
|------|-------|
|`user`|A user is a user of the web access system. This is keyed on an email address (which can be changed). A user can be assigned with different permissions to different organisations on the system.|
|`fob`|A fob relates to a key fob or card configured for the system. Fobs can be linked to one or more aids.|
|`organisation`|This is normally a company or group that owns one or more sites.|
|`site`|A site is normally a physical location and devices relate to one site. The key factor is that all devices on a site are programmed to work together and must be in range to operate on a mesh WiFi link. As such a very large campus may need to be broken down in to separate logical sites.|
|`area`|An area defines a grouping of devices on a site, usually a location such as a building or a floor. Devices can be in multiple areas, and as such areas can overlap and intersect as required. Areas are given letters `A` to `Z` which are used in logs and on displays, etc. so it is useful to try and make these mnemonic of the logical area where possible - the server allows names to be associated with each letter.|
|`aid`|An AID is a 3 byte code that only applies to a specific site. It is used to control the application files on fobs and the keys used by devices such as doors. A site usually has one AID, but can have more than one if needed.|
|`device`|A devices is one of the ESP32 modules which could be a door, or bell box, or keypad, or inputs or outputs on the system. A device has a specific site and aid and only works on that one aid. A device is set up with areas in various ways, e.g. areas allowed access, arming, or disarming, etc.|
|`access`|A class of access permissions for a fob in an aid. This includes times of day and areas allowed access.|

## User permissions for web access

Users of the system can register using an email address and can the edit their name, change email, change password (including forgotten password logic).

A user can then be allocated to one or more organisations, by email address. A user can be in more than one organisation, but all operations on the web control pages relate to a selected organisation.

The user/organisation has a set of permissions of what the user can do, e.g. `caneditdevice`. All things relate to an organisation, so this set of permissions can relate to edit or view of any aspect of the system for that organisation and user.

A user/organisation can also be marked admin, allowing everything, including a number of high level management functions not covered by the `can…` settings, for that organisation.

There is also a top level admin for system administrator (e.g. creating organisations and provisioning fobs).

## User permissions for access

Access permissions, e.g. for doors and alarm arming and disarming, is set for a fob based on the `access` mode of the `fob` in each `aid`. This includes which areas can be opened or disarmed as doors and access times. A fob can be in more than one aid, and the access is set separately for each aid.

## Multiple AID on a site

An AID (application ID) is a 3 byte number that is unique across the whole system, and relates to only one site. Each device is in one AID. Each fob is programmed with one or more AIDs as needed. Normally a site has only one AID, however there may be reasons to deploy more than one AID.

The security relies on an AID having an AES key which has to be secret. However this is deployed to devices and fobs that need them. Both should be secure (modules should have secure boot and encrypted storage), but you cannot completely rule out someone finding a way to extra the key if they have physical access. If the key is found the whole AID is no longer secure. As such it may be prudent for some devices (i.e. doors) to be on a separate AID if considered specifically sensitive for limited use - this would reduce the number of devices and fobs that get given the AES key for that AID.

## PCBs and devices

One of the jobs of the server is managing devices. When a device is connected it needs to be configured for a specific site.

The PCB templates allow a device's various settings, such as GPIO pins, to be defined. Then the configuration for the specific device allows each of its inputs and outputs to be assigned to specific rules and areas and states. The device setings also have a number of device specific timeouts and settings.

Devices can be marked out of service, and can also be upgraded, all from the web interface. It is usually sensible to mark all out of service so they connect directly to WiFi, then upgrade, then put back in service to work as a mesh again. However, if a site has devices that cannot reach wifi themselves they will need to be upgraded over the mesh - this is a lot slower but normally works.

It is recommended to keep devices up to date.
