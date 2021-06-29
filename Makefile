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

ifneq ($(wildcard /usr/bin/mysql_config),)
SQLINC=$(shell mysql_config --include)
SQLLIB=$(shell mysql_config --libs)
SQLVER=$(shell mysql_config --version | sed 'sx\..*xx')
endif
ifneq ($(wildcard /usr/bin/mariadb_config),)
SQLINC=$(shell mariadb_config --include)
SQLLIB=$(shell mariadb_config --libs)
SQLVER=$(shell mariadb_config --version | sed 'sx\..*xx')
endif

ifndef KCONFIG_CONFIG
KCONFIG_CONFIG=solarsystem.conf
endif

all: alarmpanel cardissue solarsystem can message makeaid login.conf SQLlib/sql xmlsql/xmlsql

update:
	git submodule update --init --remote --recursive
	git commit -a -m "Library update"
	git push
	make -C login
	make -C xmlsql
	make -C SQLlib
	make -C AJL
	make -C DESFireAES

PCBCase/case: PCBCase/case.c
	make -C PCBCase

scad: KiCad/Bell.scad

stl: KiCad/Access.stl KiCad/Bell.stl

%.stl: %.scad
	echo "Making $@"
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@
	echo "Made $@"

KiCad/Bell.scad: KiCad/Bell.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=3 --top=5.2
xmlsql/xmlsql: xmlsql/xmlsql.c
	make -C xmlsql
AXL/axl.o: AXL/axl.c
	make -C AXL
AJL/ajl.o: AJL/ajl.c
	make -C AJL
SQLlib/sqllib.o: SQLlib/sqllib.c
	make -C SQLlib
SQLlib/sql: SQLlib/sql.c
	make -C SQLlib
Dataformat/dataformat.o: Dataformat/dataformat.c
	make -C Dataformat
websocket/websocketxml.o: websocket/websocket.c
	make -C websocket
DESFireAES/desfireaes.o: DESFireAES/desfireaes.c
	make -C DESFireAES
login.conf: login/Kconfig
	make -C login ../login.conf
login/redirect.o: login/redirect.c
	make -C login redirect.o

cardissue: cardissue.c DESFireAES/desfireaes.o AXL/axl.o AJL/ajl.o afile.o Makefile
	gcc -g -Wall -Wextra -O -o $@ $< -I. -IDESFireAES/include DESFireAES/desfireaes.o -IAXL AXL/axl.o -IAJL AJL/ajl.o -lcrypto -lpopt -pthread -lcurl -lmosquitto afile.o

alarmpanel: alarmpanel.c galaxybus.o galaxybus.h port.o port.h afile.o door.o door.h AXL/axl.o AJL/ajl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o trace.h Makefile
	gcc -g -Wall -Wextra -O -o $@ $< galaxybus.o port.o afile.o door.o -I. -IAXL -IAJL -IDataformat -Iwebsocket -IDESFireAES/include AXL/axl.o AJL/ajl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o -lcurl -pthread -lpopt -DLIBWS ${LIBEMAIL} ${LIBMQTT} -lcrypto -lssl

galaxybus.o: galaxybus.c galaxybus.h port.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< -I. -DLIB -pthread

afile.o: afile.c afile.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< -I. -IAJL -IDESFireAES/include -DLIB -pthread

door.o: door.c door.h galaxybus.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< -I. -DLIB -pthread

port.o: port.c port.h galaxybus.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< -I. -DLIB -pthread

ssdatabase.o: ssdatabase.c ssdatabase.m ssdatabase.h config.h types.m Makefile ESP32/main/states.m
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC}

ssmqtt.o: ssmqtt.c ssmqtt.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC}

sscert.o: sscert.c sscert.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC}

fobcommand.o: fobcommand.c fobcommand.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC}

mqttmsg.o: mqttmsg.c mqttmsg.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC}

ssafile.o: ssafile.c ssafile.h Makefile
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC}

solarsystem: solarsystem.c config.h AXL/axl.o AJL/ajl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o SQLlib/sqllib.o Makefile ssdatabase.o ssmqtt.o sscert.o fobcommand.o mqttmsg.o ssafile.o
	gcc -g -Wall -Wextra -O -o $@ $< ssdatabase.o ssmqtt.o sscert.o AJL/ajl.o AXL/axl.o Dataformat/dataformat.o websocket/websocketxml.o DESFireAES/desfireaes.o SQLlib/sqllib.o ${SQLINC} ${SQLLIB} -lpopt -lcrypto -pthread -lcurl -lssl fobcommand.o mqttmsg.o ssafile.o

can: can.c config.h Makefile login/redirect.o
	gcc -g -Wall -Wextra -O -o $@ $< SQLlib/sqllib.o ${SQLINC} ${SQLLIB} -lpopt -lcurl AJL/ajl.o login/redirect.o

message: message.c config.h Makefile sscert.o mqttmsg.o
	gcc -g -Wall -Wextra -O -o $@ $< -lpopt AJL/ajl.o sscert.o -lcrypto -lssl mqttmsg.o

makeaid: makeaid.c config.h Makefile
	gcc -g -Wall -Wextra -O -o $@ $< -lpopt  SQLlib/sqllib.o ${SQLINC} ${SQLLIB}

clean:
	rm -f *.o alarmpanel

menuconfig:
	login/makek ${KCONFIG_CONFIG} config.h

config.h: ${KCONFIG_CONFIG}
	login/makek ${KCONFIG_CONFIG} config.h

${KCONFIG_CONFIG}: Kconfig
	login/makek ${KCONFIG_CONFIG} config.h
