#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := SS
SUFFIX := $(shell components/ESP32-RevK/suffix)

all:
	@echo Make: $(PROJECT_NAME)$(SUFFIX).bin
	@idf.py build
	@cp build/$(PROJECT_NAME).bin ../www/$(PROJECT_NAME)$(SUFFIX).bin
	@echo Done: $(PROJECT_NAME)$(SUFFIX).bin

set:	pico wroom solo

pico:
	@sed -i -e 's/# CONFIG_ESP32_SPIRAM_SUPPORT is not set/CONFIG_ESP32_SPIRAM_SUPPORT=y/' -e 's/# CONFIG_ESPTOOLPY_FLASHSIZE_8MB is not set/CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y/' -e 's/CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y/# CONFIG_ESPTOOLPY_FLASHSIZE_4MB is not set/' -e 's/CONFIG_FREERTOS_UNICORE=y/# CONFIG_FREERTOS_UNICORE is not set/' sdkconfig
	@make

wroom:
	@sed -i -e 's/CONFIG_ESP32_SPIRAM_SUPPORT=y/# CONFIG_ESP32_SPIRAM_SUPPORT is not set/' -e 's/# CONFIG_ESPTOOLPY_FLASHSIZE_4MB is not set/CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y/' -e 's/CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y/# CONFIG_ESPTOOLPY_FLASHSIZE_8MB is not set/' -e 's/CONFIG_FREERTOS_UNICORE=y/# CONFIG_FREERTOS_UNICORE is not set/' sdkconfig
	@make

solo:
	@sed -i -e 's/CONFIG_ESP32_SPIRAM_SUPPORT=y/# CONFIG_ESP32_SPIRAM_SUPPORT is not set/' -e 's/# CONFIG_ESPTOOLPY_FLASHSIZE_4MB is not set/CONFIG_ESPTOOLPY_FLASHSIZE_4MB=y/' -e 's/CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y/# CONFIG_ESPTOOLPY_FLASHSIZE_8MB is not set/' -e 's/# CONFIG_FREERTOS_UNICORE is not set/CONFIG_FREERTOS_UNICORE=y/' sdkconfig
	@make

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
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus3-mode=17 --self-powered=1

ftdiAccess: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus3-mode=17 --self-powered=1

ftdiBell: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus3-mode=17 --self-powered=1

ftdi: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus0-mode=8 --cbus1-mode=4 --cbus2-mode=13 --cbus3-mode=5

# Program the FTDI for Generic module
ftdi-invert: ftdizap/ftdizap
	./ftdizap/ftdizap --serial="RevK" --description="SolarSystem" --cbus0-mode=7 --cbus1-mode=13 --invert-rts=1 --invert-dtr=1

ftdizap/ftdizap: ftdizap/ftdizap.c
	make -C ftdizap
