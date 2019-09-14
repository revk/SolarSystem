ifneq ("$(wildcard /projects/tools/c/libemail.c)","")
LIBEMAIL=-DLIBEMAIL -I/projects/tools/c/ /projects/tools/lib/libemaillight.o
else
LIBEMAIL=
endif

ifneq ("$(wildcard /usr/include/mosquitto.h)","")
LIBMQTT=-DLIBMQTT -lmosquitto
else
LIBMQTT=
endif

all: git alarmpanel cardissue

update:
	git submodule update --init --remote --merge
	git commit -a -m "Library update"

AXL/axl.o: AXL/axl.c
	make -C AXL
Dataformat/dataformat.o: Dataformat/dataformat.c
	make -C Dataformat
websocket/websocket.o: websocket/websocket.c
	make -C websocket
DESFireAES/desfireaes.o: DESFireAES/desfireaes.c
	make -C DESFireAES

cardissue: cardissue.c DESFireAES/desfireaes.o AXL/axl.o afile.o
	cc -g -Wall -Wextra -O -o cardissue cardissue.c -I. -IDESFireAES/include DESFireAES/desfireaes.o -IAXL AXL/axl.o -lcrypto -lpopt -pthread -lcurl -lmosquitto afile.o

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h port.o port.h afile.o door.o door.h AXL/axl.o Dataformat/dataformat.o websocket/websocket.o DESFireAES/desfireaes.o trace.h
	cc -g -Wall -Wextra -O -o alarmpanel alarmpanel.c galaxybus.o port.o afile.o door.o -I. -IAXL -IDataformat -Iwebsocket -IDESFireAES/include AXL/axl.o Dataformat/dataformat.o websocket/websocket.o DESFireAES/desfireaes.o -lcurl -pthread -lpopt -DLIBWS ${LIBEMAIL} ${LIBMQTT} -lcrypto -lssl

galaxybus.o: galaxybus.c galaxybus.h port.h
	cc -g -Wall -Wextra -O -c -o galaxybus.o galaxybus.c -I. -IAXL -DLIB -pthread

afile.o: afile.c afile.h
	cc -g -Wall -Wextra -O -c -o afile.o afile.c -I. -IAXL -IDESFireAES/include -DLIB -pthread

door.o: door.c door.h galaxybus.h
	cc -g -Wall -Wextra -O -c -o door.o door.c -I. -IAXL -DLIB -pthread

port.o: port.c port.h galaxybus.h
	cc -g -Wall -Wextra -O -c -o port.o port.c -I. -IAXL -DLIB -pthread

clean:
	rm -f *.o alarmpanel

git:
	git submodule update --init
