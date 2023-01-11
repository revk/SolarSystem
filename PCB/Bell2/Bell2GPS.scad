// Generated case design for PCB/Bell2/Bell2.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-01-11 10:27:42
// title:	Bell box controller
// date:	${DATE}
// rev:	2
// company:	Adrian Kennard, Andrews & Arnold Ltd
//

// Globals
margin=0.500000;
overlap=2.000000;
lip=0.000000;
casebase=2.000000;
casetop=5.600000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=1.600000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[17.750000,14.000000],[17.750000,21.750000],[17.783493,21.875000],[17.875000,21.966507],[18.000000,22.000000],[35.000000,22.000000],[35.382684,22.076120],[35.707107,22.292893],[35.923880,22.617316],[36.000000,23.000000],[36.000000,27.000000],[36.038061,27.191342],[36.146447,27.353553],[36.308658,27.461939],[36.500000,27.500000],[52.250000,27.500000],[52.375000,27.466507],[52.466507,27.375000],[52.500000,27.250000],[52.500000,0.250000],[52.466507,0.125000],[52.375000,0.033493],[52.250000,0.000000],[18.250000,0.000000],[18.058658,0.038061],[17.896447,0.146447],[17.788061,0.308658],[17.750000,0.500000],[17.750000,8.000000],[17.701057,8.309017],[17.559017,8.587785],[17.337785,8.809017],[17.059017,8.951057],[16.750000,9.000000],[16.440983,8.951057],[16.162215,8.809017],[15.940983,8.587785],[15.798943,8.309017],[15.750000,8.000000],[15.750000,0.500000],[15.711939,0.308658],[15.603553,0.146447],[15.441342,0.038061],[15.250000,0.000000],[0.500000,0.000000],[0.308658,0.038061],[0.146447,0.146447],[0.038061,0.308658],[0.000000,0.500000],[0.000000,21.500000],[0.038061,21.691342],[0.146447,21.853553],[0.308658,21.961939],[0.500000,22.000000],[15.250000,22.000000],[15.441342,21.961939],[15.603553,21.853553],[15.711939,21.691342],[15.750000,21.500000],[15.750000,14.000000],[15.798943,13.690983],[15.940983,13.412215],[16.162215,13.190983],[16.440983,13.048943],[16.750000,13.000000],[17.059017,13.048943],[17.337785,13.190983],[17.559017,13.412215],[17.701057,13.690983]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68]]);}

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[17.750000,14.000000],[17.750000,21.750000],[17.783493,21.875000],[17.875000,21.966507],[18.000000,22.000000],[35.000000,22.000000],[35.382684,22.076120],[35.707107,22.292893],[35.923880,22.617316],[36.000000,23.000000],[36.000000,27.000000],[36.038061,27.191342],[36.146447,27.353553],[36.308658,27.461939],[36.500000,27.500000],[52.250000,27.500000],[52.375000,27.466507],[52.466507,27.375000],[52.500000,27.250000],[52.500000,0.250000],[52.466507,0.125000],[52.375000,0.033493],[52.250000,0.000000],[18.250000,0.000000],[18.058658,0.038061],[17.896447,0.146447],[17.788061,0.308658],[17.750000,0.500000],[17.750000,8.000000],[17.701057,8.309017],[17.559017,8.587785],[17.337785,8.809017],[17.059017,8.951057],[16.750000,9.000000],[16.440983,8.951057],[16.162215,8.809017],[15.940983,8.587785],[15.798943,8.309017],[15.750000,8.000000],[15.750000,0.500000],[15.711939,0.308658],[15.603553,0.146447],[15.441342,0.038061],[15.250000,0.000000],[0.500000,0.000000],[0.308658,0.038061],[0.146447,0.146447],[0.038061,0.308658],[0.000000,0.500000],[0.000000,21.500000],[0.038061,21.691342],[0.146447,21.853553],[0.308658,21.961939],[0.500000,22.000000],[15.250000,22.000000],[15.441342,21.961939],[15.603553,21.853553],[15.711939,21.691342],[15.750000,21.500000],[15.750000,14.000000],[15.798943,13.690983],[15.940983,13.412215],[16.162215,13.190983],[16.440983,13.048943],[16.750000,13.000000],[17.059017,13.048943],[17.337785,13.190983],[17.559017,13.412215],[17.701057,13.690983]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68]]);}
spacing=68.500000;
pcbwidth=52.500000;
pcblength=27.500000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([45.900000,13.800000,1.600000])rotate([0,0,90.000000])translate([0.000000,-3.600000,2.500000])rotate([-0.000000,-0.000000,-180.000000])m0(pushed,hulled); // RevK:Molex_MiniSPOX_H10RA 22057105 (back)
translate([35.750000,9.750000,1.600000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([35.825000,11.250000,1.600000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([40.750000,6.600000,1.600000])translate([0.000000,-3.600000,2.500000])rotate([-0.000000,-0.000000,-180.000000])m2(pushed,hulled); // RevK:Molex_MiniSPOX_H2RA 22057025 (back)
translate([33.500000,17.750000,1.600000])rotate([0,0,180.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([22.250000,0.700000,1.600000])m3(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([25.550000,8.200000,1.600000])m4(pushed,hulled); // RevK:C_0805_ C_0805_2012Metric (back)
translate([43.800000,23.425000,1.600000])rotate([0,0,-90.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([33.500000,19.000000,1.600000])rotate([0,0,180.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([43.800000,20.750000,1.600000])rotate([0,0,90.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([37.750000,26.500000,1.600000])rotate([0,0,180.000000])m3(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([21.900000,9.900000,1.600000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([40.750000,16.500000,1.600000])m5(pushed,hulled); // Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm (back)
translate([40.750000,11.000000,1.600000])m5(pushed,hulled); // Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm (back)
translate([19.200000,9.900000,1.600000])m3(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric (back)
translate([29.500000,8.250000,1.600000])rotate([0,0,180.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([7.000000,11.000000,1.600000])rotate([0,0,90.000000])translate([0.000000,0.150000,0.000000])rotate([90.000000,-0.000000,-180.000000])m6(pushed,hulled); // RevK:L96-M33 L96-M33 (back)
translate([18.500000,4.500000,1.600000])rotate([0,0,-90.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([25.550000,1.950000,1.600000])m4(pushed,hulled); // RevK:C_0805_ C_0805_2012Metric (back)
translate([34.500000,8.250000,1.600000])rotate([0,0,180.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([33.500000,13.000000,1.600000])rotate([0,0,180.000000])m7(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([18.500000,1.750000,1.600000])rotate([0,0,90.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([22.000000,2.800000,1.600000])rotate([0,0,90.000000])rotate([-0.000000,-0.000000,-90.000000])m8(pushed,hulled); // RevK:SOT-23-Thin-6-Reg SOT-23-6 (back)
translate([33.500000,16.500000,1.600000])rotate([0,0,180.000000])m1(pushed,hulled); // RevK:R_0603 R_0603_1608Metric (back)
translate([33.625000,20.750000,1.600000])rotate([0,0,180.000000])translate([0.000000,-0.400000,0.000000])m9(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric (back)
translate([33.625000,20.750000,1.600000])rotate([0,0,180.000000])translate([0.000000,0.400000,0.000000])m9(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric (back)
translate([24.400000,16.400000,1.600000])translate([0.000000,2.700000,0.000000])rotate([90.000000,-0.000000,-0.000000])m10(pushed,hulled); // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02 (back)
translate([32.000000,0.000000,1.600000])translate([0.000000,3.350000,0.000000])rotate([90.000000,-0.000000,-0.000000])m11(pushed,hulled); // RevK:USC16-TR CSP-USC16-TR (back)
translate([25.170000,5.080000,1.600000])rotate([0,0,90.000000])m7(pushed,hulled); // RevK:D_1206 D_1206_3216Metric (back)
translate([22.000000,6.700000,1.600000])rotate([0,0,90.000000])rotate([-0.000000,-0.000000,-90.000000])m12(pushed,hulled); // RevK:L_4x4_ TYA4020 (back)
translate([39.600000,22.500000,1.600000])m13(pushed,hulled); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm (back)
translate([48.300000,16.300000,0.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([48.500000,18.700000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m14(pushed,hulled); // Package_TO_SOT_SMD:SOT-363_SC-70-6 SOT-363_SC-70-6
translate([48.300000,21.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(pushed=false,hulled=false)
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

module m1(pushed=false,hulled=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m2(pushed=false,hulled=false)
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

module m3(pushed=false,hulled=false)
{ // RevK:C_0603_ C_0603_1608Metric
b(0,0,0,1.6,0.95,0.2); // Pad size
b(0,0,0,1.6,0.8,1); // Chip
}

module m4(pushed=false,hulled=false)
{ // RevK:C_0805_ C_0805_2012Metric
b(0,0,0,2,1.45,0.2); // Pad size
b(0,0,0,2,1.2,1); // Chip
}

module m5(pushed=false,hulled=false)
{ // Package_SO:SO-8_3.9x4.9mm_P1.27mm SOIC-8_3.9x4.9mm_P1.27mm
b(0,0,0,5,4,1);
b(0,0,0,3.9,4.9,2.1);
}

module m6(pushed=false,hulled=false)
{ // RevK:L96-M33 L96-M33
rotate([90,0,0])
translate([-9.6/2,-14/2,0])
{
	cube([9.6,14,2]);
}
}

module m7(pushed=false,hulled=false)
{ // RevK:D_1206 D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m8(pushed=false,hulled=false)
{ // RevK:SOT-23-Thin-6-Reg SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m9(pushed=false,hulled=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(!hulled&&pushed)b(0,0,0,1,1,20);
}

module m10(pushed=false,hulled=false)
{ // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
rotate([-90,0,0])
translate([-13.2/2,-16.6/2,0])
{
	if(!hulled)cube([13.2,16.6,0.8]);
	cube([13.2,11.2,2.4]);
}
}

module m11(pushed=false,hulled=false)
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

module m12(pushed=false,hulled=false)
{ // RevK:L_4x4_ TYA4020
b(0,0,0,4,4,2.1);
}

module m13(pushed=false,hulled=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m14(pushed=false,hulled=false)
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
    	translate([0,0,-casebase-d])pcbh(height+d*2,margin/2+d);
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
