// Generated case design for KiCad/Bell2.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2022-03-03 17:41:41
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
pcbwidth=34.750000;
pcblength=27.500000;
spacing=50.750000;

// PCB
module pcb(h=pcbthickness){linear_extrude(height=h)polygon([[0.000000,0.000000],[34.750000,0.000000],[34.750000,27.500000],[18.250000,27.500000],[18.250000,22.000000],[0.000000,22.000000]]);}

// Populated PCB
module board(pushed=false){
	pcb();
translate([28.150000,13.800000,1.600000])rotate([0,0,90.000000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m0(pushed); // RevK:Molex_MiniSPOX_H10RA 22057105
translate([18.000000,9.750000,1.600000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([18.075000,11.250000,1.600000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([23.000000,6.600000,1.600000])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m2(pushed); // RevK:Molex_MiniSPOX_H2RA 22057025
translate([15.750000,17.750000,1.600000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([26.050000,23.425000,1.600000])rotate([0,0,-90.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([15.750000,19.000000,1.600000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([26.050000,20.750000,1.600000])rotate([0,0,90.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([20.000000,26.500000,1.600000])rotate([0,0,180.000000])m3(pushed); // RevK:C_0603 C_0603_1608Metric
translate([1.500000,10.000000,1.600000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
// Missing Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm
// Missing Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm
translate([11.750000,8.250000,1.600000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([0.750000,4.500000,1.600000])rotate([0,0,-90.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([16.750000,8.250000,1.600000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([15.750000,13.000000,1.600000])rotate([0,0,180.000000])m5(pushed); // Diode_SMD:D_1206_3216Metric_Pad1.42x1.75mm_HandSolder D_1206_3216Metric
translate([0.750000,1.750000,1.600000])rotate([0,0,90.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([4.250000,2.800000,1.600000])rotate([0,0,90.000000])rotate([0.000000,0.000000,90.000000])m6(pushed); // RevK:RegulatorBlockFB SOT-23-6
translate([4.250000,2.800000,1.600000])rotate([0,0,90.000000])translate([-2.100000,-0.250000,0.000000])rotate([0.000000,0.000000,90.000000])m3(pushed); // RevK:C_0603 C_0603_1608Metric
translate([4.250000,2.800000,1.600000])rotate([0,0,90.000000])translate([-0.800000,-3.550000,0.000000])rotate([0.000000,0.000000,90.000000])m7(pushed); // RevK:RegulatorBlockFB C_0805_2012Metric
translate([4.250000,2.800000,1.600000])rotate([0,0,90.000000])translate([5.400000,-3.550000,0.000000])rotate([0.000000,0.000000,90.000000])m7(pushed); // RevK:RegulatorBlockFB C_0805_2012Metric
translate([4.250000,2.800000,1.600000])rotate([0,0,90.000000])translate([3.900000,0.000000,0.000000])rotate([0.000000,0.000000,90.000000])m8(pushed); // RevK:RegulatorBlockFB TYA4020
translate([4.250000,2.800000,1.600000])rotate([0,0,90.000000])translate([2.275000,-3.200000,0.000000])m5(pushed); // Diode_SMD:D_1206_3216Metric_Pad1.42x1.75mm_HandSolder D_1206_3216Metric
translate([15.750000,16.500000,1.600000])rotate([0,0,180.000000])m1(pushed); // RevK:R_0603 R_0603_1608Metric
translate([15.875000,20.750000,1.600000])rotate([0,0,180.000000])translate([0.000000,-0.400000,0.000000])m9(pushed); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([15.875000,20.750000,1.600000])rotate([0,0,180.000000])translate([0.000000,0.400000,0.000000])m9(pushed); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([6.550000,16.400000,1.600000])translate([0.000000,2.500000,0.000000])rotate([-90.000000,0.000000,0.000000])m10(pushed); // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
translate([14.250000,0.000000,1.600000])translate([0.000000,3.385000,0.000000])rotate([-90.000000,0.000000,0.000000])m11(pushed); // RevK:USC16-TR-Round CSP-USC16-TR
translate([21.850000,22.500000,1.600000])m12(pushed); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
translate([31.750000,17.500000,0.000000])rotate([180,0,0])m3(pushed); // RevK:C_0603 C_0603_1608Metric
translate([31.750000,20.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m13(pushed); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([31.750000,22.500000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m3(pushed); // RevK:C_0603 C_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(pushed=false)
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

module m1(pushed=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m2(pushed=false)
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

module m3(pushed=false)
{ // RevK:C_0603 C_0603_1608Metric
b(0,0,0,2.8,0.95,1); // Pad size
}

module m5(pushed=false)
{ // Diode_SMD:D_1206_3216Metric_Pad1.42x1.75mm_HandSolder D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m6(pushed=false)
{ // RevK:RegulatorBlockFB SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m7(pushed=false)
{ // RevK:RegulatorBlockFB C_0805_2012Metric
b(0,0,0,3.2,1.45,1); // Pad size
}

module m8(pushed=false)
{ // RevK:RegulatorBlockFB TYA4020
b(0,0,0,4,4,2.1);
}

module m9(pushed=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(pushed)b(0,0,0,1,1,20);
}

module m10(pushed=false)
{ // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
rotate([90,0,0])
translate([-13.2/2,-16.6/2,0])
{
	cube([13.2,16.6,0.8]);
	cube([13.2,11.2,2.4]);
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
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m13(pushed=false)
{ // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
b(0,0,0,1.15,2.0,1.1);
b(0,0,0,2.1,2.0,0.6);
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
