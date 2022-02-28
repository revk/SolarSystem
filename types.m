// gpiotype

#ifndef	i
#define i(g,t)
#endif

#ifndef	o
#define o(g,t)
#endif

#ifndef g
#define g(g)
#endif

#ifndef n
#define n(g,i)
#endif

#ifndef io
#define io(g,t)
#endif

i(I,Input);
i(I1,Exit button);
i(I2,Door open contact);
i(I3,Lock disengaged);
i(I4,Deadlock disengaged);
o(P,Fixed output);
o(O,Output);
o(O1,Unlock output);
o(O2,Undeadlock output);
o(O3,Beep output);
o(O4,Error output);

io(IO,Input/Output);
io(I,Input);
io(O,Output);

g(2);
g(4);
g(5);
g(7);
g(8);
g(12);
g(13);
g(14);
g(15);
g(16);
g(17);
g(18);
g(19);
g(20);
g(21);
g(22);
g(23);
g(25);
g(26);
g(27);
g(32);
g(33);
g(34);
g(35);
g(36);
g(39);

n(30,48);
n(31,49);
n(32,50);
n(33,51);
n(34,52);
n(35,53);
n(71,54);
n(72,55);

#undef i
#undef o
#undef g
#undef n
#undef io
