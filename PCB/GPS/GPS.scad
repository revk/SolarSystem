// Generated case design for PCB/GPS/GPS.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2023-04-04 12:22:23
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

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[36.000000,10.000000],[36.000000,26.000000],[35.951847,26.980171],[35.807853,27.950903],[35.569404,28.902847],[35.238795,29.826834],[34.819213,30.713967],[34.314696,31.555702],[33.730105,32.343933],[33.071068,33.071068],[32.343933,33.730105],[31.555702,34.314696],[30.713967,34.819213],[29.826834,35.238795],[28.902847,35.569404],[27.950903,35.807853],[26.980171,35.951847],[26.000000,36.000000],[10.000000,36.000000],[9.019829,35.951847],[8.049097,35.807853],[7.097153,35.569404],[6.173166,35.238795],[5.286033,34.819213],[4.444298,34.314696],[3.656067,33.730105],[2.928932,33.071068],[2.269895,32.343933],[1.685304,31.555702],[1.180787,30.713967],[0.761205,29.826834],[0.430596,28.902847],[0.192147,27.950903],[0.048153,26.980171],[0.000000,26.000000],[0.000000,10.000000],[0.048153,9.019828],[0.192147,8.049096],[0.430596,7.097153],[0.761204,6.173165],[1.180786,5.286031],[1.685303,4.444296],[2.269894,3.656065],[2.928931,2.928930],[3.656066,2.269893],[4.444297,1.685302],[5.286032,1.180785],[6.173165,0.761202],[7.097153,0.430594],[8.049096,0.192145],[9.019828,0.048151],[10.000000,0.000000],[26.000000,0.000000],[26.980171,0.048153],[27.950903,0.192147],[28.902847,0.430596],[29.826834,0.761205],[30.713967,1.180787],[31.555702,1.685304],[32.343933,2.269895],[33.071068,2.928932],[33.730105,3.656067],[34.314696,4.444298],[34.819213,5.286033],[35.238795,6.173166],[35.569404,7.097153],[35.807853,8.049097],[35.951847,9.019829]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[36.000000,10.000000],[36.000000,26.000000],[35.951847,26.980171],[35.807853,27.950903],[35.569404,28.902847],[35.238795,29.826834],[34.819213,30.713967],[34.314696,31.555702],[33.730105,32.343933],[33.071068,33.071068],[32.343933,33.730105],[31.555702,34.314696],[30.713967,34.819213],[29.826834,35.238795],[28.902847,35.569404],[27.950903,35.807853],[26.980171,35.951847],[26.000000,36.000000],[10.000000,36.000000],[9.019829,35.951847],[8.049097,35.807853],[7.097153,35.569404],[6.173166,35.238795],[5.286033,34.819213],[4.444298,34.314696],[3.656067,33.730105],[2.928932,33.071068],[2.269895,32.343933],[1.685304,31.555702],[1.180787,30.713967],[0.761205,29.826834],[0.430596,28.902847],[0.192147,27.950903],[0.048153,26.980171],[0.000000,26.000000],[0.000000,10.000000],[0.048153,9.019828],[0.192147,8.049096],[0.430596,7.097153],[0.761204,6.173165],[1.180786,5.286031],[1.685303,4.444296],[2.269894,3.656065],[2.928931,2.928930],[3.656066,2.269893],[4.444297,1.685302],[5.286032,1.180785],[6.173165,0.761202],[7.097153,0.430594],[8.049096,0.192145],[9.019828,0.048151],[10.000000,0.000000],[26.000000,0.000000],[26.980171,0.048153],[27.950903,0.192147],[28.902847,0.430596],[29.826834,0.761205],[30.713967,1.180787],[31.555702,1.685304],[32.343933,2.269895],[33.071068,2.928932],[33.730105,3.656067],[34.314696,4.444298],[34.819213,5.286033],[35.238795,6.173166],[35.569404,7.097153],[35.807853,8.049097],[35.951847,9.019829]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67]]);}
spacing=52.000000;
pcbwidth=36.000000;
pcblength=36.000000;
// Populated PCB
module board(pushed=false,hulled=false){
translate([18.000000,18.000000,0.800000])rotate([0,0,-90.000000])m0(pushed,hulled); // RevK:L86-M33 L86-M33 (back)
translate([18.000000,21.000000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])rotate([-0.000000,-0.000000,-180.000000])m5(pushed,hulled); // RevK:Battery-Holder-2032 BatteryHolder_Keystone_3034_1x20mm
translate([18.000000,8.130000,0.000000])rotate([0,0,180.000000])rotate([180,0,0])m7(pushed,hulled,5); // RevK:PTSM-HH-5-RA-P-SMD PTSM-HH-5-RA-P-SMD
}

module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(pushed=false,hulled=false)
{ // RevK:L86-M33 L86-M33
rotate([90,0,0])b(0,0,0,18.4,18.4,6.95);
}

module m5(pushed=false,hulled=false)
{ // RevK:Battery-Holder-2032 BatteryHolder_Keystone_3034_1x20mm
b(0,-0.2,0,22.9,15.5,4.2);
cylinder(d=20,h=3.2);
}

module m7(pushed=false,hulled=false,n=0)
{ // RevK:PTSM-HH-5-RA-P-SMD PTSM-HH-5-RA-P-SMD
// Plug on PCB
b(0,-7.5/2-0.4,0,0.5+n*2.5,7.5,5);
b(0,0,0,0.4+2.5*(n-1),3.2,0.4); // pins
b(0,-5.85/2-0.4,0,9.18+n*2.5,5.6,0.4); // tab
b(0,-5.85/2-0.4,0,3.2+n*2.5,5.6,4); // side
hull()
{ // Socket
	b(0,-7.5/2-7.5-0.4,0,1.7+n*2.5,7.5,4);
	b(0,-7.5/2-7.5-0.4,0,1.7+n*2.5-2,7.5,5);
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
