// Alarm state
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define i(x,c) extern area_t state_##x; // system wide aggregated input states
#define	c(x) extern area_t control_##x; // local control flags
#define s(x,c) extern area_t state_##x; // system wide calculated states
#include "states.m"
extern area_t areaenter;
extern area_t areaarm;
extern area_t areastrongarm;
extern area_t areadisarm;
extern area_t areawarning;
extern area_t areafault;
extern area_t areatamper;
extern area_t areadeadlock;
extern area_t latch_tamper;
extern area_t live_tamper;
extern area_t latch_fault;
extern area_t live_fault;
extern area_t latch_warning;
extern area_t live_warning;
extern area_t latch_presence;
extern area_t live_presence;

void jo_area(jo_t j, const char *tag, area_t area);     // Store area
area_t jo_read_area(jo_t);      // Read area
void alarm_rx(const char *target, jo_t j);
void alarm_arm(area_t a, jo_t *);
void alarm_strongarm(area_t a, jo_t *);
void alarm_disarm(area_t a, jo_t *);
void send_sms(const char *fmt, ...);
area_t alarm_armed(void);       // What areas are, in effect, armed