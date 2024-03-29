// Common functions and data
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#include "revk.h"
#include "esp_task_wdt.h"
#include "areas.h"

//#if     ESP_IDF_VERSION_MAJOR > 5 || ESP_IDF_VERSION_MAJOR == 5 && ESP_IDF_VERSION_MINOR > 0
//#error  Not safe with ESP IDF 5.1 or greater yet - memory issues
//#endif

#define	LOGIC_PORT	48
#define	LOGIC_PORT2	56
extern uint32_t logical_gpio;   // Logical GPIO (from GPIO 48, covers NFC, keypad...)
#define l(t,g)	static const uint32_t logical_##t=(1<<(((g<0?16-g:g))-LOGIC_PORT));
#include "logicalgpio.m"

const char *app_callback (int client, const char *prefix, const char *target, const char *suffix, jo_t j);
const char *port_check (uint8_t p, const char *module, uint8_t i);      // Check port is OK, NULL if OK, else error
uint8_t bcdutctime (time_t now, uint8_t datetime[7]);
uint8_t bcdlocaltime (time_t now, uint8_t datetime[7]);
