// Solar System Door Control V4 box


w=40;
h=28;
t=1.6+0.2;
b=5;
f=11;
s=2;
e=1;
d=0.99;

$fn=100;

h1x=11.387-0.5;h1y=4.412-0.5; // From SVG
h2x=4.967-0.4;h2y=3.588-0.4;
h3x=35.502+1.45-3-1;h3y=10.801+1.45-3-1;
        
module board(base)
{
    mirror([1,0,0])
    translate([-w/2,-h/2,b+e])
    {
        cube([w,h,base?100:t]);
        translate([0,h2y-2.5-0.75,0])
        cube([8.5,5*4+1.5,12]);
        translate([h1x-3.5/2-1,h1y-4,0])
        cube([3.5*8+2,8,11]);
        translate([7.5,7.5,0])
        cube([14,20.5,6]);
        translate([1.5,20,-3])
        cube([5,7,4]);
        translate([1.5,1.5,-3])
        cube([36.5,26,4]);
        translate([14,8,-5])
        cube([17,19,6]);
        translate([13,8,-1.1])
        cube([20,28,1.201]);   
        for(h=[0:1:2])
        translate([h3x-0.5,h3y+h*6.604-0.5,0])
        cube([7,7,9]);
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
        translate([h1x+3.5*h,h1y,0])
        cylinder(d=4,h=20);
        hull()
        {
            translate([h1x,3.9,5])
            rotate([90,0,0])
            cylinder(d=5,h=10);        
            translate([h1x+3.5*7,h1y,5])
            rotate([90,0,0])
            cylinder(d=5,h=10); 
        }
        for(h=[0:1:3])
        translate([h2x,h2y+5*h,0])
        cylinder(d=4,h=20);
        hull()
        {
            translate([h2x,h2y,5])
            rotate([0,-90,0])
            cylinder(d=5,h=10);        
            translate([h2x,h2y+3*5,5])
            rotate([0,-90,0])
            cylinder(d=5,h=10); 
        }
        for(h=[0:1:2])
        translate([h3x+0.5,h3y+h*6.604+0.5,0])
        {
            cube([5,5,20]);
            translate([4,1.5,0])
            cube([2,1.5,20]);
        }
    }
}

module cut()
{
    difference()
    {
        union()
        {
            translate([-w/2-s*2,-h/2-s*2,d-e])
            cube([w+s*4,h+s*4,e*2+b+t+2]);
            translate([-w/2-s/2,-h/2-s/2,d-e])
            cube([w+s,h+s,e*2+b+t+2+s/2]);
        }
        mirror([1,0,0])
        translate([-w/2,-h/2,b+e])
        translate([13,8,0])
        cube([20,28,10]);   
    }
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
