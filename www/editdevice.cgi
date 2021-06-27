#!../login/loggedin /bin/csh -f
if($?PATH_INFO) then
	setenv device "$PATH_INFO:t"
endif
can --redirect --device='$device' editdevice
if($status) exit 0

if($?DELETE) then
	sql "DB" 'DELETE FROM devicegpio WHERE device=$device'
	setenv C `sql -c "$DB" 'DELETE FROM device WHERE device=$device'`
	if("$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use"
		goto done
	endif
	unsetenv pcb
	setenv MSG Deleted
	goto done
endif
if($?description) then # save
	sqlwrite -o "$DB" device description
	echo "Location: ${ENVCGI_SERVER}/editdevice.cgi"
	echo ""
	exit 0
endif
done:
echo "Content-Type: text/html"
echo ""
source ../pick
xmlsql -d "$DB" head.html - foot.html << END
<h1>Device</h1>
<if not device>
<table>
<sql select="*,pcb.description AS D" table="device LEFT JOIN pcb USING (pcb)" order=device.description WHERE="site=\$SESSION_SITE"><set found=1>
<tr>
<td><output name=device href="/editdevice.cgi/\$device"></td>
<td><if online><output name=online></if><if else>Last online <output name=lastonline missing="never"></if></td>
<td><output name=D></td>
<td><output name=description blank="Unspecified" missing="Unnamed"></td>
</tr>
</sql>
</table>
<if not found><p>No devices found.</p></if>
</if><if else>
<form method=post action=/editdevice.cgi><input type=hidden name=device>
<sql select="*,pcb.description AS D" table="device LEFT JOIN pcb USING (pcb)" WHERE="device='\$device'">
<table>
<tr><td>Name</td><td><input name=description ize=40 autofocus></td></tr>
<tr><td>Last only</td><td><if online><output name=online></if><if else>Last online <output name=lastonline missing="never"></if></td></tr>
<tr><td>PCB</td><td><output name=D></td></tr>
<sql table="device LEFT JOIN devicegpio USING (device) LEFT JOIN pcbgpio ON (device.pcb=pcbgpio.pcb AND devicegpio.gpio=pcbgpio.gpio)" WHERE="device='\$device'">
<tr><td><output name=pinname href="/editgpio.cgi/\$devicegpio"></td>
<td><output name=type></td>
</tr>
</sql>
</table>
<input type=submit value="Update">
<IF not device=0><input type=submit value="Delete" name=DELETE></if>
</sql>
</form>
</if>
END
