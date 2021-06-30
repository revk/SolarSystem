#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := SS

include $(IDF_PATH)/make/project.mk

# Set GPIO low (whichever CBUS is set to mode 8/GPIO)
bootmode: ftdizap/ftdizap
	./ftdizap/ftdizap --cbus=0

# Flash with GPIO control
zap:    bootmode flash
	./ftdizap/ftdizap --cbus=1 --reset

# Program the FTDI
ftdi: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus0-mode=8 --cbus1-mode=4 --cbus2-mode=13 --cbus3-mode=5

ftdizap/ftdizap: ftdizap/ftdizap.c
	make -C ftdizap
