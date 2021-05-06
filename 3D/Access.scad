// Access controller case
$fn=120;

module top()
{
    difference()
    {
        hull()for(x=[0,50])for(y=[0,30])translate([x,y,0])cylinder(r=4,h=7);
        translate([-2,-2,-1])cube([54,37,7]);
        translate([3-2.45,27.35+6.6-4.9,2])cube([48.9,6,6]);
    }
    for(x=[10-2,54-10-2])translate([x-5,-3,0])hull(){cube([10,2,1]);cube([10,1,2]);}
    translate([-3,-3,5.25])cube([19.5,21,1.5]);
    translate([41,12,5.1])cube([9+3,13,1.5]);
}

module base()
{
    difference()
    {
        translate([-2,-2,0])cube([54,37,3]);
        for(x=[10-2,54-10-2])translate([x-5,-3,-0.001])hull(){cube([10,2,1]);cube([10,1,2]);}
        translate([-0.2,-0.2,1.2])cube([50,34,2]);
        for(x=[1,2,3,4,5,6,8,9,10,11,13,14,15,17,18])translate([x*2.5+0.5,27.35,0.2])cylinder(d1=1,d2=2,h=1.5);
        for(y=[-0.635,0.635])translate([42.1,24.3+y,0.2])cylinder(d1=1,d2=2,h=1.5);
        translate([3-2.45,27.35+6.6-4.9,2])cube([48.9,10,4.9]);
        translate([29.5,16.5,-1])cylinder(d=3.5,h=9);
        translate([43.8-0.2,15.4-0.2,1])cube([7.4+0.4,9+0.4,4.2]);
    }
    translate([-1,-1,1])cube([7,25,2]);
}

rotate([0,180,0])translate([-50,0,-7])top();
translate([0,40,0])base();