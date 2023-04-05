// Generated case design for PCB/Access2/Access2.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-04-04 15:39:52
// title:	Access Control
// date:	${DATE}
// rev:	3
// company:	Adrian Kennard Andrews & Arnold Ltd
// comment:	toot.me.uk/@RevK
// comment:	www.me.uk
//

// Globals
margin=0.500000;
overlap=2.000000;
lip=0.000000;
casebase=2.000000;
casetop=6.000000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[50.000000,27.000000],[0.000000,27.000000],[0.000000,18.100000],[6.400000,18.100000],[6.400000,0.000000],[50.000000,0.000000]],paths=[[0,1,2,3,4,5]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[50.000000,27.000000],[0.000000,27.000000],[0.000000,18.100000],[6.400000,18.100000],[6.400000,0.000000],[50.000000,0.000000]],paths=[[0,1,2,3,4,5]]);}
spacing=66.000000;
pcbwidth=50.000000;
pcblength=27.000000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([45.750000,19.800000,0.800000])rotate([0,0,180.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([36.500000,19.800000,0.800000])rotate([0,0,180.000000])m1(pushed,hulled,3); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([24.750000,19.800000,0.800000])rotate([0,0,180.000000])m4(pushed,hulled,4); // RevK:PTSM-HH-4-RA PTSM-HH-4-RA (back)
translate([9.250000,19.800000,0.800000])rotate([0,0,180.000000])m4(pushed,hulled,6); // RevK:PTSM-HH-4-RA PTSM-HH-4-RA (back)
translate([38.250000,13.500000,0.800000])rotate([0,0,180.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([38.800000,4.350000,0.800000])rotate([0,0,-90.000000])translate([2.050000,0.000000,1.150000])rotate([-0.000000,-0.000000,-90.000000])m10(pushed,hulled); // RevK:ESE13 ESE13V01D (back)
translate([13.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([43.700000,2.500000,0.800000])rotate([0,0,90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([29.500000,9.600000,0.800000])rotate([0,0,90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([41.900000,15.400000,0.800000])rotate([0,0,180.000000])m13(pushed,hulled); // RevK:C_0805 C_0805_2012Metric (back)
translate([48.300000,2.300000,0.800000])rotate([0,0,-90.000000])translate([0.000000,-0.100000,0.400000])rotate([-90.000000,-0.000000,-0.000000])m15(pushed,hulled); // RevK:MHS190RGBCT LED_0603_1608Metric (back)
translate([45.535000,9.750000,0.800000])rotate([0,0,90.000000])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m17(pushed,hulled); // RevK:USC16-TR CSP-USC16-TR (back)
translate([29.800000,1.800000,0.800000])rotate([0,0,90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([42.600000,17.200000,0.800000])rotate([0,0,180.000000])m20(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([22.500000,13.100000,0.800000])rotate([-0.000000,-0.000000,-90.000000])m22(pushed,hulled); // RevK:SOT-23-Thin-6-Reg SOT-23-6 (back)
translate([48.500000,15.200000,0.800000])rotate([0,0,180.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([12.000000,6.750000,0.800000])rotate([0,0,90.000000])m23(pushed,hulled); // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02 (back)
translate([17.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([26.400000,13.100000,0.800000])rotate([-0.000000,-0.000000,-90.000000])m25(pushed,hulled); // RevK:L_4x4_ TYA4020 (back)
translate([27.900000,1.800000,0.800000])rotate([0,0,90.000000])m13(pushed,hulled); // RevK:C_0805 C_0805_2012Metric (back)
translate([24.780000,9.930000,0.800000])m20(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([20.400000,5.100000,0.800000])rotate([0,0,90.000000])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([24.770000,2.190000,0.800000])m20(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([38.700000,7.800000,0.800000])rotate([0,0,180.000000])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([21.650000,1.800000,0.800000])rotate([0,0,90.000000])m13(pushed,hulled); // RevK:C_0805 C_0805_2012Metric (back)
translate([23.500000,17.000000,0.800000])rotate([0,0,-90.000000])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([27.900000,9.550000,0.800000])rotate([0,0,90.000000])m13(pushed,hulled); // RevK:C_0805 C_0805_2012Metric (back)
translate([18.300000,14.100000,0.800000])rotate([0,0,90.000000])m33(pushed,hulled); // RevK:SOT-323_SC-70 SOT-323_SC-70 (back)
translate([19.950000,2.150000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([38.250000,11.000000,0.800000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([15.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([29.500000,13.600000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([46.300000,2.500000,0.800000])rotate([0,0,90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([41.750000,12.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([9.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([36.000000,7.800000,0.800000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([29.800000,4.700000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([21.650000,9.550000,0.800000])rotate([0,0,90.000000])m13(pushed,hulled); // RevK:C_0805 C_0805_2012Metric (back)
translate([26.400000,5.350000,0.800000])rotate([-0.000000,-0.000000,-90.000000])m25(pushed,hulled); // RevK:L_4x4_ TYA4020 (back)
translate([41.750000,7.400000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([21.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([36.500000,16.250000,0.800000])rotate([0,0,90.000000])rotate([-0.000000,-0.000000,90.000000])m37(pushed,hulled); // RevK:Special-SOP-4-3.7x4.55 SO-4_4.4x3.6mm_P2.54mm (back)
translate([30.500000,16.750000,0.800000])rotate([0,0,180.000000])m38(pushed,hulled); // Package_TO_SOT_SMD:SOT-23 SOT-23 (back)
translate([33.750000,11.500000,0.800000])rotate([0,0,90.000000])m39(pushed,hulled); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm (back)
translate([45.800000,15.200000,0.800000])rotate([0,0,180.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([47.300000,17.200000,0.800000])m20(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([19.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([22.500000,5.350000,0.800000])rotate([-0.000000,-0.000000,-90.000000])m22(pushed,hulled); // RevK:SOT-23-Thin-6-Reg SOT-23-6 (back)
translate([11.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([32.050000,7.800000,0.800000])rotate([0,0,180.000000])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([7.500000,17.000000,0.800000])rotate([0,0,-90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([20.400000,12.850000,0.800000])rotate([0,0,90.000000])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([38.250000,12.250000,0.800000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([45.000000,2.500000,0.800000])rotate([0,0,90.000000])m8(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([15.000000,25.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([11.000000,25.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([7.000000,25.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([18.000000,22.500000,0.000000])rotate([180,0,0])m40(pushed,hulled); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([6.000000,22.500000,0.000000])rotate([180,0,0])m40(pushed,hulled); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([17.000000,24.975000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([10.000000,22.500000,0.000000])rotate([180,0,0])m40(pushed,hulled); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([5.000000,25.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([9.000000,25.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([14.000000,22.500000,0.000000])rotate([180,0,0])m40(pushed,hulled); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([13.000000,25.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m31(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m1(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA
hull()
{ // Socket
	b(0,-7.5/2+0.3,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2+0.3,0,1.7+n*2.5-2,7.5,5);
}
// Plug
b(0,-10.5/2-7.5+0.3,0,1.1+n*2,10.5,5);
// Pins
if(!hulled)for(p=[0:n-1])translate([-2.5*(n-1)/2+p*2.5,0,-2.1])cylinder(r=0.3,h=2.1);
}

module m4(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH-4-RA PTSM-HH-4-RA
hull()
{ // Socket
	b(0,-7.5/2+0.3,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2+0.3,0,1.7+n*2.5-2,7.5,5);
}
// Plug
b(0,-10.5/2-7.5+0.3,0,1.1+n*2,10.5,5);
// Pins
if(!hulled)for(p=[0:n-1])translate([-2.5*(n-1)/2+p*2.5,0,-2.1])cylinder(r=0.3,h=2.1);
}

module m8(pushed=false,hulled=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m10(pushed=false,hulled=false)
{ // RevK:ESE13 ESE13V01D
if(!hulled&&pushed) translate([0,2.05,-1.15])
{
	b(0,0,0,3.6+0.4,4.2+0.4,1.2);
	b(0,-3.08,0,1.2+0.4,1.95+0.4,0.9);
}

}

module m13(pushed=false,hulled=false)
{ // RevK:C_0805 C_0805_2012Metric
b(0,0,0,2,1.45,0.2); // Pad size
b(0,0,0,2,1.2,1); // Chip
}

module m15(pushed=false,hulled=false)
{ // RevK:MHS190RGBCT LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(!hulled&&pushed)b(0,0,0,1,1,20);
}

module m17(pushed=false,hulled=false)
{ // RevK:USC16-TR CSP-USC16-TR
rotate([-90,0,0])translate([-4.47,-3.84,0])
{
	translate([1.63,0,1.63])
	rotate([-90,0,0])
	hull()
	{
		cylinder(d=3.26,h=7.75,$fn=24);
		translate([5.68,0,0])
		cylinder(d=3.26,h=7.75,$fn=24);
	}
	translate([0,6.65,0])cube([8.94,1.1,1.63]);
	translate([0,2.2,0])cube([8.94,1.6,1.63]);
	if(!hulled)
	{
		// Plug
		translate([1.63,-20,1.63])
		rotate([-90,0,0])
		hull()
		{
			cylinder(d=2.5,h=21,$fn=24);
			translate([5.68,0,0])
			cylinder(d=2.5,h=21,$fn=24);
		}
		translate([1.63,-22.5,1.63])
		rotate([-90,0,0])
		hull()
		{
			cylinder(d=7,h=21,$fn=24);
			translate([5.68,0,0])
			cylinder(d=7,h=21,$fn=24);
		}
	}
}

}

module m20(pushed=false,hulled=false)
{ // RevK:D_1206 D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m22(pushed=false,hulled=false)
{ // RevK:SOT-23-Thin-6-Reg SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m23(pushed=false,hulled=false)
{ // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
translate([-13.2/2,-16.6/2+2.7,0])
{
	if(!hulled)cube([13.2,16.6,0.8]);
	cube([13.2,11.2,2.4]);
}
}

module m25(pushed=false,hulled=false)
{ // RevK:L_4x4_ TYA4020
b(0,0,0,4,4,2.1);
}

module m31(pushed=false,hulled=false)
{ // RevK:C_0603_ C_0603_1608Metric
b(0,0,0,1.6,0.95,0.2); // Pad size
b(0,0,0,1.6,0.8,1); // Chip
}

module m33(pushed=false,hulled=false)
{ // RevK:SOT-323_SC-70 SOT-323_SC-70
b(0,0,0,1.26,2.2,1.2);
b(0,0,0,2.2,2.2,0.6);
}

module m37(pushed=false,hulled=false)
{ // RevK:Special-SOP-4-3.7x4.55 SO-4_4.4x3.6mm_P2.54mm
b(0,0,0,4.55+0.25,3.7+0.25,2.1+0.2);
b(0,0,0,7.0+0.44,3,1.5);
}

module m38(pushed=false,hulled=false)
{ // Package_TO_SOT_SMD:SOT-23 SOT-23
b(0,0,0,2.92,1.3,1.2);
b(0,0,0,2.27,2.4,0.6);
}

module m39(pushed=false,hulled=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m40(pushed=false,hulled=false)
{ // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
b(0,0,0,1.15,2.0,1.1);
b(0,0,0,2.1,2.0,0.6);
}

height=casebase+pcbthickness+casetop;
$fn=48;

module boardh(pushed=false)
{ // Board with hulled parts
	union()
	{
		if(!nohull)intersection()
		{
			translate([0,0,hullcap-casebase])outline(casebase+pcbthickness+casetop-hullcap*2,-hulledge);
			hull()board(pushed,true);
		}
		board(pushed,false);
		pcb();
	}
}

module boardf()
{ // This is the board, but stretched up to make a push out in from the front
	render()
	{
		intersection()
		{
			translate([-casewall-1,-casewall-1,-casebase-1]) cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,height+2]);
			union()
			{
				minkowski()
				{
					boardh(true);
					cylinder(h=height+100,d=margin,$fn=8);
				}
				board(false,false);
			}
		}
	}
}

module boardb()
{ // This is the board, but stretched down to make a push out in from the back
	render()
	{
		intersection()
		{
			translate([-casewall-1,-casewall-1,-casebase-1]) cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,height+2]);
			union()
			{
				minkowski()
				{
					boardh(true);
					translate([0,0,-height-100])
					cylinder(h=height+100,d=margin,$fn=8);
				}
				board(false,false);
			}
		}
	}
}

module boardm()
{
	render()
	{
 		minkowski()
 		{
			translate([0,0,-margin/2])cylinder(d=margin,h=margin,$fn=8);
 			boardh(false);
		}
		//intersection()
    		//{
        		//translate([0,0,-(casebase-hullcap)])pcb(pcbthickness+(casebase-hullcap)+(casetop-hullcap));
        		//translate([0,0,-(casebase-hullcap)])outline(pcbthickness+(casebase-hullcap)+(casetop-hullcap));
			boardh(false);
    		//}
 	}
}

module pcbh(h=pcbthickness,r=0)
{ // PCB shape for case
	if(useredge)outline(h,r);
	else hull()outline(h,r);
}

module pyramid()
{ // A pyramid
 polyhedron(points=[[0,0,0],[-height,-height,height],[-height,height,height],[height,height,height],[height,-height,height]],faces=[[0,1,2],[0,2,3],[0,3,4],[0,4,1],[4,3,2,1]]);
}

module wall(d=0)
{ // The case wall
	translate([0,0,-casebase-d])
	{
		if(useredge)
			intersection()
			{
				pcb(height+d*2,margin/2+d);
				pcbh(height+d*2,margin/2+d);
			}
		else pcbh(height+d*2,margin/2+d);
	}
}

module cutf()
{ // This cut up from base in the wall
	intersection()
	{
		boardf();
		difference()
		{
			translate([-casewall+0.01,-casewall+0.01,-casebase+0.01])cube([pcbwidth+casewall*2-0.02,pcblength+casewall*2-0.02,casebase+overlap+lip]);
			wall();
			boardb();
		}
	}
}

module cutb()
{ // The cut down from top in the wall
	intersection()
	{
		boardb();
		difference()
		{
			translate([-casewall+0.01,-casewall+0.01,0.01])cube([pcbwidth+casewall*2-0.02,pcblength+casewall*2-0.02,casetop+pcbthickness]);
			wall();
			boardf();
		}
	}
}

module cutpf()
{ // the push up but pyramid
	render()
	intersection()
	{
		minkowski()
		{
			pyramid();
			cutf();
		}
		difference()
		{
			translate([-casewall-0.01,-casewall-0.01,-casebase-0.01])cube([pcbwidth+casewall*2+0.02,pcblength+casewall*2+0.02,casebase+overlap+lip+0.02]);
			wall();
			boardh(true);
		}
		translate([-casewall,-casewall,-casebase])case();
	}
}

module cutpb()
{ // the push down but pyramid
	render()
	intersection()
	{
		minkowski()
		{
			scale([1,1,-1])pyramid();
			cutb();
		}
		difference()
		{
			translate([-casewall-0.01,-casewall-0.01,-0.01])cube([pcbwidth+casewall*2+0.02,pcblength+casewall*2+0.02,casetop+pcbthickness+0.02]);
			wall();
			boardh(true);
		}
		translate([-casewall,-casewall,-casebase])case();
	}
}

module case()
{ // The basic case
	hull()
	{
		translate([casewall,casewall,0])pcbh(height,casewall-edge);
		translate([casewall,casewall,edge])pcbh(height-edge*2,casewall);
	}
}

module cut(d=0)
{ // The cut point in the wall
	translate([casewall,casewall,casebase+lip])pcbh(casetop+pcbthickness-lip+1,casewall/2+d/2+margin/4);
}

module base()
{ // The base
	difference()
	{
		case();
		difference()
		{
			union()
			{
				translate([-1,-1,casebase+overlap+lip])cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,casetop+1]);
				cut(fit);
			}
		}
		translate([casewall,casewall,casebase])boardf();
		translate([casewall,casewall,casebase])boardm();
		translate([casewall,casewall,casebase])cutpf();
	}
	translate([casewall,casewall,casebase])cutpb();
}

module top()
{
	translate([0,pcblength+casewall*2,height])rotate([180,0,0])
	{
		difference()
		{
			case();
			difference()
			{
				translate([-1,-1,-1])cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,casebase+overlap+lip-margin+1]);
				cut(-fit);
			}
			translate([casewall,casewall,casebase])boardb();
			translate([casewall,casewall,casebase])boardm();
			translate([casewall,casewall,casebase])cutpb();
		}
		translate([casewall,casewall,casebase])cutpf();
	}
}

module test()
{
	translate([0*spacing,0,0])base();
	translate([1*spacing,0,0])top();
	translate([2*spacing,0,0])pcb();
	translate([3*spacing,0,0])outline();
	translate([4*spacing,0,0])wall();
	translate([5*spacing,0,0])board();
	translate([6*spacing,0,0])board(false,true);
	translate([7*spacing,0,0])board(true);
	translate([8*spacing,0,0])boardh();
	translate([9*spacing,0,0])boardf();
	translate([10*spacing,0,0])boardb();
	translate([11*spacing,0,0])cutpf();
	translate([12*spacing,0,0])cutpb();
	translate([13*spacing,0,0])cutf();
	translate([14*spacing,0,0])cutb();
	translate([15*spacing,0,0])case();
}

module parts()
{
	base();
	translate([spacing,0,0])top();
}
base(); translate([spacing,0,0])top();
