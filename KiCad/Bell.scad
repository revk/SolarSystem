// Generated case design for KiCad/Bell.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2021-06-17 11:01:36
// title:	Bell box controller
// date:	2021-06-12
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
module pcb(h=pcbthickness){linear_extrude(height=h)polygon([[36.000000,0.000000],[0.000000,0.000000],[0.000000,29.000000],[24.000000,29.000000],[24.000000,35.000000],[36.000000,35.000000]]);}

// Populated PCB
module board(pushed=false){
	pcb();
translate([11.500000,4.500000,0.800000])rotate([0,0,90.000000])m0(pushed); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
translate([23.500000,28.000000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([29.400000,21.250000,0.800000])rotate([0,0,90.000000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m2(pushed); // RevK:Molex_MiniSPOX_H10RA 22057105
translate([23.500000,26.000000,0.800000])rotate([0,0,180.000000])m3(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([26.000000,26.000000,0.800000])rotate([0,0,270.000000])m4(pushed); // RevK:C_0603 C_0603_1608Metric
translate([25.500000,15.000000,0.800000])m3(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([22.000000,15.000000,0.800000])m3(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([22.000000,20.500000,0.800000])m3(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([25.500000,20.500000,0.800000])m3(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([9.500000,19.500000,0.800000])m5(pushed); // RevK:ESP32-WROOM-32 ESP32-WROOM-32
translate([20.000000,4.500000,0.800000])translate([-1.950000,0.650000,0.000000])rotate([0.000000,0.000000,90.000000])m6(pushed); // RevK:RegulatorBlock SOT-23-6
translate([20.000000,4.500000,0.800000])translate([-4.050000,0.600000,0.000000])rotate([0.000000,0.000000,90.000000])m4(pushed); // RevK:C_0603 C_0603_1608Metric
translate([20.000000,4.500000,0.800000])translate([-2.800000,-2.900000,0.000000])rotate([0.000000,0.000000,90.000000])m7(pushed); // RevK:RegulatorBlock C_0805_2012Metric
translate([20.000000,4.500000,0.800000])translate([3.450000,-2.900000,0.000000])rotate([0.000000,0.000000,90.000000])m7(pushed); // RevK:RegulatorBlock C_0805_2012Metric
translate([20.000000,4.500000,0.800000])translate([1.950000,0.600000,0.000000])rotate([0.000000,0.000000,90.000000])m8(pushed); // RevK:RegulatorBlock TYA4020
translate([20.000000,4.500000,0.800000])translate([0.275000,-2.500000,0.000000])m9(pushed); // RevK:RegulatorBlock D_1206_3216Metric
translate([25.500000,13.000000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([26.000000,32.500000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([25.500000,17.000000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,13.000000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,11.500000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([25.500000,11.500000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([23.500000,24.000000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,17.000000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,18.500000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([22.000000,22.500000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([25.500000,18.500000,0.800000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([25.500000,22.500000,0.800000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([4.000000,7.000000,0.800000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m10(pushed); // RevK:Molex_MiniSPOX_H2RA 22057025
translate([30.000000,0.000000,0.800000])translate([0.000000,3.350000,0.000000])rotate([-90.000000,0.000000,0.000000])m11(pushed); // RevK:USC16-TR-Round CSP-USC16-TR
translate([25.000000,9.500000,0.800000])m9(pushed); // RevK:RegulatorBlock D_1206_3216Metric
translate([25.875000,34.000000,0.800000])rotate([0,0,180.000000])m12(pushed); // LED_SMD:LED_0603_1608Metric_Pad1.05x0.95mm_HandSolder LED_0603_1608Metric
translate([21.000000,26.000000,0.800000])rotate([0,0,90.000000])m4(pushed); // RevK:C_0603 C_0603_1608Metric
translate([9.400000,0.900000,0.800000])m4(pushed); // RevK:C_0603 C_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(pushed=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m1(pushed=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m2(pushed=false)
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

module m3(pushed=false)
{ // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
b(0,0,0,2.2,1.26,1.2);
b(0,0,0,2.2,2.2,0.6);
}

module m4(pushed=false)
{ // RevK:C_0603 C_0603_1608Metric
b(0,0,0,2.8,0.95,1); // Pad size
}

module m5(pushed=false)
{ // RevK:ESP32-WROOM-32 ESP32-WROOM-32
translate([-9,-9.75,0])
{
	cube([18,25.50,0.80]);
	translate([1.1,1.05,0])cube([15.80,17.60,3.10]);
	translate([-0.5,0.865,0])cube([19,17.4,0.8]);
	translate([2.65,-0.5,0])cube([12.7,2,0.8]);

}
}

module m6(pushed=false)
{ // RevK:RegulatorBlock SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m7(pushed=false)
{ // RevK:RegulatorBlock C_0805_2012Metric
b(0,0,0,3.2,1.45,1); // Pad size
}

module m8(pushed=false)
{ // RevK:RegulatorBlock TYA4020
b(0,0,0,4,4,2.1);
}

module m9(pushed=false)
{ // RevK:RegulatorBlock D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m10(pushed=false)
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

module m11(pushed=false)
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

module m12(pushed=false)
{ // LED_SMD:LED_0603_1608Metric_Pad1.05x0.95mm_HandSolder LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
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
