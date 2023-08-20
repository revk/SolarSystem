// Generated case design for Output/Output.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-08-20 07:06:11
// title:	PCB-OUTPUT
// rev:	1
// company:	Adrian Kennard, Andrews & Arnold Ltd
//

// Globals
margin=0.500000;
overlap=2.000000;
lip=0.000000;
casebase=1.400000;
casetop=6.000000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[8.600000,34.300000],[8.600000,31.000000],[8.676120,30.617317],[8.892893,30.292894],[9.217317,30.076121],[9.600000,30.000000],[38.100000,30.000000],[38.482683,30.076120],[38.807106,30.292893],[39.023879,30.617317],[39.100000,31.000000],[39.100000,34.300000],[47.700000,34.300000],[47.700000,0.000000],[0.000000,0.000000],[0.000000,34.300000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[8.600000,34.300000],[8.600000,31.000000],[8.676120,30.617317],[8.892893,30.292894],[9.217317,30.076121],[9.600000,30.000000],[38.100000,30.000000],[38.482683,30.076120],[38.807106,30.292893],[39.023879,30.617317],[39.100000,31.000000],[39.100000,34.300000],[47.700000,34.300000],[47.700000,0.000000],[0.000000,0.000000],[0.000000,34.300000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]]);}
spacing=63.700000;
pcbwidth=47.700000;
pcblength=34.300000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([7.250000,17.140000,0.800000])rotate([0,0,-90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([23.850000,24.500000,0.800000])m2(pushed,hulled); // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02 (back)
translate([35.200000,5.000000,0.800000])rotate([0,0,-90.000000])m5(pushed,hulled); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([33.900000,7.500000,0.800000])m9(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([23.100000,8.400000,0.800000])rotate([0,0,-90.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
// Missing U7.1 SO-8_3.9x4.9mm_P1.27mm
translate([21.600000,4.400000,0.800000])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m16(pushed,hulled); // RevK:USC16-TR CSP-USC16-TR (back)
translate([7.250000,30.860000,0.800000])rotate([0,0,-90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([40.450000,30.860000,0.800000])rotate([0,0,90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([33.200000,5.000000,0.800000])rotate([0,0,-90.000000])m9(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
// Missing U6.1 SO-8_3.9x4.9mm_P1.27mm
translate([40.450000,24.000000,0.800000])rotate([0,0,90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([33.900000,2.500000,0.800000])m9(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
// Missing U11.1 MSOP-10_3x3mm_P0.5mm
translate([37.100000,5.000000,0.800000])rotate([0,0,-90.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([17.350000,11.250000,0.800000])rotate([0,0,90.000000])m23(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([31.400000,27.700000,0.800000])rotate([0,0,180.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([12.600000,7.242500,0.800000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([7.250000,10.280000,0.800000])rotate([0,0,-90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([16.700000,27.500000,0.800000])rotate([0,0,90.000000])m26(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([40.450000,3.420000,0.800000])rotate([0,0,90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
// Missing U4.1 SO-8_3.9x4.9mm_P1.27mm
translate([40.450000,17.140000,0.800000])rotate([0,0,90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([40.450000,10.280000,0.800000])rotate([0,0,90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([37.100000,3.200000,0.800000])rotate([0,0,-90.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([33.700000,28.200000,0.800000])translate([0.000000,-0.400000,0.000000])m30(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric (back)
translate([33.700000,28.200000,0.800000])translate([0.000000,0.400000,0.000000])m30(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric (back)
translate([7.250000,3.420000,0.800000])rotate([0,0,-90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
translate([30.600000,5.000000,0.800000])rotate([0,0,-90.000000])rotate([-0.000000,-0.000000,-90.000000])m33(pushed,hulled); // RevK:L_4x4_ TYA4020 (back)
translate([20.100000,8.400000,0.800000])rotate([0,0,90.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
// Missing U5.1 SO-8_3.9x4.9mm_P1.27mm
translate([37.100000,6.800000,0.800000])rotate([0,0,-90.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([31.400000,28.600000,0.800000])rotate([0,0,180.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([31.400000,26.800000,0.800000])rotate([0,0,180.000000])m12(pushed,hulled); // RevK:R_0402 R_0402_1005Metric (back)
translate([20.400000,14.700000,0.800000])rotate([0,0,90.000000])m26(pushed,hulled); // RevK:C_0402 C_0402_1005Metric (back)
translate([7.250000,24.000000,0.800000])rotate([0,0,-90.000000])m1(pushed,hulled,2); // RevK:PTSM-HH1-2-RA PTSM-HH1-2-RA (back)
// Missing U3.1 SO-8_3.9x4.9mm_P1.27mm
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m1(pushed=false,hulled=false,n=0)
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

module m2(pushed=false,hulled=false)
{ // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
translate([-13.2/2,-16.6/2+2.7,0])
{
	if(!hulled)cube([13.2,16.6,0.8]);
	cube([13.2,11.2,2.4]);
}
}

module m5(pushed=false,hulled=false)
{ // RevK:SOT-23-6-MD8942 SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m9(pushed=false,hulled=false)
{ // RevK:C_0603_ C_0603_1608Metric
b(0,0,0,1.6,0.95,0.2); // Pad size
b(0,0,0,1.6,0.8,1); // Chip
}

module m12(pushed=false,hulled=false)
{ // RevK:R_0402 R_0402_1005Metric
b(0,0,0,1.5,0.65,0.2); // Pad size
b(0,0,0,1.0,0.5,0.5); // Chip
}

module m16(pushed=false,hulled=false)
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

module m23(pushed=false,hulled=false)
{ // RevK:D_1206 D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m26(pushed=false,hulled=false)
{ // RevK:C_0402 C_0402_1005Metric
b(0,0,0,1.5,0.65,0.2); // Pad size
b(0,0,0,1.0,0.5,1); // Chip
}

module m30(pushed=false,hulled=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(!hulled&&pushed)b(0,0,0,1,1,20);
}

module m33(pushed=false,hulled=false)
{ // RevK:L_4x4_ TYA4020
b(0,0,0,4,4,3);
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
