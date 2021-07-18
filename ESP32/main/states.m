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

i(doorbell,W)
i(warning,Y)
i(fault,M)
i(access,B)
i(presence,)
i(tamper,R)
i(fire,RY)
i(panic,RM)

c(arm)
c(strongarm)
c(disarm)

s(prearm,)
s(armed,C)
s(prealarm,)
s(alarm,RB)
s(alarmed,)
s(faulted,)
s(tampered,)
s(engineer,)

#undef i
#undef s
#undef c
