// Database template

#ifndef table
#define table(n,l)              // Table (n is name, and field name for auto increment id if l=0, or fixed string len l, as primary key)
#endif

#ifndef	join
#define	join(a,b)
#endif

#ifndef	link
#define	link(n)                 // Foreign key link to table n
#endif

#ifndef	link2
#define	link2(a,b)              // Foreign key link to table n under different name
#endif

#ifndef	unique
#define unique(a,b)             // Unique key for a/b
#endif

#ifndef	key
#define key(n,l)                // Unique key for a(l)
#endif

#ifndef	index
#define index(n)                // Simple index
#endif

#ifndef text
#define text(n,l)               // Text field (l set for fixed size)
#endif

#ifndef num
#define num(n)                  // Numeric field (integer)
#endif

#ifndef numd
#define numd(n,d)                  // Numeric field (integer) with default
#endif

#ifndef datetime
#define datetime(n)             // Time field
#endif

#ifndef date
#define date(n)                 // Time field
#endif

#ifndef time0000
#define time0000(n)             // Time field (default 00:00:00)
#endif

#ifndef time2359
#define time2359(n)             // Time field (default 23:59:59)
#endif

#ifndef ip
#define ip(n)                   // IP field
#endif

#ifndef gpio
#define gpio(n)                 // GPIO number field (and inverted)
#endif

#ifndef gpionfc
#define gpionfc(n)              // NFC GPIO number field (and inverted)
#endif

#ifndef gpiopcb
#define gpiopcb(n)              // PCB template GPIO type
#endif

#ifndef gpiotype
#define gpiotype(n)             // GPIO assignment field
#endif

#ifndef gpiofunc
#define gpiofunc(n)             // GPIO function codes
#endif

#ifndef bool
#define bool(n)                 // Boolean field
#endif

#ifndef areas
#define areas(n)                // Area list
#endif

#ifndef area
#define area(n)                 // Area
#endif

table(user, 0);                 // Users of the system (web)
text(username, 0);              // Users name
text(email, 0);                 // Users email - we expect unique
key(email, 128);
text(hash, 0);                  // Password hash
bool (admin);                   // Top level admin user - can do anything
link(organisation);             // Selected organisation
link(site);                     // Selected site

table(session, 36);             // Login session
link(user);                     // Which user
datetime(logintime);            // Login time
datetime(expires);              // Session expiry
index(expires);                 // Easier to delete
ip(loginip);                    // Login ip
text(useragent, 0);             // User agent

join(user, organisation);
text(userorganisationname, 0);  // Job Title
bool (admin);                   // Customer level admin user - can do anything relating to this organisation
bool (caneditorganisation);     // Can edit organisation
bool (caneditaccess);           // Can edit access
bool (caneditsite);             // Can edit sites
bool (canedituser);             // Can edit users
bool (caneditdevice);           // Can edit devices
bool (canviewdevice);           // Can view devices
bool (canadoptfob);             // Can adopt fobs
bool (caneditfob);              // Can edit fobs
bool (caneditarea);             // Can edit areas
bool (canarm);                  // Can arm
bool (canstrong);               // Can strong arm
bool (candisarm);               // Can disarm
bool (canunlock);               // Can unlock doors
bool (canviewlog);              // Can view logs
bool (canapi);                  // Can access API
bool (apiexpires);              // Can set fob expiries via API
bool (apiarm);                  // Can arm sites via API
bool (apistrong);               // Can strong arm sites via API
bool (apidisarm);               // Can disarm sites via API

table(organisation, 0);         // Customer (may be more than one site)
text(organisationname, 0);

