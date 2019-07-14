// Door images

h=2000;
w=900;
t=45;
mode="FAULT";

wall=200;
$fn=100;

module prop()
{
    translate([w*.8,-t-100,0])
    cylinder(r=100,h=h/4);
}

module tamper()
{
    translate([w*0.8,-t-20,h/2])
    {
        rotate([10,70,0])
        cylinder(r=20,h=w*0.2);
    }
}

module force()
{
    translate([w*0.8,-t-20,h/2-200])
    rotate([90,0,0])
    cylinder(r=100,h=300);
}

module bolt(x=0,y=h/2)
{
    translate([w*0.8-x,-t-20,y])
    {
        rotate([0,90,0])
        cylinder(r=20,h=w*0.2);
    }
}

module padlock(x=0,y=h/2)
{
    translate([w*0.85-x,-t-25,y-130])
    {
        scale([1,0.5,1])
        cylinder(r=50,h=100);
    }
}

module lock(y=h/2)
{
    bolt(0,y);
    padlock(0,y);
}

module unlock(y=h/2)
{
    bolt(20,y);
}

module door(a=0)
{
    color("BurlyWood")
    rotate([0,0,a])
    {
        translate([0,-t,0])
        cube([w,t,h]);
        if(mode=="PROPPED")prop();
        if(mode=="LOCKING"){padlock();unlock();}
        if(mode=="UNLOCKED")unlock();
        if(mode=="UNLOCKING"){padlock();unlock();}
        if(mode=="LOCKED")lock();
        if(mode=="DEADLOCKED"){lock(h/5);lock();lock(4*h/5);}
        if(mode=="FORCED"){lock();force();}
        if(mode=="TAMPER")tamper();
    }
    if(mode=="UNUSED")
    {
        translate([w/2,-wall-t/2,h/2])
        {
            rotate([0,30,0])
            cube([w*1.5,t,w*0.2],true);
            rotate([0,-25,0])
            cube([w*1.5,t,w*0.2],true);
        }
    }
}

module frame()
{
    color("LightYellow")
    difference()
    {
        translate([-1000,-wall,0])
        cube([w+2000,wall,h+1000]);
        translate([0,-wall-1,-1])
        cube([w,wall+2,h]);
    }
}

frame();
if(mode=="UNUSED")door(0);
if(mode=="NEW")door(-10);
//if(mode=="OFFLINE")door(0);
if(mode=="DEADLOCKED")door(0);
if(mode=="LOCKING")door(0);
if(mode=="LOCKED")door(4);
if(mode=="UNLOCKING")door(5);
if(mode=="UNLOCKED")door(0);
if(mode=="OPEN")door(70);
if(mode=="CLOSED")door(0);
if(mode=="LOCKING")door(0);
if(mode=="PROPPED")door(80);
if(mode=="PROPPEDOK")door(80);
if(mode=="AJAR")door(10);
if(mode=="FAULT")rotate([-30,-10,0])door(10);
if(mode=="TAMPER")door(0);
if(mode=="FORCED")door(50);