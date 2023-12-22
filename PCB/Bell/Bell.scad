// Generated case design for Bell/Bell.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-12-22 09:36:58
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

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.250000,0.000000],[34.950000,0.000000],[35.200000,0.250000],[35.200000,28.450000],[34.950000,28.700000],[18.800000,28.700000],[18.446447,28.553553],[18.300000,28.200000],[18.300000,25.150000],[18.265926,24.891181],[18.166026,24.650000],[18.007107,24.442893],[17.800000,24.283974],[17.558819,24.184074],[17.300000,24.150000],[0.250000,24.150000],[0.000000,23.900000],[0.000000,0.250000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[0.250000,0.000000],[34.950000,0.000000],[35.200000,0.250000],[35.200000,28.450000],[34.950000,28.700000],[18.800000,28.700000],[18.446447,28.553553],[18.300000,28.200000],[18.300000,25.150000],[18.265926,24.891181],[18.166026,24.650000],[18.007107,24.442893],[17.800000,24.283974],[17.558819,24.184074],[17.300000,24.150000],[0.250000,24.150000],[0.000000,23.900000],[0.000000,0.250000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17]]);}
spacing=51.200000;
pcbwidth=35.200000;
pcblength=28.700000;
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
translate([28.000000,17.800000,0.800000])rotate([0,0,90.000000])m0(part,hole,block,casetop,8); // J3 (back)
translate([25.800000,9.050000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([25.800000,19.050000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([25.800000,14.050000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([23.300000,7.200000,0.800000])m0(part,hole,block,casetop,2); // J3 (back)
translate([17.850000,23.100000,0.800000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([8.650000,16.400000,0.800000])m3(part,hole,block,casetop); // U6 (back)
translate([3.935000,6.900000,0.800000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([23.650000,24.100000,0.800000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([25.800000,10.300000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([0.735000,2.600000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([23.750000,21.200000,0.800000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([0.735000,6.200000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([0.735000,4.400000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([28.000000,3.400000,0.800000])rotate([0,0,90.000000])m0(part,hole,block,casetop,2); // J3 (back)
translate([25.800000,24.050000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([25.800000,11.550000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
// Missing model U4.1 SO-8_3.9x4.9mm_P1.27mm
// Missing model U3.1 SO-8_3.9x4.9mm_P1.27mm
translate([8.450000,7.900000,0.800000])rotate([0,0,180.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([25.750000,27.825000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([21.225000,27.100000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([7.235000,4.400000,0.800000])rotate([0,0,90.000000])rotate([-0.000000,-0.000000,-90.000000])m6(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
translate([3.935000,1.900000,0.800000])rotate([0,0,180.000000])m4(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([25.800000,12.800000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([25.800000,26.550000,0.800000])rotate([0,0,135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([19.400000,3.800000,0.800000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
translate([16.900000,23.100000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([20.350000,21.900000,0.800000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D3 (back)
translate([25.800000,22.780000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([25.800000,17.780000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([25.800000,25.280000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([22.950000,21.200000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([19.950000,27.100000,0.800000])rotate([0,0,-135.000000])m1(part,hole,block,casetop); // D9 (back)
translate([4.635000,4.400000,0.800000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
translate([23.250000,25.400000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([2.635000,4.400000,0.800000])rotate([0,0,90.000000])m8(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
translate([14.300000,4.400000,0.800000])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m9(part,hole,block,casetop); // RevK:USB-C-Socket-H CSP-USC16-TR (back)
translate([22.850000,24.100000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
translate([23.350000,22.400000,0.800000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
}

parts_top=12;
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
// Missing model J5.1 1815154 (back)
// Missing model J5.2 1778793 (back)
}

parts_bottom=0;
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
	b(0,0,0,1,1,.8);
}
if(hole)
{
	hull()
	{
		b(0,0,.8,1,1,1);
		b(0,0,height,2,2,1);
	}
}
if(block)
{
	hull()
	{
		b(0,0,0,3,3,1);
		b(0,0,height,4,4,1);
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
{ // RevK:L_4x4_ TYA4020
// 4x4 Inductor
if(part)
{
	b(0,0,0,4,4,3);
}
}

module m7(part=false,hole=false,block=false,height)
{ // D3
// SOD-123 Diode
if(part)
{
	b(0,0,0,2.85,1.8,1.35); // part
	b(0,0,0,4.2,1.2,0.7); // pads
}
}

module m8(part=false,hole=false,block=false,height)
{ // RevK:SOT-23-6-MD8942 SOT-23-6
// SOT-23-6
if(part)
{
	b(0,0,0,1.726,3.026,1.2); // Part
	b(0,0,0,3.6,2.5,0.5); // Pins
}
}

module m9(part=false,hole=false,block=false,height)
{ // RevK:USB-C-Socket-H CSP-USC16-TR
// USB connector
rotate([-90,0,0])translate([-4.47,-3.84,0])
{
	if(part)
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
				cylinder(d=margin,h=height,$fn=4);
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
				translate([0,0,-height])cylinder(d=margin,h=height,$fn=4);
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
