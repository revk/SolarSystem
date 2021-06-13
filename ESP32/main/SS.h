// Common functions and data
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#include "revk.h"
#include "esp_task_wdt.h"
#include "areas.h"
#include "states.h"

void status (const char *);     // Report change in tamper/fault (arg is ignored, but used xxx_fault="whatever");

#define	MAX_PORT	40
const char *port_check (int p, const char *module, int i);      // Check port is OK, NULL if OK, else error
uint8_t bcdtime (time_t now, uint8_t datetime[7]);

extern area_t area;
