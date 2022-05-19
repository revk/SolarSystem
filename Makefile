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

ifneq ("$(wildcard ESP32/build/config/sdkconfig.h)","")
BUILD_ESP32_USING_CMAKE=-DBUILD_ESP32_USING_CMAKE
else
BUILD_ESP32_USING_CMAKE=
endif

ifndef KCONFIG_CONFIG
KCONFIG_CONFIG=solarsystem.conf
endif

<<<<<<< HEAD
MODELS := Access2 Bell2 Bell2GPS GPS Access Bell Relay
=======
MODELS := Access2 Bell2GPS GPS Access Bell
>>>>>>> 14c91b2f141e9116917bb73e4a21a235f27f67a2

all: solarsystem can message makeaid sscert login.conf SQLlib/sql xmlsql/xmlsql .git/hooks/pre-commit

.git/hooks/pre-commit: pre-commit
	cp $< $@

apt:
	sudo apt install libpopt-dev  libssl-dev  libcurl4-openssl-dev  libmosquitto-dev  libmariadbd-dev  kconfig-frontends libargon2-dev python3-certbot-apache libgpgme-dev

pull:
	git checkout ESP32/version.txt
	git pull
	git submodule update --recursive

update:
	git pull
	git submodule update --init --remote --recursive
	git commit -a -m "Library update"
	git push
	make -C login
	make -C login/SQLlib
	make -C xmlsql
	make -C SQLlib
	make -C AJL
	make -C DESFireAES
	make -C email

PCBCase/case: PCBCase/case.c
	make -C PCBCase

scad:	$(patsubst %,KiCad/%.scad,$(MODELS))
stl:	$(patsubst %,KiCad/%.stl,$(MODELS))

%.stl: %.scad
	echo "Making $@"
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@
	echo "Made $@"

KiCad/GPS.scad: KiCad/GPS.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=5 --top=5.2
KiCad/Bell.scad: KiCad/Bell.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.2
KiCad/Bell2GPS.scad: KiCad/Bell2.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.2
KiCad/Bell2.scad: KiCad/Bell2.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.2 --edge2
KiCad/Access2.scad: KiCad/Access2.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.2
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

ssdatabase.o: ssdatabase.c ssdatabase.m ssdatabase.h config.h types.m Makefile ESP32/main/states.m ESP32/main/logicalgpio.m
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC} -DLIB

ssmqtt.o: ssmqtt.c ssmqtt.h Makefile config.h
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC} -DLIB

sscert.o: sscert.c sscert.h Makefile config.h
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC} -DLIB

fobcommand.o: fobcommand.c fobcommand.h Makefile config.h
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC} -DLIB

mqttmsg.o: mqttmsg.c mqttmsg.h Makefile config.h
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC} -DLIB

ssafile.o: ssafile.c ssafile.h Makefile config.h
	gcc -g -Wall -Wextra -O -c -o $@ $< ${SQLINC} -DLIB

solarsystem: solarsystem.c config.h AXL/axl.o AJL/ajl.o Dataformat/dataformat.o DESFireAES/desfireaes.o SQLlib/sqllib.o Makefile ssdatabase.o ssmqtt.o sscert.o fobcommand.o mqttmsg.o ssafile.o email/libemaillight.o
	gcc -g -Wall -Wextra -O ${BUILD_ESP32_USING_CMAKE} -o $@ $< ssdatabase.o ssmqtt.o sscert.o AJL/ajlcurl.o AXL/axl.o Dataformat/dataformat.o DESFireAES/desfireaes.o SQLlib/sqllib.o ${SQLINC} ${SQLLIB} -lpopt -lcrypto -pthread -lcurl -lssl fobcommand.o mqttmsg.o ssafile.o -Iemail email/libemaillight.o

can: can.c config.h Makefile login/redirect.o
	gcc -g -Wall -Wextra -O -o $@ $< SQLlib/sqllib.o ${SQLINC} ${SQLLIB} -lpopt -lcurl AJL/ajl.o login/redirect.o

sscert: sscert.c sscert.o SQLlib/sqllib.o AJL/ajl.o
	gcc -g -Wall -Wextra -O -o $@ $< AJL/ajl.o SQLlib/sqllib.o -lssl -lcrypto ${SQLINC} ${SQLLIB}

message: message.c config.h Makefile sscert.o mqttmsg.o
	gcc -g -Wall -Wextra -O -o $@ $< -lpopt AJL/ajl.o sscert.o -lcrypto -lssl mqttmsg.o

makeaid: makeaid.c config.h Makefile
	gcc -g -Wall -Wextra -O -o $@ $< -lpopt SQLlib/sqllib.o ${SQLINC} ${SQLLIB}

menuconfig:
	login/makek ${KCONFIG_CONFIG} config.h

config.h: ${KCONFIG_CONFIG}
	login/makek ${KCONFIG_CONFIG} config.h

${KCONFIG_CONFIG}: Kconfig
	login/makek ${KCONFIG_CONFIG} config.h
