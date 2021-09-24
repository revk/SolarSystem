// Inputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define MAXINPUT 16
typedef uint16_t input_t;       // Enough for number of inputs

extern input_t input_stable;    // holds current input state
extern input_t input_latch;     // inputs have changed state to on at some point
extern input_t input_flip;      // inputs have changed state at some point
extern char *inputname[MAXINPUT];
#define i(t,x,c) extern area_t input##x[MAXINPUT];
#include "states.m"

void input_set(int p, int v);
int input_get(int p);
int input_active(int p);
