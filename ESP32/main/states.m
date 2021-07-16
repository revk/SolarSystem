// States

#ifndef i
#define i(n,c)	// Input states
#endif

#ifndef c
#define c(n)	// Control states
#endif

#ifndef s
#define s(n,c)	// Internal states
#endif

i(doorbell,)
i(warning,GY-)
i(fault,BY-)
i(access,GM-)
i(presence,)
i(tamper,M-)
i(fire,RR-RR-)
i(panic,RG-)

c(arm)
c(forcearm)
c(disarm)

s(prearm,)
s(armed,Y-)
s(prealarm,)
s(alarm,RR-BB-)
s(alarmed,W-)
s(faulted,)
s(tampered,)
s(engineer,)

#undef i
#undef s
#undef c
