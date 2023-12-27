// Alarm state
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define i(t,x,c) extern area_t state_##x;       // system wide aggregated input states
#define s(t,x,c) extern area_t state_##x;       // system wide calculated states
#define	c(t,x) extern area_t control_##x;       // local control flags
#include "states.m"
typedef enum
{
#define i(t,x,c) priority_##x,
#define s(t,x,c) priority_##x,
#include "states.m"
   priorities
} priority_t;
extern const char *state_name[priorities];

#define	MAX_MIX 5

extern area_t arealed;
extern area_t areaenter;
extern area_t areaarm;
extern area_t areastrong;
extern area_t areadisarm;
extern area_t areadeadlock;
extern area_t areakeypad;
extern area_t areakeydisarm;
extern area_t areakeystrong;
extern area_t areakeyarm;

char *area_list (char set[sizeof (area_t) * 8 + 1], area_t area);
void jo_area (jo_t j, const char *tag, area_t area);    // Store area
area_t jo_read_area (jo_t);     // Read area
void alarm_rx (const char *target, jo_t j);
void alarm_arm (area_t a, jo_t *);
void alarm_strong (area_t a, jo_t *);
void alarm_disarm (area_t a, jo_t *);
void send_sms (const char *number, const char *fmt, ...);
area_t alarm_armed (void);      // What areas are, in effect, armed
area_t andset (area_t a);       // Do and/set logic
const char *alarm_event (const char *event, jo_t * jp, char copy);
