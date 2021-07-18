#!../login/loggedin /bin/csh -f
if($?PATH_INFO) then
	setenv device "$PATH_INFO:t"
endif
can --redirect --organisation='$SESSION_ORGANISATION' editdevice
if($status) exit 0

if($?UPGRADEALL) then
	can --redirect --device='$device' editdevice
	sql "$DB" 'UPDATE device SET upgrade=NOW() WHERE upgrade is NULL AND organisation="$SESSION_ORGANISATION" AND site="$SESSION_SITE"'
        message --poke
	redirect editdevice.cgi
	exit 0
endif
if($?UPGRADE) then
	can --redirect --device='$device' editdevice
	sql "$DB" 'UPDATE device SET upgrade=NOW() WHERE device="$device"'
	setenv MSG `message --device="$device" --command=upgrade`
	if("$MSG" != "") goto done
	redirect editdevice.cgi
	exit 0
endif
if($?RESTART) then
	can --redirect --device='$device' editdevice
	setenv MSG `message --device="$device" --command=restart`
	if("$MSG" != "") goto done
	redirect editdevice.cgi
	exit 0
endif
if($?DELETE && "$USER_ADMIN" == "true" || $?FACTORY) then
	can --redirect --device='$device' editdevice
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
	sql "$DB" 'DELETE FROM devicegpio WHERE device="$device"'
	setenv C `sql -c "$DB" 'DELETE FROM device WHERE device="$device"'`
	if("$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use"
		goto done
	endif
	redirect editdevice.cgi
	exit 0
endif
if($?devicename) then # save
	can --redirect --device='$device' editdevice
	setenv pcb `sql "$DB" 'SELECT pcb FROM device WHERE device="$device"'`
	if($?pcb) then
		setenv rgb `sql "$DB" 'SELECT IF(ledr="-","false","true") FROM pcb WHERE pcb=$pcb'`
		setenv nfc `sql "$DB" 'SELECT IF(nfctx="-","false","true") FROM pcb WHERE pcb=$pcb'`
		setenv keypad `sql "$DB" 'SELECT IF(keypadtx="-","false","true") FROM pcb WHERE pcb=$pcb'`
	endif
	if(! $?nfcadmin) setenv nfcadmin false
	if(! $?nfctrusted) setenv nfctrusted false
	if(! $?door) setenv door false
	if(! $?doorexitarm) setenv doorexitarm false
	if(! $?doorexitdisarm) setenv doorexitdisarm false
	if(! $?iotstatedoor) setenv iotstatedoor false
	if(! $?iotstateinput) setenv iotstateinput false
	if(! $?iotstateoutput) setenv iotstateoutput false
	if(! $?iotstatefault) setenv iotstatefault false
	if(! $?iotstatetamper) setenv iotstatetamper false
	if(! $?iotstatesystem) setenv iotstatesystem false
	if(! $?ioteventfob) setenv ioteventfob false
	setenv allow "devicename areawarning areafault areatamper areaenter areastrongarm areadeadlock areaarm areadisarm areabell arealed nfc rgb nfcadmin door doorexitarm doorexitdisarm aid site iotstatedoor iotstateinput iotstateoutput iotstatefault iotstatetamper iotstatesystem ioteventfob doorunlock doorlock dooropen doorclose doorprop doorexit"
	if("$USER_ADMIN" == "true") setenv allow "$allow nfctrusted"
	sqlwrite -qon "$DB" device $allow
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE device="$device"'
        message --poke
	redirect editdevice.cgi
	exit 0
endif
done:
source ../types
xmlsql -C -d "$DB" head.html - foot.html << END
<h1>Device</h1>
<if not device>
<table border=1>
<tr>
<th>Device</th>
<th>Name</th>
<th>Online</th>
<th>IP</th>
<th>Version</th>
<th>PCB</th>
<th>Notes</th></tr>
<sql table="device" where="organisation=$SESSION_ORGANISATION" select="max(version) AS V,max(build) AS B"><set V="\$V"><set B="\$B"></sql>
<sql where="device.organisation=$SESSION_ORGANISATION" table="device LEFT JOIN pcb USING (pcb) LEFT JOIN device AS device2 ON (device.via=device2.device)" order="device2.devicename,device.devicename" WHERE="site=\$SESSION_SITE" select="device.*,pcb.pcbname,device2.devicename AS P"><set found=1>
<tr>
<td><tt><output name=device href="/editdevice.cgi/\$device"></tt></td>
<td><output name=devicename blank="Unnamed" missing="Unnamed"></td>
<set s=""><if lastonline><set s="background:green;"></if><if not online><set s="background:yellow;"></if>
<td style="\$s"><if online><tt><output name=online></tt><if via><br><i>via</i> <tt><output name=via></tt></if><if else><br><tt><output name=bssid>#<output name=chan></tt> <i><output name=ssid></i></if></if><if else><tt><output name=lastonline missing="never"></tt><br><i><output name=offlinereason missing=Offline></i></if></td>
<td><output name=address></td>
<td><if upgrade><i style='background:cyan;'>Upgrade scheduled</i><br></if><set s="background:red;"><if version="\$V"><set s="background:green;"></if><tt style="\$s"><output name=version></tt><br><set s="background:yellow;"><if build="\$B"><set s="background:green;"></if><tt style="\$s"><output name=build></tt></td>
<td><output name=pcbname><br>Flash:<output name=flash type=mega></td>
<set s><if not fault="{}"><set s="background:yellow;"></if><if not tamper="{}"><set s="background:red;"></if>
<td style="\$s"><if door=true>Door </if><if nfc=true>NFC reader </if><if nfcadmin=true> (admin)</if><if nfctrusted=true><b> (trusted)</b></if><br>
<if not tamper="{}">Tamper:<b><output name=tamper></b><br></if>
<if not fault="{}">Fault:<b><output name=fault></b><br></if>
</td>
</tr>
</sql>
</table>
<if found><form method=post><input name=UPGRADEALL value="Upgrade all" type=submit></form></if><if else><p>No devices found.</p></if>
</if><if else>
<form method=post action=/editdevice.cgi><input type=hidden name=device>
<sql table="device LEFT JOIN pcb USING (pcb)" KEY=device>
<table>
<tr><td>PCB</td><td><output name=pcbname></td></tr>
<tr><td>Name</td><td><input name=devicename size=20 maxlength=20 autofocus></td></tr>
<tr><td>Site</td><td><select name=site><sql table=site where="organisation=$SESSION_ORGANISATION"><option value='\$site'><output name=sitename></option></sql></select></td></tr>
<sql table=site where="site=\$site">
<if not iothost="">
<tr><td>IoT (<output name=iothost>)</td><td>
<input id=iotstatedoor name=iotstatedoor value=true type=checkbox><label for=iotstatedoor>Door</label>
<input id=iotstateinput name=iotstateinput value=true type=checkbox><label for=iotstateinput>Input</label>
<input id=iotstateoutput name=iotstateoutput value=true type=checkbox><label for=iotstateoutput>Output</label>
<input id=iotstatefault name=iotstatefault value=true type=checkbox><label for=iotstatefault>Fault</label>
<input id=iotstatetamper name=iotstatetamper value=true type=checkbox><label for=iotstatetamper>Tamper</label>
<input id=iotstatesystem name=iotstatesystem value=true type=checkbox><label for=iotstatesystem>System</label>
<input id=ioteventfob name=ioteventfob value=true type=checkbox><label for=ioteventfob>Fob events</label>
</td>
</if>
</sql>
<if nfc=true><tr><td>AID</td><td><select name=aid><sql table=aid where="site=\$site"><option value="\$aid"><output name=aidname></option></sql></select></td></tr></if>
<tr><td>Online</td><td><if online><output name=online></if><if else>Last online <output name=lastonline missing="never"></if><if upgrade> (upgrade scheduled)</if></td></tr>
<tr><td>Version</td><td><output name=version></td></tr>
<sql table=pcb where="pcb=\$pcb">
<if nfc=true><tr><td><input type=checkbox id=door name=door value=true></td><td><label for=door>Door control</label></td></tr></if>
<if nfc=true door=true><tr><td><input size=5 id=doorunlock name=doorunlock>ms</td><td><label for=doorunlock>Door unlock timer</label></td></tr></if>
<if nfc=true door=true><tr><td><input size=5 id=doorlock name=doorlock>ms</td><td><label for=doorlock>Door lock timer</label></td></tr></if>
<if nfc=true door=true><tr><td><input size=5 id=dooropen name=dooropen>ms</td><td><label for=dooropen>Door open timer</label></td></tr></if>
<if nfc=true door=true><tr><td><input size=5 id=doorclose name=doorclose>ms</td><td><label for=doorclose>Door close timer</label></td></tr></if>
<if nfc=true door=true><tr><td><input size=5 id=doorprop name=doorprop>ms</td><td><label for=doorprop>Door prop timer</label></td></tr></if>
<if nfc=true door=true><tr><td><input size=5 id=doorexit name=doorexit>ms</td><td><label for=doorexit>Door exit button hold timer</label></td></tr></if>
<if nfc=true door=true><tr><td><input type=checkbox id=doorexitdisarm name=doorexitdisarm value=true></td><td><label for=doorexitdisarm>Door exit button disarm</label></td></tr></if>
<if nfc=true door=true><tr><td><input type=checkbox id=doorexitarm name=doorexitarm value=true></td><td><label for=doorexitarm>Door exit button arm on hold</label></td></tr></if>
<if nfc=true><tr><td><input type=checkbox id=nfcadmin name=nfcadmin value=true></td><td><label for=nfcadmin>Admin NFC reader</label></td></tr></if>
<if USER_ADMIN=true nfc=true><tr><td><input type=checkbox id=nfctrusted name=nfctrusted value=true></td><td><label for=nfctrusted>Trusted NFC reader</label></td></tr></if>
</sql>
<sql select="device.device,gpio.*,devicegpio.*" table="device JOIN gpio USING (pcb) LEFT JOIN devicegpio ON (devicegpio.device=device.device AND devicegpio.gpio=gpio.gpio)" WHERE="device.device='\$device'">
<tr><td><output name=name href="/editgpio.cgi/\$device/\$gpio" blank="GPIO" missing="GPIO"></td>
<if type><td><b><output name=type $GPIOTYPEOUT><if not pulse=0 AND type=*O>(<eval #=1 s="\$pulse/10"><output name=s>s)</if><if not hold=0 AND type=*I>(<output name=hold>ms)</if><if invert=true> (inverted)</if></b><for space S="$STATELIST"><if not "\$S"=''> <output name=S></if></for></td></if>
<if not type><td><b><output name=io $GPIOIOOUT></td></if>
</tr>
</sql>
</table>
<table border=1>
<set tags="warning fault tamper">
<if rgb=true><set tags="\$tags led"></if>
<if nfc=true><set tags="\$tags deadlock enter arm strongarm disarm bell"></if>
<tr><th></th>
<for SPACE T="\$tags"><th><output name=T></th></for>
<th>Areas</th>
</tr>
<sql table=area where="site=$SESSION_SITE">
<tr>
<th><output name=tag></th>
<for SPACE T="\$tags"><td><input name="area\$T" type=checkbox value="\$tag"></td></for>
<td><output name=areaname></td>
</tr>
</sql>
</table>
<input type=submit value="Update">
<if online><input type=submit value="Restart" name=RESTART></if>
<if not upgrade><input type=submit value="Upgrade" name=UPGRADE></if>
<if USER_ADMIN=true><input type=submit value="Delete" name=DELETE></if>
<if online><input type=submit value="Factory Reset" name=FACTORY></if>
<input type=checkbox name=SURE title='Tick this to say you are sure'>
</sql>
</form>
</if>
END
