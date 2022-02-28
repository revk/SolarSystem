#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := SS
SUFFIX := $(shell components/ESP32-RevK/suffix)

all:
	@echo Make: build/$(PROJECT_NAME)$(SUFFIX).bin
	@idf.py build
	@cp build/$(PROJECT_NAME).bin build/$(PROJECT_NAME)$(SUFFIX).bin
	@echo Done: build/$(PROJECT_NAME)$(SUFFIX).bin

flash:
	idf.py flash

monitor:
	idf.py monitor

clean:
	idf.py clean

menuconfig:
	idf.py menuconfig

#include $(IDF_PATH)/make/project.mk

pull:
	git pull
	git submodule update --recursive

# Set GPIO low (whichever CBUS is set to mode 8/GPIO)
bootmode: ftdizap/ftdizap
	./ftdizap/ftdizap --cbus=0

# Flash with GPIO control
zap:    bootmode flash
	./ftdizap/ftdizap --cbus=1 --reset

# Program the FTDI
ftdiGPS: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus3-mode=17 --self-powered

ftdiAccess: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus3-mode=17 --self-powered

ftdiBell: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus3-mode=17 --self-powered

ftdi: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus0-mode=8 --cbus1-mode=4 --cbus2-mode=13 --cbus3-mode=5

# Program the FTDI for Generic module
ftdi-invert: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus0-mode=7 --cbus1-mode=13 --invert-rts=1 --invert-dtr=1

ftdizap/ftdizap: ftdizap/ftdizap.c
	make -C ftdizap
