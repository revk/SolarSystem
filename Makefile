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
	git push


PCBCase/case: PCBCase/case.c
	make -C PCBCase

scad:

stl: KiCad/Access.stl

%.stl: %.scad
	echo "Making $@"
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@
	echo "Made $@"

AXL/axl.o: AXL/axl.c
	make -C AXL
AJL/axl.o: AJL/axl.c
	make -C AJL
Dataformat/dataformat.o: Dataformat/dataformat.c
	make -C Dataformat
websocket/websocketxml.o: websocket/websocket.c
	make -C websocket
DESFireAES/desfireaes.o: DESFireAES/desfireaes.c
	make -C DESFireAES

cardissue: cardissue.c DESFireAES/desfireaes.o AXL/axl.o AJL/ajl.o afile.o
	gcc -g -Wall -Wextra -O -o cardissue cardissue.c -I. -IDESFireAES/include DESFireAES/desfireaes.o -IAXL AXL/axl.o -IAJL AJL/ajl.o -lcrypto -lpopt -pthread -lcurl -lmosquitto afile.o

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h port.o port.h afile.o door.o door.h AXL/axl.o AJL/ajl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o trace.h
	gcc -g -Wall -Wextra -O -o alarmpanel alarmpanel.c galaxybus.o port.o afile.o door.o -I. -IAXL -IAJL -IDataformat -Iwebsocket -IDESFireAES/include AXL/axl.o AJL/ajl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o -lcurl -pthread -lpopt -DLIBWS ${LIBEMAIL} ${LIBMQTT} -lcrypto -lssl

galaxybus.o: galaxybus.c galaxybus.h port.h
	gcc -g -Wall -Wextra -O -c -o galaxybus.o galaxybus.c -I. -DLIB -pthread

afile.o: afile.c afile.h
	gcc -g -Wall -Wextra -O -c -o afile.o afile.c -I. -IAXL -IDESFireAES/include -DLIB -pthread

door.o: door.c door.h galaxybus.h
	gcc -g -Wall -Wextra -O -c -o door.o door.c -I. -DLIB -pthread

port.o: port.c port.h galaxybus.h
	gcc -g -Wall -Wextra -O -c -o port.o port.c -I. -DLIB -pthread

clean:
	rm -f *.o alarmpanel

git:
	git submodule update --init
