#!../login/loggedin /bin/csh -fx
if($?PATH_INFO) then
	setenv device "$PATH_INFO:t"
endif
can --redirect --device='$device' editdevice
if($status) exit 0

if($?UPGRADE) then
	setenv RESTART
	sql "$DB" 'UPDATE device SET upgrade=NOW() WHERE device="$device"'
endif
if($?RESTART) then
	setenv MSG `message --device="$device" --command=restart`
	if(! $status) setenv MSG "Restarting"
	unsetenv device
	goto done
endif
if($?DELETE || $?FACTORY) then
	if(! $?SURE) then
		setenv MSG "Tick to say you are sure"
		goto done
	endif
	if($?FACTORY)	then
		setenv MSG `message --device="$device" --command=factory '"'"$device"'SS"'`
		if($status) goto done
	else
		setenv MSG `message --device="$device" --command=restart`
	endif
	sql "$DB" 'DELETE FROM devicegpio WHERE device=$device'
	setenv C `sql -c "$DB" 'DELETE FROM device WHERE device=$device'`
	if("$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use"
		goto done
	endif
	setenv MSG Deleted
	unsetenv device
	goto done
endif
if($?description) then # save
	if(! $?doorauto) setenv doorauto false
	sqlwrite -v -o -n "$DB" device description area doorauto aid
	setenv MSG Updated
	unsetenv device
	goto done
endif
done:
source ../types
xmlsql -C -d "$DB" head.html - foot.html << END
<h1>Device</h1>
<if not device>
<table border=1>
<tr><th>Device</th><th>Online</th><th>IP</th><th>Version</th><th>PCB</th><th>Name</th></tr>
<sql select="*,pcb.description AS D" table="device LEFT JOIN pcb USING (pcb)" order=device.description WHERE="site=\$SESSION_SITE"><set found=1>
<tr>
<td><output name=device href="/editdevice.cgi/\$device"></td>
<td><if online><output name=online></if><if else>Last online <output name=lastonline missing="never"></if></td>
<td><output name=address></td>
<td><output name=version><if upgrade> (upgrade scheduled)</if></td>
<td><output name=D></td>
<td><output name=description blank="Unspecified" missing="Unnamed"></td>
</tr>
</sql>
</table>
<if not found><p>No devices found.</p></if>
</if><if else>
<form method=post action=/editdevice.cgi><input type=hidden name=device>
<sql table=device KEY=device>
<table>
<tr><td>Name</td><td><input name=description ize=40 autofocus></td></tr>
<tr><td>Online</td><td><if online><output name=online></if><if else>Last online <output name=lastonline missing="never"></if><if upgrade> (upgrade scheduled)</if></td></tr>
<sql table=pcb where="pcb=\$pcb">
<tr><td>PCB</td><td><output name=description></td></tr>
<if not nfctx=='-'><tr><td>Area</td><td><sql select="tag" table=area where="site=\$site"><label for=\$tag><output name=tag>:</label><input id=\$tag name=area type=checkbox value=\$tag></sql></td></tr></if>
<if not nfctx=='-'><tr><td>AID</td><td><select name=aid><sql table=aid where="organisation=$SESSION_ORGANISATION"><option value="\$aid"><output name=description></option></sql></select></td></tr></if>
<if not nfctx=='-'><tr><td><label for doorauto>Door control</label></td><td><input type=checkbox id=doorauto name=doorauto value=true></td></tr></if>
</sql>
<sql table="device JOIN gpio USING (pcb) LEFT JOIN devicegpio ON (devicegpio.device=device.device AND devicegpio.gpio=gpio.gpio)" WHERE="device.device='\$device'">
<tr><td><output name=pinname href="/editgpio.cgi/\$device/\$gpio"></td>
<if type><td><b><output name=type $GPIOTYPEOUT><if invert=true> (inverted)</if></b><for space S="$STATELIST"><if not "\$S"=''> <output name=S></if></for></td></if>
<if not type><td><b><output name=io $GPIOIOOUT></td></if>
</tr>
</sql>
</table>
<input type=submit value="Update">
<if online><input type=submit value="Restart" name=RESTART></if>
<if not upgrade><input type=submit value="Upgrade" name=UPGRADE></if>
<input type=submit value="Delete" name=DELETE>
<if online><input type=submit value="Factory Reset" name=FACTORY></if>
<input type=checkbox name=SURE title='Tick this to say you are sure'>
</sql>
</form>
</if>
END
