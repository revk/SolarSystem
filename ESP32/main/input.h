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

int input_func_active(uint8_t);	// Does a function exist at all (expected to be one bit set)
int input_func_all(uint8_t);	// Are all inputs for a function set (expected to be one bit set)
const char *input_func_any(uint8_t);	// Are any inputs for a function set (expected to be one bit set) (returns input name)

#define	INPUT_FUNCS	"EOLDM"
#define	INPUT_FUNC_E	0x80	// Exit pressed
#define	INPUT_FUNC_O	0x40	// Open door
#define	INPUT_FUNC_L	0x20	// Lock unlocked
#define	INPUT_FUNC_D	0x10	// Deadlock unlocked
#define	INPUT_FUNC_M	0x08	// Manual active
