stl:	Access/Access.stl GPS/GPS.stl Generic/Generic.stl Bell/Bell.stl Output/Output.stl

PCBCase/case: PCBCase/case.c
	make -C PCBCase

scad:	$(patsubst %,KiCad/%.scad,$(MODELS))

%.stl: %.scad
	echo "Making $@"
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD $< -o $@
	echo "Made $@"

GPS/GPS.scad: GPS/GPS.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=5 --top=5.6
Generic/Generic.scad: Generic/Generic.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.6 --pcb=2
Bell/Bell.scad: Bell/Bell.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.6
Access/Access.scad: Access/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=6 --pcb=2 --no-hull
Output/Output.scad: Output/Output.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=1.4 --top=6 --pcb=2

