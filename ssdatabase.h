// Database template

#ifndef table
#define table(n,l)              // Table (n is name, and field name for auto increment id if l=0, or fixed string len l, as primary key)
#endif

#ifndef	link
#define	link(n)                 // Foreign key link to table n
#endif

#ifndef text
#define text(n,l)               // Text field (l set for fixed size)
#endif

#ifndef num
#define num(n)                  // Simple numeric field (integer)
#endif

table(user, 0);

table(usersite, 0);
link(user);
link(site);

table(usercustomer, 0);
link(user);
link(customer);

table(customer, 0);

table(site, 0);
link(customer);

table(sitearea,0);
link(site);
text(area,1);

table(fob, 14);
link(user);

table(device, 12);
link(site);

table(template, 0);

table(templategpio, 0);
link(template);
num(pin);
num(gpio);
text(description, 0);

table(aid, 6);
text(key, 32);

table(fobaid, 0);
link(fob);
link(aid);
text(key, 32);

#undef table
#undef link
#undef text
#undef num
