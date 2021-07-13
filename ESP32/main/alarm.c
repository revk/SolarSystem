// Alarm management
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char __attribute__((unused)) TAG[] = "alarm";
#include "SS.h"
const char *alarm_fault = NULL;
const char *alarm_tamper = NULL;

// TODO keypad UI
// TODO commands to clean latched states

#define settings	\

#define u16(n,v) uint16_t n
settings
#undef u16

const char *alarm_command(const char *tag, jo_t j)
{
   return NULL;
}

void alarm_init(void)
{
#include "states.m"
#define u16(n,v) revk_register(#n,0,sizeof(n),&n,#v,0);
   settings
#undef u16
}
