stl:	Access/Access.stl GPS/GPS.stl Generic/Generic.stl Bell/Bell.stl Output/Output.stl

PCBCase/case: PCBCase/case.c
	make -C PCBCase

scad:	$(patsubst %,KiCad/%.scad,$(MODELS))

%.stl: %.scad
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@

GPS/GPS.scad: GPS/GPS.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=5 --top=5.6
Generic/Generic.scad: Generic/Generic.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=5.6 --pcb=2
Bell/Bell.scad: Bell/Bell.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=5.6
Access/Access.scad: Access/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=2 --top=6 --pcb=2
Output/Output.scad: Output/Output.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --bottom=1.4 --top=6 --pcb=2

