// Generated case design for Generic/Generic.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2025-08-18 13:34:03
// title:	PCB-GENERIC-GPS
// rev:	5
// company:	Adrian Kennard Andrews & Arnold Ltd
// comment:	www.me.uk
// comment:	@TheRealRevK
//

// Globals
margin=0.200000;
lip=3.000000;
lipa=0;
lipt=2;
casebottom=2.000000;
casetop=6.000000;
casewall=3.000000;
fit=0.000000;
snap=0.150000;
edge=2.000000;
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;
spacing=46.000000;
pcbwidth=30.000000;
pcblength=30.200000;
originx=100.000000;
originy=89.200000;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[7.100000,15.100000],[-7.100000,15.100000],[-7.100000,8.100000],[-7.126795,8.000000],[-7.200000,7.926795],[-7.300000,7.900000],[-12.000000,7.900000],[-12.585271,7.842356],[-13.148050,7.671638],[-13.666710,7.394408],[-14.121320,7.021320],[-14.494408,6.566710],[-14.771638,6.048050],[-14.942356,5.485271],[-15.000000,4.900000],[-15.000000,-15.100000],[15.000000,-15.100000],[15.000000,4.900000],[14.942356,5.485271],[14.771638,6.048050],[14.494408,6.566710],[14.121320,7.021320],[13.666710,7.394408],[13.148050,7.671638],[12.585271,7.842356],[12.000000,7.900000],[7.300000,7.900000],[7.200000,7.926795],[7.126795,8.000000],[7.100000,8.100000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[7.100000,15.100000],[-7.100000,15.100000],[-7.100000,8.100000],[-7.126795,8.000000],[-7.200000,7.926795],[-7.300000,7.900000],[-12.000000,7.900000],[-12.585271,7.842356],[-13.148050,7.671638],[-13.666710,7.394408],[-14.121320,7.021320],[-14.494408,6.566710],[-14.771638,6.048050],[-14.942356,5.485271],[-15.000000,4.900000],[-15.000000,-15.100000],[15.000000,-15.100000],[15.000000,4.900000],[14.942356,5.485271],[14.771638,6.048050],[14.494408,6.566710],[14.121320,7.021320],[13.666710,7.394408],[13.148050,7.671638],[12.585271,7.842356],[12.000000,7.900000],[7.300000,7.900000],[7.200000,7.926795],[7.126795,8.000000],[7.100000,8.100000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29]]);}
module part_PCB1(part=true,hole=false,block=false)
{
};
module part_Logo2(part=true,hole=false,block=false)
{
};
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
part_PCB1(part,hole,block);
part_Logo2(part,hole,block);
}

