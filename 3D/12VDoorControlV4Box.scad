// Solar System Door Control V4 box


w=50;
h=30;
t=1.6;
b=4;
f=11;
s=2;
e=1;
v4a=1; // hole for 2x2 connector for PN532

$fn=100;

module board()
{
    translate([-w/2,-h/2,b+e])
    {
        cube([50,30,1.6]);
        translate([2.5,1,-3])
        cube([45,28,3.1]);
        translate([8,6,-b])
        cube([17,18,4.1]);
        translate([8,0,-1])
        cube([17,24,1.1]);
        translate([30,0,-1])
        cube([12,2,1.1]);
        cube([w,h,t+9]);
        translate([w-8,0,0])
        cube([8,h,t+f]);
    }
}

module box()
{
    translate([-w/2-s,-h/2-s,0])
    cube([w+s*2,h+s*2,b+t+f+e*2]);
}

module holes()
{
    // Small screws
    translate([4.14-w/2,2.75-h/2,b+e+t+2.5])
    rotate([0,-90,0])
    hull()
    {
        cylinder(d=5,h=5+s);
        translate([0,7*3.5,0])
        cylinder(d=5,h=5+s);
    }
    for(y=[0:7])
    translate([4.14-w/2,2.75-h/2+y*3.5,b+e+t])
    cylinder(d=3.2,h=f+s*2);
    // Large screws
    translate([w/2-4,2.5-h/2,b+e+t+2.5])
    rotate([0,90,0])
    hull()
    {
        cylinder(d=5,h=5+s);
        translate([0,5*5,0])
        cylinder(d=5,h=5+s);
    }
    for(y=[0:5])
    translate([w/2-4,2.5-h/2+y*5,b+e+t])
    cylinder(d=3.6,h=f+s*2);
    // Square
    translate([24.746-w/2,12.705-h/2,b+e+t])
    cube([6.65,6.3,f+e*2]);
    if(v4a)
    translate([12.09-w/2,22.873-h/2,b+e+t])
    cube([6.65,6.3,f+e*2]);
    // Wire
    if(!v4a)
    translate([30-w/2,-h/2-s*2,b+e+t+2.5])
    rotate([-90,0,0])
    hull()
    {
        cylinder(d=5,h=s*4);
        translate([0,10-f,0])
        cylinder(d=5,h=s*4);
    }
}

module cut()
{
    translate([-w/2-s*2,-h/2-s*2,-e])
    cube([w+s*4,h+s*4,e*2+b+t+2]);
    translate([-w/2-s/2,-h/2-s/2,-e])
    cube([w+s,h+s,e*2+b+t+2+s/2]);
    
}

module base()
{
    translate([0,0,(b+t+f+e*2)])
    rotate([180,0,0])
    difference()
    {
        box();
        cut();
        board();
        holes();
    }
}

module lid()
{
    difference()
    {
        intersection()
        {
            box();
            cut();
        }
        board();
        holes();
    }
}

translate([0,-h/2-s*2])
base();
translate([0,h/2+s*2,0])
lid();
