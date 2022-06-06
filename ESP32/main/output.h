// Outputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define MAXOUTPUT 8             // Note 10 makes names too long
typedef uint8_t output_t;       // Enough for number of outputs

extern output_t output_forced;  // Output forced externally
#define i(t,x,c) extern area_t output##x[MAXOUTPUT];
#define s(t,x,c) extern area_t output##x[MAXOUTPUT];
#include "states.m"

void output_set(int p, int v);
int output_get(int p);
int output_active(int p);

int output_func_active(uint8_t); // Does a function exist at all (expected to be one bit set)
void output_func_set(uint8_t,int v);    // Set all outputs for a function set (expected to be one bit set)

#define	OUTPUT_FUNCS	"LDBE"
#define	OUTPUT_FUNC_L	0x80	// Lock release
#define	OUTPUT_FUNC_D	0x40	// Deadlock release
#define	OUTPUT_FUNC_B	0x20	// Beep
#define	OUTPUT_FUNC_E	0x10	// Error

