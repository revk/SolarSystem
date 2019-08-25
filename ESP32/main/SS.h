// Common functions and data

#include "revk.h"

void status (const char *);     // Report change in tamper/fault (arg is ignored, but used xxx_fault="whatever");

#define	MAX_PORT	40
const char *port_check (int p, const char *module, int i);      // Check port is OK, NULL if OK, else error
extern uint8_t offlinemode;
