// Outputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define MAXOUTPUT 8             // Note 10 makes names too long
typedef uint8_t output_t;       // Enough for number of outputs

extern output_t output_forced;  // Output forced externally
#define i(x,c) extern area_t output##x[MAXOUTPUT];
#define s(x,c) i(x,c)
#include "states.m"

void output_set(int p, int v);
int output_get(int p);
int output_active(int p);
