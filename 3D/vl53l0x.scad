// VL53L0X template


module vl53l0x(front=25,back=90)
{ // Front is angle for sensor (25 degrees in spec). back is angle for back
    e=10;
    pw=14; // Size of PCB
    pw2=pw-e*tan(90-back);
    ph=11;
    pt=1.5;
    // PCB
    t=(back?e:pt);
    hull()
    {
        translate([-pw/2,-ph/2,0])
        cube([pw,ph,pt]);
        if(back)
        translate([-pw/2,-ph/2,-e])
        cube([pw2,ph,pt]);
    }
    // sensor
    sw=2.5; // Size of sensor
    sh=4.5;
    st=1.5;
    sx=8.8;
    sy=3.25;
    w2=sw+e*tan(front);
    h2=sh+e*tan(front);
    hull()
    {
        translate([sx-pw/2-sw/2,sy-ph/2-sh/2,0])
        cube([sw,sh,pt+st]);
        if(front)
        translate([sx-pw/2-w2/2,sy-ph/2-h2/2,st+e])
        cube([w2,h2,pt]);
    }
    // cap
    cw=1;
    ch=2;
    cx=sx-0.8;
    cy=sy+3.3;
    ct=1.5;
    translate([cx-pw/2-cw/2,cy-ph/2-ch/2,0])
    cube([cw,ch,pt+ct]);
    // back
    bw=8;
    bh=9;
    bt=1.5;
    bx=7;
    by=5.5;
    translate([bx-pw/2-bw/2,by-ph/2-bh/2,-bt])
    cube([bw,bh,bt]);
    wx=1.7;
    wy=(ph-3*2.54)/2;
    wt=3+pt;
    wd=2;
    hull($fn=100)
    {
        translate([wx-pw/2,ph/2-wy,pt/2-wt/2])
        cylinder(d=wd,h=wt);
        translate([wx-pw/2,wy-ph/2,pt/2-wt/2])
        cylinder(d=wd,h=wt);
    }
}

vl53l0x();