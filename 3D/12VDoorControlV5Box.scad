// 3D case for 12V Door Controller V5
// Copyright (c) 2019 Adrian Kennard, Andrews & Arnold Limited, see LICENSE file (GPL)

use <PCBCase/case.scad>

width=40;
height=28;

// Box thickness reference to component cube
base=11;
top=4;

$fn=48;

module pcb(s=0)
{
    translate([-1,-1,0])
    { // 1mm ref edge of PCB vs SVG design, and pcb surface
        esp32(s,15.279,9.200);
        screw5mm(s,1.517,1.768,-90,4);
        screw3mm5(s,10.137,1.000,0,8);
        d24v5f3(s,9.589,8.340,-90);
        d24v5f3(s,9.589,18.500,-90);
        milligrid(s,33.626,9.102);
        milligrid(s,33.626,15.706);
        milligrid(s,33.626,22.310);
        smd1206(s,7.150,22.718,-90);
        smdrelay(s,2.836,21.654,-90);
        molex(s,35.920,7.409);
    }
}

case(width,height,base,top){pcb(0);pcb(-1);pcb(1);};

