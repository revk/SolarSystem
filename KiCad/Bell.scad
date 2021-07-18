// Generated case design for KiCad/Bell.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2021-07-18 12:10:54
// title:	Bell box controller
// date:	2021-06-18
// rev:	1.000000
// company:	Adrian Kennard, Andrews & Arnold Ltd
//

// Globals
margin=0.500000;
overlap=2.000000;
casebase=3.000000;
casetop=5.200000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=0.800000;
pcbwidth=36.000000;
pcblength=35.000000;
spacing=52.000000;

// PCB
module pcb(h=pcbthickness){linear_extrude(height=h)polygon([[36.000000,0.500000],[35.543578,0.001903],[35.586824,0.007596],[35.629410,0.017037],[35.671010,0.030154],[35.711309,0.046846],[35.750000,0.066987],[35.786788,0.090424],[35.821394,0.116978],[35.853553,0.146447],[35.883022,0.178606],[35.909576,0.213212],[35.933013,0.250000],[35.953154,0.288691],[35.969846,0.328990],[35.982963,0.370590],[35.992404,0.413176],[35.998097,0.456422],[35.500000,0.000000],[0.000000,0.000000],[0.000000,28.500000],[0.456422,28.998097],[0.413176,28.992404],[0.370590,28.982963],[0.328990,28.969846],[0.288691,28.953154],[0.250000,28.933013],[0.213212,28.909576],[0.178606,28.883022],[0.146447,28.853553],[0.116978,28.821394],[0.090424,28.786788],[0.066987,28.750000],[0.046846,28.711309],[0.030154,28.671010],[0.017037,28.629410],[0.007596,28.586824],[0.001903,28.543578],[0.500000,29.000000],[24.000000,29.000000],[24.000000,34.500000],[24.456422,34.998097],[24.413176,34.992404],[24.370590,34.982963],[24.328990,34.969846],[24.288691,34.953154],[24.250000,34.933013],[24.213212,34.909576],[24.178606,34.883022],[24.146447,34.853553],[24.116978,34.821394],[24.090424,34.786788],[24.066987,34.750000],[24.046846,34.711309],[24.030154,34.671010],[24.017037,34.629410],[24.007596,34.586824],[24.001903,34.543578],[24.500000,35.000000],[36.000000,35.000000]]);}

