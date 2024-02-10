// States

// Note, update settings_output.def for i() and s()
// Note, update settings_input.def for i() and c()

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
i(F,fault,M)
i(O,access,B)
i(I,presence,)
i(T,tamper,R)
i(X,fire,RY)
i(P,panic,RM)

c(+,arm)
c(*,strong)
c(-,disarm)

// @ used for name
// # used for count

#undef i
#undef s
#undef c
