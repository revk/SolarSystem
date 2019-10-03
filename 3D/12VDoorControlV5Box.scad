// 3D case for Environmental monitor
// Copyright (c) 2019 Adrian Kennard, Andrews & Arnold Limited, see LICENSE file (GPL)

use <PCBCase/case.scad>
use <PCBCase/parts.scad>

compw=40;
comph=28;
compt=1.6;
compclear=0.4;

// Box thickness reference to component cube
base=11;
top=4;
side=2.4;
sidet=0.08; // Gap in side clips

$fn=48;

module pcb(s=0)
{
    translate([-1,-1,compt])
    { // 1mm ref edge of PCB vs SVG design, and pcb surface
        esp32(s,15.279,9.200);
        screw5mm(s,1.517,1.768,-90,4);
        screw3mm5(s,10.137,1.000,0,8);
        if(!s)
        {
            d24v5f3(9.589,8.340,-90);
            d24v5f3(9.589,18.500,-90);
            milligrid(33.626,9.102);
            milligrid(33.626,15.706);
            milligrid(33.626,22.310);
            smd1206(7.150,22.718,-90);
            smdrelay(2.836,21.654,-90);
            molex(35.920,7.409);
        }
    }
}

case(compw,comph,compt,base,top,compclear,side,sidet){pcb(0);pcb(-1);pcb(1);};

