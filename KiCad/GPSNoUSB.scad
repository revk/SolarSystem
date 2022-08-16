// Generated case design for KiCad/GPS.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2022-08-16 09:25:59
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
casebase=5.000000;
casetop=5.600000;
casewall=3.000000;
fit=0.000000;
edge=1.000000;
pcbthickness=0.800000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[23.500000,34.000000],[26.000000,34.000000],[26.980171,34.048153],[27.950903,34.192147],[28.902847,34.430596],[29.826834,34.761205],[30.713967,35.180787],[31.555702,35.685304],[32.343933,36.269895],[33.071068,36.928932],[33.730105,37.656067],[34.314696,38.444298],[34.819213,39.286033],[35.238795,40.173166],[35.569404,41.097153],[35.807853,42.049097],[35.951847,43.019829],[36.000000,44.000000],[36.000000,60.000000],[35.951847,60.980171],[35.807853,61.950903],[35.569404,62.902847],[35.238795,63.826834],[34.819213,64.713967],[34.314696,65.555702],[33.730105,66.343933],[33.071068,67.071068],[32.343933,67.730105],[31.555702,68.314696],[30.713967,68.819213],[29.826834,69.238795],[28.902847,69.569404],[27.950903,69.807853],[26.980171,69.951847],[26.000000,70.000000],[10.000000,70.000000],[9.019829,69.951847],[8.049097,69.807853],[7.097153,69.569404],[6.173166,69.238795],[5.286033,68.819213],[4.444298,68.314696],[3.656067,67.730105],[2.928932,67.071068],[2.269895,66.343933],[1.685304,65.555702],[1.180787,64.713967],[0.761205,63.826834],[0.430596,62.902847],[0.192147,61.950903],[0.048153,60.980171],[0.000000,60.000000],[0.000000,44.000000],[0.048153,43.019828],[0.192147,42.049096],[0.430596,41.097153],[0.761204,40.173165],[1.180786,39.286031],[1.685303,38.444296],[2.269894,37.656065],[2.928931,36.928930],[3.656066,36.269893],[4.444297,35.685302],[5.286032,35.180785],[6.173165,34.761202],[7.097153,34.430594],[8.049096,34.192145],[9.019828,34.048151],[10.000000,34.000000],[12.500000,34.000000],[12.969303,33.963065],[13.427051,33.853169],[13.861971,33.673019],[14.263355,33.427051],[14.621320,33.121320],[14.927051,32.763355],[15.173019,32.361971],[15.353169,31.927051],[15.463065,31.469303],[15.500000,31.000000],[15.500000,26.000000],[15.463065,25.530697],[15.353169,25.072949],[15.173019,24.638029],[14.927051,24.236645],[14.621320,23.878680],[14.263355,23.572949],[13.861971,23.326981],[13.427051,23.146831],[12.969303,23.036935],[12.500000,23.000000],[6.000000,23.000000],[5.414729,22.942356],[4.851950,22.771638],[4.333290,22.494408],[3.878680,22.121320],[3.505592,21.666710],[3.228362,21.148050],[3.057644,20.585271],[3.000000,20.000000],[3.000000,0.000000],[33.000000,0.000000],[33.000000,20.000000],[32.942356,20.585271],[32.771638,21.148050],[32.494408,21.666710],[32.121320,22.121320],[31.666710,22.494408],[31.148050,22.771638],[30.585271,22.942356],[30.000000,23.000000],[23.500000,23.000000],[23.030697,23.036935],[22.572949,23.146831],[22.138029,23.326981],[21.736645,23.572949],[21.378680,23.878680],[21.072949,24.236645],[20.826981,24.638029],[20.646831,25.072949],[20.536935,25.530697],[20.500000,26.000000],[20.500000,31.000000],[20.536935,31.469303],[20.646831,31.927051],[20.826981,32.361971],[21.072949,32.763355],[21.378680,33.121320],[21.736645,33.427051],[22.138029,33.673019],[22.572949,33.853169],[23.030697,33.963065]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131]]);}

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[23.500000,34.000000],[26.000000,34.000000],[26.980171,34.048153],[27.950903,34.192147],[28.902847,34.430596],[29.826834,34.761205],[30.713967,35.180787],[31.555702,35.685304],[32.343933,36.269895],[33.071068,36.928932],[33.730105,37.656067],[34.314696,38.444298],[34.819213,39.286033],[35.238795,40.173166],[35.569404,41.097153],[35.807853,42.049097],[35.951847,43.019829],[36.000000,44.000000],[36.000000,60.000000],[35.951847,60.980171],[35.807853,61.950903],[35.569404,62.902847],[35.238795,63.826834],[34.819213,64.713967],[34.314696,65.555702],[33.730105,66.343933],[33.071068,67.071068],[32.343933,67.730105],[31.555702,68.314696],[30.713967,68.819213],[29.826834,69.238795],[28.902847,69.569404],[27.950903,69.807853],[26.980171,69.951847],[26.000000,70.000000],[10.000000,70.000000],[9.019829,69.951847],[8.049097,69.807853],[7.097153,69.569404],[6.173166,69.238795],[5.286033,68.819213],[4.444298,68.314696],[3.656067,67.730105],[2.928932,67.071068],[2.269895,66.343933],[1.685304,65.555702],[1.180787,64.713967],[0.761205,63.826834],[0.430596,62.902847],[0.192147,61.950903],[0.048153,60.980171],[0.000000,60.000000],[0.000000,44.000000],[0.048153,43.019828],[0.192147,42.049096],[0.430596,41.097153],[0.761204,40.173165],[1.180786,39.286031],[1.685303,38.444296],[2.269894,37.656065],[2.928931,36.928930],[3.656066,36.269893],[4.444297,35.685302],[5.286032,35.180785],[6.173165,34.761202],[7.097153,34.430594],[8.049096,34.192145],[9.019828,34.048151],[10.000000,34.000000],[12.500000,34.000000],[12.969303,33.963065],[13.427051,33.853169],[13.861971,33.673019],[14.263355,33.427051],[14.621320,33.121320],[14.927051,32.763355],[15.173019,32.361971],[15.353169,31.927051],[15.463065,31.469303],[15.500000,31.000000],[15.500000,26.000000],[15.463065,25.530697],[15.353169,25.072949],[15.173019,24.638029],[14.927051,24.236645],[14.621320,23.878680],[14.263355,23.572949],[13.861971,23.326981],[13.427051,23.146831],[12.969303,23.036935],[12.500000,23.000000],[6.000000,23.000000],[5.414729,22.942356],[4.851950,22.771638],[4.333290,22.494408],[3.878680,22.121320],[3.505592,21.666710],[3.228362,21.148050],[3.057644,20.585271],[3.000000,20.000000],[3.000000,0.000000],[33.000000,0.000000],[33.000000,20.000000],[32.942356,20.585271],[32.771638,21.148050],[32.494408,21.666710],[32.121320,22.121320],[31.666710,22.494408],[31.148050,22.771638],[30.585271,22.942356],[30.000000,23.000000],[23.500000,23.000000],[23.030697,23.036935],[22.572949,23.146831],[22.138029,23.326981],[21.736645,23.572949],[21.378680,23.878680],[21.072949,24.236645],[20.826981,24.638029],[20.646831,25.072949],[20.536935,25.530697],[20.500000,26.000000],[20.500000,31.000000],[20.536935,31.469303],[20.646831,31.927051],[20.826981,32.361971],[21.072949,32.763355],[21.378680,33.121320],[21.736645,33.427051],[22.138029,33.673019],[22.572949,33.853169],[23.030697,33.963065]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131]]);}
spacing=52.000000;
pcbwidth=36.000000;
pcblength=70.000000;
// Populated PCB
module board(pushed=false,hulled=false){
// Missing Connector_PinHeader_1.27mm:PinHeader_1x05_P1.27mm_Vertical PinHeader_1x05_P1.27mm_Vertical
// Missing Connector_PinHeader_1.27mm:PinHeader_1x05_P1.27mm_Vertical PinHeader_1x05_P1.27mm_Vertical
translate([18.000000,52.000000,0.800000])rotate([0,0,-90.000000])rotate([-90.000000,0.000000,0.000000])m1(pushed,hulled); // RevK:L86-M33 L86-M33
translate([19.000000,16.500000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m2(pushed,hulled); // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
translate([15.500000,18.325000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([23.500000,17.250000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([5.750000,20.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])translate([0.000000,-0.400000,0.000000])m4(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([5.750000,20.000000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])translate([0.000000,0.400000,0.000000])m4(pushed,hulled); // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
translate([23.500000,14.250000,0.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([18.630000,5.580000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:D_1206_ D_1206_3216Metric
translate([23.500000,16.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([9.200000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([9.750000,5.693250,0.000000])rotate([180,0,0])translate([0.000000,2.700000,0.000000])rotate([-90.000000,0.000000,0.000000])m6(pushed,hulled); // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
translate([29.000000,9.500000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m5(pushed,hulled); // RevK:D_1206_ D_1206_3216Metric
translate([20.000000,13.000000,0.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([7.800000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([21.800000,7.200000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])rotate([0.000000,0.000000,90.000000])m7(pushed,hulled); // RevK:L_4x4_ TYA4020
translate([21.800000,3.300000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])rotate([0.000000,0.000000,90.000000])m8(pushed,hulled); // RevK:SOT-23-Thin-6-Reg SOT-23-6
translate([21.550000,1.200000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m9(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([24.500000,1.500000,0.000000])rotate([0,0,90.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([18.250000,2.450000,0.000000])rotate([180,0,0])m10(pushed,hulled); // RevK:C_0805_ C_0805_2012Metric
translate([18.250000,8.700000,0.000000])rotate([180,0,0])m10(pushed,hulled); // RevK:C_0805_ C_0805_2012Metric
translate([20.000000,11.500000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([10.600000,20.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([18.000000,55.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])rotate([0.000000,0.000000,180.000000])m11(pushed,hulled); // RevK:Battery-Holder-2032 BatteryHolder_Keystone_3034_1x20mm
translate([19.000000,20.000000,0.000000])rotate([180,0,0])m9(pushed,hulled); // RevK:C_0603_ C_0603_1608Metric
translate([23.500000,19.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
translate([29.000000,6.650000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])translate([0.000000,-3.600000,2.500000])rotate([0.000000,0.000000,180.000000])m12(pushed,hulled); // RevK:Molex_MiniSPOX_H2RA 22057025
translate([24.500000,9.000000,0.000000])rotate([0,0,-90.000000])rotate([180,0,0])m3(pushed,hulled); // RevK:R_0603 R_0603_1608Metric
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m1(pushed=false,hulled=false)
{ // RevK:L86-M33 L86-M33
rotate([90,0,0])b(0,0,0,18.4,18.4,6.95);
}

module m2(pushed=false,hulled=false)
{ // RevK:QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm QFN-20-1EP_4x4mm_P0.5mm_EP2.5x2.5mm
cube([4,4,1],center=true);
}

module m3(pushed=false,hulled=false)
{ // RevK:R_0603 R_0603_1608Metric
b(0,0,0,2.8,0.95,0.5); // Pad size
}

module m4(pushed=false,hulled=false)
{ // RevK:LED-RGB-1.6x1.6 LED_0603_1608Metric
b(0,0,0,1.6,0.8,0.25);
b(0,0,0,1.2,0.8,0.55);
b(0,0,0,0.8,0.8,0.95);
if(!hulled&&pushed)b(0,0,0,1,1,20);
}

module m5(pushed=false,hulled=false)
{ // RevK:D_1206_ D_1206_3216Metric
b(0,0,0,4.4,1.75,1.2); // Pad size
}

module m6(pushed=false,hulled=false)
{ // RevK:ESP32-PICO-MINI-02 ESP32-PICO-MINI-02
rotate([90,0,0])
translate([-13.2/2,-16.6/2,0])
{
	if(!hulled)cube([13.2,16.6,0.8]);
	cube([13.2,11.2,2.4]);
}
}

module m7(pushed=false,hulled=false)
{ // RevK:L_4x4_ TYA4020
b(0,0,0,4,4,2.1);
}

module m8(pushed=false,hulled=false)
{ // RevK:SOT-23-Thin-6-Reg SOT-23-6
b(0,0,0,3.05,3.05,0.5);
b(0,0,0,1.45,3.05,1.1);
}

module m9(pushed=false,hulled=false)
{ // RevK:C_0603_ C_0603_1608Metric
b(0,0,0,1.6,0.95,0.2); // Pad size
b(0,0,0,1.6,0.8,1); // Chip
}

module m10(pushed=false,hulled=false)
{ // RevK:C_0805_ C_0805_2012Metric
b(0,0,0,2,1.45,0.2); // Pad size
b(0,0,0,2,1.2,1); // Chip
}

module m11(pushed=false,hulled=false)
{ // RevK:Battery-Holder-2032 BatteryHolder_Keystone_3034_1x20mm
b(0,-0.2,0,22.9,15.5,4.2);
cylinder(d=20,h=3.2);
}

module m12(pushed=false,hulled=false)
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
