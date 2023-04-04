// Generated case design for PCB/Generic/Generic.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-04-04 12:22:53
// title:	GPS reference
// date:	${DATE}
// rev:	5
// company:	Adrian Kennard Andrews & Arnold Ltd
// comment:	www.me.uk
// comment:	@TheRealRevK
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
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.000000,20.000000],[0.000000,0.000000],[30.000000,0.000000],[30.000000,20.000000],[29.942356,20.585271],[29.771638,21.148050],[29.494408,21.666710],[29.121320,22.121320],[28.666710,22.494408],[28.148050,22.771638],[27.585271,22.942356],[27.000000,23.000000],[22.100000,23.000000],[22.100000,30.200000],[7.900000,30.200000],[7.900000,23.000000],[3.000000,23.000000],[2.414729,22.942356],[1.851950,22.771638],[1.333290,22.494408],[0.878680,22.121320],[0.505592,21.666710],[0.228362,21.148050],[0.057644,20.585271]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.000000,20.000000],[0.000000,0.000000],[30.000000,0.000000],[30.000000,20.000000],[29.942356,20.585271],[29.771638,21.148050],[29.494408,21.666710],[29.121320,22.121320],[28.666710,22.494408],[28.148050,22.771638],[27.585271,22.942356],[27.000000,23.000000],[22.100000,23.000000],[22.100000,30.200000],[7.900000,30.200000],[7.900000,23.000000],[3.000000,23.000000],[2.414729,22.942356],[1.851950,22.771638],[1.333290,22.494408],[0.878680,22.121320],[0.505592,21.666710],[0.228362,21.148050],[0.057644,20.585271]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23]]);}
spacing=46.000000;
pcbwidth=30.000000;
pcblength=30.200000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([16.000000,16.500000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m2(pushed,hulled); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
translate([12.500000,18.325000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([20.500000,17.250000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([2.750000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])translate([0.000000,-0.100000,0.400000])rotate([-90.000000,-0.000000,-0.000000])m7(pushed,hulled); // RevK:MHS190RGBCT LED_0603_1608Metric
translate([20.500000,14.250000,0.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([15.630000,5.580000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m10(pushed,hulled); // RevK:D_1206_ D_1206_3216Metric
translate([20.500000,16.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([25.600000,16.500000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m12(pushed,hulled); // RevK:USC16-TR CSP-USC16-TR
translate([6.200000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([6.750000,5.693250,0.000000])rotate([180,0,0])m13(pushed,hulled); // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
translate([26.000000,9.500000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m10(pushed,hulled); // RevK:D_1206_ D_1206_3216Metric
translate([17.000000,13.000000,0.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([4.800000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([18.800000,7.200000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])rotate([-0.000000,-0.000000,-90.000000])m18(pushed,hulled); // RevK:L_4x4_ TYA4020
translate([18.550000,1.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m21(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([21.500000,1.500000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([15.250000,2.450000,0.000000])rotate([180,0,0])m23(pushed,hulled); // RevK:C_0805_ C_0805_2012Metric
translate([15.250000,8.700000,0.000000])rotate([180,0,0])m23(pushed,hulled); // RevK:C_0805_ C_0805_2012Metric
translate([17.000000,11.500000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([7.600000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([16.000000,20.000000,0.000000])rotate([180,0,0])m21(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([20.500000,19.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([26.000000,7.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m27(pushed,hulled,2); // RevK:PTSM-HH-2-RA PTSM-HH-2-RA
translate([15.000000,22.957500,0.000000])rotate([180,0,0])m27(pushed,hulled,5); // RevK:PTSM-HH-2-RA PTSM-HH-2-RA
translate([21.500000,9.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([18.800000,3.300000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])rotate([-0.000000,-0.000000,-90.000000])m30(pushed,hulled); // RevK:SOT-23-Thin-6-Reg SOT-23-6
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m2(pushed=false,hulled=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m5(pushed=false,hulled=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m7(pushed=false,hulled=false)
{ // RevK:MHS190RGBCT LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(!hulled&&pushed)b(0,0,0,1,1,20);
}

module m10(pushed=false,hulled=false)
{ // RevK:D_1206_ D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m12(pushed=false,hulled=false)
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

module m13(pushed=false,hulled=false)
{ // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
translate([-13.2/2,-16.6/2+2.7,0])
{
	if(!hulled)cube([13.2,16.6,0.8]);
	cube([13.2,11.2,2.4]);
}
}

module m18(pushed=false,hulled=false)
{ // RevK:L_4x4_ TYA4020
b(0,0,0,4,4,2.1);
}

module m21(pushed=false,hulled=false)
{ // RevK:C_0603_ C_0603_1608Metric
b(0,0,0,1.6,0.95,0.2); // Pad size
b(0,0,0,1.6,0.8,1); // Chip
}

module m23(pushed=false,hulled=false)
{ // RevK:C_0805_ C_0805_2012Metric
b(0,0,0,2,1.45,0.2); // Pad size
b(0,0,0,2,1.2,1); // Chip
}

module m27(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH-2-RA PTSM-HH-2-RA
hull()
{ // Socket
	b(0,-7.5/2+0.3,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2+0.3,0,1.7+n*2.5-2,7.5,5);
}
// Plug
b(0,-10.5/2-7.5+0.3,0,1.1+n*2,10.5,5);
// Pins
for(p=[0:n-1])translate([-2.5*(n-1)/2+p*2.5,0,-2.1])cylinder(r=0.3,h=2.1);
}

module m30(pushed=false,hulled=false)
{ // RevK:SOT-23-Thin-6-Reg SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
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