table(site, 0);                 // Site
link(organisation);
num(rollover);			// Number of days for auto rollover
text(sitename, 0);		// Site name
text(wifissid, 32);             // Site WiFi setting
text(wifibssid, 12);            // Site WiFi setting
text(wifipass, 32);             // Site WiFi setting
num(wifichan);                  // Site WiFi setting
text(iottopic, 16);		// Topic for local mqtt
text(iothost, 64);              // IoT host for local MQTT
bool (nomesh);                  // Don't mesh - i.e. where separate devices on-line
text(meshid, 12);               // Mesh ID (MAC)
key(meshid, 12);
text(meshpass, 32);             // Mesh WiFi passphrase
text(meshkey, 32);              // AES key
bool (meshlr);                  // Mesh is LR
key(meshkey, 32);
link2(device, root);            // Preferred root
text(smsuser, 16);		// Credentials
text(smspass, 32);
text(smsfrom, 20);              // From, where settable by carrier
text(sms2user, 16);		// Credentials for secondary SMS (e.g. important)
text(sms2pass, 32);
text(sms2from, 20);             // From, where settable by carrier
text(toothost,0);		// Server for mastodon
text(tootbearer,0);		// Bearer for mastodon
text(hookoffline, 0);           // Off line devices
text(hookbearer, 0);            // Bearer for web hooks
text(hookfob, 0);               // Fob event web hook
text(hookfobdeny, 0);           // Fob event web hook when deny set
text(hookalarm, 0);             // Alarm event web hook
text(hookpanic, 0);             // Panic event web hook
text(hookfire, 0);              // Fire event web hook
text(hooktamper, 0);            // Tamper event web hook
text(hookwarning, 0);           // Warning event web hook
text(hookfault, 0);             // Fault event web hook
text(hooktrigger, 0);           // Trigger event web hook
text(hookinhibit, 0);           // Inhibit event web hook
text(hookarm, 0);               // Arm event web hook
text(hookstrong, 0);            // Strongarm event web hook
text(hookdisarm, 0);            // Disarm event web hook
text(hookarmfail, 0);           // Arm fail event web hook
text(hooknotclose, 0);          // Not close event web hook
text(hooknotopen, 0);           // Not open event web hook
text(hookopen, 0);              // Open event web hook
text(hookforced, 0);            // Forced event web hook
text(hookpropped, 0);           // Propped event web hook
text(hookwrongpin, 0);          // Wrong PIN entry
text(status, 0);                // Site level status
num(nodes);                     // Site reported nodes
num(missing);                   // Site reported nodes missing
num(armcancel);                 // Pre arm time
num(armdelay);                  // Pre arm delay
num(alarmdelay);                // Pre alarm time
num(alarmhold);                 // Hold alarm state time
numd(mqttdied,600);			// MQTT restart time
bool (ioteventarm);             // Copy stuff to IoT
bool (iotstatesystem);          // Copy stuff to IoT
#define s(t,state,c) areas(state)
#include "ESP32/main/states.m"  // Related areas
text(smsnumber, 20);            // Number to SMS
text(emailfrom, 0);             // From email
areas(smsarm);                  // When to SMS
areas(smsdisarm);
areas(smscancel);
areas(smsarmfail);
areas(smsalarm);
areas(smspanic);
areas(smsfire);
areas(mixand1);
areas(mixset1);
areas(mixand2);
areas(mixset2);
areas(mixand3);
areas(mixset3);
areas(mixand4);
areas(mixset4);
areas(mixand5);
areas(mixset5);
bool (debug);                   // MQTT debug logging

table(area, 0);
link(organisation);             // Quicker
link(site);
area(tag);
unique(site, tag);
text(areaname, 0);

table(access, 0);               // Fob access permissions - available site wide, set on an aid
link(organisation);             // Quicker access
link(site);
text(accessname, 0);
areas(enter);                   // Allow enter
areas(arm);                     // Allow arm
areas(strong);                  // Allow strong arm
areas(prop);                    // Allow prop
areas(disarm);                  // Allow disarm
num(expiry);                    // Auto expiry (days)
time0000(sunfrom);
time2359(sunto);
time0000(monfrom);
time2359(monto);
time0000(tuefrom);
time2359(tueto);
time0000(wedfrom);
time2359(wedto);
time0000(thufrom);
time2359(thuto);
time0000(frifrom);
time2359(frito);
time0000(satfrom);
time2359(satto);
bool (clock);                   // Ignore time if clock not set
bool (override);                // Open regardless
bool (armlate);                 // Out of hours arming
bool (log);                     // Log access
bool (count);                   // Count access
bool (commit);                  // Commit log/count

