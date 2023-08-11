stl:	GPS/GPS.stl Generic/Generic.stl Bell2/Bell.stl Access2/Access.stl Access2/AccessNoUSB.stl Access3/Access.stl Access3/AccessNoUSB.stl Access4/Access.stl Output/Output.stl

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
	PCBCase/case -o $@ $< --base=2 --top=5.6 
Bell2/Bell2.scad: Bell2/Bell2.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=5.6
Access2/Access.scad: Access2/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=6
Access2/AccessNoUSB.scad: Access2/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=6 --ignore=J1
Access3/Access.scad: Access3/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=6
Access3/AccessNoUSB.scad: Access3/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=6 --ignore=J1
Access4/Access.scad: Access4/Access.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=2 --top=6
Output/Output.scad: Output/Output.kicad_pcb PCBCase/case Makefile
	PCBCase/case -o $@ $< --base=1.4 --top=6

