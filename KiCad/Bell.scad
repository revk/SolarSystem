// Generated case design for KiCad/Bell.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2022-03-09 12:05:01
// title:	Bell box controller
// date:	${DATE}
// rev:	2
// company:	Adrian Kennard, Andrews & Arnold Ltd
//

// Globals
margin=0.500000;
overlap=2.000000;
casebase=2.000000;
casetop=5.200000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=1.600000;
nohull=false;
useredge=false;

module pcb(h=pcbthickness){linear_extrude(height=h)polygon(points=[[36.000000,35.000000],[36.000000,0.500000],[35.961939,0.308658],[35.853553,0.146447],[35.691342,0.038061],[35.500000,0.000000],[0.000000,0.000000],[0.000000,28.500000],[0.038061,28.691342],[0.146447,28.853553],[0.308658,28.961939],[0.500000,29.000000],[24.000000,29.000000],[24.000000,34.500000],[24.038061,34.691342],[24.146447,34.853553],[24.308658,34.961939],[24.500000,35.000000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,0]]);}

module outline(h=pcbthickness){linear_extrude(height=h)polygon(points=[[36.000000,35.000000],[36.000000,0.500000],[35.961939,0.308658],[35.853553,0.146447],[35.691342,0.038061],[35.500000,0.000000],[0.000000,0.000000],[0.000000,28.500000],[0.038061,28.691342],[0.146447,28.853553],[0.308658,28.961939],[0.500000,29.000000],[24.000000,29.000000],[24.000000,34.500000],[24.038061,34.691342],[24.146447,34.853553],[24.308658,34.961939],[24.500000,35.000000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,0]]);}
spacing=52.000000;
pcbwidth=36.000000;
pcblength=35.000000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([12.000000,5.800000,1.600000])rotate([0,0,180.000000])m0(pushed,hulled); // RevK:C_0603 C_0603_1608Metric
translate([25.000000,9.500000,1.600000])m1(pushed,hulled); // Diode_SMD:D_1206_3216Metric_Pad1.42x1.75mm_HandSolder D_1206_3216Metric
translate([30.000000,0.100000,1.600000])translate([0.000000,3.385000,0.000000])rotate([-90.000000,0.000000,0.000000])m2(pushed,hulled); // RevK:USC16-TR-Round CSP-USC16-TR
translate([3.700000,6.600000,1.600000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m3(pushed,hulled); // RevK:Molex_MiniSPOX_H2RA 22057025
translate([25.500000,11.500000,1.600000])rotate([0,0,180.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([12.000000,2.800000,1.600000])rotate([0,0,90.000000])m5(pushed,hulled); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
translate([26.300000,28.900000,1.600000])rotate([0,0,180.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,11.500000,1.600000])rotate([0,0,180.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([26.300000,25.500000,1.600000])rotate([0,0,-90.000000])m0(pushed,hulled); // RevK:C_0603 C_0603_1608Metric
translate([24.200000,25.500000,1.600000])rotate([0,0,180.000000])m6(pushed,hulled); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([27.500000,25.500000,1.600000])rotate([0,0,90.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([22.100000,25.500000,1.600000])rotate([0,0,90.000000])m0(pushed,hulled); // RevK:C_0603 C_0603_1608Metric
translate([20.900000,25.500000,1.600000])rotate([0,0,-90.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
// Missing Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm
// Missing Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm
translate([26.200000,33.000000,1.600000])translate([0.000000,-0.400000,0.000000])m8(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([26.200000,33.000000,1.600000])translate([0.000000,0.400000,0.000000])m8(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([26.300000,30.100000,1.600000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([26.300000,31.300000,1.600000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([29.400000,21.300000,1.600000])rotate([0,0,90.000000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m9(pushed,hulled); // RevK:Molex_MiniSPOX_H10RA 22057105
translate([8.700000,7.900000,1.600000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([9.600000,19.300000,1.600000])m10(pushed,hulled); // RevK:ESP32-WROOM-32 ESP32-WROOM-32
translate([20.675000,8.900000,1.600000])rotate([0,0,180.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([11.250000,7.000000,1.600000])rotate([0,0,180.000000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([9.400000,5.800000,1.600000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([18.100000,5.170000,1.600000])rotate([0.000000,0.000000,90.000000])m11(pushed,hulled); // RevK:RegulatorBlockFB SOT-23-6
translate([18.100000,5.170000,1.600000])translate([-2.100000,-0.250000,0.000000])rotate([0.000000,0.000000,90.000000])m0(pushed,hulled); // RevK:C_0603 C_0603_1608Metric
translate([18.100000,5.170000,1.600000])translate([-0.800000,-3.550000,0.000000])rotate([0.000000,0.000000,90.000000])m12(pushed,hulled); // RevK:RegulatorBlockFB C_0805_2012Metric
translate([18.100000,5.170000,1.600000])translate([5.400000,-3.550000,0.000000])rotate([0.000000,0.000000,90.000000])m12(pushed,hulled); // RevK:RegulatorBlockFB C_0805_2012Metric
translate([18.100000,5.170000,1.600000])translate([3.900000,0.000000,0.000000])rotate([0.000000,0.000000,90.000000])m13(pushed,hulled); // RevK:RegulatorBlockFB TYA4020
translate([18.100000,5.170000,1.600000])translate([2.275000,-3.200000,0.000000])m1(pushed,hulled); // Diode_SMD:D_1206_3216Metric_Pad1.42x1.75mm_HandSolder D_1206_3216Metric
translate([15.000000,7.200000,1.600000])m4(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(pushed=false,hulled=false)
{ // RevK:C_0603 C_0603_1608Metric
b(0,0,0,2.8,0.95,1); // Pad size
}

module m1(pushed=false,hulled=false)
{ // Diode_SMD:D_1206_3216Metric_Pad1.42x1.75mm_HandSolder D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m2(pushed=false,hulled=false)
{ // RevK:USC16-TR-Round CSP-USC16-TR
rotate([90,0,0])translate([-4.47,-3.84,0])
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

module m3(pushed=false,hulled=false)
{ // RevK:Molex_MiniSPOX_H2RA 22057025
N=2;
A=2.4+N*2.5;
rotate([0,0,180])
translate([-A/2,-2.94,-2.5])
{
	cube([A,4.9,4.9]);
	cube([A,5.9,3.9]);
	hull()
	{
		cube([A,7.4,1]);
		cube([A,7.9,0.5]);
	}
	translate([1,6,-2])cube([A-2,1.2,4.5]); // Assumes cropped pins
	// Plug
	translate([0.5,-20,0.6])cube([A-1,21,4.1]);
	translate([0,-23,0])cube([A,20,4.9]);
}

}

module m4(pushed=false,hulled=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m5(pushed=false,hulled=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m6(pushed=false,hulled=false)
{ // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
b(0,0,0,1.15,2.0,1.1);
b(0,0,0,2.1,2.0,0.6);
}

module m8(pushed=false,hulled=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(pushed)b(0,0,0,1,1,20);
}

module m9(pushed=false,hulled=false)
{ // RevK:Molex_MiniSPOX_H10RA 22057105
N=10;
A=2.4+N*2.5;
rotate([0,0,180])
translate([-A/2,-2.94,-2.5])
{
	cube([A,4.9,4.9]);
	cube([A,5.9,3.9]);
	hull()
	{
		cube([A,7.4,1]);
		cube([A,7.9,0.5]);
	}
	translate([1,6,-2])cube([A-2,1.2,4.5]); // Assumes cropped pins
	// Plug
	translate([0.5,-20,0.6])cube([A-1,21,4.1]);
	translate([0,-23,0])cube([A,20,4.9]);
}

}

module m10(pushed=false,hulled=false)
{ // RevK:ESP32-WROOM-32 ESP32-WROOM-32
translate([-9,-9.75,0])
{
	cube([18,25.50,0.80]);
	translate([1.1,1.05,0])cube([15.80,17.60,3.10]);
	if(!hulled)translate([-0.5,0.865,0])cube([19,17.4,0.8]);
	translate([2.65,-0.5,0])cube([12.7,2,0.8]);

}
}

module m11(pushed=false,hulled=false)
{ // RevK:RegulatorBlockFB SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m12(pushed=false,hulled=false)
{ // RevK:RegulatorBlockFB C_0805_2012Metric
b(0,0,0,3.2,1.45,1); // Pad size
}

module m13(pushed=false,hulled=false)
{ // RevK:RegulatorBlockFB TYA4020
b(0,0,0,4,4,2.1);
}

height=casebase+pcbthickness+casetop;

module boardh(pushed=false)
{ // Board with hulled parts
	union()
	{
		pcb();
		board(pushed,false);
		if(!nohull)intersection()
		{
			translate([0,0,-casebase])outline(casebase+pcbthickness+casetop);
			hull()board(pushed,true);
		}
	}
}

module boardf()
{ // This is the board, but stretched up to make a push out in from the front
	render()
	{
		intersection()
		{
			translate([-casewall-1,-casewall-1,-casebase-1]) cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,height+2]);
			minkowski()
			{
				boardh(true);
				cylinder(h=height+100,d=margin,$fn=8);
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
			minkowski()
			{
				boardh(true);
				translate([0,0,-height-100])
				cylinder(h=height+100,d=margin,$fn=8);
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
		intersection()
    		{
        		translate([0,0,-(casebase-1)])pcb(pcbthickness+(casebase-1)+(casetop-1));
        		translate([0,0,-(casebase-1)])outline(pcbthickness+(casebase-1)+(casetop-1));
			boardh(false);
    		}
 	}
}

module pcbh()
{ // PCB shape for case
	if(useredge)outline();
	else hull()outline();
}

module pyramid()
{ // A pyramid
 polyhedron(points=[[0,0,0],[-height,-height,height],[-height,height,height],[height,height,height],[height,-height,height]],faces=[[0,1,2],[0,2,3],[0,3,4],[0,4,1],[4,3,2,1]]);
}

module wall(d=0)
{ // The case wall
    	translate([0,0,-casebase-1])
    	minkowski()
    	{
    		pcbh();
	        cylinder(d=margin+d*2,h=height+2-pcbthickness,$fn=8);
   	}
}

module cutf()
{ // This cut up from base in the wall
	intersection()
	{
		boardf();
		difference()
		{
			translate([-casewall+0.01,-casewall+0.01,-casebase+0.01])cube([pcbwidth+casewall*2-0.02,pcblength+casewall*2-0.02,casebase+overlap]);
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
			translate([-casewall-0.01,-casewall-0.01,-casebase-0.01])cube([pcbwidth+casewall*2+0.02,pcblength+casewall*2+0.02,casebase+overlap+0.02]);
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
        minkowski()
        {
            pcbh();
            hull()
		{
			translate([edge,0,edge])
			cube([casewall*2-edge*2,casewall*2,height-edge*2-pcbthickness]);
			translate([0,edge,edge])
			cube([casewall*2,casewall*2-edge*2,height-edge*2-pcbthickness]);
			translate([edge,edge,0])
			cube([casewall*2-edge*2,casewall*2-edge*2,height-pcbthickness]);
		}
        }
}

module cut(d=0)
{ // The cut point in the wall
	minkowski()
	{
        	pcbh();
		hull()
		{
			translate([casewall/2-d/2-margin/4+casewall/3,casewall/2-d/2-margin/4,casebase])
				cube([casewall+d+margin/2-2*casewall/3,casewall+d+margin/2,casetop+pcbthickness+1]);
			translate([casewall/2-d/2-margin/4,casewall/2-d/2-margin/4+casewall/3,casebase])
				cube([casewall+d+margin/2,casewall+d+margin/2-2*casewall/3,casetop+pcbthickness+1]);
		}
	}
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
				translate([-1,-1,casebase+overlap])cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,casetop+1]);
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
				translate([-1,-1,-1])cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,casebase+overlap-margin+1]);
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
