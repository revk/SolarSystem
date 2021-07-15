// Inputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

#define MAXINPUT 16
typedef uint16_t input_t;       // Enough for number of inputs

extern input_t input_latch;     // holds resettable state of input
extern input_t input_flip;      // holds flipped flag for each input, i.e. state has changed
#define i(x) extern area_t input##x[MAXINPUT];
#include "states.m"

void input_set(int p, int v);
int input_get(int p);
int input_active(int p);
