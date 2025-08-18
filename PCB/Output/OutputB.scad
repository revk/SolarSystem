// Generated case design for Output/Output.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2025-08-18 13:35:42
// title:	PCB-OUTPUT
// rev:	1
// company:	Adrian Kennard, Andrews & Arnold Ltd
//

// Globals
margin=0.200000;
lip=3.000000;
lipa=0;
lipt=2;
casebottom=1.400000;
casetop=6.500000;
casewall=3.000000;
fit=0.000000;
snap=0.150000;
edge=2.000000;
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;
spacing=63.700000;
pcbwidth=47.700000;
pcblength=34.300000;
originx=100.050000;
originy=100.450000;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[-13.650000,17.150000],[-13.650000,13.000000],[-13.573880,12.617316],[-13.357107,12.292893],[-13.032684,12.076120],[-12.650000,12.000000],[12.650000,12.000000],[13.032684,12.076120],[13.357107,12.292893],[13.573880,12.617316],[13.650000,13.000000],[13.650000,17.150000],[23.850000,17.150000],[23.850000,-17.150000],[-23.850000,-17.150000],[-23.850000,17.150000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[-13.650000,17.150000],[-13.650000,13.000000],[-13.573880,12.617316],[-13.357107,12.292893],[-13.032684,12.076120],[-12.650000,12.000000],[12.650000,12.000000],[13.032684,12.076120],[13.357107,12.292893],[13.573880,12.617316],[13.650000,13.000000],[13.650000,17.150000],[23.850000,17.150000],[23.850000,-17.150000],[-23.850000,-17.150000],[-23.850000,17.150000]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]]);}
module part_C11(part=true,hole=false,block=false)
{
translate([-14.850000,4.850000,0.800000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_U2(part=true,hole=false,block=false)
{
translate([-0.050000,4.250000,0.800000])m1(part,hole,block,casetop); // U2 (back)
};
module part_D5(part=true,hole=false,block=false)
{
translate([-14.850000,-0.010000,0.800000])rotate([0,0,-45.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_PCB1(part=true,hole=false,block=false)
{
};
module part_C8(part=true,hole=false,block=false)
{
translate([-14.850000,-15.750000,0.800000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C3(part=true,hole=false,block=false)
{
translate([10.650000,-11.850000,0.800000])m3(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_R5(part=true,hole=false,block=false)
{
translate([8.850000,-7.950000,0.800000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_J5(part=true,hole=false,block=false)
{
translate([-16.600000,-6.870000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_U7(part=true,hole=false,block=false)
{
translate([10.950000,7.250000,0.800000])rotate([0,0,90.000000])m6(part,hole,block,casetop); // U7 (back)
};
module part_D12(part=true,hole=false,block=false)
{
translate([14.850000,-13.730000,0.800000])rotate([0,0,135.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_C14(part=true,hole=false,block=false)
{
translate([14.850000,-4.750000,0.800000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_J1(part=true,hole=false,block=false)
{
translate([-10.000000,-9.882500,0.800000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_V4(part=true,hole=false,block=false)
{
};
module part_C6(part=true,hole=false,block=false)
{
translate([9.150000,1.750000,0.800000])rotate([0,0,-90.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C5(part=true,hole=false,block=false)
{
translate([-9.950000,1.550000,0.800000])rotate([0,0,-90.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_J9(part=true,hole=false,block=false)
{
translate([16.600000,13.710000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_C15(part=true,hole=false,block=false)
{
translate([14.850000,2.250000,0.800000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R2(part=true,hole=false,block=false)
{
translate([10.650000,-7.950000,0.800000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_J11(part=true,hole=false,block=false)
{
translate([16.600000,-0.010000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_C9(part=true,hole=false,block=false)
{
translate([-14.850000,-8.850000,0.800000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_V3(part=true,hole=false,block=false)
{
};
module part_U6(part=true,hole=false,block=false)
{
translate([10.950000,-2.950000,0.800000])rotate([0,0,90.000000])m6(part,hole,block,casetop); // U7 (back)
};
module part_J10(part=true,hole=false,block=false)
{
translate([16.600000,6.850000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_C4(part=true,hole=false,block=false)
{
translate([-9.050000,1.550000,0.800000])rotate([0,0,-90.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R7(part=true,hole=false,block=false)
{
translate([-10.850000,1.550000,0.800000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_C12(part=true,hole=false,block=false)
{
translate([-14.850000,11.650000,0.800000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_D4(part=true,hole=false,block=false)
{
translate([-14.850000,-6.870000,0.800000])rotate([0,0,-45.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_C10(part=true,hole=false,block=false)
{
translate([-14.850000,-2.050000,0.800000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_J13(part=true,hole=false,block=false)
{
translate([16.600000,-13.730000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_D2(part=true,hole=false,block=false)
{
translate([-5.650000,-13.650000,0.800000])rotate([0,0,135.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_U1(part=true,hole=false,block=false)
{
translate([10.650000,-9.850000,0.800000])m7(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
};
module part_J2(part=true,hole=false,block=false)
{
translate([-0.050000,-11.410000,0.800000])translate([0.000000,-2.400000,0.000000])rotate([90.000000,-0.000000,-0.000000])m8(part,hole,block,casetop); // RevK:USB-C-Socket-H CSP-USC16-TR (back)
};
module part_J6(part=true,hole=false,block=false)
{
translate([-16.600000,-0.010000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_C1(part=true,hole=false,block=false)
{
translate([8.150000,-11.150000,0.800000])rotate([0,0,90.000000])m3(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_J4(part=true,hole=false,block=false)
{
translate([-16.600000,-13.730000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_C18(part=true,hole=false,block=false)
{
translate([6.050000,-6.250000,0.800000])rotate([0,0,-90.000000])m9(part,hole,block,casetop); // RevK:C_1206 C_1206_3216Metric (back)
};
module part_J7(part=true,hole=false,block=false)
{
translate([-16.600000,6.850000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_J12(part=true,hole=false,block=false)
{
translate([16.600000,-6.870000,0.800000])rotate([0,0,90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_R6(part=true,hole=false,block=false)
{
translate([10.150000,1.750000,0.800000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_U4(part=true,hole=false,block=false)
{
translate([-10.750000,-3.440000,0.800000])rotate([0,0,90.000000])m6(part,hole,block,casetop); // U7 (back)
};
module part_V2(part=true,hole=false,block=false)
{
};
module part_C13(part=true,hole=false,block=false)
{
translate([14.850000,15.850000,0.800000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_J8(part=true,hole=false,block=false)
{
translate([-16.600000,13.710000,0.800000])rotate([0,0,-90.000000])m5(part,hole,block,casetop,2); // J5 (back)
};
module part_C17(part=true,hole=false,block=false)
{
translate([14.850000,-11.650000,0.800000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R1(part=true,hole=false,block=false)
{
translate([5.850000,-15.750000,0.800000])rotate([0,0,-90.000000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_C7(part=true,hole=false,block=false)
{
translate([-5.650000,-12.250000,0.800000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_D11(part=true,hole=false,block=false)
{
translate([14.850000,-6.870000,0.800000])rotate([0,0,135.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_D6(part=true,hole=false,block=false)
{
translate([-14.850000,6.850000,0.800000])rotate([0,0,-45.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_D9(part=true,hole=false,block=false)
{
translate([14.850000,6.850000,0.800000])rotate([0,0,135.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_D3(part=true,hole=false,block=false)
{
translate([-14.850000,-13.730000,0.800000])rotate([0,0,-45.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_L1(part=true,hole=false,block=false)
{
translate([10.650000,-14.450000,0.800000])rotate([-0.000000,-0.000000,-90.000000])m10(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
};
module part_R4(part=true,hole=false,block=false)
{
translate([12.450000,-7.950000,0.800000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_D8(part=true,hole=false,block=false)
{
translate([14.850000,13.710000,0.800000])rotate([0,0,135.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_D7(part=true,hole=false,block=false)
{
translate([-14.850000,13.710000,0.800000])rotate([0,0,-45.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_D10(part=true,hole=false,block=false)
{
translate([14.850000,-0.010000,0.800000])rotate([0,0,135.000000])m2(part,hole,block,casetop); // D5 (back)
};
module part_U5(part=true,hole=false,block=false)
{
translate([-0.050000,-6.550000,0.800000])rotate([0,0,180.000000])m6(part,hole,block,casetop); // U7 (back)
};
module part_V1(part=true,hole=false,block=false)
{
};
module part_C16(part=true,hole=false,block=false)
{
translate([14.850000,8.950000,0.800000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R3(part=true,hole=false,block=false)
{
translate([6.750000,-15.750000,0.800000])rotate([0,0,90.000000])m4(part,hole,block,casetop); // RevK:R_0402_ R_0402_1005Metric (back)
};
module part_C2(part=true,hole=false,block=false)
{
translate([13.150000,-11.150000,0.800000])rotate([0,0,90.000000])m3(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_D13(part=true,hole=false,block=false)
{
translate([-11.850000,1.565000,0.800000])rotate([0,0,-90.000000])m11(part,hole,block,casetop); // D13 (back)
};
module part_U3(part=true,hole=false,block=false)
{
translate([-10.750000,7.250000,0.800000])rotate([0,0,90.000000])m6(part,hole,block,casetop); // U7 (back)
};
module part_D1(part=true,hole=false,block=false)
{
translate([6.250000,-11.150000,0.800000])rotate([0,0,-90.000000])m12(part,hole,block,casetop); // D1 (back)
};
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
part_C11(part,hole,block);
part_U2(part,hole,block);
part_D5(part,hole,block);
part_PCB1(part,hole,block);
part_C8(part,hole,block);
part_C3(part,hole,block);
part_R5(part,hole,block);
part_J5(part,hole,block);
part_U7(part,hole,block);
part_D12(part,hole,block);
part_C14(part,hole,block);
part_J1(part,hole,block);
part_V4(part,hole,block);
part_C6(part,hole,block);
part_C5(part,hole,block);
part_J9(part,hole,block);
part_C15(part,hole,block);
part_R2(part,hole,block);
part_J11(part,hole,block);
part_C9(part,hole,block);
part_V3(part,hole,block);
part_U6(part,hole,block);
part_J10(part,hole,block);
part_C4(part,hole,block);
part_R7(part,hole,block);
part_C12(part,hole,block);
part_D4(part,hole,block);
part_C10(part,hole,block);
part_J13(part,hole,block);
part_D2(part,hole,block);
part_U1(part,hole,block);
part_J2(part,hole,block);
part_J6(part,hole,block);
part_C1(part,hole,block);
part_J4(part,hole,block);
part_C18(part,hole,block);
part_J7(part,hole,block);
part_J12(part,hole,block);
part_R6(part,hole,block);
part_U4(part,hole,block);
part_V2(part,hole,block);
part_C13(part,hole,block);
part_J8(part,hole,block);
part_C17(part,hole,block);
part_R1(part,hole,block);
part_C7(part,hole,block);
part_D11(part,hole,block);
part_D6(part,hole,block);
part_D9(part,hole,block);
part_D3(part,hole,block);
part_L1(part,hole,block);
part_R4(part,hole,block);
part_D8(part,hole,block);
part_D7(part,hole,block);
part_D10(part,hole,block);
part_U5(part,hole,block);
part_V1(part,hole,block);
part_C16(part,hole,block);
part_R3(part,hole,block);
part_C2(part,hole,block);
part_D13(part,hole,block);
part_U3(part,hole,block);
part_D1(part,hole,block);
}

parts_top=30;
module part_J3(part=true,hole=false,block=false)
{
};
module part_J14(part=true,hole=false,block=false)
{
};
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
part_J3(part,hole,block);
part_J14(part,hole,block);
}

parts_bottom=0;
module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(part=false,hole=false,block=false,height)
{ // RevK:C_0402 C_0402_1005Metric
// 0402 Capacitor
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
}

module m1(part=false,hole=false,block=false,height)
{ // U2
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

module m2(part=false,hole=false,block=false,height)
{ // D5
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
{ // RevK:R_0402_ R_0402_1005Metric
// 0402 Resistor
if(part)
{
	b(0,0,0,1.5,0.65,0.2); // Pad size
	b(0,0,0,1.0,0.5,0.5); // Chip
}
}

module m5(part=false,hole=false,block=false,height,N=0)
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

module m6(part=false,hole=false,block=false,height)
{ // U7
// SO-8_3.9x4.9mm_P1.27mm 
if(part)
{
	b(0,0,0,5.0,4.0,1.75); // Part
	b(0,0,0,6.9,4.1,1); // Pins
}
}

module m7(part=false,hole=false,block=false,height)
{ // RevK:SOT-23-6-MD8942 SOT-23-6
// SOT-23-6
if(part)
{
	b(0,0,0,1.726,3.026,1.2); // Part
	b(0,0,0,3.6,2.5,0.5); // Pins
}
}

module m8(part=false,hole=false,block=false,height)
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

module m9(part=false,hole=false,block=false,height)
{ // RevK:C_1206 C_1206_3216Metric
// 1206 Capacitor
if(part)
{
	b(0,0,0,3.2,1.6,1.5); // Part
	b(0,0,0,4.1,1.8,0.2); // Pads
}
}

module m10(part=false,hole=false,block=false,height)
{ // RevK:L_4x4_ TYA4020
// 4x4 Inductor
if(part)
{
	b(0,0,0,4,4,3);
}
}

module m11(part=false,hole=false,block=false,height)
{ // D13
// 0402 LED
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
if(hole)
{
	hull()
	{
		b(0,0,0,1.001,0.501,1);
		b(0,0,height,2.0,1.5,1);
	}
}
}

module m12(part=false,hole=false,block=false,height)
{ // D1
// SOD-123 Diode
if(part)
{
	b(0,0,0,2.85,1.8,1.35); // part
	b(0,0,0,4.2,1.2,0.7); // pads
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
bottom();
