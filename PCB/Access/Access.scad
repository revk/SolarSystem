// Generated case design for Access/Access.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-08-20 06:41:21
// title:	Access Control
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
pcbthickness=1.200000;
nohull=true;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.000000,0.000000],[0.000000,24.400000],[49.300000,24.400000],[49.300000,0.000000]],paths=[[0,1,2,3]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.000000,0.000000],[0.000000,24.400000],[49.300000,24.400000],[49.300000,0.000000]],paths=[[0,1,2,3]]);}
spacing=65.300000;
pcbwidth=49.300000;
pcblength=24.400000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([45.950000,17.250000,1.200000])rotate([0,0,180.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA-W PTSM-HH1-2-RA-W (back)
translate([32.150000,17.250000,1.200000])rotate([0,0,180.000000])m3(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([22.750000,17.257500,1.200000])rotate([0,0,180.000000])m5(pushed,hulled,4); // RevK:PTSM-HH-4-RA PTSM-HH-4-RA (back)
translate([8.350000,17.257500,1.200000])rotate([0,0,180.000000])m5(pushed,hulled,6); // RevK:PTSM-HH-4-RA PTSM-HH-4-RA (back)
translate([37.300000,8.500000,1.200000])rotate([0,0,90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([46.550000,11.900000,1.200000])rotate([0,0,-90.000000])translate([2.050000,0.000000,1.150000])rotate([-0.000000,-0.000000,-90.000000])m11(pushed,hulled); // RevK:ESE13 ESE13V01D (back)
translate([9.200000,15.400000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([14.200000,14.400000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([39.800000,8.500000,1.200000])rotate([0,0,90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([4.100000,12.800000,1.200000])rotate([0,0,180.000000])m18(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([16.700000,0.700000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([9.200000,14.400000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([7.400000,10.300000,1.200000])rotate([0,0,90.000000])rotate([-0.000000,-0.000000,-90.000000])m21(pushed,hulled); // RevK:L_4x4_ TYA4020 (back)
translate([1.300000,6.800000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([44.335000,4.400000,1.200000])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m23(pushed,hulled); // RevK:USC16-TR CSP-USC16-TR (back)
translate([4.200000,14.400000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([10.000000,1.500000,1.200000])rotate([0,0,-90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([8.100000,3.300000,1.200000])rotate([0,0,-90.000000])m28(pushed,hulled); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([14.200000,15.400000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([14.800000,11.400000,1.200000])m29(pushed,hulled); // RevK:SOT-363_SC-70-6 SOT-363_SC-70-6 (back)
translate([14.800000,4.200000,1.200000])m29(pushed,hulled); // RevK:SOT-363_SC-70-6 SOT-363_SC-70-6 (back)
translate([6.800000,0.800000,1.200000])m18(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([36.832500,2.700000,1.200000])rotate([0,0,-90.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([6.100000,3.300000,1.200000])rotate([0,0,-90.000000])m18(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([36.182500,4.800000,1.200000])rotate([0,0,180.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([4.300000,7.800000,1.200000])rotate([0,0,180.000000])m18(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([16.500000,14.900000,1.200000])rotate([0,0,90.000000])m30(pushed,hulled); // RevK:SOT-323_SC-70 SOT-323_SC-70 (back)
translate([39.050000,17.257500,1.200000])rotate([0,0,180.000000])m3(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([0.900000,12.100000,1.200000])rotate([0,0,90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([11.700000,15.400000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([0.900000,8.500000,1.200000])rotate([0,0,90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([36.182500,6.600000,1.200000])rotate([0,0,180.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([17.800000,13.300000,1.200000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([4.800000,10.300000,1.200000])rotate([0,0,90.000000])m18(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([38.800000,1.300000,1.200000])rotate([0,0,90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([4.200000,15.400000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([10.000000,3.300000,1.200000])rotate([0,0,-90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([14.800000,6.600000,1.200000])m29(pushed,hulled); // RevK:SOT-363_SC-70-6 SOT-363_SC-70-6 (back)
translate([18.500000,0.700000,1.200000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([37.900000,1.300000,1.200000])rotate([0,0,-90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([1.700000,14.400000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([18.200000,14.800000,1.200000])rotate([0,0,-90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([37.505000,11.800000,1.200000])rotate([-0.000000,-0.000000,90.000000])m34(pushed,hulled); // RevK:Special-SOP-4-3.7x4.55 SO-4_4.4x3.6mm_P2.54mm (back)
// Missing D5.1 D_SOD-123
translate([36.182500,5.700000,1.200000])rotate([0,0,180.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([2.800000,10.300000,1.200000])rotate([0,0,90.000000])m28(pushed,hulled); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([6.700000,14.400000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([19.200000,14.800000,1.200000])rotate([0,0,-90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([27.500000,7.700000,1.200000])rotate([0,0,180.000000])m37(pushed,hulled); // RevK:ESP32-S3-MINI-1 ESP32-S3-MINI-1 (back)
translate([6.700000,15.400000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([42.105000,11.800000,1.200000])rotate([-0.000000,-0.000000,90.000000])m34(pushed,hulled); // RevK:Special-SOP-4-3.7x4.55 SO-4_4.4x3.6mm_P2.54mm (back)
translate([14.800000,9.000000,1.200000])m29(pushed,hulled); // RevK:SOT-363_SC-70-6 SOT-363_SC-70-6 (back)
translate([38.132500,5.700000,1.200000])rotate([0,0,90.000000])translate([0.000000,-0.400000,0.000000])m40(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric (back)
translate([38.132500,5.700000,1.200000])rotate([0,0,90.000000])translate([0.000000,0.400000,0.000000])m40(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric (back)
translate([3.500000,3.300000,1.200000])rotate([0,0,-90.000000])rotate([-0.000000,-0.000000,-90.000000])m21(pushed,hulled); // RevK:L_4x4_ TYA4020 (back)
translate([6.800000,5.800000,1.200000])m18(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([1.700000,15.400000,1.200000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([35.832500,2.700000,1.200000])rotate([0,0,-90.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([11.700000,14.400000,1.200000])rotate([0,0,180.000000])m15(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([0.900000,10.300000,1.200000])rotate([0,0,90.000000])m9(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m1(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH1-2-RA-W PTSM-HH1-2-RA-W
// Socket
hull()
{
	b(0,-7.5/2+0.3,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2+0.3,0,1.7+n*2.5-2,7.5,5);
}
if(!hulled)
{
	// Plug
	b(0,-10.5/2-7.5+0.3,0,1.1+n*2.5,10.5,5);
	// Pins
	for(p=[0:n-1])translate([-2.5*(n-1)/2+p*2.5,0,-2.1])cylinder(r=0.3,h=2.1);
}
}

module m3(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA
// Socket
hull()
{
	b(0,-7.5/2+0.3,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2+0.3,0,1.7+n*2.5-2,7.5,5);
}
if(!hulled)
{
	// Plug
	b(0,-10.5/2-7.5+0.3,0,1.1+n*2.5,10.5,5);
	// Pins
	for(p=[0:n-1])translate([-2.5*(n-1)/2+p*2.5,0,-2.1])cylinder(r=0.3,h=2.1);
}
}

module m5(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH-4-RA PTSM-HH-4-RA
// Socket
hull()
{
	b(0,-7.5/2+0.3,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2+0.3,0,1.7+n*2.5-2,7.5,5);
}
if(!hulled)
{
	// Plug
	b(0,-10.5/2-7.5+0.3,0,1.1+n*2.5,10.5,5);
	// Pins
	for(p=[0:n-1])translate([-2.5*(n-1)/2+p*2.5,0,-2.1])cylinder(r=0.3,h=2.1);
}
}

module m9(pushed=false,hulled=false)
{ // RevK:R_0402 R_0402_1005Metric
b(0,0,0,1.5,0.65,0.2); // Pad size
b(0,0,0,1.0,0.5,0.5); // Chip
}

module m11(pushed=false,hulled=false)
{ // RevK:ESE13 ESE13V01D
if(!hulled&&pushed) translate([0,2.05,-1.15])
{
	b(0,0,0,3.6+0.4,4.2+0.4,1.2);
	b(0,-3.08,0,1.2+0.4,1.95+0.4,0.9);
}

}

module m15(pushed=false,hulled=false)
{ // RevK:C_0402 C_0402_1005Metric
b(0,0,0,1.5,0.65,0.2); // Pad size
b(0,0,0,1.0,0.5,1); // Chip
}

module m18(pushed=false,hulled=false)
{ // RevK:C_0603_ C_0603_1608Metric
b(0,0,0,1.6,0.95,0.2); // Pad size
b(0,0,0,1.6,0.8,1); // Chip
}

module m21(pushed=false,hulled=false)
{ // RevK:L_4x4_ TYA4020
b(0,0,0,4,4,3);
}

module m23(pushed=false,hulled=false)
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

module m28(pushed=false,hulled=false)
{ // RevK:SOT-23-6-MD8942 SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m29(pushed=false,hulled=false)
{ // RevK:SOT-363_SC-70-6 SOT-363_SC-70-6
b(0,0,0,1.15,2.0,1.1);
b(0,0,0,2.1,2.0,0.6);
}

module m30(pushed=false,hulled=false)
{ // RevK:SOT-323_SC-70 SOT-323_SC-70
b(0,0,0,1.26,2.2,1.2);
b(0,0,0,2.2,2.2,0.6);
}

module m34(pushed=false,hulled=false)
{ // RevK:Special-SOP-4-3.7x4.55 SO-4_4.4x3.6mm_P2.54mm
b(0,0,0,4.55+0.25,3.7+0.25,2.1+0.2);
b(0,0,0,7.0+0.44,3,1.5);
}

module m37(pushed=false,hulled=false)
{ // RevK:ESP32-S3-MINI-1 ESP32-S3-MINI-1
translate([-15.4/2,-15.45/2,0])
{
	if(!hulled)cube([15.4,20.5,0.8]);
	translate([0.7,0.5,0])cube([14,13.55,2.4]);
}
}

module m40(pushed=false,hulled=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(!hulled&&pushed)b(0,0,0,1,1,20);
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
