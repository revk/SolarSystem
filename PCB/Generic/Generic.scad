// Generated case design for Generic/Generic.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-12-28 10:59:13
// title:	PCB-GENERIC-GPS
// rev:	5
// company:	Adrian Kennard Andrews & Arnold Ltd
// comment:	www.me.uk
// comment:	@TheRealRevK
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

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[22.100000,30.200000],[7.900000,30.200000],[7.900000,23.200000],[7.873205,23.100000],[7.800000,23.026795],[7.700000,23.000000],[3.000000,23.000000],[2.414729,22.942356],[1.851950,22.771638],[1.333290,22.494408],[0.878680,22.121320],[0.505592,21.666710],[0.228362,21.148050],[0.057644,20.585271],[0.000000,20.000000],[0.000000,0.000000],[30.000000,0.000000],[30.000000,20.000000],[29.942356,20.585271],[29.771638,21.148050],[29.494408,21.666710],[29.121320,22.121320],[28.666710,22.494408],[28.148050,22.771638],[27.585271,22.942356],[27.000000,23.000000],[22.300000,23.000000],[22.200000,23.026795],[22.126795,23.100000],[22.100000,23.200000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[22.100000,30.200000],[7.900000,30.200000],[7.900000,23.200000],[7.873205,23.100000],[7.800000,23.026795],[7.700000,23.000000],[3.000000,23.000000],[2.414729,22.942356],[1.851950,22.771638],[1.333290,22.494408],[0.878680,22.121320],[0.505592,21.666710],[0.228362,21.148050],[0.057644,20.585271],[0.000000,20.000000],[0.000000,0.000000],[30.000000,0.000000],[30.000000,20.000000],[29.942356,20.585271],[29.771638,21.148050],[29.494408,21.666710],[29.121320,22.121320],[28.666710,22.494408],[28.148050,22.771638],[27.585271,22.942356],[27.000000,23.000000],[22.300000,23.000000],[22.200000,23.026795],[22.126795,23.100000],[22.100000,23.200000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29]]);}
spacing=46.000000;
pcbwidth=30.000000;
pcblength=30.200000;
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
}

parts_top=0;
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
translate([19.200000,7.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m0(part,hole,block,casebottom); // RevK:SOT-23-6-MD8942 SOT-23-6
translate([19.200000,2.600000,0.000000])rotate([180,0,0])rotate([-0.000000,-0.000000,-90.000000])m1(part,hole,block,casebottom); // RevK:L_4x4_ TYA4020
translate([20.700000,14.900000,0.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
translate([19.200000,5.200000,0.000000])rotate([180,0,0])m3(part,hole,block,casebottom); // RevK:C_0603_ C_0603_1608Metric
translate([21.000000,9.100000,0.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
translate([25.600000,16.500000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])translate([0.000000,-1.050000,0.000000])rotate([90.000000,-0.000000,-0.000000])m4(part,hole,block,casebottom); // RevK:USC16-TR CSP-USC16-TR
// Missing model D4.1 SOT-363_SC-70-6 (back)
// Missing model D1.1 D_1206_3216Metric (back)
translate([21.700000,5.900000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m3(part,hole,block,casebottom); // RevK:C_0603_ C_0603_1608Metric
translate([24.437500,9.750000,0.000000])rotate([180,0,0])m5(part,hole,block,casebottom); // Q1
translate([17.400000,9.100000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
translate([8.100000,8.100000,0.000000])rotate([180,0,0])m6(part,hole,block,casebottom); // U1
// Missing model D5.1 SOT-363_SC-70-6 (back)
translate([16.250000,1.550000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m7(part,hole,block,casebottom); // RevK:C_0402 C_0402_1005Metric
translate([16.250000,3.450000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
translate([2.250000,20.550000,0.000000])rotate([0,0,135.000000])rotate([180,0,0])m8(part,hole,block,casebottom); // D2
translate([19.200000,9.100000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
translate([18.350000,18.650000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m7(part,hole,block,casebottom); // RevK:C_0402 C_0402_1005Metric
translate([20.700000,17.900000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
translate([26.000000,7.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m9(part,hole,block,casebottom,2); // J1
translate([15.000000,22.957500,0.000000])rotate([180,0,0])m9(part,hole,block,casebottom,5); // J1
translate([16.700000,5.900000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(part,hole,block,casebottom); // RevK:C_0603_ C_0603_1608Metric
}

parts_bottom=5;
module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(part=false,hole=false,block=false,height)
{ // RevK:SOT-23-6-MD8942 SOT-23-6
// SOT-23-6
if(part)
{
	b(0,0,0,1.726,3.026,1.2); // Part
	b(0,0,0,3.6,2.5,0.5); // Pins
}
}

module m1(part=false,hole=false,block=false,height)
{ // RevK:L_4x4_ TYA4020
// 4x4 Inductor
if(part)
{
	b(0,0,0,4,4,3);
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
{ // RevK:C_0603_ C_0603_1608Metric
// 0603 Capacitor
if(part)
{
	b(0,0,0,1.6,0.8,1); // Chip
	b(0,0,0,1.6,0.95,0.2); // Pad size
}
}

module m4(part=false,hole=false,block=false,height)
{ // RevK:USC16-TR CSP-USC16-TR
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

module m5(part=false,hole=false,block=false,height)
{ // Q1
// SOT-23
if(part)
{
	b(0,0,0,1.4,3.0,1.1); // Body
	b(-0.9375,-0.95,0,1.475,0.6,0.5); // Pad
	b(-0.9375,0.95,0,1.475,0.6,0.5); // Pad
	b(0.9375,0,0,1.475,0.6,0.5); // Pad
}
}

module m6(part=false,hole=false,block=false,height)
{ // U1
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

module m7(part=false,hole=false,block=false,height)
{ // RevK:C_0402 C_0402_1005Metric
// 0402 Capacitor
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
}

module m8(part=false,hole=false,block=false,height)
{ // D2
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

module m9(part=false,hole=false,block=false,height,N=0)
{ // J1
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