table(fob, 14);
datetime(provisioned);          // When provisioned
num(capacity);                  // Total memory capacity
num(mem);                       // Free memory

join(fob, aid);                 // Fob is in AID (adopted)
datetime(adopted);              // When adopted
datetime(firstinday);		// First used in day
datetime(lastused);		// Last used
text(ver, 2);                   // Key version
link(access);                   // Access permissions

join(fob, organisation);        // Yes, per org, for security reasons
datetime(blocked);              // Block card (when blocked)
datetime(confirmed);            // When confirmed blocked
text(fobname, 15);              // Name of fob user
text(fobsms, 20);               // SMS of the fob user
text(fobdata,0);		// User data for fob logs, etc
datetime(expires);              // Fixed expiry (local time on server)

table(device, 12);
link2(device, via);             // Via another device using mesh
text(devicename, 16);           // Device name
text(keypadidle, 16);           // Keypad display name
text(keypadpin, 16);            // Keypad disarm pin
link(organisation);             // Yes, can get from site, but useful to reference quickly
link(site);                     // The site the device is on
link(pcb);                      // What type of device this is
bool (outofservice);            // Marked out of service
bool (excludeall);              // Don't include in all functions
bool (iotstatedoor);            // Copy stuff to IoT
bool (iotstateinput);           // Copy stuff to IoT
bool (iotstateoutput);          // Copy stuff to IoT
bool (ioteventfob);             // Copy stuff to IoT
bool (iotkeypad);               // Copy stuff to IoT
bool (iotgps);                  // Copy stuff to IoT
bool (nfc);                     // Yes, can get from pcb, but useful to reference quickly
bool (gps);                     // Yes, can get from pcb, but useful to reference quickly
bool (rgb);                     // Yes, can get from pcb, but useful to reference quickly
bool (nfcadmin);                // NFC reader for admin use, e.g. on a desk
bool (nfctrusted);              // Trusted device for fob provisioning
bool (door);                    // This is a door
bool (doorexitarm);             // Exit button hold arm
bool (doorexitdisarm);          // Exit button disarm
bool (doordebug);               // Lock state debug
bool (doorcatch);               // Main lock catch logic (re-engage on opening)
bool (keypad);                  // Keypad
text(status, 0);                // Summary status from device
text(version, 0);               // S/w version
text(build, 0);                 // S/w build
text(build_suffix, 0);          // S/w build suffix
num(rst);			// Last rst reason
num(mem);			// LWM mem
num(spi);			// LWM spi mem
num(chan);                      // WiFi channel
text(bssid, 0);                 // WiFi SSID
text(ssid, 0);                  // WiFi SSID
bool (encryptednvs);            // Built with encrypted NVS
bool (secureboot);              // Built with secure boot
num(flash);                     // Flash size
link(aid);                      // The AID for door access (defines what site it is)
areas(areaenter);               // Areas allowed enter by fob
areas(areaarm);                 // Areas allowed arm by fob
areas(areastrong);              // Areas allowed arm by fob
areas(areadisarm);              // Areas allowed disarm by fob
areas(areadeadlock);            // Areas for deadlock input
areas(arealed);                 // LED area
areas(areakeypad);              // Keypad area
areas(areakeyarm);              // Keypad arm using A
areas(areakeystrong);           // Keypad strong using B
areas(areakeydisarm);           // Keypad disarm using PIN
datetime(online);               // When online, if online
datetime(boot);                 // Boot time
index(online);
datetime(lastonline);           // When last went offline
index(lastonline);
datetime(offlinereport);        // When reported offline
index(offlinereport);
text(offlinereason, 0);         // If sent offline reason, what was it
datetime(upgrade);              // When to do upgrade
num(progress);                  // Upgrade percent
index(upgrade);
ip(address);                    // IP address when last online
num(id);                        // Instance for communications when on line
datetime(poke);                 // Poke settings
index(poke);
num(doorunlock);                // Door timer
num(doorlock);                  // Door timer
num(dooropen);                  // Door timer
num(doorclose);                 // Door timer
num(doorprop);                  // Door timer
num(doorexit);                  // Door timer
num(doorpoll);                  // Door timer
num(doordebounce);              // Door timer
text(dooriotopen, 64);          // IoT on open
text(dooriotdead, 64);          // IoT on deadlock
text(dooriotundead, 64);        // IoT on undeadlock
text(dooriotlock, 64);          // IoT on lock
text(dooriotunlock, 64);        // IoT on unlock
time0000(timer1);		// Timer1 event (active as a logical input for 1 minute)

