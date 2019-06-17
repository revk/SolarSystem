// Exit plug

use <vl53l0x.scad>

d=22;
d0=30;
t=2;
t0=2;
t1=5;
s=1;

$fn=100;

difference()
{
    union()
    {
        hull()
        {
            cylinder(d=d0-s*2,h=t0);
            translate([0,0,s])
            cylinder(d=d0,h=t0-s);
        }
        cylinder(d=d,h=t0+t+t1);
    }
    translate([1,-2.2,5])
    {
        rotate([180,30,0])
        {
            vl53l0x(30,60); // Spec is 25, but 30 is safer
            #vl53l0x(false,false); // Shows location
        }
        translate([-17.6,-5.5,1.5])
        cube([10,11,10]);
    }
}