parts_top=0;
module part_U4(part=true,hole=false,block=false)
{
translate([4.200000,-7.900000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m0(part,hole,block,casebottom); // RevK:SOT-23-6-MD8942 SOT-23-6
};
module part_L2(part=true,hole=false,block=false)
{
translate([4.200000,-12.500000,0.000000])rotate([180,0,0])rotate([-0.000000,-0.000000,-90.000000])m1(part,hole,block,casebottom); // RevK:L_4x4_ TYA4020
};
module part_V2(part=true,hole=false,block=false)
{
};
module part_R3(part=true,hole=false,block=false)
{
translate([5.700000,-0.200000,0.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
};
module part_C7(part=true,hole=false,block=false)
{
translate([4.200000,-9.900000,0.000000])rotate([180,0,0])m3(part,hole,block,casebottom); // RevK:C_0603_ C_0603_1608Metric
};
module part_R13(part=true,hole=false,block=false)
{
translate([6.000000,-6.000000,0.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
};
module part_J2(part=true,hole=false,block=false)
{
translate([9.200000,1.400000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])translate([0.000000,-2.400000,0.000000])rotate([90.000000,-0.000000,-0.000000])m4(part,hole,block,casebottom); // RevK:USB-C-Socket-H CSP-USC16-TR
};
module part_D4(part=true,hole=false,block=false)
{
translate([-1.250000,5.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(part,hole,block,casebottom); // D4
};
module part_D1(part=true,hole=false,block=false)
{
// Missing model D1.1 D_1206_3216Metric (back)
};
module part_V1(part=true,hole=false,block=false)
{
};
module part_C5(part=true,hole=false,block=false)
{
translate([6.700000,-9.200000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m3(part,hole,block,casebottom); // RevK:C_0603_ C_0603_1608Metric
};
module part_Q1(part=true,hole=false,block=false)
{
translate([9.437500,-5.350000,0.000000])rotate([180,0,0])m6(part,hole,block,casebottom); // Q1
};
module part_J5(part=true,hole=false,block=false)
{
};
module part_R12(part=true,hole=false,block=false)
{
translate([2.400000,-6.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
};
module part_U1(part=true,hole=false,block=false)
{
translate([-6.900000,-7.000000,0.000000])rotate([180,0,0])m7(part,hole,block,casebottom); // U1
};
module part_D5(part=true,hole=false,block=false)
{
translate([3.750000,5.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(part,hole,block,casebottom); // D4
};
module part_C2(part=true,hole=false,block=false)
{
translate([1.250000,-13.550000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m8(part,hole,block,casebottom); // RevK:C_0402 C_0402_1005Metric
};
module part_R4(part=true,hole=false,block=false)
{
translate([1.250000,-11.650000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
};
module part_D2(part=true,hole=false,block=false)
{
translate([-12.750000,5.450000,0.000000])rotate([0,0,135.000000])rotate([180,0,0])m9(part,hole,block,casebottom); // D2
};
module part_R8(part=true,hole=false,block=false)
{
translate([4.200000,-6.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
};
module part_C1(part=true,hole=false,block=false)
{
translate([3.350000,3.550000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m8(part,hole,block,casebottom); // RevK:C_0402 C_0402_1005Metric
};
module part_R2(part=true,hole=false,block=false)
{
translate([5.700000,2.800000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m2(part,hole,block,casebottom); // RevK:R_0402 R_0402_1005Metric
};
module part_J1(part=true,hole=false,block=false)
{
translate([11.000000,-7.900000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m10(part,hole,block,casebottom,2); // J1
};
module part_J4(part=true,hole=false,block=false)
{
translate([0.000000,7.857500,0.000000])rotate([180,0,0])m10(part,hole,block,casebottom,5); // J1
};
module part_C6(part=true,hole=false,block=false)
{
translate([1.700000,-9.200000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(part,hole,block,casebottom); // RevK:C_0603_ C_0603_1608Metric
};
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
part_U4(part,hole,block);
part_L2(part,hole,block);
part_V2(part,hole,block);
part_R3(part,hole,block);
part_C7(part,hole,block);
part_R13(part,hole,block);
part_J2(part,hole,block);
part_D4(part,hole,block);
part_D1(part,hole,block);
part_V1(part,hole,block);
part_C5(part,hole,block);
part_Q1(part,hole,block);
part_J5(part,hole,block);
part_R12(part,hole,block);
part_U1(part,hole,block);
part_D5(part,hole,block);
part_C2(part,hole,block);
part_R4(part,hole,block);
part_D2(part,hole,block);
part_R8(part,hole,block);
part_C1(part,hole,block);
part_R2(part,hole,block);
part_J1(part,hole,block);
part_J4(part,hole,block);
part_C6(part,hole,block);
}

parts_bottom=7;
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
		translate([1.63,-20,1.63])
		rotate([-90,0,0])
	{
		// Plug
		hull()
		{
			cylinder(d=2.5,h=21,$fn=24);
			translate([5.68,0,0])
			cylinder(d=2.5,h=21,$fn=24);
		}
		hull()
		{
			cylinder(d=7,h=21,$fn=24);
			translate([5.68,0,0])
			cylinder(d=7,h=21,$fn=24);
		}
		translate([2.84,0,-100])
			cylinder(d=5,h=100,$fn=24);
	}
}
}

module m5(part=false,hole=false,block=false,height)
{ // D4
if(part)
{
	b(0,0,0,1.15,2.0,1.1);
	b(0,0,0,2.1,2.0,0.6);
}
}

module m6(part=false,hole=false,block=false,height)
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

module m7(part=false,hole=false,block=false,height)
{ // U1
// ESP32-S3-MINI-1
translate([-15.4/2,-15.45/2,0])
{
	if(part)
	{
		cube([15.4,20.5,0.8]);
		translate([0.7,0.5,0])cube([14,13.55,2.4]);
		cube([15.4,20.5,0.8]);
	}
}
}

module m8(part=false,hole=false,block=false,height)
{ // RevK:C_0402 C_0402_1005Metric
// 0402 Capacitor
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
}

module m9(part=false,hole=false,block=false,height)
{ // D2
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
                b(0,0,.8,2.8,2.8,1);
                translate([0,0,height])cylinder(d=4,h=1,$fn=16);
        }
}
}

module m10(part=false,hole=false,block=false,height,N=0)
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

module top_half(fit=0)
{
	difference()
	{
		translate([-casebottom-100,-casewall-100,pcbthickness+0.01]) cube([pcbwidth+casewall*2+200,pcblength+casewall*2+200,height]);
		translate([0,0,pcbthickness])
        	{
			snape=lip/5;
			snaph=(lip-snape*2)/3;
			if(lipt==1)rotate(lipa)hull()
			{
				translate([0,-pcblength,lip/2])cube([0.001,pcblength*2,0.001]);
				translate([-lip/2,-pcblength,0])cube([lip,pcblength*2,0.001]);
			} else if(lipt==2)for(a=[0,90,180,270])rotate(a+lipa)hull()
			{
				translate([0,-pcblength-pcbwidth,lip/2])cube([0.001,pcblength*2+pcbwidth*2,0.001]);
				translate([-lip/2,-pcblength-pcbwidth,0])cube([lip,pcblength*2+pcbwidth*2,0.001]);
			}
            		difference()
            		{
                		pcb_hulled(lip,casewall);
				if(snap)
                        	{
					hull()
					{
						pcb_hulled(0.1,casewall/2-snap/2+fit);
						translate([0,0,snape])pcb_hulled(snaph,casewall/2+snap/2+fit);
						translate([0,0,lip-snape-snaph])pcb_hulled(0.1,casewall/2-snap/2+fit);
					}
					translate([0,0,lip-snape-snaph])pcb_hulled(snaph,casewall/2-snap/2+fit);
					hull()
					{
						translate([0,0,lip-snape])pcb_hulled(0.1,casewall/2-snap/2+fit);
						translate([0,0,lip])pcb_hulled(0.1,casewall/2+snap/2+fit);
					}
                        	}
				else pcb_hulled(lip,casewall/2+fit);
				if(lipt==0)translate([-pcbwidth,-pcblength,0])cube([pcbwidth*2,pcblength*2,lip]);
				else if(lipt==1) rotate(lipa)translate([0,-pcblength,0])hull()
				{
					translate([lip/2,0,0])cube([pcbwidth,pcblength*2,lip]);
					translate([-lip/2,0,lip])cube([pcbwidth,pcblength*2,lip]);
				}
				else if(lipt==2)for(a=[0,180])rotate(a+lipa)hull()
                		{
                            		translate([lip/2,lip/2,0])cube([pcbwidth+pcblength,pcbwidth+pcblength,lip]);
                            		translate([-lip/2,-lip/2,lip])cube([pcbwidth+pcblength,pcbwidth+pcblength,lip]);
                		}
            		}
            		difference()
            		{
				if(snap)
                        	{
					hull()
					{
						translate([0,0,-0.1])pcb_hulled(0.1,casewall/2+snap/2-fit);
						translate([0,0,snape-0.1])pcb_hulled(0.1,casewall/2-snap/2-fit);
					}
					translate([0,0,snape])pcb_hulled(snaph,casewall/2-snap/2-fit);
					hull()
					{
						translate([0,0,snape+snaph])pcb_hulled(0.1,casewall/2-snap/2-fit);
						translate([0,0,lip-snape-snaph])pcb_hulled(snaph,casewall/2+snap/2-fit);
						translate([0,0,lip-0.1])pcb_hulled(0.1,casewall/2-snap/2-fit);
					}
                        	}
				else pcb_hulled(lip,casewall/2-fit);
				if(lipt==1)rotate(lipa+180)translate([0,-pcblength,0])hull()
				{
					translate([lip/2,0,0])cube([pcbwidth,pcblength*2,lip+0.1]);
					translate([-lip/2,0,lip])cube([pcbwidth,pcblength*2,lip+0.1]);
				}
				else if(lipt==2)for(a=[90,270])rotate(a+lipa)hull()
                		{
                            		translate([lip/2,lip/2,0])cube([pcbwidth+pcblength,pcbwidth+pcblength,lip]);
                            		translate([-lip/2,-lip/2,lip])cube([pcbwidth+pcblength,pcbwidth+pcblength,lip]);
                		}
			}
            	}
		minkowski()
                {
                	union()
                	{
                		parts_top(part=true);
                		parts_bottom(part=true);
                	}
                	translate([-0.01,-0.01,-height])cube([0.02,0.02,height]);
                }
        }
	minkowski()
        {
        	union()
                {
                	parts_top(part=true);
                	parts_bottom(part=true);
                }
                translate([-0.01,-0.01,0])cube([0.02,0.02,height]);
        }
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
	difference()
	{
		intersection()
		{
			parts_top(hole=true);
			case_wall();
		}
		translate([0,0,-casebottom])pcb_hulled(height,casewall-edge);
	}
}

module bottom_side_hole()
{
	difference()
	{
		intersection()
		{
			parts_bottom(hole=true);
			case_wall();
		}
		translate([0,0,edge-casebottom])pcb_hulled(height-edge*2,casewall);
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

module top_cut(fit=0)
{
	difference()
	{
		top_half(fit);
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
		 top_cut(-fit);
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
		}
		if(parts_top)minkowski()
		{
			if(nohull)parts_top(part=true);
			else hull(){parts_top(part=true);pcb_hulled();}
			translate([0,0,margin-height])cylinder(r=margin*2,h=height,$fn=8);
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

module top_pos()
{ // Position for plotting bottom
	translate([casewall,casewall,pcbthickness+casetop])rotate([180,0,0])children();
}

module pcb_pos()
{	// Position PCB relative to base 
		translate([0,0,pcbthickness-height])children();
}

module top()
{
	top_pos()difference()
	{
		union()
		{
			top_body();
			top_edge();
		}
		parts_space();
		pcb_pos()pcb(height,r=margin);
	}
}

module bottom_body()
{ // Position for plotting top
	difference()
	{
		intersection()
		{
			solid_case();
			translate([0,0,-height])pcb_hulled(height+pcbthickness);
		}
		if(parts_bottom)minkowski()
		{
			if(nohull)parts_bottom(part=true);
			else hull()parts_bottom(part=true);
			translate([0,0,-margin])cylinder(r=margin*2,h=height,$fn=8);
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

module bottom_pos()
{
	translate([casewall,casewall,casebottom])children();
}

module bottom()
{
	bottom_pos()difference()
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
top();
