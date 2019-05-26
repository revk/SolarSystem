// Exit button front

w=81.5;       // Width
h=63.5;     // Height
r=6.985;        // Round corners
t=2;        // Thickness
s=60;       // Screw spacing
m=3.5;        // Screw hole size

w2=30;  // Centre area
h2=30;
t2=2.5;

pcbw=13.9;
pcbh=10.9;
pcbo=1;

sensorx=8.9;
sensory=2.9;
sensorw=2.75;
sensorh=4.75;

resx=8.2;
resy=6.5;
resw=1.2;
resh=2.3;
rest=0.8;

postx=2.54;
posty=2.54;
postd=2.5;

padw=2.54;

difference()
{
    union()
    {
        translate([-w/2,-h/2,0])
        minkowski()
        {
            translate([r,r,0])
            cube([w-r*2,h-r*2,t/2]);
            cylinder(r=r,h=t/2,$fn=100);
        }
        translate([-w2/2,-h2/2,0])
        cube([w2,h2,t2]);
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
    rotate(90)
    mirror([0,1,0])
    translate([-sensorx,-sensory,-0.01])
    {
        translate([0,0,pcbo])
        cube([pcbw,pcbh,t2]);
        translate([sensorx-sensorw/2,sensory-sensorh/2,0])
        cube([sensorw,sensorh,t2+1]);
        translate([resx-resw/2,resy-resh/2,pcbo-rest])
        cube([resw,resh,t2]);
    }
    rotate(90)
    translate([-sensorx,sensory-pcbh,pcbo-rest])
    cube([padw,pcbh,t2]);
}

rotate(90)
translate([pcbw-sensorx-postx,sensory-pcbh+posty,0])
cylinder(d=postd,h=t,$fn=100);
