#!../login/loggedin /bin/csh -f
can --redirect --site='$SESSION_SITE' admin
if($status) exit 0
if($?pending) then
	if("$pcb" == 0) then
		setenv MSG "Pick PCB to provision"
		goto done
	endif
	setenv p `sql "$DB" 'SELECT pcb FROM device WHERE device="$pending"'`
	if("$p" != "") then
	 	if("$p" != "$pcb") then
			sql "$DB" 'UPDATE device SET pcb="$pcb" WHERE device="$pending"'
			sql "$DB" 'DELETE FROM devicegpio WHERE device="$pending"'
			sql "$DB" 'INSERT INTO devicegpio (device,gpio,type) SELECT "$pending",gpio,init FROM pcbgpio WHERE pcb=$pcb AND init<>"-"'
		endif
	else
		sql "$DB" 'INSERT INTO device SET device="$pending"'
	endif
	setenv i `sql "$DB" 'SELECT instance FROM pending WHERE pending="$pending"'`
	setenv MSG "Provisioning..."
	message --instance="$i" --provision="$pending"
	if($status) setenv MSG "Failed"
endif
done:
echo "Content-Type: text/html"
echo ""
xmlsql -d "$DB" head.html - foot.html << 'END'
<h1>Provision device</h1>
<table border=1>
<sql table="pending LEFT JOIN device ON (pending=device)" order="pending.online"><set found=1>
<tr>
<td><form style="display:inline;" method=post>
<input type=hidden name=pending>
<select name=pcb><option value=0>-- Pick PCB --</option><sql table=pcb order=description><option value=$pcb><output name=description></option></sql></select>
<input type=submit value="Provision"></form></td>
<td><output name=online></td>
<td><output name=pending></td>
<td><output name=address></td>
<td><output name=version></td>
<td><output name=flash></td>
<td><if secureboot=false><b>Not secure boot. </b></if><if encryptednvs=false><b>Not encrypted NVS. </b></if><if device><b>Device previously in use. </b></if></td>
</tr>
</sql>
</table>
<if not found><p>No pending device</p></if>
'END'
