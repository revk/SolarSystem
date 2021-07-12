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

i(doorbell);
i(warning);
i(fault);
i(access);
i(presence);
i(tamper);
i(fire);
i(panic);

c(arm);
c(unarm);
c(unalarm);
c(untamper);
c(unfault);

s(prearm);
s(armed);
s(prealarm);
s(alarm);
s(alarmed);
s(faulted);
s(tampered);
s(engineering);

#undef i
#undef s
#undef c
