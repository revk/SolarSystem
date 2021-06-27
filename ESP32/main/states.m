// States

#ifndef i
#define i(n)	// Input states
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
s(arming);
s(armed);
s(disarming);
s(prealarm);
s(alarm);
s(alarmed);
s(faulted);
s(tampered);
s(engineering);

#undef i
#undef s
