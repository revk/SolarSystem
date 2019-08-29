// Solar System Door Control V4 box


w=40;
h=28;
t=1.6;
b=5;
f=11;
s=2;
e=1;
d=-1;
v4a=1; // hole for 2x2 connector for PN532

$fn=100;

module board(base)
{
    mirror([1,0,0])
    translate([-w/2,-h/2,b+e])
    {
        cube([w,h,base?100:t]);
        cube([8.5,22,12]);
        translate([9,0,0])
        cube([30,8,11]);
        translate([32,7,0])
        cube([8,20,9]);
        translate([7.5,7.5,0])
        cube([13,18,6]);
        translate([1.5,20,-3])
        cube([5,8,4]);
        translate([1.5,1.5,-3])
        cube([36.5,26,4]);
        translate([14,8,-5])
        cube([17,19,6]);
        translate([14,8,-1])
        cube([18,28,1.001]);   
    }
}

module box()
{
    translate([-w/2-s,-h/2-s,0])
    cube([w+s*2,h+s*2,b+t+f+e*2]);
}

module holes()
{
    mirror([1,0,0])
    translate([-w/2,-h/2,b+e])
    {
        for(h=[0:1:7])
        translate([11.9+3.5*h,3.9,0])
        cylinder(d=4,h=20);
        hull()
        {
            translate([11.9,3.9,3])
            rotate([90,0,0])
            cylinder(d=5,h=10);        
            translate([11.9+3.5*7,3.9,3])
            rotate([90,0,0])
            cylinder(d=5,h=10); 
        }
        for(h=[0:1:3])
        translate([4.6,3.2+5*h,0])
        cylinder(d=4,h=20);
        hull()
        {
            translate([4.6,3.2,3])
            rotate([0,-90,0])
            cylinder(d=5,h=10);        
            translate([4.6,3.2+3*5,3])
            rotate([0,-90,0])
            cylinder(d=5,h=10); 
        }
        for(h=[8.1,14.7,21.3])
        translate([32.6,h,0])
        cube([6,6,20]);
    }
}

module cut()
{
    translate([-w/2-s*2,-h/2-s*2,d-e])
    cube([w+s*4,h+s*4,e*2+b+t+2]);
    translate([-w/2-s/2,-h/2-s/2,d-e])
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
        board(1);
        holes();
    }
}

translate([0,-h/2-s*2])
base();
translate([0,h/2+s*2,0])
lid();
