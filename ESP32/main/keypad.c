// Controller for Honeywell Galaxy keypad over RS485
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0
static const char TAG[] = "keypad";
#include "SS.h"
const char *keypad_fault = NULL;
const char *keypad_tamper = NULL;

#include "galaxybus.h"

const char *
keypad_command (const char *tag, unsigned int len, const unsigned char *value)
{
   // TODO
   return NULL;
}

void
keypad_init (void)
{
   // TODO
}
