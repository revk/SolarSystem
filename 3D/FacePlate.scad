// Exit button front

w=81.5;       // Width
h=63.5;     // Height
r=6.985;        // Round corners
t=2.2;        // Thickness
s=60;       // Screw spacing
m=3.5;        // Screw hole size

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
    translate([-sensorx,sensory-pcbh,pcbo-rest])
    cube([padw,pcbh,t]);
}

translate([pcbw-sensorx-postx,sensory-pcbh+posty,0])
cylinder(d=postd,h=t,$fn=100);
