// Database template

#ifndef table
#define table(n)		// Table (n is name, and field name for auto increment id of table as primary key)
#endif

#ifndef	link
#define	link(n)			// Foreign key link to table n
#endif

#ifndef text
#define text(n)			// Text field
#endif

table(user);

table(customer);

table(site);

table(fob);

table(device);

table(template);

#undef table
#undef link
#undef text
