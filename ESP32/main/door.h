// Autonomous door control
// Copyright © 2019 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

const char *door_fob(fob_t *);
uint8_t door_unlock(const uint8_t * a, const char *why);
uint8_t door_lock(const uint8_t * a);
