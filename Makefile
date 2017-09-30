ifneq ("$(wildcard /projects/tools/c/libemail.c)","")
LIBEMAIL=-DLIBEMAIL -I/projects/tools/c/ /projects/tools/lib/libemaillight.o
else
LIBEMAIL=
endif

ifneq ("$(wildcard /projects/tools/c/libemail.c)","")
LIBWS=-DLIBWS -I/projects/tools/c/ /projects/tools/lib/websocket.o -lcrypto -lssl
else
LIBWS=
endif

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h ../AXL/axl.o ../AXL/axl.h ../Dataformat/dataformat.o ../Dataformat/dataformat.h trace.h
	cc -Wall -Wextra -O -o alarmpanel alarmpanel.c galaxybus.o -I. -I../AXL -I ../Dataformat ../AXL/axl.o ../Dataformat/dataformat.o -lcurl -lexpat -pthread -lpopt ${LIBEMAIL} ${LIBWS} -g

galaxybus.o: galaxybus.c
	cc -Wall -Wextra -O -c -o galaxybus.o galaxybus.c -I. -DLIB -pthread -g

clean:
	rm -f *.o alarmpanel
