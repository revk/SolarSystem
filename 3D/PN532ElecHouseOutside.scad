// Case for NFC reader
// Reader type: PN532 reader by Elecehgouse
// Connector type: Molex 6 way
// Note, order genuine elechouse from china. NFC MODULE V4

// Tolerances for your printer
t=0.1;  // General xy tolerance/margin
z=0.2;  // General z tolerance/margin
clip=0.1;  // Clip size

// Thickness of walls
sidet=2;
frontt=1;
backt=2;

// Fixings to match Max Reader
screwd=4.5; // Diameter (screw hols top/bottom)
screwv=61; // Spacing
screwhd=8; // Screw head diameter
screwht=3; // Screw head thickness
egressx=-11; // Engress hole position relative to cenrte
egressy=22;
egressd=9; // Diameter

// PCB size
pcbw=42.7; // width
pcbh=40.3; // height
pcbt=1.5; // thickness of pcb
pcbf=2.5; // space on front for components
pcbb=0; // space on back for components
pcbm=3; // border margin with no components

// PCB fixings?
//fixd=4; // Holes on PCB - need to allow in pcbf;
//fixh=31;
//fixv=31;

// Connector
connw=18;
connh=22;
connt=7;
connx=11.5; // position from edge of board
conny=5.3;

// Box size
boxw=sidet*2+t*2+pcbw;
boxh=screwv+screwhd+t*2+sidet*2;
boxt=frontt+t+pcbf+pcbt+connt;

pcby=-screwv/2+sidet+t+screwhd/2+sidet+t; // edge of PCB

$fn=100;

module lid()
{
    difference()
    {
        translate([-boxw/2,-boxh/2,0])
        cube([boxw,boxh,boxt]);
        hull()
        {
            translate([sidet-boxw/2,sidet-boxh/2,frontt])
            cube([boxw-sidet*2,boxh-sidet*2,boxt-frontt-backt]);
            translate([sidet-boxw/2,sidet+clip-boxh/2,boxt])
            cube([boxw-sidet*2,boxh-sidet*2-clip*2,1]);
        }
    }
}

module base()
{
    difference()
    {
        union()
        {
            translate([sidet+t-boxw/2,sidet+t-boxh/2,0])
            cube([boxw-sidet*2-t*2,boxh-sidet*2-t*2,backt]);
            hull()
            {
                translate([-pcbw/2,pcby-sidet-t,0])
                cube([pcbw,pcbh+sidet*2+t*2,boxt-frontt-t]);
                translate([-pcbw/2,pcby-sidet-t-connt+backt,0])
                cube([pcbw,pcbh+sidet*2+t*2+(connt-backt)*2,backt]);
            }
        }
        // Board
        hull()
        {
            translate([-t*2-pcbw/2,pcby-t,connt+t])
            cube([pcbw+t*4,pcbh+t*2,0.01]);
            translate([-t*2-pcbw/2,pcby+-t+clip,connt+t+pcbt])
            cube([pcbw+t*4,pcbh+t*2-clip*2,0.01]);
        }
        translate([-t*2-pcbw/2,pcby-t,connt+t+pcbt])
        cube([pcbw+t*4,pcbh+t*2,boxt]);
        translate([-t*2-pcbw/2+pcbm,pcby+pcbm-t,connt-pcbb])
        cube([pcbw+t*2-pcbm*2,pcbh+t*2-pcbm*2,pcbb+1]);
        // Screws
        for(y=[-screwv/2,screwv/2])
        translate([0,y,-1])
        {
            cylinder(d=screwd,h=boxt);
            translate([0,0,1.01+backt])
            cylinder(d=screwhd,h=boxt);
        }
        // Egress + connector
        hull()
        {
            translate([egressx,egressy,-1])
            cylinder(d=egressd,boxt);
            translate([connx-pcbw/2-t,pcby+conny-t,-1])
            cube([connw+t*2,connh+t*2,boxt]);
        }
    }
}

translate([boxw+sidet,0,0])lid();
base();
