// States

#ifndef i
#define i(n)	// Input states
#endif

#ifndef c
#define c(n)	// Control states
#endif

#ifndef s
#define s(n)	// Internal states
#endif

i(doorbell)
i(warning)
i(fault)
i(access)
i(presence)
i(tamper)
i(fire)
i(panic)

c(arm)
c(disarm)

s(prearm)
s(armed)
s(prealarm)
s(alarm)
s(alarmed)
s(faulted)
s(tampered)
s(engineer)

#undef i
#undef s
#undef c
