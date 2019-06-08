// Case for NFC reader
// Reader type: PN532 reader by Elecehgouse
// Connector type: Molex 6 way
// Note, order genuine elechouse from china. NFC MODULE V4

wire=5; // Build for desk not wall (0 for not)

// Tolerances for your printer
t=0.1;  // General xy tolerance/margin
z=0.4;  // General z tolerance/margin
clip=0.1;  // Clip size

// Thickness of walls
sidet=2;
frontt=1;
backt=2;

// Fixings to match Max Reader
screwd=4.5; // Diameter (screw hols top/bottom)
screwv=61; // Spacing
screwhd=9; // Screw head diameter
screwht=3; // Screw head thickness
screwl=2;   // Latitude

egressx=(wire?0:-11); // Engress hole position relative to cenrte
egressy=22;
egressd=9; // Diameter

// PCB size
pcbw=42.7; // width
pcbh=40.3; // height
pcbt=1.5; // thickness of pcb
pcbf=2.5; // space on front for components
pcbb=0; // space on back for components
pcbm=3; // border margin with no components

// LED hole (t[ 0.00, 0.00, 0.00 ]op left)
ledx=8; // Hole (from corner)
ledy=8;
ledd=5; // diameter
ledd2=6.2; // base diameter
ledd3=8.5; // support diameter
ledt=8.7-1; // thickness (height) - allow for sticking out of top
ledw=4; // Wires
ledx2=0; // Wires to PCB (from centre)
ledy2=10;

// PCB fixings?
//fixd=4; // Holes on PCB - need to allow in pcbf;
//fixh=31;
//fixv=31;

// Tamper switch holder (top right)
tamperw=6;
tamperh=6;
tampert=4.7; // Height when contact closed
tamperm=1; // Margin
tampere=1.5;  // Edge

// Connector
connw=17; // Connector size as fitted
connh=22;
conne=8; // Extra to fit
connt=7; // Thickness
connx=12.75; // position from edge of board
conny=5.3;

// Box size
lip=2;
boxw=sidet*2+t*2+pcbw;
boxh=screwv+screwhd+t*2+sidet*2;
boxt=frontt+t+pcbf+pcbt+connt;

pcby=-screwv/2+sidet+t+screwhd/2+sidet+t; // edge of PCB

$fn=100;

module cable(x,z)
{
    if(wire)
    translate([x,boxh/2+t,z])
    hull()
    {
        rotate([90,0,0])
        cylinder(d=wire,h=boxh/2-egressy);
        translate([0,0,-wire/2-backt])
        rotate([90,0,0])
        cylinder(d=wire,h=boxh/2-egressy);
    }
}

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
        translate([boxw/2-ledx,boxh/2-ledy,-0.01])
        cylinder(d=ledd,h=frontt+1);
        cable(-egressx,boxt);
    }
    difference()
    {
        translate([-boxw/2,boxh/2-sidet,boxt-lip*2])
        hull()
        {
            cube([boxw,lip,lip]);
            translate([0,-lip,lip])
            cube([boxw,lip*2,lip]);
        }
        if(!wire)
        translate([0,screwv/2,boxt-backt-lip*2])
        cylinder(d=screwhd,h=backt+lip*2+t);
        cable(-egressx,boxt);
    }
}

