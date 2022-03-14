// GPS time reference
// Copyright © 2019-22 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

extern unsigned char gpslocked; // Time
extern unsigned char gpsfixed; // Location
extern unsigned char gpstime; // Remote time from GPS
extern double gpslat,gpslon; // Where
void gps_send_status(void);
