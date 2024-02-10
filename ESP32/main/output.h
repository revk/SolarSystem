// Outputs
// Copyright © 2019-21 Adrian Kennard, Andrews & Arnold Ltd. See LICENCE file for details. GPL 3.0

typedef uint8_t output_t;       // Enough for number of outputs

extern output_t output_forced;  // Output forced externally

void output_set (int p, int v);
int output_get (int p);
int output_active (int p);

int output_func_active (uint8_t);       // Does a function exist at all (expected to be one bit set)
void output_func_set (uint8_t, int v);  // Set all outputs for a function set (expected to be one bit set)
int output_func_get (uint8_t);  // Check logical output state for func (expected to be one bit set)

#ifdef  CONFIG_REVK_LED_STRIP
void led_set (int led, char c);
#endif