module base()
{
    difference()
    {
        union()
        {
            translate([sidet+t-boxw/2,sidet+t-boxh/2,0])
            cube([boxw-sidet*2-t*2,boxh-sidet*2-t*2-lip,backt]);
            if(!wire)
            translate([0,screwv/2,0])
            cylinder(d=screwhd-t,h=backt+lip*2);
            hull()
            {
                translate([-pcbw/2,pcby-sidet-t,0])
                cube([pcbw,pcbh+sidet*2+t*2,boxt-frontt-z]);
                translate([-pcbw/2,pcby-sidet-t-connt+backt,0])
                cube([pcbw,pcbh+sidet*2+t*2+(connt-backt)*2,backt]);
            }
            // Tamper base
            translate([boxw/2-tamperw-sidet-t-tamperm-tampere,boxh/2-tamperh-sidet-t-tamperm-tampere*2-lip,0])
            difference()
            {
                cube([tamperw+tampere,tamperh+tampere*2,boxt-frontt-tampere]);
                translate([-1,tampere,boxt-tampert-frontt])
                cube([tamperw+1,tamperh,tampert]);
            }
            // Lip
            translate([sidet+t-boxw/2,boxh/2-lip*2-sidet,lip])
            hull()
            {
                cube([boxw-sidet*2-t*2,lip,lip]);
                translate([0,lip,lip])
                cube([boxw-sidet*2-t*2,lip,lip]);
            }
            // LED base
            translate([ledx-boxw/2,boxh/2-ledy,0])
            cylinder(d=ledd3,h=boxt-frontt-z);
        }
        translate([ledx-boxw/2,boxh/2-ledy,boxt-ledt])
        cylinder(d=ledd2,h=boxt);  
        // Wires for tamper
        hull()
        {
            translate([boxw/2-tamperw-sidet-t-tamperm-tampere-3,boxh/2-tamperh/2-sidet-t-tamperm-tampere*2-lip,connt+pcbt])
            cylinder(d=2,h=boxt);
            translate([boxw/2-tamperw-sidet-t-tamperm-tampere-3,0,connt+pcbt])
            cylinder(d=2,h=boxt);
            
        }
        cable(egressx,wire/2+backt+lip*2);
        // Board
        hull()
        {
            translate([-t*2-pcbw/2,pcby-t,connt+t])
            cube([pcbw+t*4,pcbh+t*2,0.01]);
            translate([-t*2-pcbw/2,pcby+-t+clip,connt+t+pcbt])
            cube([pcbw+t*4,pcbh+t*2-clip*2,0.01]);
        }
        hull()
        { // Messy
            translate([-t*2-pcbw/2,pcby,connt+t+pcbt])
            cube([pcbw+t*4,pcbh+t,0.01]);
            translate([-t*2-pcbw/2,pcby-t+lip,boxt-frontt])
            cube([pcbw+t*4,pcbh+t*2-lip,0.01]);
        }
        translate([-t*2-pcbw/2+pcbm,pcby+pcbm-t,connt-pcbb])
        cube([pcbw+t*2-pcbm*2,pcbh+t*2-pcbm*2,pcbb+1]);
        // Screws
        if(!wire)
        {
            translate([0,screwv/2,-1])
            {
                cylinder(d=screwd,h=boxt);
                translate([0,0,1.01+backt])
                cylinder(d=screwhd,h=boxt);
            }
            translate([0,-screwv/2,-1])
            {
                hull()
                {
                    translate([-screwl,0,0])
                    cylinder(d=screwd,h=boxt);
                    translate([screwl,0,0])
                    cylinder(d=screwd,h=boxt);
                }
                hull()
                {
                    translate([-screwl,0,1.01+backt])
                    cylinder(d=screwhd,h=boxt);
                    translate([screwl,0,1.01+backt])
                    cylinder(d=screwhd,h=boxt);
                }
            }
        }
        // Egress + connector
        translate([connx-pcbw/2-t,pcby+conny-t,-1])
        cube([connw+t*2,connh+t*2,boxt]);
        hull()
        {
            translate([egressx,egressy,-1])
            cylinder(d=egressd,boxt);
            translate([connx-pcbw/2-t,pcby+conny+connh,-1])
            cube([connw+t*2,conne+t,boxt]);
        }
        // LED
        hull()
        {
            translate([ledx-boxw/2,boxh/2-ledy,boxt-ledt-2])
            cylinder(d=ledw,h=boxt);
            translate([ledx2,ledy2,connt-ledw])
            cylinder(d=ledw,h=boxt);
        }
    }
}

translate([boxw+sidet,0,0])lid();
base();
