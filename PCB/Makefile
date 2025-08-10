all:	png stl

stl:	AccessWAGO/Access.stl Access/Access.stl Generic/Generic.stl Bell/Bell.stl Output/Output.stl

png:	$(patsubst %.kicad_pcb,%.png,$(wildcard */*.kicad_pcb))

PCBCase/case: PCBCase/case.c
	make -C PCBCase

PCBCase/clean: PCBCase/clean.c
	make -C PCBCase

scad:	$(patsubst %,KiCad/%.scad,$(MODELS))

%.stl: %.scad
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@

Generic/Generic.scad: Generic/Generic.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=5.6 --pcb=2
Bell/Bell.scad: Bell/Bell.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=5.6
Access/Access.scad: Access/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=6 --pcb=2
AccessWAGO/Access.scad: AccessWAGO/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=6 --pcb=2
Output/Output.scad: Output/Output.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=1.4 --top=6 --pcb=2

%.png:	%.kicad_pcb PCBCase/clean PCBCase/render Makefile
	PCBCase/render $<
