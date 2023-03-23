# Web control server

## Cloud server
There is a cloud based server available at https://access.me.uk. Contact A&A for more details.

## Running your own server

The system is designed to operate using self contained devices on a site, but with the management and control provided using a back end (e.g. cloud) based server. The on-site devices form a private mesh WiFi network with one node acting as a gateway on a local Internet connected WiFi. This gateway allows the back end server access to all of the on-site devices. 

The back end server takes very few arguments and is normally run by simply running `solarsystem`. A `--debug` option provides information via *stderr*, otherwise the system runs as a daemon. Logging to *syslog* is also provided to a limited extent. Use `--help` to see other options. An example `solarsystem.service` file for `systemd` usage is included.

The server needs access to port 8883 (or other configured port) for incoming MQTT TLS connections from the on-site devices. Whilst the server talks MQTT, it is not an MQTT broker, and uses MQTT for communications to/from the remote devices only, so you cannot simply connect to it and snoop on all traffic, for example.

### Configuration

Most configuration is by means of *Kconfig*, use `make menuconfig` to set up the server. The web interface uses a `login` library, so use `make -C login menuconfig` to set up parameters for the web interface.

## Security

There are a number of aspects of the system that relate to security.
The system as a whole has users, fobs, customers, sites, devices, and so on. These relate to the permissions for who can do what. Web user permissions are finely controlled.
Each device has a client certificate, this is issued under a system wide CA with a common name of the device MAC address. The process for loading the key in to the device must be tightly controlled with no possibility of snooping - so on a local network only as part of setting up a device to be shipped to a site. The key is never stored apart from in the device itself.
Each door controller device is set for an application ID (AID). The AID is normally for a whole site, but a site can have more than one AID if needed. A fob can be set up in more than one AID (subject to memory available in the fob). There is a key for the AID that is known to devices using that AID, and the fobs - this is held in a separate database on the server, and a system exists for updates of this key when fobs are used.
Fobs are set with a master key - this is stored in a a separate database and must be kept secure. Once this is set it is possible to communicate with the fob securely including remotely loading new applications and keys without those being snooped on. However the initial config of a new blank fob with a master key is inherently insecure and must only be done locally, e.g. on a local network. Once this is done, a fob config can be done via any connected NFC reader. Obviously it is important to know who owns a fob and ensure that only they can update the fob.
The server has two databases, one for AES keys and one for everything else. The keys can only be accessed by the back end server and not the web interface user.
System structure
The system is structured with various types of object in database:-

User permissions for web access
Users of the system can register using and email address and can the edit their name, change email, change password (including forgotten password logic).
A user can then be adopted to an organisation, by email address. A user can be in more than one organisation, but all operations on the web control pages relate to a selected organisation.
The user/organisation has a set of permissions of what the user can do, e.g. caneditdevice. All things relate to an organisation, so this set of permissions can relate to edit or view of any aspect of the system for that organisation.
A user/organisation can also be marked admin, allowing everything, including a number of high level management functions not covered by the can… settings, for that organisation.
There is also a top level admin for system administrator (e.g. creating organisations and provisioning fobs).
User permissions for access
Access permissions, e.g. for doors and alarm arming and disarming, is set for a fob based on the access of the fob, in the aid. This includes which areas can be opened or disarmed as doors and access times. A fob can be in more than one aid, and the access is set separately for each aid.
Multiple AID on a site
An AID (application ID) is a 3 byte number that is unique across the whole system, and relates to only one site. Each device is in one AID. Each fob is programmed with one or more AIDs as needed. Normally a site has only one AID, however there may be reasons to deploy more than one AID.
The security relies on an AID having an AES key which has to be secret. However this is deployed to devices and fobs that need them. Both should be secure (fobs should have secure boot and encrypted storage), but you cannot completely rule out someone finding a way to extra the key if they have physical access. If the key is found the whole AID is no longer secure. As such it may be prudent for some devices (i.e. doors) to be on a separate AID if considered specifically sensitive for limited use - this would reduce the number of devices and fobs that get given the AES key for that AID.