join(device, gpio);
index(device);
gpiotype(type);                 // Pin Usage (in/out/power)
gpiofunc(func);			// Pin Function codes (exit/lock/etc)
bool (invert);                  // Invert normal polarity for pin
text(name, 16);                 // Port name, default to pin name
num(hold);                      // Port hold time
num(pulse);                     // Port output time
#define i(t,state,c) areas(state)
#define c(t,state) areas(state)
#define s(t,state,c) areas(state)
#include "ESP32/main/states.m"  // Related areas

table(pending, 12);
datetime(online);
ip(address);
num(id);
text(version, 0);               // S/w version
text(build_suffix, 0);          // S/w build suffix
text(build, 0);                 // S/w build
num(chan);                      // WiFi channel
text(bssid, 0);                 // WiFi SSID
text(ssid, 0);                  // WiFi SSID
bool (encryptednvs);            // Built with encrypted NVS
bool (secureboot);              // Built with secure boot
bool (authenticated);           // New authenticated device
num(flash);                     // Flash size
num(rst);			// Last rst reason
num(mem);			// LWM mem
num(spi);			// LWM spi mem

table(pcb, 0);                  // PCB type
text(pcbname, 0);
gpio(leda);
gpio(ledr);
gpio(ledg);
gpio(ledb);
gpio(keypadtx);
gpio(keypadrx);
gpio(keypadde);
gpio(keypadre);
gpio(nfctx);
gpio(nfcrx);
gpio(nfcpower);
numd(nfcbaud,4);
gpio(gpstx);
gpio(gpsrx);
gpio(gpstick);
gpionfc(nfcred);                // NFC GPIO (actually NFC PCB specific, but set here, so PCB for PCB+NFC combination)
gpionfc(nfcamber);
gpionfc(nfcgreen);
gpionfc(nfccard);

table(gpio, 0);
link(pcb);
gpio(pin);
unique(pcb, gpio);
gpiopcb(io);
gpiotype(inittype);
gpiofunc(initfunc);
text(initname, 16);             // Default port pin name
num(inithold);                  // Default port hold time
num(initpulse);                 // Default port pulse time
bool (initinvert);              // Default port invert setting
text(value0, 0);                // Value name for 0
text(value1, 0);                // Value name for 1
num(rgb);			// RGB LED number

table(aid, 6);                  // AID
link(organisation);
link(site);
text(aidname, 0);
datetime(ver1date)		// The date/time of current key
text(ver1, 2);			// The Key versions that exist, in order, starting with current key
datetime(ver2date)		// The date/time of key
text(ver2, 2);
datetime(ver3date)		// The date/time of key
text(ver3, 2);

table(event, 0);                // Logging
link(device);
link(fob);
datetime(logged);
text(suffix, 0);
text(data, 0);

#undef table
#undef join
#undef link
#undef link2
#undef unique
#undef key
#undef index
#undef text
#undef num
#undef numd
#undef datetime
#undef date
#undef time0000
#undef time2359
#undef ip
#undef gpio
#undef gpionfc
#undef gpiopcb
#undef gpiotype
#undef gpiofunc
#undef bool
#undef areas
#undef area
