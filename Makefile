ifneq ("$(wildcard /projects/tools/c/libemail.c)","")
LIBEMAIL=-DLIBEMAIL -I/projects/tools/c/ /projects/tools/lib/libemaillight.o
else
LIBEMAIL=
endif

all: git alarmpanel

update:
	git submodule update --remote --merge

AXL/axl.o: AXL/axl.c
	make -C AXL
Dataformat/dataformat.o: Dataformat/dataformat.c
	make -C Dataformat
websocket/websocket.o: websocket/websocket.c
	make -C websocket

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h AXL/axl.o Dataformat/dataformat.o websocket/websocket.o trace.h
	cc -g -Wall -Wextra -O -o alarmpanel alarmpanel.c galaxybus.o -I. -IAXL -IDataformat -Iwebsocket AXL/axl.o Dataformat/dataformat.o websocket/websocket.o -lcurl -pthread -lpopt -DLIBWS ${LIBEMAIL} -lcrypto -lssl

galaxybus.o: galaxybus.c
	cc -g -Wall -Wextra -O -c -o galaxybus.o galaxybus.c -I. -DLIB -pthread

clean:
	rm -f *.o alarmpanel

git:
	git submodule update --init
