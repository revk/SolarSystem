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

s(engineer,GY) // In order or importance
s(faulted,)
s(tampered,)
s(alarmed,)
s(prearm,)
s(armed,C)
s(prealarm,)
s(alarm,RB)

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


#undef i
#undef s
#undef c
