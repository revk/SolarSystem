// Generated case design for Access/Access.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2024-04-19 10:39:51
// title:	Access Control
// rev:	3
// company:	Adrian Kennard Andrews & Arnold Ltd
// comment:	toot.me.uk/@RevK
// comment:	www.me.uk
//

// Globals
margin=0.200000;
lip=2.000000;
casebottom=2.000000;
casetop=6.000000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=1.200000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.000000,0.000000],[0.000000,24.400000],[49.300000,24.400000],[49.300000,0.000000]],paths=[[0,1,2,3]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.000000,0.000000],[0.000000,24.400000],[49.300000,24.400000],[49.300000,0.000000]],paths=[[0,1,2,3]]);}
spacing=65.300000;
pcbwidth=49.300000;
pcblength=24.400000;
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
translate([45.950000,17.250000,1.200000])rotate([0,0,180.000000])m0(part,hole,block,casetop,2); // J5 (back)
translate([32.150000,17.250000,1.200000])rotate([0,0,180.000000])m1(part,hole,block,casetop,2); // J6 (back)
translate([22.750000,17.257500,1.200000])rotate([0,0,180.000000])m2(part,hole,block,casetop,4); // J2 (back)
translate([8.350000,17.257500,1.200000])rotate([0,0,180.000000])m2(part,hole,block,casetop,6); // J2 (back)
translate([10.850000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([37.500000,6.800000,1.200000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([7.101000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([16.900000,6.400000,1.200000])rotate([0,0,90.000000])translate([2.050000,0.000000,1.150000])rotate([-0.000000,-0.000000,-90.000000])m6(part,hole,block,casetop); // RevK:ESE13 ESE13V01D (back)
translate([9.100000,13.100000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([13.200000,13.100000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([48.200000,10.500000,1.200000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([1.556250,3.900000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([19.100000,3.400000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([8.200000,13.100000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([18.350000,15.300000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([2.400000,13.150000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([4.056250,7.200000,1.200000])rotate([0,0,180.000000])rotate([-0.000000,-0.000000,-90.000000])m8(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
translate([7.700000,8.300000,1.200000])rotate([0,0,90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([44.335000,4.400000,1.200000])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m9(part,hole,block,casetop); // RevK:USB-C-Socket-H CSP-USC16-TR (back)
translate([18.100000,12.800000,1.200000])rotate([0,0,180.000000])m10(part,hole,block,casetop); // Q2 (back)
translate([3.200000,13.100000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([13.050000,8.750000,1.200000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([11.250000,6.850000,1.200000])m11(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([37.750000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([14.100000,13.100000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([44.700000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([0.850000,15.150000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([15.000000,13.175000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([4.601000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([7.400000,13.175000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([13.750000,5.550000,1.200000])rotate([0,0,90.000000])m7(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([35.800000,4.800000,1.200000])rotate([0,0,90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([11.250000,4.850000,1.200000])m7(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([4.900000,13.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([5.850000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([6.556250,4.100000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([39.050000,17.257500,1.200000])rotate([0,0,180.000000])m1(part,hole,block,casetop,2); // J6 (back)
translate([2.256250,0.700000,1.200000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([11.600000,13.100000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([3.350000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([5.856250,0.700000,1.200000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([9.601000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([17.550000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([13.350000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([16.000000,1.900000,1.200000])rotate([0,0,180.000000])m12(part,hole,block,casetop); // RevK:C_1210 C_1210_3225Metric (back)
translate([37.500000,3.300000,1.200000])rotate([0,0,90.000000])m13(part,hole,block,casetop); // RevK:C_1206 C_1206_3216Metric (back)
translate([4.056250,4.600000,1.200000])rotate([0,0,180.000000])m7(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([39.000000,2.100000,1.200000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([15.950000,14.425000,1.200000])rotate([0,0,90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([4.100000,13.100000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([11.250000,8.750000,1.200000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([9.900000,13.175000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([19.100000,1.600000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([2.101000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([46.800000,13.400000,1.200000])m14(part,hole,block,casetop); // D19 (back)
translate([39.000000,5.400000,1.200000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([0.700000,13.100000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([16.750000,15.200000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([39.000000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([37.500000,10.200000,1.200000])m15(part,hole,block,casetop); // U3 (back)
translate([42.400000,13.400000,1.200000])rotate([0,0,180.000000])m14(part,hole,block,casetop); // D19 (back)
translate([4.056250,2.600000,1.200000])rotate([0,0,180.000000])m11(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([5.700000,13.100000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([19.150000,15.150000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([27.500000,8.200000,1.200000])rotate([0,0,180.000000])m16(part,hole,block,casetop); // U10 (back)
translate([12.101000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([6.600000,13.100000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([8.350000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([44.800000,10.200000,1.200000])rotate([0,0,90.000000])m15(part,hole,block,casetop); // U3 (back)
translate([45.900000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([12.400000,13.175000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([14.601000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([11.250000,2.250000,1.200000])rotate([-0.000000,-0.000000,-90.000000])m8(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
translate([8.750000,5.550000,1.200000])rotate([0,0,90.000000])m7(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([1.600000,13.100000,1.200000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([36.500000,15.200000,1.200000])rotate([0,0,45.000000])m5(part,hole,block,casetop); // D16 (back)
translate([35.800000,3.000000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([40.250000,15.200000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([10.700000,13.100000,1.200000])rotate([0,0,-90.000000])m3(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([4.056250,0.700000,1.200000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
}

parts_top=20;
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
}

parts_bottom=0;
module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(part=false,hole=false,block=false,height,N=0)
{ // J5
// PTSM socket
if(part)
{
	hull()
	{
		b(0,-7.5/2+0.3,0,1.7+N*2.5,7.5,4);
		b(0,-7.5/2+0.3,0,1.7+N*2.5-2,7.5,5);
	}
	// Pins
	for(p=[0:N-1])translate([-2.5*(N-1)/2+p*2.5,0,-2.1])cylinder(r1=0.3,r2=1,h=2.1);
}
if(hole)
{
	b(0,-10.5/2-7.5+0.3,0,1.1+N*2.5,10.5,5);
}
}

module m1(part=false,hole=false,block=false,height,N=0)
{ // J6
// PTSM socket
if(part)
{
	hull()
	{
		b(0,-7.5/2+0.3,0,1.7+N*2.5,7.5,4);
		b(0,-7.5/2+0.3,0,1.7+N*2.5-2,7.5,5);
	}
	// Pins
	for(p=[0:N-1])translate([-2.5*(N-1)/2+p*2.5,0,-2.1])cylinder(r1=0.3,r2=1,h=2.1);
}
if(hole)
{
	b(0,-10.5/2-7.5+0.3,0,1.1+N*2.5,10.5,5);
}
}

module m2(part=false,hole=false,block=false,height,N=0)
{ // J2
// PTSM socket
if(part)
{
	hull()
	{
		b(0,-7.5/2+0.3,0,1.7+N*2.5,7.5,4);
		b(0,-7.5/2+0.3,0,1.7+N*2.5-2,7.5,5);
	}
	// Pins
	for(p=[0:N-1])translate([-2.5*(N-1)/2+p*2.5,0,-2.1])cylinder(r1=0.3,r2=1,h=2.1);
}
if(hole)
{
	b(0,-10.5/2-7.5+0.3,0,1.1+N*2.5,10.5,5);
}
}

module m3(part=false,hole=false,block=false,height)
{ // RevK:C_0402 C_0402_1005Metric
// 0402 Capacitor
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
}

module m4(part=false,hole=false,block=false,height)
{ // RevK:R_0402 R_0402_1005Metric
// 0402 Resistor
if(part)
{
	b(0,0,0,1.5,0.65,0.2); // Pad size
	b(0,0,0,1.0,0.5,0.5); // Chip
}
}

module m5(part=false,hole=false,block=false,height)
{ // D16
// 1x1mm LED
if(part)
{
        b(0,0,0,1,1,.8);
}
if(hole)
{
        hull()
        {
                b(0,0,.8,1,1,1);
                translate([0,0,height])cylinder(d=2,h=1,$fn=16);
        }
}
if(block)
{
        hull()
        {
                b(0,0,0,2,2,1);
                translate([0,0,height])cylinder(d=4,h=1,$fn=16);
        }
}
}

module m6(part=false,hole=false,block=false,height)
{ // RevK:ESE13 ESE13V01D
// Tamper switch
rotate([0,0,180])translate([0,2.05,-1.15])
{
	if(part)
	{
		b(0,0,0,3.6,4.2,1.4);
		b(0,-3.08,0,1.2,1.95,0.9);
	}
	if(block)
	{
		b(0,-3.08,1.5,1.2+2,1.95+2,height);
	}
}

}

module m7(part=false,hole=false,block=false,height)
{ // RevK:C_0603_ C_0603_1608Metric
// 0603 Capacitor
if(part)
{
	b(0,0,0,1.6,0.8,1); // Chip
	b(0,0,0,1.6,0.95,0.2); // Pad size
}
}

module m8(part=false,hole=false,block=false,height)
{ // RevK:L_4x4_ TYA4020
// 4x4 Inductor
if(part)
{
	b(0,0,0,4,4,3);
}
}

module m9(part=false,hole=false,block=false,height)
{ // RevK:USB-C-Socket-H CSP-USC16-TR
// USB connector
rotate([-90,0,0])translate([-4.47,-3.84,0])
{
	if(part)
	{
		b(4.47,7,0,7,2,0.2);	// Pads
		translate([1.63,-0.2,1.63])
		rotate([-90,0,0])
		hull()
		{
			cylinder(d=3.26,h=7.55,$fn=24);
			translate([5.68,0,0])
			cylinder(d=3.26,h=7.55,$fn=24);
		}
		translate([0,6.25,0])cube([8.94,1.1,1.63]);
		translate([0,1.7,0])cube([8.94,1.6,1.63]);
	}
	if(hole)
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

module m10(part=false,hole=false,block=false,height)
{ // Q2
// SOT-323_SC-70
if(part)
{
	b(0,0,0,1.26,2.2,1.2);
	b(0,0,0,2.2,2.2,0.6);
}
}

module m11(part=false,hole=false,block=false,height)
{ // RevK:SOT-23-6-MD8942 SOT-23-6
// SOT-23-6
if(part)
{
	b(0,0,0,1.726,3.026,1.2); // Part
	b(0,0,0,3.6,2.5,0.5); // Pins
}
}

module m12(part=false,hole=false,block=false,height)
{ // RevK:C_1210 C_1210_3225Metric
// 1210 Capacitor
if(part)
{
	b(0,0,0,3.2,2.6,2); // Part
	b(0,0,0,4.1,2.7,0.2); // Pads
}
}

module m13(part=false,hole=false,block=false,height)
{ // RevK:C_1206 C_1206_3216Metric
// 1206 Capacitor
if(part)
{
	b(0,0,0,3.2,1.6,1.5); // Part
	b(0,0,0,4.1,1.8,0.2); // Pads
}
}

module m14(part=false,hole=false,block=false,height)
{ // D19
// SOD-123 Diode
if(part)
{
	b(0,0,0,2.85,1.8,1.35); // part
	b(0,0,0,4.2,1.2,0.7); // pads
}
}

module m15(part=false,hole=false,block=false,height)
{ // U3
// SO-4_4.4x4.3mm_P2.54mm 
if(part)
{
	b(0,0,0,4.5,4.6,2.2); // Part
	b(0,0,0,3.34,6.8,1.5); // Pins
}
}

module m16(part=false,hole=false,block=false,height)
{ // U10
// ESP32-S3-MINI-1
translate([-15.4/2,-15.45/2,0])
{
	if(part)
	{
		cube([15.4,20.5,0.8]);
		translate([0.7,0.5,0])cube([14,13.55,2.4]);
	}
	if(hole)
	{
		cube([15.4,20.5,0.8]);
	}
}
}

// Generate PCB casework

height=casebottom+pcbthickness+casetop;
$fn=48;

module pyramid()
{ // A pyramid
 polyhedron(points=[[0,0,0],[-height,-height,height],[-height,height,height],[height,height,height],[height,-height,height]],faces=[[0,1,2],[0,2,3],[0,3,4],[0,4,1],[4,3,2,1]]);
}


module pcb_hulled(h=pcbthickness,r=0)
{ // PCB shape for case
	if(useredge)outline(h,r);
	else hull()outline(h,r);
}

module solid_case(d=0)
{ // The case wall
	hull()
        {
                translate([0,0,-casebottom])pcb_hulled(height,casewall-edge);
                translate([0,0,edge-casebottom])pcb_hulled(height-edge*2,casewall);
        }
}

module preview()
{
	pcb();
	color("#0f0")parts_top(part=true);
	color("#0f0")parts_bottom(part=true);
	color("#f00")parts_top(hole=true);
	color("#f00")parts_bottom(hole=true);
	color("#00f8")parts_top(block=true);
	color("#00f8")parts_bottom(block=true);
}

module top_half(step=false)
{
	difference()
	{
		translate([-casebottom-100,-casewall-100,pcbthickness-lip/2+0.01]) cube([pcbwidth+casewall*2+200,pcblength+casewall*2+200,height]);
		if(step)translate([0,0,pcbthickness-lip/2-0.01])pcb_hulled(lip,casewall/2+fit);
	}
}

module bottom_half(step=false)
{
	translate([-casebottom-100,-casewall-100,pcbthickness+lip/2-height-0.01]) cube([pcbwidth+casewall*2+200,pcblength+casewall*2+200,height]);
	if(step)translate([0,0,pcbthickness-lip/2])pcb_hulled(lip,casewall/2-fit);
}

module case_wall()
{
	difference()
	{
		solid_case();
		translate([0,0,-height])pcb_hulled(height*2);
	}
}

module top_side_hole()
{
	intersection()
	{
		parts_top(hole=true);
		case_wall();
	}
}

module bottom_side_hole()
{
	intersection()
	{
		parts_bottom(hole=true);
		case_wall();
	}
}

module parts_space()
{
	minkowski()
	{
		union()
		{
			parts_top(part=true,hole=true);
			parts_bottom(part=true,hole=true);
		}
		sphere(r=margin,$fn=6);
	}
}

module top_cut()
{
	difference()
	{
		top_half(true);
		if(parts_top)difference()
		{
			minkowski()
			{ // Penetrating side holes
				top_side_hole();
				rotate([180,0,0])
				pyramid();
			}
			minkowski()
			{
				top_side_hole();
				rotate([0,0,45])cylinder(r=margin,h=height,$fn=4);
			}
		}
	}
	if(parts_bottom)difference()
	{
		minkowski()
		{ // Penetrating side holes
			bottom_side_hole();
			pyramid();
		}
			minkowski()
			{
				bottom_side_hole();
				rotate([0,0,45])translate([0,0,-height])cylinder(r=margin,h=height,$fn=4);
			}
	}
}

module bottom_cut()
{
	difference()
	{
		 translate([-casebottom-50,-casewall-50,-height]) cube([pcbwidth+casewall*2+100,pcblength+casewall*2+100,height*2]);
		 top_cut();
	}
}

module top_body()
{
	difference()
	{
		intersection()
		{
			solid_case();
			pcb_hulled(height);
			top_half();
		}
		if(parts_top)minkowski()
		{
			if(nohull)parts_top(part=true);
			else hull()parts_top(part=true);
			translate([0,0,margin-height])cylinder(r=margin,h=height,$fn=8);
		}
	}
	intersection()
	{
		solid_case();
		parts_top(block=true);
	}
}

module top_edge()
{
	intersection()
	{
		case_wall();
		top_cut();
	}
}

module top()
{
	translate([casewall,casewall+pcblength,pcbthickness+casetop])rotate([180,0,0])difference()
	{
		union()
		{
			top_body();
			top_edge();
		}
		parts_space();
		translate([0,0,pcbthickness-height])pcb(height,r=margin);
	}
}

module bottom_body()
{
	difference()
	{
		intersection()
		{
			solid_case();
			translate([0,0,-height])pcb_hulled(height);
			bottom_half();
		}
		if(parts_bottom)minkowski()
		{
			if(nohull)parts_bottom(part=true);
			else hull()parts_bottom(part=true);
			translate([0,0,-margin])cylinder(r=margin,h=height,$fn=8);
		}
	}
	intersection()
	{
		solid_case();
		parts_bottom(block=true);
	}
}

module bottom_edge()
{
	intersection()
	{
		case_wall();
		bottom_cut();
	}
}

module bottom()
{
	translate([casewall,casewall,casebottom])difference()
	{
		union()
		{
        		bottom_body();
        		bottom_edge();
		}
		parts_space();
		pcb(height,r=margin);
	}
}
bottom(); translate([spacing,0,0])top();
