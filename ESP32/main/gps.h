// GPS time reference
// Copyright © 2019-22 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

extern unsigned char gpslocked;
extern unsigned char gpsfixed;
extern double gpslat,gpslon;
void gps_send_status(void);
