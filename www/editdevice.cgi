#!../login/loggedin /bin/csh -f
if($?PATH_INFO) then
	setenv device "$PATH_INFO:t"
endif
can --redirect --organisation='$USER_ORGANISATION' viewdevice
if($status) exit 0

if($?UNLOCK) then
	can --redirect --device='$device' unlock
	if($status) exit 0
	setenv MSG `message --device="$device" --command=unlock`
	if("$MSG" != "") goto done
	redirect editdevice.cgi
	exit 0
endif

if($?OOSALL) then
	can --redirect --site='$USER_SITE' admin
	if($status) exit 0
	sql "$DB" 'UPDATE device SET outofservice="true",poke=NOW() WHERE outofservice="false" AND organisation="$USER_ORGANISATION" AND site="$USER_SITE" AND excludeall<>"true"'
        message --poke
	redirect editdevice.cgi
	exit 0
endif
if($?ISALL) then
	can --redirect --site='$USER_SITE' admin
	if($status) exit 0
	sql "$DB" 'UPDATE device SET outofservice="false",poke=NOW() WHERE outofservice="true" AND organisation="$USER_ORGANISATION" AND site="$USER_SITE" AND excludeall<>"true"'
        message --poke
	redirect editdevice.cgi
	exit 0
endif
if($?UPGRADEALL) then
	can --redirect --site='$USER_SITE' editdevice
	if($status) exit 0
	sql "$DB" 'UPDATE device SET upgrade=NOW() WHERE upgrade is NULL AND organisation="$USER_ORGANISATION" AND site="$USER_SITE" AND excludeall<>"true"'
        message --poke
	redirect editdevice.cgi
	exit 0
endif
if($?UPGRADE) then
	can --redirect --device='$device' editdevice
	if($status) exit 0
	sql "$DB" 'UPDATE device SET upgrade=NOW() WHERE device="$device"'
	setenv BUILD_SUFFIX `sql "$DB" 'SELECT build_suffix FROM device WHERE device="$device"'`
	setenv MSG `message --device="$device" --command=upgrade '"'"/SS$BUILD_SUFFIX.bin"'"'`
	if("$MSG" != "") goto done
	redirect editdevice.cgi
	exit 0
endif
if($?RESTART) then
	can --redirect --device='$device' editdevice
	if($status) exit 0
	setenv MSG `message --device="$device" --command=restart`
	if("$MSG" != "") goto done
	redirect editdevice.cgi
	exit 0
endif
if($?DELETE && "$USER_ADMIN" == "true" || $?FACTORY) then
	can --redirect --device='$device' editdevice
	if($status) exit 0
	if(! $?SURE) then
		setenv MSG "Are you sure?"
		goto done
	endif
	if($?FACTORY)	then
		setenv MSG `message --device="$device" --command=factory '"'"$device"'SS"'`
		if($status) goto done
	else
		setenv MSG `message --device="$device" --command=restart`
	endif
	sql "$DB" 'UPDATE device SET site=NULL WHERE device="$device"'
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
        message --poke
	redirect editdevice.cgi
	exit 0
endif
if($?devicename) then # save
	can --redirect --device='$device' editdevice
	if($status) exit 0
	setenv wassite `sql "$DB" 'SELECT site FROM device WHERE device="$device"'`
	if("$site" != "$wassite") then
		setenv aid `sql "$DB" 'SELECT aid FROM aid WHERE site=$site ORDER BY aid LIMIT 1'`
	endif
	if($?pcb) then
		setenv rgb `sql "$DB" 'SELECT IF(ledr="-","false","true") FROM pcb WHERE pcb=$pcb'`
		setenv nfc `sql "$DB" 'SELECT IF(nfctx="-","false","true") FROM pcb WHERE pcb=$pcb'`
		setenv gps `sql "$DB" 'SELECT IF(gpstx="-","false","true") FROM pcb WHERE pcb=$pcb'`
		setenv keypad `sql "$DB" 'SELECT IF(keypadtx="-","false","true") FROM pcb WHERE pcb=$pcb'`
	endif
	if(! $?outofservice) setenv outofservice false
	if(! $?excludeall) setenv excludeall false
	if(! $?nfcadmin) setenv nfcadmin false
	if(! $?nfctrusted) setenv nfctrusted false
	if(! $?door) setenv door false
	if(! $?doorexitarm) setenv doorexitarm false
	if(! $?doorexitdisarm) setenv doorexitdisarm false
	if(! $?doordebug) setenv doordebug false
	if(! $?doorcatch) setenv doorcatch false
	if(! $?iotstatedoor) setenv iotstatedoor false
	if(! $?iotstateinput) setenv iotstateinput false
	if(! $?iotstateoutput) setenv iotstateoutput false
	if(! $?iotkeypad) setenv iotkeypad false
	if(! $?iotgps) setenv iotgps false
	if(! $?ioteventfob) setenv ioteventfob false
	if(! $?keypadpinarm) setenv keypadpinarm false
	setenv allow "devicename timer1 areaenter areastrong areadeadlock areaarm areadisarm areabell arealed areakeypad areakeyarm areakeystrong areakeydisarm nfc gps rgb nfcadmin door doorexitarm doorexitdisarm aid site iotstatedoor iotstateinput iotstateoutput ioteventfob iotkeypad iotgps doorunlock doorlock dooropen doorclose doorprop doorexit doordebounce keypadidle keypadpin keypadpinarm keypad pcb dooriotopen dooriotlock dooriotunlock dooriotdead dooriotundead excludeall outofservice doordebug doorcatch"
	if("$USER_ADMIN" == "true") setenv allow "$allow nfctrusted"
	sqlwrite -qon "$DB" device $allow
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
	if("$site" != "$wassite") then
		sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$wassite'
	endif
        message --poke
	redirect editdevice.cgi
	exit 0
