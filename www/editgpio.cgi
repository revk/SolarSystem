#!../login/loggedin /bin/csh -f
if($?PATH_INFO) then
	setenv device "$PATH_INFO:h:t"
	setenv gpio "$PATH_INFO:t"
endif
can --redirect --device='$device' editdevice
if($status) exit 0
source ../types

if($?type) then
	if(! $?invert) setenv invert false
	sqlwrite -q -o -n "$DB" devicegpio device gpio type invert name hold pulse $STATELIST
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE device="$device"'
	message --poke
	../login/redirect "editdevice.cgi/$device"
	exit 0
endif

done:
sql "$DB" 'INSERT IGNORE INTO devicegpio SET device="$device",gpio="$gpio"'
xmlsql -C -d "$DB" head.html - foot.html << END
<form method=post action="/editgpio.cgi"><input name=devicegpio type=hidden>
<sql table=area where="site=$SESSION_SITE"><IF AREAS><SET AREAS="\$AREAS "></IF><SET AREAS="\$AREAS\$area"></sql>
<sql table="devicegpio LEFT JOIN device USING (device) LEFT JOIN gpio ON (devicegpio.gpio=gpio.gpio)" WHERE="devicegpio.device='\$device' AND devicegpio.gpio=\$gpio">
<input name=device type=hidden><input name=gpio type=hidden>
<table>
<tr><td>Device</td><td><output name=devicename blank="Unnamed" missing="Unnamed"></td></tr>
<tr><td>PCB</td><td><sql table=pcb where="pcb=\$pcb"><output name=pcbname></sql></td></tr>
<tr><td>GPIO</td><td><output name=pinname></td></tr>
<if not type=P><tr><td>Name</td><td><input name=name size=10 autofocus></td></tr></if>
<if type=*I><tr><td>Hold</td><td><input name=hold size=3>ms</td></tr></if>
<if type=*O><tr><td>Pulse</td><td><input name=pulse size=3>s/10</td></tr></if>
<tr><td>Type</td><td><output name=io $GPIOIOOUT>:<if io=I><select name=type>$GPIOTYPEPICKI</select></if><if io=O><select name=type>$GPIOTYPEPICKO</select></if><if io=IO><select name=type>$GPIOTYPEPICK</select></if></td></tr>
<tr><td><label for=invert>Invert</label></td><td><input id=invert name=invert type=checkbox value=true></td></tr>
</table>
<if not type=*P>
<if type=*O><set SL="$STATELIST"></if><if else><set SL="$STATELISTI"></if>
<table>
<tr><th></th><for space STATE="\$SL"><th><output name=STATE></th></for></tr>
<sql table=area where="site=$SESSION_SITE" order=tag>
<tr>
<td><output name=tag></td>
<for space STATE="\$SL"><td><input type=checkbox name="\$STATE" value="\$tag"></td></for>
<td><output name=areaname></td>
</sql>
</for>
</tr>
</table>
</if>
</sql>
<input type=submit value="Update">
</form>
END
