// Generated case design for AccessWAGO/Access.kicad_pcb
// By https://github.com/revk/PCBCase
// Generated 2025-08-10 09:15:23
// title:	Access Control
// rev:	3
// company:	Adrian Kennard Andrews & Arnold Ltd
// comment:	toot.me.uk/@RevK
// comment:	www.me.uk
//

// Globals
margin=0.200000;
lip=3.000000;
casebottom=2.000000;
casetop=6.000000;
casewall=3.000000;
fit=0.000000;
edge=2.000000;
pcbthickness=1.200000;
nohull=false;
hullcap=1.000000;
hulledge=1.000000;
useredge=false;
spacing=76.000000;
pcbwidth=60.000000;
pcblength=30.500000;
originx=102.500000;
originy=92.750000;

module outline(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[7.200000,-15.250000],[29.000000,-15.250000],[29.382684,-15.173880],[29.707107,-14.957107],[29.923880,-14.632684],[30.000000,-14.250000],[30.000000,15.250000],[-30.000000,15.250000],[-30.000000,-14.250000],[-29.923880,-14.632684],[-29.707107,-14.957107],[-29.382684,-15.173880],[-29.000000,-15.250000],[-12.200000,-15.250000],[-12.177164,-15.135195],[-12.112132,-15.037868],[-12.014805,-14.972836],[-11.900000,-14.950000],[6.900000,-14.950000],[7.014805,-14.972836],[7.112132,-15.037868],[7.177164,-15.135195]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21]]);}

