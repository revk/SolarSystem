// Autonomous door control
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

extern uint8_t door;

uint32_t door_crc (void);
const char *door_fob (char *id, uint32_t * crcp);
uint8_t door_unlock (const uint8_t * a);
