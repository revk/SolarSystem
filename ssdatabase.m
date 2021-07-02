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

#ifndef datetime
#define datetime(n)                 // Time field
#endif

#ifndef date
#define date(n)                 // Time field
#endif

#ifndef time0000
#define time0000(n)                 // Time field (default 00:00:00)
#endif

#ifndef time2359
#define time2359(n)                 // Time field (default 23:59:59)
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
text(username, 0);           // Users name
text(email, 0);                 // Users email - we expect unique
key(email, 128);
text(hash, 0);                  // Password hash
bool (admin);                   // Top level admin user - can do anything

table(session, 36);             // Login session
link(user);                     // Which user
datetime(logintime);                // Login time
datetime(expires);                  // Session expiry
index(expires);                 // Easier to delete
ip(loginip);                    // Login ip
text(useragent, 0);             // User agent
link(organisation);             // Current selected organisation
link(site);                     // Current selected site

join(user, organisation);
text(userorganisationname, 0);              // Job Title
bool (admin);                   // Customer level admin user - can do anything relating to this organisation
bool (caneditorganisation);
bool (caneditaccess);
bool (caneditsite);
bool (canedituser);
bool (caneditdevice);
bool (canadoptfob);
bool (caneditfob);
bool (caneditarea);
bool (canviewlog);

table(organisation, 0);         // Customer (may be more than one site)
text(organisationname, 0);

table(site, 0);                 // Site
link(organisation);
text(sitename, 0);
text(wifissid,0);		// Site WiFi settings
text(wifipass,0);		// Site WiFi settings
text(meshid, 12);               // Hex Mesh ID
// TODO key / crypto?

table(area, 0);
link(organisation);		// Quicker
link(site);
area(tag);
unique(site, tag);
text(areaname, 0);

table(access,0);		// Fob access permissions - available site wide, set on an aid
link(organisation)		// Quicker access
link(site);
text(accessname,0)
areas(open);			// Allow open
areas(arm);			// Allow disarm/arm
num(expiry);			// Auto expiry (days)
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
bool(clock);			// Ignore time if clock not set
bool(override);			// Open regardless
bool(log);			// Log access
bool(count);			// Count access
bool(commit);			// Commit log/count

table(fob, 14);
datetime(provisioned);              // When provisioned
bool(format);			// Admin only - reformat fob when seen
num(mem);			// Free memory

join(fob, aid);                 // Fob is in AID (adopted)
datetime(adopted);			// When adopted
text(ver,2);			// Key version
link(access);			// Access permissions

join(fob, organisation);        // Yes, per org, for security reasons
datetime(blocked);			// Block card (when blocked)
datetime(confirmed);		// When confirmed blocked
text(fobname,0);		// Name of fob user
datetime(expires);			// Fixed expiry (local time on server)

table(device, 12);
text(devicename, 0);
link(organisation);		// Yes, can get from site, but useful to reference quickly
link(site);                     // The site the device is on
link(pcb);                      // What type of device this is
text(iothost,0);		// IoT host for local MQTT
bool(iotstatedoor);		// Copy stuff to IoT
bool(iotstateinput);		// Copy stuff to IoT
bool(iotstateoutput);		// Copy stuff to IoT
bool(iotfobevent);		// Copy stuff to IoT
bool(nfc);			// Yes, can get from pcb, but useful to reference quickly
bool(nfcadmin);			// NFC reader for admin use, e.g. on a desk
bool (nfctrusted);              // Trusted device for fob provisioning
bool(door);			// This is a door
text(version, 0);               // S/w version
bool (encryptednvs);            // Built with encrypted NVS
bool (secureboot);              // Built with secure boot
bool (adoptnext);		// Adopt next unknown fob seen, if possible
bool (formatnext);		// Format next unknown fob seen, if possible
num(flash);                     // Flash size
link(aid);                      // The AID for door access (defines what site it is)
areas(area);   	                // Areas covered by this door
datetime(online);               // When online, if online
datetime(lastonline);           // When last went offline
datetime(upgrade);              // When to do upgrade
ip(address);                    // IP address when last online
num(id);	               	// Instance for communications when on line
datetime(poke);			// Poke settings
// TODO hold times, lock times, etc...
// TODO status, like door states, tamper, fault, etc

join(device,gpio)
gpiotype(type);			// Pin Usage (in/out/power/exit/etc)
bool (invert);                  // Invert normal polarity for pin
#define i(state) areas(state)
#define s(state) areas(state)
#include "ESP32/main/states.m"	// Related areas

table(pending, 12);
datetime(online);
ip(address);
num(id);
text(version, 0);               // S/w version
bool (encryptednvs);            // Built with encrypted NVS
bool (secureboot);              // Built with secure boot
bool (authenticated);           // New authenticated device
num(flash);                     // Flash size

table(pcb, 0);                  // PCB type
text(pcbname, 0);
gpio(tamper);                   // Fixed GPIOs
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
gpionfc(nfcred);                // NFC GPIO (actually NFC PCB specific, but set here, so PCB for PCB+NFC combination)
gpionfc(nfcamber);
gpionfc(nfcgreen);
gpionfc(nfctamper);
gpionfc(nfcbell);
gpionfc(nfccard);

table(gpio, 0);
link(pcb);
gpio(pin);
unique(pcb, gpio);
gpiopcb(io);
gpiotype(init);
text(pinname, 0);

table(aid, 6);                  // AID
link(organisation);
link(site);
text(aidname, 0);

#undef table
#undef join
#undef link
#undef unique
#undef key
#undef index
#undef text
#undef num
#undef datetime
#undef date
#undef time0000
#undef time2359
#undef ip
#undef gpio
#undef gpionfc
#undef gpiopcb
#undef gpiotype
#undef bool
#undef areas
#undef area
