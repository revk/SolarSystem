all:	png stl

stl:	AccessWAGO/AccessT.stl Access/AccessT.stl Generic/GenericT.stl Bell/BellT.stl Output/OutputT.stl AccessWAGO/AccessB.stl Access/AccessB.stl Generic/GenericB.stl Bell/BellB.stl Output/OutputB.stl

png:	$(patsubst %.kicad_pcb,%.png,$(wildcard */*.kicad_pcb))

PCBCase/case: PCBCase/case.c
	make -C PCBCase

PCBCase/clean: PCBCase/clean.c
	make -C PCBCase

scad:	$(patsubst %,KiCad/%.scad,$(MODELS))

%.stl: %.scad
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@

Generic/GenericT.scad: Generic/Generic.kicad_pcb PCBCase/case
	PCBCase/case -o $@ $< --bottom=2 --top=6 --pcb=2 --two-file

Bell/BellT.scad: Bell/Bell.kicad_pcb PCBCase/case
	PCBCase/case -o $@ $< --bottom=2 --top=6 --two-file

Access/AccessT.scad: Access/Access.kicad_pcb PCBCase/case
	PCBCase/case -o $@ $< --bottom=2 --top=6.5 --pcb=2 --two-file

AccessWAGO/AccessT.scad: AccessWAGO/Access.kicad_pcb PCBCase/case
	PCBCase/case -o $@ $< --bottom=2 --top=6.5 --pcb=2 --two-file

Output/OutputT.scad: Output/Output.kicad_pcb PCBCase/case
	PCBCase/case -o $@ $< --bottom=1.4 --top=6.5 --pcb=2 --two-file

%.png:	%.kicad_pcb PCBCase/clean PCBCase/render
	PCBCase/render $<
