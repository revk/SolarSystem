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

scad: KiCad/Controller.scad KiCad/Access.scad
stl: KiCad/Controller.stl KiCad/Access.stl

%.stl: %.scad
	echo "Making $@"
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@
	echo "Made $@"

KiCad/Controller.scad: KiCad/Controller.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --wall=4 --top=4.9

KiCad/Access.scad: KiCad/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=1 --wall=3 --top=4.9 --overlap=3 --margin=0.4 --edge=2 --width=50
	echo "translate([4,29,0])cube([50,5,0.6]);" >> $@

AXL/axl.o: AXL/axl.c
	make -C AXL
Dataformat/dataformat.o: Dataformat/dataformat.c
	make -C Dataformat
websocket/websocketxml.o: websocket/websocket.c
	make -C websocket
DESFireAES/desfireaes.o: DESFireAES/desfireaes.c
	make -C DESFireAES

cardissue: cardissue.c DESFireAES/desfireaes.o AXL/axl.o afile.o
	gcc -g -Wall -Wextra -O -o cardissue cardissue.c -I. -IDESFireAES/include DESFireAES/desfireaes.o -IAXL AXL/axl.o -lcrypto -lpopt -pthread -lcurl -lmosquitto afile.o

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h port.o port.h afile.o door.o door.h AXL/axl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o trace.h
	gcc -g -Wall -Wextra -O -o alarmpanel alarmpanel.c galaxybus.o port.o afile.o door.o -I. -IAXL -IDataformat -Iwebsocket -IDESFireAES/include AXL/axl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o -lcurl -pthread -lpopt -DLIBWS ${LIBEMAIL} ${LIBMQTT} -lcrypto -lssl

galaxybus.o: galaxybus.c galaxybus.h port.h
	gcc -g -Wall -Wextra -O -c -o galaxybus.o galaxybus.c -I. -IAXL -DLIB -pthread

afile.o: afile.c afile.h
	gcc -g -Wall -Wextra -O -c -o afile.o afile.c -I. -IAXL -IDESFireAES/include -DLIB -pthread

door.o: door.c door.h galaxybus.h
	gcc -g -Wall -Wextra -O -c -o door.o door.c -I. -IAXL -DLIB -pthread

port.o: port.c port.h galaxybus.h
	gcc -g -Wall -Wextra -O -c -o port.o port.c -I. -IAXL -DLIB -pthread

clean:
	rm -f *.o alarmpanel

git:
	git submodule update --init