endif
done:
source ../types
source ../setcan
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>🪛 Manage devices <output name=VERSION></h1>
<if device CANVIEWLOG><p><a href="/log.cgi/$device">View logs</a></p></if>
<if not device CANVIEWDEVICE>
<table>
<tr>
<th>Device</th>
<th>Online</th>
<th>Connect</th>
<th>Version</th>
<th>PCB</th>
<th>Flash</th>
<th>Mem</th>
<th>SPI</th>
<th>Notes</th></tr>
<sql table="site" where="site=$USER_SITE"><set root="$root"></sql>
<sql table="device" where="site=$USER_SITE" select="max(version) AS V,max(build) AS B"><set V="$V"><set B="$B"></sql>
<sql table="device LEFT JOIN pcb USING (pcb) LEFT JOIN device AS device2 ON (device.via=device2.device) LEFT JOIN device AS device3 ON (conv(device.bssid,16,10)=conv(device3.device,16,10)+1)" order="if(device.device='$root',0,1),device.outofservice,device.devicename" WHERE="device.site=$USER_SITE" select="device.*,pcb.pcbname,device2.devicename AS VIA,device3.devicename AS PARENT"><set found=1>
<set s="">
<if outofservice=false><if lastonline><set s="background:green;"><if not via><set s="background:lightgreen;"></if></if><if not online><set s="background:yellow;"></if></if>
<if outofservice=true><if lastonline><set s="background:lightblue;"></if><if not online><set s="background:blue;"></if></if>
<tr style="$s">
<td title="$device"><IF CANEDITDEVICE><output href="/editdevice.cgi/$device" name=devicename blank="Unnamed" missing="Unnamed"></if><if else><output name=devicename></if></td>
<td align=right><if online><tt title="When online"><output name=boot missing="Just now" type=recent></if><if else><tt title="Last online"><output name=lastonline missing="never" type=recent></tt></if></td>
<td><if online><if via><i>via</i> <if PARENT NOT PARENT="$VIA"><output name=PARENT> &amp; </if><output name=VIA></if><if else><tt title="BSSID#channel"><output name=bssid>#<output name=chan></tt> <i title="SSID"><output name=ssid></i></if></if><if else><i title="Why offline"><output name=offlinereason></i></if></td>
<td><if upgrade><if online OR outofservice=false><i style='background:cyan;'>Upgrade <output name=progress 0=Started missing=Scheduled></i><br></if></if><set s="background:red;"><if version="$V"><set s="background:green;"></if><tt style="$s"><output name=version><output name=build_suffix></tt></td>
<td><output name=pcbname></td>
<td align=right><output name=flash type=mebi replace .00="" .0Mi="Mi">B</td>
<td align=right><output name=mem type=mebi replace .00="" .0Mi="Mi">B</td>
<td align=right><if spi=0>-</if><if else><output name=spi type=mebi replace .00="" .0Mi="Mi">B</if></td>
<td>
<if excludeall=true>*</if>
<if outofservice=true>Out of service</if>
<if else>
<if online door=true CANUNLOCK><form style="display:inline;" method=post><input type=hidden name=device><input type=submit name=UNLOCK value="Unlock"></form></if>
<if door=true>Door </if><if gps=true>GPS </if><if nfc=true>NFC reader </if><if nfcadmin=true> (admin)</if><if nfctrusted=true><b> (trusted)</b></if>
</if>
<if online not via><output name=address></if>
<if USER_ADMIN=true><i title='Restart reason'><output name=rst missing='' 0='' 1='power-on' 2='' 3='' 4='panic' 5='int-wdt' 6='task-wdt' 7='rst-wdt' 8='deepsleep' 9='brownout' 10='rst-sdio' 11='rst-usb'></i></IF>
</td>
</tr>
</sql>
</table>
<if found>
<if CANEDITDEVICE><form method=post style="display:inline-block;"><input name=UPGRADEALL value="Upgrade all" type=submit></form></if>
<if USER_ADMIN=true><form method=post style="display:inline-block;"><input name=OOSALL value="Out off service all" type=submit></form></if>
<if USER_ADMIN=true><form method=post style="display:inline-block;"><input name=ISALL value="In service all" type=submit></form></if>
</if><if else><p>No devices found.</p></if>
</if><if else CANEDITDEVICE>
<form method=post action=/editdevice.cgi><input type=hidden name=device>
<sql table="device LEFT JOIN pcb USING (pcb)" KEY=device>
<table>
<tr><td><input type=checkbox id=outofservice name=outofservice value=true></td><td colspan=2><label for=outofservice>Out of service (recommended before upgrade, as faster)</label></td></tr>
<tr><td><input type=checkbox id=excludeall name=excludeall value=true></td><td colspan=2><label for=excludeall>Do not include in "all" buttons</label></td></tr>
<tr><td>PCB</td><td colspan=2><select name=pcb><sql table=pcb order=pcbname><option value="$pcb"><output name=pcbname></option></sql></select></td></tr>
<tr><td>Name</td><td colspan=2><input name=devicename size=20 maxlength=20 autofocus></td></tr>
<if keypad=true><tr><td>Keypad</td><td colspan=2><input name=keypadidle size=16 maxlength=16 autofocus></td></tr></if>
<if keypad=true><tr><td>Keypad PIN</td><td colspan=2><input name=keypadpin size=16 maxlength=16 autofocus> to disarm, <input type=checkbox id=keypadpinarm name=keypadpinarm value=true><label for=keypadpinarm> to arm</label></td></tr></if>
<tr><td>Site</td><td colspan=2><select name=site><sql table=site where="organisation=$USER_ORGANISATION"><option value='$site'><output name=sitename></option></sql></select></td></tr>
<sql table=site where="site=$site">
<if not iothost="">
<tr><td>IoT (<output name=iothost>)</td><td colspan=2>
<input id=iotstateinput name=iotstateinput value=true type=checkbox><label for=iotstateinput>Input</label>
<input id=iotstateoutput name=iotstateoutput value=true type=checkbox><label for=iotstateoutput>Output</label>
<if door=true><input id=iotstatedoor name=iotstatedoor value=true type=checkbox><label for=iotstatedoor>Door</label></if>
<if door=true><input id=doordebug name=doordebug value=true type=checkbox><label for=doordebug>Door debug</label></if>
<if nfc=true><input id=ioteventfob name=ioteventfob value=true type=checkbox><label for=ioteventfob>Fob events</label></if>
<if keypad=true><input id=iotkeypad name=iotkeypad value=true type=checkbox><label for=iotkeypad>Keypad events</label></if>
<if gps=true><input id=iotgps name=iotgps value=true type=checkbox><label for=iotgps>GPS events</label></if>
</td>
</if>
</sql>
<if nfc=true><tr><td>AID</td><td colspan=2><select name=aid><sql table=aid where="site=$site"><option value="$aid"><output name=aidname></option></sql></select></td></tr></if>
<tr><td>Online</td><td colspan=2><if online><output name=online></if><if else>Last online <output name=lastonline missing="never"></if><if upgrade> (upgrade scheduled)</if></td></tr>
<tr><td>Version</td><td colspan=2><output name=version></td></tr>
<tr><td>Timer1</td><td><input name=timer1 type=time> timer logical input if clock set</td></tr>
<sql table=pcb where="pcb=$pcb">
<tr><td><input type=checkbox id=door name=door value=true></td><td colspan=2><label for=door>Door control<if door=true> (set times 0 for default, -1 to force no timer)</if></label></td></tr>
<if door=true><tr><td><input size=5 id=doorunlock name=doorunlock>ms</td><td colspan=2><label for=doorunlock>Lock disengage timer (set -1 for EL56X)</label></td></tr></if>
<if door=true><tr><td><input size=5 id=doorlock name=doorlock>ms</td><td colspan=2><label for=doorlock>Lock engage timer</label></td></tr></if>
<if door=true><tr><td><input size=5 id=doordebounce name=doordebounce>ms</td><td colspan=2><label for=doordebounce>Lock engaged debounce timer</label></td></tr></if>
<if door=true><tr><td><input size=5 id=dooropen name=dooropen>ms</td><td colspan=2><label for=dooropen>Door waiting to open timer</label></td></tr></if>
<if door=true><tr><td><input size=5 id=doorclose name=doorclose>ms</td><td colspan=2><label for=doorclose>Door waiting to close timer</label></td></tr></if>
<if door=true><tr><td><input size=5 id=doorprop name=doorprop>ms</td><td colspan=2><label for=doorprop>Door unauthorised prop timer</label></td></tr></if>
<if door=true><tr><td><input size=5 id=doorexit name=doorexit>ms</td><td colspan=2><label for=doorexit>Door exit button hold timer</label></td></tr></if>
<if door=true><tr><td><input type=checkbox id=doorexitdisarm name=doorexitdisarm value=true></td><td colspan=2><label for=doorexitdisarm>Door exit button disarm</label></td></tr></if>
<if door=true><tr><td><input type=checkbox id=doorexitarm name=doorexitarm value=true></td><td colspan=2><label for=doorexitarm>Door exit button arm on hold</label></td></tr></if>
<if door=true><tr><td><input type=checkbox id=doorcatch name=doorcatch value=true></td><td colspan=2><label for=doorcatch>Door catch mode (re-lock on opening)</label></td></tr></if>
<if door=true><tr><td>IoT on lock</td><td colspan=2><input name=dooriotlock></td></tr></if>
<if door=true><tr><td>IoT on deadlock</td><td colspan=2><input name=dooriotdead></td></tr></if>
<if door=true><tr><td>IoT on un-deadlock</td><td colspan=2><input name=dooriotundead></td></tr></if>
<if door=true><tr><td>IoT on unlock</td><td colspan=2><input name=dooriotunlock></td></tr></if>
<if door=true><tr><td>IoT on open</td><td colspan=2><input name=dooriotopen></td></tr></if>
<if nfc=true><tr><td><input type=checkbox id=nfcadmin name=nfcadmin value=true></td><td colspan=2><label for=nfcadmin>Admin NFC reader</label></td></tr></if>
<if USER_ADMIN=true nfc=true><tr><td><input type=checkbox id=nfctrusted name=nfctrusted value=true></td><td colspan=2><label for=nfctrusted>Trusted NFC reader</label></td></tr></if>
</sql>
<sql select="device.device,gpio.*,devicegpio.*" table="device JOIN gpio USING (pcb) LEFT JOIN devicegpio ON (devicegpio.device=device.device AND devicegpio.gpio=gpio.gpio)" WHERE="device.device='$device'" order=initname>
<tr><td><output name=name href="/editgpio.cgi/$device/$gpio" blank="GPIO" missing="GPIO"><if not name="$initname"> (<output name=initname>)</if></td>
<if type><td><if type=*-><i>Unused</i></if><if else><b><output name=type $GPIOTYPEOUT> <if type=*I><i><output name=func replace $GPIOFUNCOUTI></i></if><if type=*O><i><output name=func $GPIOFUNCOUTO></i></if> (<output name=invert false="$value1" true="$value0"><if not pulse=0 AND type=*O> <eval #=1 s="$pulse/10"><output name=s>s</if><if not hold=0 AND type=*I> <output name=hold>ms</if>)</b></td><td><for space S="$STATELIST"><if not "$S"=''> <output name=S> (<output name="$S">)</if></for></if></td></if>
<if not type><td><b><output name=io $GPIOIOOUT></td><td><i>Undefined</i></td></if>
</tr>
</sql>
</table>
<if rgb=true><set tags="$tags led"></if>
<if keypad=true><set tags="$tags keypad keyarm keystrong keydisarm"></if>
<if nfc=true><set tags="$tags deadlock enter arm strong disarm"></if>
<if tags>
<table>
<tr><th></th>
<for SPACE T="$tags"><if not T=""><th><output name=T></th></if></for>
<th>Areas</th>
</tr>
<sql table=area where="site=$USER_SITE">
<tr>
<th><output name=tag></th>
<for SPACE T="$tags"><if not T=""><td><input name="area$T" type=checkbox value="$tag"></td></if></for>
<td><output name=areaname></td>
</tr>
</sql>
</table>
</if>
<if nfc=true><p>arm/disarm/strong allow if any areas covered by fob match. enter/prop apply if all enter areas covered by fob match. deadlocked if any deadlock area is armed.</p></if>
<input type=submit value="Update">
<if online><input type=submit value="Restart" name=RESTART></if>
<if not upgrade><input type=submit value="Upgrade" name=UPGRADE></if>
<if USER_ADMIN=true><input type=submit value="Delete" name=DELETE></if>
<if online><input type=submit value="Factory Reset" name=FACTORY></if>
<input type=checkbox name=SURE title='Tick this to say you are sure'>
</sql>
</form>
</if>
'END'
