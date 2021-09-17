// States

#ifndef i
#define i(t,n,c)	// Input states
#endif

#ifndef c
#define c(t,n)		// Control states
#endif

#ifndef s
#define s(t,n,c)	// Internal states
#endif

s(e,engineer,GY) // In order or importance
s(f,faulted,)
s(t,tampered,)
s(a,alarmed,)
s(S,armed,C)
s(s,prearm,)
s(p,prealarm,)
s(A,alarm,RB)

i(D,doorbell,W)
i(W,warning,Y)
i(E,fault,M)
i(O,access,B)
i(I,presence,)
i(T,tamper,R)
i(F,fire,RY)
i(P,panic,RM)

c(aa,arm)
c(sa,strongarm)
c(da,disarm)

#undef i
#undef s
#undef c
