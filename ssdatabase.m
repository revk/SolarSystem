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

#ifndef time
#define time(n)                 // Time field
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
time(logintime);                // Login time
time(expires);                  // Session expiry
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
link(site);
area(tag);
unique(site, tag);
text(areaname, 0);

table(access,0);		// Fob access permissions
link(site);

join(access,aid);		// Access permissions per AID
areas(open);			// Allow open
areas(disarm);			// Allow disarm/arm

table(fob, 14);
time(provisioned);              // When provisioned
bool(format);			// Admin only - reformat fob when seen
num(mem);			// Free memory

join(fob, aid);                 // Fob is in AID (adopted)
time(adopted);			// When adopted
text(crc,8);			// Afile CRC
text(ver,2);			// Key version

join(fob, organisation);        // Yes, per org, for security reasons
time(blocked);                  // When blocked
time(confirmed);                // When confirmed blocked by fob read (no need to be in blacklist now)

table(device, 12);
text(devicename, 0);
link(organisation);		// Yes, can get from site, but useful to reference quickly
link(site);                     // The site the device is on
link(pcb);                      // What type of device this is
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
time(online);                   // When online, if online
time(lastonline);               // When last went offline
time(upgrade);                  // When to do upgrade
ip(address);                    // IP address when last online
num(id);                  // Instance for communications when on line

join(device,gpio)
gpiotype(type);			// Pin Usage (in/out/power/exit/etc)
bool (invert);                  // Invert normal polarity for pin
#define i(state) areas(state)
#define s(state) areas(state)
#include "ESP32/main/states.m"	// Related areas

table(pending, 12);
time(online);
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
gpio(blink);
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
#undef time
#undef ip
#undef gpio
#undef gpionfc
#undef gpiopcb
#undef gpiotype
#undef bool
#undef areas
#undef area
