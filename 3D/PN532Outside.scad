// Box for PN532 (wired back to door control)

t=0.1;  // Print xy tolerance adjust to make fit
tl=0.4; // Layer tolerance

wt=2;   // Wall thickness
pw=41; // PCB size
ph=1.7*25.4;
pt=1.5; // PCB thickness
pc=2.5; // Component thickness
pb=7-wt; // Back connector thickness
hd=3; // Hole diameter
hx=0.3*25.4; // Hole position
hy=hx;
st=4; // Edge support thickness
bw=ph+wt*2; // Box size
bt=wt+pc+pt+pb;
bh=75;
sd=5; // Screw hold diameter
ss=60; // SCrew hole spacing
bhw=32;
bhh=20;
lip=0.1;
ex=11; // exit hole for cable to match Max
ey=20;
ed=8;

$fn=100;

module lid()
{
    difference()
    {
        translate([-bw/2,-bh/2,0])
        cube([bw,bh,bt]);
        translate([wt-bw/2-t,wt-bh/2-t,wt+pc])
        cube([bw-wt*2+t*2,bh-wt*2+t*2,bt]);
        translate([st-pw/2,st-ph/2,wt])
        cube([pw-st*2,ph-st*2,bt]);
    }
    for(y=[0:1])
    mirror([0,y,0])
    hull()
    {
        translate([-bw/2,bh/2-wt,bt-wt-0.02])
        cube([bw,wt,0.01]);
        translate([-bw/2,bh/2-wt-lip,bt])
        cube([bw,wt,0.01]);
    }
}

module base()
{
    difference()
    {
        union()
        {
            translate([wt-bw/2,wt-bh/2,0])
            cube([bw-wt*2,bh-wt*2,wt]);
            translate([-pw/2,-ph/2,0.01])
            cube([pw,ph,pb]);
            for(y=[-ph/2-wt,ph/2+wt])
            translate([-pw/2,y-wt,0.01])
            cube([pw,wt*2,pb+pt-t]);
        }
        translate([st-pw/2,st-ph/2,wt-0.01])
        cube([pw-st*2,ph-st*2,pb+1]);
        translate([-bhw/2,-bhh/2,-1])
        cube([bhw,bhh,wt+2]);
        for(y=[0,1])
        mirror([0,y,0])
        translate([0,ss/2,-1])
        mirror([0,y,0])
        cylinder(d=sd,h=wt+2);
        for(r=[0,180])rotate(r)
        hull()
        {
            cylinder(d=ed,h=bt+2);
            translate([-ex,ey,-1])
            cylinder(d=ed,h=bt+2);
        }
    }
    for(r=[0,180])rotate(r)
        translate([hx-pw/2,hy-ph/2,0.01])
        difference()
        {
            cylinder(d=hd+wt,h=pb);
            translate([0,0,1])
            cylinder(d=hd-1,h=pb);
        }
}

lid();
translate([bw+wt,0,0])base();