module pcb(h=pcbthickness,r=0){linear_extrude(height=h)offset(r=r)polygon(points=[[7.200000,-15.250000],[29.000000,-15.250000],[29.382684,-15.173880],[29.707107,-14.957107],[29.923880,-14.632684],[30.000000,-14.250000],[30.000000,15.250000],[-30.000000,15.250000],[-30.000000,-14.250000],[-29.923880,-14.632684],[-29.707107,-14.957107],[-29.382684,-15.173880],[-29.000000,-15.250000],[-12.200000,-15.250000],[-12.177164,-15.135195],[-12.112132,-15.037868],[-12.014805,-14.972836],[-11.900000,-14.950000],[6.900000,-14.950000],[7.014805,-14.972836],[7.112132,-15.037868],[7.177164,-15.135195]],paths=[[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21]]);}
module part_J5(part=true,hole=false,block=false)
{
translate([22.000000,-11.250000,1.200000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // J5 (back)
};
module part_J6(part=true,hole=false,block=false)
{
translate([23.100000,7.250000,1.200000])rotate([0,0,-90.000000])m1(part,hole,block,casetop); // J6 (back)
};
module part_J2(part=true,hole=false,block=false)
{
translate([22.000000,-3.150000,1.200000])rotate([0,0,180.000000])m0(part,hole,block,casetop); // J5 (back)
};
module part_J3(part=true,hole=false,block=false)
{
translate([-11.900000,7.250000,1.200000])rotate([0,0,-90.000000])m1(part,hole,block,casetop); // J6 (back)
};
module part_C12(part=true,hole=false,block=false)
{
translate([-19.150000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_D16(part=true,hole=false,block=false)
{
translate([-22.899000,-0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_SW1(part=true,hole=false,block=false)
{
translate([-13.100000,-9.050000,1.200000])rotate([0,0,90.000000])translate([2.050000,0.000000,1.150000])rotate([-0.000000,-0.000000,-90.000000])m4(part,hole,block,casetop); // RevK:ESE13 ESE13V01D (back)
};
module part_V2(part=true,hole=false,block=false)
{
};
module part_R10(part=true,hole=false,block=false)
{
translate([-20.900000,-2.350000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C3(part=true,hole=false,block=false)
{
translate([-16.800000,-2.350000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R25(part=true,hole=false,block=false)
{
translate([14.600000,-5.000000,1.200000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C22(part=true,hole=false,block=false)
{
translate([-28.443750,-11.550000,1.200000])rotate([0,0,-90.000000])m6(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_R26(part=true,hole=false,block=false)
{
translate([-10.900000,-12.050000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C5(part=true,hole=false,block=false)
{
translate([-21.800000,-2.350000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_D10(part=true,hole=false,block=false)
{
translate([-11.650000,-0.150000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_D8(part=true,hole=false,block=false)
{
translate([-27.600000,-2.300000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_L4(part=true,hole=false,block=false)
{
translate([-25.943750,-8.250000,1.200000])rotate([0,0,180.000000])scale([1.000000,1.000000,1.400000])rotate([0.000000,0.000000,-90.000000])m8(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
};
module part_C17(part=true,hole=false,block=false)
{
translate([-22.300000,-7.150000,1.200000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_J1(part=true,hole=false,block=false)
{
translate([10.600000,-9.095000,1.200000])translate([0.000000,-2.400000,0.000000])rotate([90.000000,-0.000000,-0.000000])m9(part,hole,block,casetop); // RevK:USB-C-Socket-H CSP-USC16-TR (back)
};
module part_Q2(part=true,hole=false,block=false)
{
translate([-11.900000,-2.650000,1.200000])rotate([0,0,180.000000])m10(part,hole,block,casetop); // Q2 (back)
};
module part_C7(part=true,hole=false,block=false)
{
translate([-26.800000,-2.350000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R2(part=true,hole=false,block=false)
{
translate([-16.950000,-6.700000,1.200000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_U1(part=true,hole=false,block=false)
{
translate([-18.750000,-8.600000,1.200000])m11(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
};
module part_R8(part=true,hole=false,block=false)
{
translate([-15.900000,-2.350000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C26(part=true,hole=false,block=false)
{
translate([14.700000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C25(part=true,hole=false,block=false)
{
translate([-29.150000,-0.300000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_D1(part=true,hole=false,block=false)
{
translate([-15.000000,-2.275000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_D17(part=true,hole=false,block=false)
{
translate([-25.399000,-0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_D4(part=true,hole=false,block=false)
{
translate([-22.600000,-2.275000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_C15(part=true,hole=false,block=false)
{
translate([-16.250000,-9.900000,1.200000])rotate([0,0,90.000000])m6(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_C19(part=true,hole=false,block=false)
{
translate([-29.300000,-5.350000,1.200000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_J9(part=true,hole=false,block=false)
{
translate([-24.000000,7.250000,1.200000])rotate([0,0,-90.000000])m1(part,hole,block,casetop); // J6 (back)
};
module part_C16(part=true,hole=false,block=false)
{
translate([-18.750000,-10.600000,1.200000])m6(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_D6(part=true,hole=false,block=false)
{
translate([-25.100000,-2.250000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_C14(part=true,hole=false,block=false)
{
translate([-24.150000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C21(part=true,hole=false,block=false)
{
translate([-23.443750,-11.350000,1.200000])rotate([0,0,-90.000000])m6(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_J7(part=true,hole=false,block=false)
{
translate([10.100000,7.250000,1.200000])rotate([0,0,-90.000000])m1(part,hole,block,casetop); // J6 (back)
};
module part_R15(part=true,hole=false,block=false)
{
translate([-27.743750,-14.750000,1.200000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_R9(part=true,hole=false,block=false)
{
translate([-18.400000,-2.350000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C24(part=true,hole=false,block=false)
{
translate([-26.650000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R27(part=true,hole=false,block=false)
{
translate([-24.143750,-14.750000,1.200000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_D15(part=true,hole=false,block=false)
{
translate([-20.399000,-0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_C2(part=true,hole=false,block=false)
{
translate([-12.450000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C11(part=true,hole=false,block=false)
{
translate([-16.650000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C28(part=true,hole=false,block=false)
{
translate([-14.100000,-13.550000,1.200000])rotate([0,0,180.000000])m12(part,hole,block,casetop); // RevK:C_1210 C_1210_3225Metric (back)
};
module part_C27(part=true,hole=false,block=false)
{
translate([6.500000,-5.275000,1.200000])rotate([0,0,-90.000000])m13(part,hole,block,casetop); // RevK:C_1206 C_1206_3216Metric (back)
};
module part_C23(part=true,hole=false,block=false)
{
translate([-25.943750,-10.850000,1.200000])rotate([0,0,180.000000])m6(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_R5(part=true,hole=false,block=false)
{
translate([5.600000,-11.650000,1.200000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_D9(part=true,hole=false,block=false)
{
translate([-14.050000,-1.025000,1.200000])rotate([0,0,90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_R12(part=true,hole=false,block=false)
{
translate([-25.900000,-2.350000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_R1(part=true,hole=false,block=false)
{
translate([-18.750000,-6.700000,1.200000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_D3(part=true,hole=false,block=false)
{
translate([-20.100000,-2.275000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_C20(part=true,hole=false,block=false)
{
translate([-10.900000,-13.850000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_PCB1(part=true,hole=false,block=false)
{
};
module part_D18(part=true,hole=false,block=false)
{
translate([-27.899000,-0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_D19(part=true,hole=false,block=false)
{
translate([12.300000,-1.850000,1.200000])m14(part,hole,block,casetop); // D19 (back)
};
module part_R4(part=true,hole=false,block=false)
{
translate([5.600000,-9.950000,1.200000])rotate([0,0,90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C8(part=true,hole=false,block=false)
{
translate([-29.300000,-2.350000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R6(part=true,hole=false,block=false)
{
translate([-13.250000,-0.850000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_D11(part=true,hole=false,block=false)
{
translate([8.000000,0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_D5(part=true,hole=false,block=false)
{
translate([7.900000,-1.850000,1.200000])rotate([0,0,180.000000])m14(part,hole,block,casetop); // D19 (back)
};
module part_U2(part=true,hole=false,block=false)
{
translate([-25.943750,-12.850000,1.200000])rotate([0,0,180.000000])m11(part,hole,block,casetop); // RevK:SOT-23-6-MD8942 SOT-23-6 (back)
};
module part_J8(part=true,hole=false,block=false)
{
translate([-0.900000,7.250000,1.200000])rotate([0,0,-90.000000])m0(part,hole,block,casetop); // J5 (back)
};
module part_C6(part=true,hole=false,block=false)
{
translate([-24.300000,-2.350000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R7(part=true,hole=false,block=false)
{
translate([-10.850000,-0.300000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_U10(part=true,hole=false,block=false)
{
translate([-2.500000,-7.250000,1.200000])rotate([0,0,180.000000])m15(part,hole,block,casetop); // U10 (back)
};
module part_D14(part=true,hole=false,block=false)
{
translate([-17.899000,-0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_R11(part=true,hole=false,block=false)
{
translate([-23.400000,-2.350000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C13(part=true,hole=false,block=false)
{
translate([-21.650000,-0.250000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_U8(part=true,hole=false,block=false)
{
translate([11.300000,-5.000000,1.200000])rotate([0,0,90.000000])m16(part,hole,block,casetop); // U8 (back)
};
module part_D7(part=true,hole=false,block=false)
{
translate([12.000000,0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_D2(part=true,hole=false,block=false)
{
translate([-17.600000,-2.275000,1.200000])rotate([0,0,-90.000000])m7(part,hole,block,casetop); // D10 (back)
};
module part_D13(part=true,hole=false,block=false)
{
translate([-15.399000,-0.250000,1.200000])rotate([0,0,45.000000])m3(part,hole,block,casetop); // D16 (back)
};
module part_V1(part=true,hole=false,block=false)
{
};
module part_L3(part=true,hole=false,block=false)
{
translate([-18.750000,-13.200000,1.200000])scale([1.000000,1.000000,1.400000])rotate([0.000000,0.000000,-90.000000])m8(part,hole,block,casetop); // RevK:L_4x4_ TYA4020 (back)
};
module part_C1(part=true,hole=false,block=false)
{
translate([-21.250000,-9.900000,1.200000])rotate([0,0,90.000000])m6(part,hole,block,casetop); // RevK:C_0603_ C_0603_1608Metric (back)
};
module part_R13(part=true,hole=false,block=false)
{
translate([-28.400000,-2.350000,1.200000])rotate([0,0,-90.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
module part_C18(part=true,hole=false,block=false)
{
translate([-28.400000,-5.350000,1.200000])rotate([0,0,90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C9(part=true,hole=false,block=false)
{
translate([6.100000,-0.450000,1.200000])rotate([0,0,180.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_C4(part=true,hole=false,block=false)
{
translate([-19.300000,-2.350000,1.200000])rotate([0,0,-90.000000])m2(part,hole,block,casetop); // RevK:C_0402 C_0402_1005Metric (back)
};
module part_R14(part=true,hole=false,block=false)
{
translate([-25.943750,-14.750000,1.200000])rotate([0,0,180.000000])m5(part,hole,block,casetop); // RevK:R_0402 R_0402_1005Metric (back)
};
// Parts to go on PCB (top)
module parts_top(part=false,hole=false,block=false){
part_J5(part,hole,block);
part_J6(part,hole,block);
part_J2(part,hole,block);
part_J3(part,hole,block);
part_C12(part,hole,block);
part_D16(part,hole,block);
part_SW1(part,hole,block);
part_V2(part,hole,block);
part_R10(part,hole,block);
part_C3(part,hole,block);
part_R25(part,hole,block);
part_C22(part,hole,block);
part_R26(part,hole,block);
part_C5(part,hole,block);
part_D10(part,hole,block);
part_D8(part,hole,block);
part_L4(part,hole,block);
part_C17(part,hole,block);
part_J1(part,hole,block);
part_Q2(part,hole,block);
part_C7(part,hole,block);
part_R2(part,hole,block);
part_U1(part,hole,block);
part_R8(part,hole,block);
part_C26(part,hole,block);
part_C25(part,hole,block);
part_D1(part,hole,block);
part_D17(part,hole,block);
part_D4(part,hole,block);
part_C15(part,hole,block);
part_C19(part,hole,block);
part_J9(part,hole,block);
part_C16(part,hole,block);
part_D6(part,hole,block);
part_C14(part,hole,block);
part_C21(part,hole,block);
part_J7(part,hole,block);
part_R15(part,hole,block);
part_R9(part,hole,block);
part_C24(part,hole,block);
part_R27(part,hole,block);
part_D15(part,hole,block);
part_C2(part,hole,block);
part_C11(part,hole,block);
part_C28(part,hole,block);
part_C27(part,hole,block);
part_C23(part,hole,block);
part_R5(part,hole,block);
part_D9(part,hole,block);
part_R12(part,hole,block);
part_R1(part,hole,block);
part_D3(part,hole,block);
part_C20(part,hole,block);
part_PCB1(part,hole,block);
part_D18(part,hole,block);
part_D19(part,hole,block);
part_R4(part,hole,block);
part_C8(part,hole,block);
part_R6(part,hole,block);
part_D11(part,hole,block);
part_D5(part,hole,block);
part_U2(part,hole,block);
part_J8(part,hole,block);
part_C6(part,hole,block);
part_R7(part,hole,block);
part_U10(part,hole,block);
part_D14(part,hole,block);
part_R11(part,hole,block);
part_C13(part,hole,block);
part_U8(part,hole,block);
part_D7(part,hole,block);
part_D2(part,hole,block);
part_D13(part,hole,block);
part_V1(part,hole,block);
part_L3(part,hole,block);
part_C1(part,hole,block);
part_R13(part,hole,block);
part_C18(part,hole,block);
part_C9(part,hole,block);
part_C4(part,hole,block);
part_R14(part,hole,block);
}

parts_top=28;
module part_J10(part=true,hole=false,block=false)
{
};
module part_J4(part=true,hole=false,block=false)
{
};
// Parts to go on PCB (bottom)
module parts_bottom(part=false,hole=false,block=false){
part_J10(part,hole,block);
part_J4(part,hole,block);
}

parts_bottom=0;
module b(cx,cy,z,w,l,h){translate([cx-w/2,cy-l/2,z])cube([w,l,h]);}
module m0(part=false,hole=false,block=false,height)
{ // J5
// WAGO-2060-45x-998-404
N=2;
if(part)
{
	translate([-0.9,0,0])hull()
	{
		b(0,0,0,12.7,N*4-0.1,1);
		translate([0.8,0,0])b(0,0,0,11.1,N*4-0.1,4.5);
	}
	for(p=[0:N-1])hull()
    {
        translate([-6,-4*(N-1)/2+p*4,2])sphere(d=3,$fn=12);
        translate([-11.510,0,2])sphere(d=4,$fn=12);
    }
}
if(hole)
{
    hull()
    {
        translate([-11.510,0,2])sphere(d=4,$fn=12);
        translate([-11.510,0,-height])sphere(d=4,$fn=12);
    }
    hull()
    {
        translate([-11.510,0,2])sphere(d=4,$fn=12);
        translate([-50,0,2])sphere(d=4,$fn=12);
    }
}
}

module m1(part=false,hole=false,block=false,height)
{ // J6
// WAGO-2060-45x-998-404
N=3;
if(part)
{
	translate([-0.9,0,0])hull()
	{
		b(0,0,0,12.7,N*4-0.1,1);
		translate([0.8,0,0])b(0,0,0,11.1,N*4-0.1,4.5);
	}
	for(p=[0:N-1])hull()
    {
        translate([-6,-4*(N-1)/2+p*4,2])sphere(d=3,$fn=12);
        translate([-11.510,0,2])sphere(d=4,$fn=12);
    }
}
if(hole)
{
    hull()
    {
        translate([-11.510,0,2])sphere(d=4,$fn=12);
        translate([-11.510,0,-height])sphere(d=4,$fn=12);
    }
    hull()
    {
        translate([-11.510,0,2])sphere(d=4,$fn=12);
        translate([-50,0,2])sphere(d=4,$fn=12);
    }
}
}

module m2(part=false,hole=false,block=false,height)
{ // RevK:C_0402 C_0402_1005Metric
// 0402 Capacitor
if(part)
{
	b(0,0,0,1.0,0.5,1); // Chip
	b(0,0,0,1.5,0.65,0.2); // Pad size
}
}

module m3(part=false,hole=false,block=false,height)
{ // D16
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

module m4(part=false,hole=false,block=false,height)
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

module m5(part=false,hole=false,block=false,height)
{ // RevK:R_0402 R_0402_1005Metric
// 0402 Resistor
if(part)
{
	b(0,0,0,1.5,0.65,0.2); // Pad size
	b(0,0,0,1.0,0.5,0.5); // Chip
}
}

module m6(part=false,hole=false,block=false,height)
{ // RevK:C_0603_ C_0603_1608Metric
// 0603 Capacitor
if(part)
{
	b(0,0,0,1.6,0.8,1); // Chip
	b(0,0,0,1.6,0.95,0.2); // Pad size
}
}

module m7(part=false,hole=false,block=false,height)
{ // D10
// DFN1006-2L
if(part)
{
	b(0,0,0,1.0,0.6,0.45); // Chip
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
{ // U10
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

module m16(part=false,hole=false,block=false,height)
{ // U8
// SO-4_4.4x4.3mm_P2.54mm 
if(part)
{
	b(0,0,0,4.5,4.6,2.2); // Part
	b(0,0,0,3.34,6.8,1.5); // Pins
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
		translate([-casebottom-100,-casewall-100,pcbthickness+0.01]) cube([pcbwidth+casewall*2+200,pcblength+casewall*2+200,height]);
		if(step)translate([0,0,pcbthickness])
        	{
            		difference()
            		{
                		pcb_hulled(lip,casewall);
                		pcb_hulled(lip,casewall/2+fit);
				for(a=[0,180])rotate(a)hull()
                		{
                            		translate([lip/2,lip/2,0])cube([pcbwidth,pcblength,lip]);
                            		translate([-lip/2,-lip/2,lip])cube([pcbwidth,pcblength,lip]);
                		}
            		}
            		difference()
            		{
                		pcb_hulled(lip,casewall/2+fit);
				for(a=[90,270])rotate(a)hull()
                		{
                            		translate([lip/2,lip/2,0])cube([pcblength,pcbwidth,lip]);
                            		translate([-lip/2,-lip/2,lip])cube([pcblength,pcbwidth,lip]);
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
			else hull(){parts_top(part=true);pcb_hulled();}
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
bottom(); translate([spacing,0,0])top();