// Populated PCB
module board(pushed=false){
	pcb();
translate([24.200000,25.500000,0.800000])rotate([0,0,180.000000])m0(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([26.300000,31.300000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([26.300000,30.100000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([26.200000,33.000000,0.800000])translate([0.000000,-0.400000,0.000000])m2(pushed); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([26.200000,33.000000,0.800000])translate([0.000000,0.400000,0.000000])m2(pushed); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([12.000000,2.800000,0.800000])rotate([0,0,90.000000])m3(pushed); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
translate([24.100000,15.100000,0.800000])m4(pushed); // Package_SO:SO-8_3.9x4.9mm_P1.27mm HSOP-8-1EP_3.9x4.9mm_P1.27mm_EP2.41x3.1mm
translate([24.100000,20.700000,0.800000])m4(pushed); // Package_SO:SO-8_3.9x4.9mm_P1.27mm HSOP-8-1EP_3.9x4.9mm_P1.27mm_EP2.41x3.1mm
translate([30.000000,0.000000,0.800000])translate([0.000000,3.385000,0.000000])rotate([-90.000000,0.000000,0.000000])m5(pushed); // RevK:USC16-TR-Round CSP-USC16-TR
translate([27.500000,25.500000,0.800000])rotate([0,0,90.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([29.400000,21.300000,0.800000])rotate([0,0,90.000000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m6(pushed); // RevK:Molex_MiniSPOX_H10RA 22057105
translate([26.300000,25.500000,0.800000])rotate([0,0,270.000000])m7(pushed); // RevK:C_0603 C_0603_1608Metric
translate([9.500000,19.300000,0.800000])m8(pushed); // RevK:ESP32-WROOM-32 ESP32-WROOM-32
translate([20.000000,4.500000,0.800000])translate([-1.950000,0.650000,0.000000])rotate([0.000000,0.000000,90.000000])m9(pushed); // RevK:RegulatorBlock SOT-23-6
translate([20.000000,4.500000,0.800000])translate([-4.050000,0.600000,0.000000])rotate([0.000000,0.000000,90.000000])m7(pushed); // RevK:C_0603 C_0603_1608Metric
translate([20.000000,4.500000,0.800000])translate([-2.800000,-2.900000,0.000000])rotate([0.000000,0.000000,90.000000])m10(pushed); // RevK:RegulatorBlock C_0805_2012Metric
translate([20.000000,4.500000,0.800000])translate([3.450000,-2.900000,0.000000])rotate([0.000000,0.000000,90.000000])m10(pushed); // RevK:RegulatorBlock C_0805_2012Metric
translate([20.000000,4.500000,0.800000])translate([1.950000,0.600000,0.000000])rotate([0.000000,0.000000,90.000000])m11(pushed); // RevK:RegulatorBlock TYA4020
translate([20.000000,4.500000,0.800000])translate([0.275000,-2.500000,0.000000])m12(pushed); // RevK:RegulatorBlock D_1206_3216Metric
translate([26.300000,28.900000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,11.500000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([25.500000,11.500000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([20.900000,25.500000,0.800000])rotate([0,0,270.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([3.700000,6.600000,0.800000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m13(pushed); // RevK:Molex_MiniSPOX_H2RA 22057025
translate([25.000000,9.500000,0.800000])m12(pushed); // RevK:RegulatorBlock D_1206_3216Metric
translate([22.100000,25.500000,0.800000])rotate([0,0,90.000000])m7(pushed); // RevK:C_0603 C_0603_1608Metric
translate([12.000000,5.800000,0.800000])rotate([0,0,180.000000])m7(pushed); // RevK:C_0603 C_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(pushed=false)
{ // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
b(0,0,0,1.15,2.0,1.1);
b(0,0,0,2.1,2.0,0.6);
}

module m1(pushed=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m2(pushed=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
b(0,0,0,1,1,20);
}

module m3(pushed=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m4(pushed=false)
{ // Package_SO:SO-8_3.9x4.9mm_P1.27mm HSOP-8-1EP_3.9x4.9mm_P1.27mm_EP2.41x3.1mm
b(0,0,0,3.9,4.9,1.75);
b(0,0,0,6.0,4.9,0.6);
}

module m5(pushed=false)
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

module m6(pushed=false)
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
	translate([1,6,-3.5])cube([A-2,1.2,6]);
	// Plug
	translate([0.5,-20,0.6])cube([A-1,21,4.1]);
	translate([0,-23,0])cube([A,20,4.9]);
}

}

module m7(pushed=false)
{ // RevK:C_0603 C_0603_1608Metric
b(0,0,0,2.8,0.95,1); // Pad size
}

module m8(pushed=false)
{ // RevK:ESP32-WROOM-32 ESP32-WROOM-32
translate([-9,-9.75,0])
{
	cube([18,25.50,0.80]);
	translate([1.1,1.05,0])cube([15.80,17.60,3.10]);
	translate([-0.5,0.865,0])cube([19,17.4,0.8]);
	translate([2.65,-0.5,0])cube([12.7,2,0.8]);

}
}

module m9(pushed=false)
{ // RevK:RegulatorBlock SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m10(pushed=false)
{ // RevK:RegulatorBlock C_0805_2012Metric
b(0,0,0,3.2,1.45,1); // Pad size
}

module m11(pushed=false)
{ // RevK:RegulatorBlock TYA4020
b(0,0,0,4,4,2.1);
}

module m12(pushed=false)
{ // RevK:RegulatorBlock D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m13(pushed=false)
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
	translate([1,6,-3.5])cube([A-2,1.2,6]);
	// Plug
	translate([0.5,-20,0.6])cube([A-1,21,4.1]);
	translate([0,-23,0])cube([A,20,4.9]);
}

}

height=casebase+pcbthickness+casetop;

module boardf()
{ // This is the board, but stretched up to make a push out in from the front
	render()
	{
		intersection()
		{
			translate([-casewall-1,-casewall-1,-casebase-1]) cube([pcbwidth+casewall*2+2,pcblength+casewall*2+2,height+2]);
			minkowski()
			{
				board(true);
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
				board(true);
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
 			board(false);
		}
		hull()intersection()
    		{
        		translate([0,0,-(casebase-1)])pcb(pcbthickness+(casebase-1)+(casetop-1));
        		board(false);
    		}
 	}
}

module pcbh()
{ // PCB shape for case
	hull()pcb();
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
			board(true);
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
			board(true);
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
	translate([0*spacing,0,0])pcb();
	translate([1*spacing,0,0])wall();
	translate([2*spacing,0,0])board();
	translate([3*spacing,0,0])board(true);
	translate([4*spacing,0,0])boardf();
	translate([5*spacing,0,0])boardb();
	translate([6*spacing,0,0])cutpf();
	translate([7*spacing,0,0])cutpb();
	translate([8*spacing,0,0])cutf();
	translate([9*spacing,0,0])cutb();
	translate([10*spacing,0,0])case();
	translate([11*spacing,0,0])base();
	translate([12*spacing,0,0])top();
}

module parts()
{
	base();
	translate([spacing,0,0])top();
}
base(); translate([spacing,0,0])top();
