// Case for Door controller PCB

exit=0; // Make exit box
ranger=exit; // If to include ranger hole
header=1;   // How for 4 pin header for ranger

// Tolerances for your printer
t=0.1;  // General xy tolerance/margin
z=0.5;  // General z tolerance/margin
clip=0.1;  // Clip size
lip=2; // Lip size

// Thickness of walls
sidet=2;
frontt=1;
backt=1;

// PCB
pcbw=31.75;
pcbh=50;
pcbt=1.8;
pcbf=11;
pcbb=3;
pcbe=0.5;
pcbm=1;
pcby=2;

// Wire size
wire=5;
wirex=12;
wirey=0;

// Box size
boxw=sidet*2+t*2+pcbw+pcbm*2;
boxh=sidet*2+t*2+pcbh+pcbm*2+lip*2;
boxt=frontt+t+pcbf+pcbt+pcbb+backt;

// Screw contacts
screwsx=4-pcbw/2;
screwsy=2.5+sidet+t+pcbm-boxh/2+pcby;
screwsd=3.5;
screwss=5;
screwsn=10;
screwst=4;

// Ranger hole
rangerw=8;
rangerh=6;
rangerx=-3;
rangery=-4.5;

// Header hole
headerw=11;
headerh=5.5+0.5;
headerx=15.875-0.5; // Centre of header
headery=13.175+0.87;

// Exit box
exitw=90;
exith=exitw;
exitx=-exitw/2+rangerx;
exity=-exith/2+rangery;
screws=60;
screwd=3.5;
slot=10;

$fn=100;

module label(x,y,s,t)
{
    translate([x,y,-0.01])
    minkowski()
    {
        linear_extrude(height=0.01)
        mirror([1,0,0])
        text(t,size=s,halign="center",valign="center",font="FiveByNineJTD:style=ExtraLight");
        cylinder(d1=1,d2=0,h=frontt-0.2,$fn=8);
    }
}

module screwlabel(p,s,t)
{
    label(12,screwsy+p*screwss,screwss*s,t);
}

module wirecut()
{
    translate([boxw/2-wirex,boxh/2+1,boxt-backt-pcbb-pcbt-wire/2-z-wirey])
    rotate([90,0,0])
    hull()
    {
        cylinder(d=wire,h=sidet+2+lip);
        translate([0,boxt,0])
        cylinder(d=wire,h=sidet+2+lip);
    }   
}

module rangerhole()
{
    // Ranger hole
    if(ranger)
    translate([rangerx-rangerw/2,rangery-rangerh/2,-t])
    cube([rangerw,rangerh,frontt+t*2]);
}

module lid()
{
    if(exit)
    {
        difference()
        {
            translate([exitx,exity,0])
            cube([exitw,exith,boxt]);
            hull()
            {
                translate([exitx+sidet,exity+sidet,frontt])
                cube([exitw-sidet*2,exith-sidet*2,boxt-frontt-backt]);
                translate([exitx+sidet,exity+sidet+clip,boxt])
                cube([exitw-sidet*2,exith-sidet*2-clip*2,1]);
            }
            rangerhole();
            label(exitx+exitw/2,exity+exith/2+exith/4,exith/4,"EXIT");
        }
    }
    else
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
        if(!exit)
        {
            // Screw holes
            for(y=[0:screwsn-1])
            translate([screwsx,screwsy+y*screwss,-1])
            cylinder(d=screwsd,h=boxt+2);
        }
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
        wirecut();
        if(!exit)
        {
            // Screws text
            screwlabel(-0.15,1,"-");
            screwlabel(0.95,1,"+");
            screwlabel(2.5,1.5,"3");
            screwlabel(4.5,1.5,"2");      
            screwlabel(6.5,1.5,"1");
            screwlabel(8.5,1.5,"R");
        }
        else
        label(exitx+exitw/2,exity+exith/2+exith/4,exith/4,"EXIT");
        rangerhole();
        if(header)
        translate([headerx-pcbw/2-headerw/2,sidet+t+pcbm-boxh/2+pcby+headery-headerh/2,-1])
        cube([headerw,headerh,frontt+2]);
    }
    // Back support
    translate([boxw/2-sidet-5,t+sidet-boxh/2+pcby+16,frontt-0.1])
    cube([5+t,32,boxt-pcbt-pcbb-backt-z-frontt]);
    if(exit)
    {
        difference()
        {
            translate([exitx,exity+exith-sidet,boxt-lip*2])
            hull()
            {
                cube([exitw,lip,lip]);
                translate([0,-lip,lip])
                cube([exitw,lip*2,lip]);
            }
            wirecut();
        }
    }else
    {  // Lip
        difference()
        {
            translate([-boxw/2,boxh/2-sidet,boxt-lip*2])
            hull()
            {
                cube([boxw,lip,lip]);
                translate([0,-lip,lip])
                cube([boxw,lip*2,lip]);
            }
            wirecut();
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
            if(!exit)
            {
                translate([sidet+t-boxw/2,boxh/2-lip*2-sidet,lip])
                hull()
                {
                    cube([boxw-sidet*2-t*2,lip,lip]);
                    translate([0,lip,lip])
                    cube([boxw-sidet*2-t*2,lip,lip]);
                }
            }else
            mirror([1,0,0])
            {
                translate([exitx+sidet+t,exity+sidet+t,0])
                cube([exitw-sidet*2-t*2,exith-sidet*2-t*2-lip,lip]);
                translate([exitx+sidet+t,exity+exith-lip*2-sidet,lip])
                hull()
                {
                    cube([exitw-sidet*2-t*2,lip,lip]);
                    translate([0,lip,lip])
                    cube([exitw-sidet*2-t*2,lip,lip]);
                }
                translate([exitx+exitw/2,exity+exith/2,0])
                for(x=[-screws/2,screws/2])
                translate([x,0,0])
                cylinder(d=screwd*3,h=lip*2+2);
            }
        }
        if(exit)
        mirror([1,0,0])
        translate([exitx+exitw/2,exity+exith/2,0])
        { // Screws and holes
            for(x=[-exitw/2+slot*1.5,exitw/2-slot*1.5])
            translate([x,exith/2-slot*1.5,-1])
            cylinder(d=slot,h=lip+2);
            hull()
            for(x=[-exitw/2+slot*1.5,exitw/2-slot*1.5])
            translate([x,-exith/2+slot*1.5,-1])
            cylinder(d=slot,h=lip+2);
            for(x=[-screws/2,screws/2])
            translate([x,0,-1])
            cylinder(d=screwd,h=lip*3+2);
        }
        // PCB
        translate([-pcbw/2,sidet+t+pcbm-boxh/2+pcby,backt+pcbb])
        cube([pcbw,pcbh,pcbt*2]);
        difference()
        {
            translate([pcbe-pcbw/2,sidet+t+pcbm+pcbe-boxh/2+pcby,backt])
            cube([pcbw-pcbe*2,pcbh-pcbe*2,backt+pcbb]);
            translate([pcbw/2-2,sidet+pcbm+t-boxh/2+pcby,0])
            cube([7,pcbh,pcbb+backt+t]);
        }
        translate([-pcbw/2,pcby-8,backt+pcbb-1])
        cube([6,17,boxt]);
    }
}

translate([(exit?exitw-2*rangerx:boxw)+sidet,0,0])lid();
base();
