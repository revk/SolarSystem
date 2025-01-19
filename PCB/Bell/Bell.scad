// Generated case design for Bell/Bell.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2025-01-19 12:01:38
// title:	Bell box controller
// date:	${DATE}
// rev:	2
// company:	Adrian Kennard, Andrews & Arnold Ltd
//

// Globals
margin=0.200000;
lip=2.000000;
casebottom=2.000000;
casetop=5.600000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[17.800000,48.850000],[17.800000,43.900000],[17.866987,43.650000],[18.050000,43.466987],[18.300000,43.400000],[18.550000,43.466987],[18.733013,43.650000],[18.800000,43.900000],[18.838061,44.091342],[18.946447,44.253553],[19.108658,44.361939],[19.300000,44.400000],[36.200000,44.400000],[36.582684,44.476120],[36.907107,44.692893],[37.123880,45.017316],[37.200000,45.400000],[37.200000,49.350000],[37.053553,49.703553],[36.700000,49.850000],[18.800000,49.850000],[18.417316,49.773880],[18.092893,49.557107],[17.876120,49.232684],[17.800000,21.400000],[17.868148,20.882362],[18.067949,20.400000],[18.385786,19.985786],[18.800000,19.667949],[19.282362,19.468148],[19.800000,19.400000],[20.050000,19.466987],[20.233013,19.650000],[20.300000,19.900000],[20.233013,20.150000],[20.050000,20.333013],[19.800000,20.400000],[19.541181,20.434074],[19.300000,20.533974],[19.092893,20.692893],[18.933974,20.900000],[18.834074,21.141181],[18.800000,21.400000],[18.733013,21.650000],[18.550000,21.833013],[18.300000,21.900000],[18.050000,21.833013],[17.866987,21.650000],[0.000000,0.000000],[70.000000,0.000000],[70.000000,70.000000],[0.000000,70.000000],[53.500000,19.400000],[53.963525,19.473415],[54.381678,19.686475],[54.713525,20.018322],[54.926585,20.436475],[55.000000,20.900000],[54.933013,21.150000],[54.750000,21.333013],[54.500000,21.400000],[54.250000,21.333013],[54.066987,21.150000],[54.000000,20.900000],[53.961939,20.708658],[53.853553,20.546447],[53.691342,20.438061],[53.500000,20.400000],[53.250000,20.333013],[53.066987,20.150000],[53.000000,19.900000],[53.066987,19.650000],[53.250000,19.466987],[54.000000,48.600000],[54.066987,48.350000],[54.250000,48.166987],[54.500000,48.100000],[54.750000,48.166987],[54.933013,48.350000],[55.000000,48.600000],[54.926585,49.063525],[54.713525,49.481678],[54.381678,49.813525],[53.963525,50.026585],[53.500000,50.100000],[53.250000,50.033013],[53.066987,49.850000],[53.000000,49.600000],[53.066987,49.350000],[53.250000,49.166987],[53.500000,49.100000],[53.691342,49.061939],[53.853553,48.953553],[53.961939,48.791342]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23],[24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47],[48,49,50,51],[52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72],[73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[17.800000,48.850000],[17.800000,43.900000],[17.866987,43.650000],[18.050000,43.466987],[18.300000,43.400000],[18.550000,43.466987],[18.733013,43.650000],[18.800000,43.900000],[18.838061,44.091342],[18.946447,44.253553],[19.108658,44.361939],[19.300000,44.400000],[36.200000,44.400000],[36.582684,44.476120],[36.907107,44.692893],[37.123880,45.017316],[37.200000,45.400000],[37.200000,49.350000],[37.053553,49.703553],[36.700000,49.850000],[18.800000,49.850000],[18.417316,49.773880],[18.092893,49.557107],[17.876120,49.232684],[17.800000,21.400000],[17.868148,20.882362],[18.067949,20.400000],[18.385786,19.985786],[18.800000,19.667949],[19.282362,19.468148],[19.800000,19.400000],[20.050000,19.466987],[20.233013,19.650000],[20.300000,19.900000],[20.233013,20.150000],[20.050000,20.333013],[19.800000,20.400000],[19.541181,20.434074],[19.300000,20.533974],[19.092893,20.692893],[18.933974,20.900000],[18.834074,21.141181],[18.800000,21.400000],[18.733013,21.650000],[18.550000,21.833013],[18.300000,21.900000],[18.050000,21.833013],[17.866987,21.650000],[0.000000,0.000000],[70.000000,0.000000],[70.000000,70.000000],[0.000000,70.000000],[53.500000,19.400000],[53.963525,19.473415],[54.381678,19.686475],[54.713525,20.018322],[54.926585,20.436475],[55.000000,20.900000],[54.933013,21.150000],[54.750000,21.333013],[54.500000,21.400000],[54.250000,21.333013],[54.066987,21.150000],[54.000000,20.900000],[53.961939,20.708658],[53.853553,20.546447],[53.691342,20.438061],[53.500000,20.400000],[53.250000,20.333013],[53.066987,20.150000],[53.000000,19.900000],[53.066987,19.650000],[53.250000,19.466987],[54.000000,48.600000],[54.066987,48.350000],[54.250000,48.166987],[54.500000,48.100000],[54.750000,48.166987],[54.933013,48.350000],[55.000000,48.600000],[54.926585,49.063525],[54.713525,49.481678],[54.381678,49.813525],[53.963525,50.026585],[53.500000,50.100000],[53.250000,50.033013],[53.066987,49.850000],[53.000000,49.600000],[53.066987,49.350000],[53.250000,49.166987],[53.500000,49.100000],[53.691342,49.061939],[53.853553,48.953553],[53.961939,48.791342]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23],[24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47],[48,49,50,51],[52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72],[73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93]]);}
spacing=86.000000;
pcbwidth=70.000000;
pcblength=70.000000;
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
translate([46.800000,38.200000,0.800000])rotate([0,0,90.000000])m0(part,hole,block,casetop,8); // J3 (back)
translate([44.650000,29.450000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([44.650000,39.450000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([44.650000,34.450000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([42.150000,27.650000,0.800000])m0(part,hole,block,casetop,2); // J3 (back)
translate([38.250000,44.800000,0.800000])rotate([0,0,180.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([27.450000,36.400000,0.800000])m3(part,hole,block,casetop); // U6 (back)
translate([22.785000,27.300000,0.800000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([43.100000,44.500000,0.800000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([44.650000,30.700000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([19.585000,23.000000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([43.100000,41.400000,0.800000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([19.585000,26.600000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([19.585000,24.800000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([46.850000,23.800000,0.800000])rotate([0,0,90.000000])m0(part,hole,block,casetop,2); // J3 (back)
translate([44.650000,44.450000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([44.650000,31.950000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([39.600000,36.843249,0.800000])m6(part,hole,block,casetop); // U4 (back)
translate([39.600000,31.300000,0.800000])m6(part,hole,block,casetop); // U4 (back)
translate([28.600000,26.300000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([44.650000,48.225000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([40.075000,47.500000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([26.085000,24.800000,0.800000])rotate([0,0,90.000000])rotate([-0.000000,-0.000000,-90.000000])m7(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
translate([22.785000,22.300000,0.800000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([44.650000,33.200000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([44.650000,46.950000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([28.600000,23.300000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([35.751000,43.000000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([40.000000,42.100000,0.800000])rotate([0,0,90.000000])m8(part,hole,block,casetop); // D3 (back)
translate([44.650000,43.180000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([44.650000,38.180000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([44.650000,45.700000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([42.300000,41.400000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([38.800000,47.500000,0.800000])rotate([0,0,-135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([23.485000,24.800000,0.800000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([42.100000,45.800000,0.800000])rotate([0,0,180.000000])m9(part,hole,block,casetop); // D1 (back)
translate([38.200000,42.100000,0.800000])rotate([0,0,-90.000000])m10(part,hole,block,casetop); // RevK:C_1206 C_1206_3216Metric (back)
translate([21.485000,24.800000,0.800000])rotate([0,0,90.000000])m11(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([33.800000,26.190000,0.800000])translate([0.000000,-2.400000,0.000000])rotate([90.000000,-0.000000,-0.000000])m12(part,hole,block,casetop); // RevK:USB-C-Socket-H CSP-USC16-TR (back)
translate([42.300000,44.500000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([42.200000,42.800000,0.800000])rotate([0,0,180.000000])m9(part,hole,block,casetop); // D1 (back)
}

parts_top=16;
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
translate([21.750000,30.200000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m13(part,hole,block,casebottom,5); // J5
}

parts_bottom=1;
module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(part=false,hole=false,block=false,height,N=0)
{ // J3
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

module m1(part=false,hole=false,block=false,height)
{ // D9
// 1x1mm LED
if(part)
{
        b(0,0,0,1.2,1.2,.8);
}
if(hole)
{
        hull()
        {
                b(0,0,.8,1.2,1.2,1);
                translate([0,0,height])cylinder(d=2,h=1,$fn=16);
        }
}
if(block)
{
        hull()
        {
                b(0,0,0,2.4,2.4,1);
                translate([0,0,height])cylinder(d=4,h=1,$fn=16);
        }
}
}

module m2(part=false,hole=false,block=false,height)
{ // RevK:R_0402 R_0402_1005Metric
// 0402 Resistor
if(part)
{
	b(0,0,0,1.5,0.65,0.2); // Pad size
	b(0,0,0,1.0,0.5,0.5); // Chip
}
}

module m3(part=false,hole=false,block=false,height)
{ // U6
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

module m4(part=false,hole=false,block=false,height)
{ // RevK:C_0603_ C_0603_1608Metric
// 0603 Capacitor
if(part)
{
	b(0,0,0,1.6,0.8,1); // Chip
	b(0,0,0,1.6,0.95,0.2); // Pad size
}
}

module m5(part=false,hole=false,block=false,height)
{ // RevK:C_0402 C_0402_1005Metric
// 0402 Capacitor
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
}

module m6(part=false,hole=false,block=false,height)
{ // U4
// SO-8_3.9x4.9mm_P1.27mm 
if(part)
{
	b(0,0,0,5.0,4.0,1.75); // Part
	b(0,0,0,6.9,4.1,1); // Pins
}
}

module m7(part=false,hole=false,block=false,height)
{ // RevK:L_4x4_ TYA4020
// 4x4 Inductor
if(part)
{
	b(0,0,0,4,4,3);
}
}

module m8(part=false,hole=false,block=false,height)
{ // D3
// SOD-123 Diode
if(part)
{
	b(0,0,0,2.85,1.8,1.35); // part
	b(0,0,0,4.2,1.2,0.7); // pads
}
}

module m9(part=false,hole=false,block=false,height)
{ // D1
// DFN1006-2L
if(part)
{
	b(0,0,0,1.0,0.6,0.45); // Chip
}
}

module m10(part=false,hole=false,block=false,height)
{ // RevK:C_1206 C_1206_3216Metric
// 1206 Capacitor
if(part)
{
	b(0,0,0,3.2,1.6,1.5); // Part
	b(0,0,0,4.1,1.8,0.2); // Pads
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
		translate([0,6.2501,0])cube([8.94,1.1,1.6301]);
		translate([0,1.7,0])cube([8.94,1.6,1.6301]);
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

module m13(part=false,hole=false,block=false,height,N=0)
{ // J5
// Socket on PCB
if(part)
{
	for(n=[0:N-1])b(-(N-1)*1.25+n*2.5,5.35,0,1.2,3.2,0.5); // Pins
	b(-8.25,1.15,0,2.2,5.6,0.2); // Tab
	b(8.25,1.15,0,2.2,5.6,0.2); // Tab
	hull()
	{
		b(0,0.9,0,1.7+N*2.5,7.5,4);
		b(0,0.9,0,1.7+N*2.5-2,7.5,5);
	}
	b(0,-7.5/2-2.85,0,0.5+N*2.5,7.5,5);
}
if(hole)
{ // Plug
	b(0,-7.5/2-2.85,0,0.5+N*2.5,7.5,5);
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
