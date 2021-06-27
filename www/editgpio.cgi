#!../login/loggedin /bin/csh -fx
if($?PATH_INFO) then
	setenv devicegpio "$PATH_INFO:t"
endif
can --redirect --gpio='$devicegpio' editdevice
if($status) exit 0
source ../types

if($?device) then
	if(! $?invert) setenv invert false
	sqlwrite -v -o -n "$DB" devicegpio invert $STATELIST
	echo "Location: ${ENVCGI_SERVER}editdevice.cgi/$device"
	echo ""
	exit 0
endif

done:
echo "Content-Type: text/html"
echo ""
xmlsql -d "$DB" head.html - foot.html << END
<form method=post action="/editgpio.cgi"><input name=devicegpio type=hidden>
<sql table=area where="site=$SESSION_SITE"><IF AREAS><SET AREAS="\$AREAS "></IF><SET AREAS="\$AREAS\$area"></sql>
<sql select="*,pcb.description AS D"  table="devicegpio LEFT JOIN device USING (device) LEFT JOIN pcb USING (pcb) LEFT JOIN pcbgpio ON (pcb.pcb=pcbgpio.pcb and pcbgpio.gpio=devicegpio.gpio)" WHERE="devicegpio=$devicegpio">
<input name=device type=hidden>
<table>
<tr><td>Device</td><td><output name=description blank="Unnamed" missing="Unnamed"></td></tr>
<tr><td>PCB</td><td><output name=D></td></tr>
<tr><td>Type</td><td><output name=type $GPIOTYPEOUT></td></tr>
<tr><td>GPIO</td><td><output name=pinname></td></tr>
<tr><td><label for=invert>Invert</label></td><td><input id=invert name=invert type=checkbox value=true></td></tr>
</table>
<if not type=*P>
<if type=*O><set SL="$STATELIST"></if><if else><set SL="$STATELISTI"></if>
<table>
<tr><th></th><for space STATE="\$SL"><th><output name=STATE></th></for></tr>
<for space A="\$AREAS">
<tr>
<td><output name=A></td>
<for space STATE="\$SL"><td><input type=checkbox name="\$STATE" value="\$A"></td></for>
<td><sql table=area where="site=$SESSION_SITE AND area='\$A'"><output name=description></sql></td>
</for>
</tr>
</for>
</table>
</if>
</sql>
<input type=submit value="Update">
</form>
END
