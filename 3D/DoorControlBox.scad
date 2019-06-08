 // Case for Door controller PCB

// Tolerances for your printer
t=0.1;  // General xy tolerance/margin
z=0.4;  // General z tolerance/margin
clip=0.1;  // Clip size
lip=2; // Lip size

// Thickness of walls
sidet=2;
frontt=1;
backt=1;

// PCB
pcbw=33.7;
pcbh=50;
pcbt=1.6;
pcbf=10.5;
pcbb=3;
pcbe=0.5;
pcbm=1;
pcby=2;

// Wire size
wire=5;

// Box size
boxw=sidet*2+t*2+pcbw+pcbm*2;
boxh=sidet*2+t*2+pcbh+pcbm*2+lip*2;
boxt=frontt+t+pcbf+pcbt+pcbb+backt;

// Screw contacts
screwsx=5.27-pcbw/2;
screwsy=2.5+sidet+t+pcbm-boxh/2+pcby;
screwsd=3.5;
screwss=5;
screwsn=10;
screwst=4;

$fn=100;

module label(p,s,t)
{
        translate([screwsx+screwss/2,screwsy+p*screwss,-1])
        linear_extrude(height=1.2)
        mirror([1,0,0])
        text(t,size=screwss*s,halign="right",valign="center");
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
        // Screw holes
        for(y=[0:screwsn-1])
        translate([screwsx,screwsy+y*screwss,-1])
        cylinder(d=screwsd,h=boxt+2);
        // Screw slot
        translate([screwsx,screwsy,boxt-backt-pcbf-pcbt+screwst])
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
        // Wire slot
        translate([boxw/2-10,0,boxt-backt-pcbb-pcbt-wire/2-z])
        rotate([90,0,0])
        hull()
        {
            cylinder(d=wire,h=pcbh);
            translate([0,boxt,0])
            cylinder(d=wire,h=pcbh);
        }   
        // Screws text
        label(0.5,1.5,"R");
        label(1.95,1,"+");
        label(2.85,1,"-");
        label(4.5,1.5,"1");      
        label(6.5,1.5,"2");
        label(8.5,1.5,"3");
    }
    // Back support
    translate([boxw/2-sidet-7.9,boxh/2-sidet-26-lip,frontt-0.1])
    cube([8,25-lip,boxt-pcbt-pcbb-backt-z-frontt]);
    // Lip    
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
        // PCB
        translate([-pcbw/2,sidet+t+pcbm-boxh/2+pcby,backt+pcbb])
        cube([pcbw,pcbh,pcbt*2]);
        difference()
        {
            translate([pcbe-pcbw/2,sidet+t+pcbm+pcbe-boxh/2+pcby,backt])
            cube([pcbw-pcbe*2,pcbh-pcbe*2,backt+pcbb]);
            translate([-8,pcby-15,t])
            cube([11,12,pcbb+backt+t]);
            translate([pcbw/2-2,sidet+pcbm+t-boxh/2+pcby,0])
            cube([7,pcbh,pcbb+backt+t]);
        }
        translate([-pcbw/2,pcby-2,backt+pcbb-1])
        cube([6,17,boxt]);
    }
}

translate([boxw+sidet,0,0])lid();
base();
