// Case for Door controller PCB

// Tolerances for your printer
t=0.1;  // General xy tolerance/margin
z=0.4;  // General z tolerance/margin
clip=0.1;  // Clip size
lip=2; // Lip size

// Thickness of walls
sidet=2;
frontt=1;
backt=2;

// PCB
pcbw=34;
pcbh=50;
pcbt=1.5;
pcbf=10.2;
pcbb=3;
pcbe=0.5;
pcbm=1;

// Screw contacts
screwsx=4.5-pcbw/2;
screwsy=2.5-pcbh/2;
screwsd=3.5;
screwss=5;
screwsn=10;

// Wire size
wire=5;

// Box size
boxw=sidet*2+t*2+pcbw+pcbm*2;
boxh=sidet*2+t*2+pcbh+pcbm*2+lip*2;
boxt=frontt+t+pcbf+pcbt+pcbb;

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
        for(y=[0:screwsn-1])
        translate([screwsx,screwsy+y*screwss,-1])
            cylinder(d=screwsd,h=boxt+2);
        translate([screwsx,screwsy,boxt-1-backt-pcbb-pcbt])
        rotate([0,-90,0])
        hull()
        {
            cylinder(d=screwsd,h=boxw);
            translate([boxt,0,0])
            cylinder(d=screwsd,h=boxw);
            translate([0,screwss*(screwsn-1),0])
            cylinder(d=screwsd,h=boxw);
            translate([boxt,screwss*(screwsn-1),0])
            cylinder(d=screwsd,h=boxw);
        }
        translate([0,0,boxt-backt-pcbb-pcbt-wire/2])
        rotate([90,0,0])
        cylinder(d=wire,h=pcbh);
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
    }
}

module base()
{
    difference()
    {
        union()
        {
            translate([sidet+t-boxw/2,sidet+t-boxh/2,0])
            cube([boxw-sidet*2-t*2,boxh-sidet*2-t*2-lip,backt+pcbb+pcbt]);
            translate([sidet+t-boxw/2,boxh/2-lip*2-sidet,lip])
            hull()
            {
                cube([boxw-sidet*2-t*2,lip,lip]);
                translate([0,lip,lip])
                cube([boxw-sidet*2-t*2,lip,lip]);
            }
        }
        translate([-pcbw/2,sidet+t+pcbm-boxh/2,backt+pcbb])
        cube([pcbw,pcbh,pcbt*2]);
        difference()
        {
            translate([pcbe-pcbw/2,sidet+t+pcbm+pcbe-boxh/2,backt])
            cube([pcbw-pcbe*2,pcbh-pcbe*2,backt+pcbb]);
            translate([-7,-7,t])
            cube([7,7,pcbb+backt+t]);
            translate([pcbw/2-2,sidet+pcbm+t-boxh/2,0])
            cube([7,pcbh,pcbb+backt+t]);
        }
        translate([-boxw/2,0,backt])
        cube([6,18,boxt]);
    }
}

translate([boxw+sidet,0,0])lid();
base();
