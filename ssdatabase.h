// Database template

#ifndef table
#define table(n,l)              // Table (n is name, and field name for auto increment id if l=0, or fixed string len l, as primary key)
#endif

#ifndef	link
#define	link(n)                 // Foreign key link to table n
#endif

#ifndef	unique
#define unique(a,b)             // Unique key for a/b
#endif

#ifndef	key
#define key(n,l)             // Unique key for a(l)
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
#define gpio(n)                 // GPIO number field
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

table(user, 0);                 // Users of the system
text(description, 0);           // Users name
text(email,0);			// Users email - we expect unique
key(email,128);			
text(hash,0);			// Password has
bool (canprovisiondevice);      // Global permission - admin who can provision new devices
bool (canprovisionfob);         // Global permission - admin who can provision new fobs

table(session,36);		// Login session
link(user);			// Which user
time(logintime);		// Login time
ip(loginip);			// Login ip

table(usersite, 0);             // Which sites a user has access to
link(user);
link(site);
unique(user, site);

table(useraid, 0);              // User capabilities per AID
areas(access);                  // Where user is allowed access
areas(arm);                     // Where user is allowed to arm/disarm
bool (dooroverride);            // Override all door controls
bool (doorclock);               // Override time when clock not set on door
bool (doorblock);               // Block this card

table(siteaid, 0);              // AIDs linked to a site (must belong to customer)
link(site);
link(aid);
unique(site, aid);

table(usercustomer, 0);         // User link to customers, many to many
link(user);
link(customer);
unique(user, customer);

table(customer, 0);             // Customer (may be more than one site)
text(description, 0);

table(site, 0);                 // Site
link(customer);

table(sitearea, 0);             // Describe a site's areas
link(site);
text(area, 1);
unique(site, area);
text(description, 0) table(fob, 14);
link(user);                     // The Fobs user
text(key, 32);                  // Fob AES master key

table(device, 12);
text(description, 0);
link(pcb);                      // What type of device this is
link(site);                     // What site this is on
link(aid);                      // The AID for door access
text(deport, 0);                // Send this device to another MQTT server
areas(doorarea);                // Areas covered by this door
time(online);                   // When online, if online
time(lastonline);               // When last went offline
time(upgrade);                  // When to do upgrade
ip(address);                    // IP address when last online
num(instance);                  // Instance for communications when on line

table(devicegpio, 0);
link(device);
gpio(gpio);
unique(device, gpio);
gpiotype(type);
bool (polarity);
num(ionum);
#define i(state) areas(state)
#define s(state) areas(state)
#include "ESP32/main/states.h"

table(pending, 12);
time(online);
ip(address);
num(instance);

table(pcb, 0);                  // PCB type
text(description, 0);

table(pcbgpio, 0);
link(pcb);
gpio(gpio);
unique(pcb, gpio);
gpiotype(type);
bool (invert);
text(description, 0);

table(aid, 6);                  // AID (linked to customer)
link(customer) text(key, 32);

table(fobaid, 0);               // Holds FOB AID key
link(fob);
link(aid);
unique(fob, aid);
text(key, 32);

#undef table
#undef link
#undef unique
#undef key
#undef text
#undef num
#undef time
#undef ip
#undef gpio
#undef gpiotype
#undef bool
#undef areas
