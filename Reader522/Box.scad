// Box for SOlar System Min Reader

rw=40;   // RC522 PCB width
rh=60;   // RC522 PCB height
rt=4;   // RC522 component thickness
bw=28;  // Daughter board width
t=1.5;  // Board thicknesses
bh=36;  // Daughter board height
by=1;   // Daughter board offset over edge of RC522
bz=5;   // Daughter board offset from RC522
bx=(rw-bw)/2; // Daughter board offset x
st=10;  // Screw thickness
w=2;    // Wall thickness
cx=5;   // Coner support
cy=1;   // Corner support
clip=1;    // clip size

module lid()
{
    difference()
    {
        cube([rw+w*2,rh+w*2+by,rt+bz+t+st+w*2]);
        translate([w,w,w])
        cube([rw,rh+by,rt+bz+t+st+w*2]);
        hull()
        {
            translate([w,w,rt+bz+t+st+w*2-clip*4-w])
            cube([rw,clip,clip*3]);
            translate([w,w-clip,rt+bz+t+st+w*2-clip*3-w])
            cube([rw,clip,clip]);
        }
        hull()
        {
            translate([w,rh+w*2+by-w*2,rt+bz+t+st+w*2-clip*4-w])
            cube([rw,clip,clip*3]);
            translate([w,rh+w*2+by-w,rt+bz+t+st+w*2-clip*3-w])
            cube([rw,clip,clip]);
        }
    }
    for(x=[0,w*2+rw-cx-w])
        for(y=[0,w*w+rh+by-cy-w])
        translate([x,y,0])
        cube([w+cx,w+cy,w+rt]);
    translate([0,w+rh/2-cy/2,0])
        cube([w*2+rw,cy,w+rt]);
}

module base()
{
    difference()
    {
        translate([w,w,0])
        {
            cube([rw,rh+by,w]);
            cube([rw,w,bz+t+st]);
        }
        // Screw holes in back

        // Cable hole
        
    }
    // screw posts

    // End
    hull()
    {
        translate([w,w,w+clip])
        cube([rw,w,clip*3]);
        translate([w,w-clip,w+clip*2])
        cube([rw,clip,clip]);
    }
}


lid();
translate([rw+w*3,0,0])
base();

