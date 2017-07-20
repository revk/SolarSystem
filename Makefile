alarmpanel: alarmpanel.c galaxybus.o galaxybus.h ../AXL/axl.o ../AXL/axl.h ../Dataformat/dataformat.o ../Dataformat/dataformat.h
	cc -O -o alarmpanel alarmpanel.c galaxybus.o -I. -I../AXL -I ../Dataformat ../AXL/axl.o ../Dataformat/dataformat.o -lcurl -lexpat -pthread -lpopt

galaxybus.o: galaxybus.c
	cc -O -c -o galaxybus.o galaxybus.c -I. -DLIB -pthread
