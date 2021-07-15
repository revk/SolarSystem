// Outputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define MAXOUTPUT 9             // As 10 makes names too long

extern uint64_t output_forced;  // Output forced externally
#define i(x) extern area_t output##x[MAXOUTPUT];
#define s(x) i(x)
#include "states.m"

void output_set(int p, int v);
int output_get(int p);
int output_active(int p);
