// Exit button front

w=82;       // Width
h=63.5;     // Height
r=5;        // Round corners
t=2;        // Thickness
s=60;       // Screw spacing
m=3;        // Screw hole size

pcbw=13.9;
pcbh=10.75;
pcbo=1;
sensorx=9;
sensory=2.8;
sensorw=2.4;
sensorh=4.4;
resx=8.2;
resy=5.8;
resw=1.1;
resh=1.7;
rest=0.8;


difference()
{
    translate([-w/2,-h/2,0])
    minkowski()
    {
        translate([r,r,0])
        cube([w-r*2,h-r*2,t/2]);
        cylinder(r=r,h=t/2,$fn=100);
    }
    for(x=[0,1])
    mirror([x,0,0])
    translate([s/2,0,0])
    {
        translate([0,0,-1])
        cylinder(d=m,h=t+2,$fn=100);
        translate([0,0,-0.01])
        cylinder(d1=m*2,d2=m,h=m/2,$fn=100);        
    }
    mirror([0,1,0])
    translate([-sensorx,-sensory,-0.01])
    {
        translate([0,0,pcbo])
        cube([pcbw,pcbh,t]);
        translate([sensorx-sensorw/2,sensory-sensorh/2,0])
        cube([sensorw,sensorh,t+1]);
        translate([resx-resw/2,resy-resh/2,pcbo-rest])
        cube([resw,resh,t]);
    }
}
