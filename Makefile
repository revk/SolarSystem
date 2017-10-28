ifneq ("$(wildcard /projects/tools/c/libemail.c)","")
LIBEMAIL=-DLIBEMAIL -I/projects/tools/c/ /projects/tools/lib/libemaillight.o
else
LIBEMAIL=
endif

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h ../AXL/axl.o ../Dataformat/dataformat.o ../websocket/websocket.o trace.h
	cc -g -Wall -Wextra -O -o alarmpanel alarmpanel.c galaxybus.o -I. -I../AXL -I../Dataformat -I../websocket ../AXL/axl.o ../Dataformat/dataformat.o ../websocket/websocket.o -lcurl -pthread -lpopt -DLIBWS ${LIBEMAIL} -lcrypto -lssl

galaxybus.o: galaxybus.c
	cc -g -Wall -Wextra -O -c -o galaxybus.o galaxybus.c -I. -DLIB -pthread

clean:
	rm -f *.o alarmpanel
