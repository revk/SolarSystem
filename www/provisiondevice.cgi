#!../login/loggedin /bin/csh -f
can --redirect --site='$SESSION_SITE' admin
if($status) exit 0
if($?PROVISION) then
	if("$pcb" == 0) then
		setenv MSG "Pick PCB to provision"
		goto done
	endif
	if("$aid" == "") then
		setenv MSG "Pick AID"
	endif
	setenv authenticated `sql "$DB" 'SELECT authenticated FROM pending WHERE pending="$PROVISION"'`
	setenv nfc `sql "$DB" 'SELECT IF(nfctx="-","false","true") FROM pcb WHERE pcb=$pcb'`
	sql "$DB" 'DELETE FROM devicegpio WHERE device="$PROVISION"'
	sql "$DB" 'DELETE FROM device WHERE device="$PROVISION"'
	sql "$DB" 'INSERT INTO device SET device="$PROVISION",pcb="$pcb",organisation="$SESSION_ORGANISATION",site="$SESSION_SITE",aid="$aid",nfc="$nfc"'
	sql "$DB" 'INSERT INTO devicegpio (device,gpio,type) SELECT "$PROVISION",gpio,init FROM gpio WHERE pcb=$pcb AND init<>"-"'
	if("$authenticated" == "true")then
		setenv MSG `message --pending="$PROVISION" --command="restart"`
	else
		setenv MSG `message --pending="$PROVISION" --provision="$PROVISION"`
	endif
	if(! $status) then
		echo "Location: ${ENVCGI_SERVER}?MSG=Provisioned"
		echo ""
		exit 0
	endif
endif
if($?DEPORT) then
	if("$deport" == "") then
		setenv MSG "Set deport MQTT server"
		goto done
	endif
	setenv MG `message --pending="$DEPORT" --deport="$deport"`
	if(! $status) then
		echo "Location: ${ENVCGI_SERVER}?MSG=Deported"
		echo ""
		exit 0
	endif
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Provision device</h1>
<form style="display:inline;" method=post>
PCB:<select name=pcb><option value=0>-- Pick PCB --</option><sql table=pcb order=description><option value=$pcb><output name=description></option></sql></select><br>
AID:<select name=aid><sql table=aid WHERE="site=$SESSION_SITE"><option value=$aid><output name=description></option></sql></select><br>
Deport:<input name=deport size=20 placeholder='MQTT server' autofocus><br>
<table border=1>
<sql select="pending.*,device.device AS D,device.online AS O" table="pending LEFT JOIN device ON (pending=device)" order="pending.online" WHERE="pending.online<NOW()"><set found=1>
<tr>
<td>
<if O><input type=submit value="Assign" name="PROVISION" onclick="this.value='$pending';"><br></if>
<if NOT O><input type=submit value="Provision" name="PROVISION" onclick="this.value='$pending';"><br></if>
<input type=submit value="Deport" name="DEPORT" onclick="this.value='$pending';">
</td>
<td><output name=online></td>
<td><output name=pending></td>
<td><output name=address></td>
<td><output name=version></td>
<td><output name=flash></td>
<td>
<if authenticated><b>Device is authenticated.</b><br></if>
<if secureboot=false><b>Not secure boot.</b><br></if>
<if encryptednvs=false><b>Not encrypted NVS.</b><br></if>
<if O><b>DEVICE IS ON LINE AS AUTHENTICATED DEVICE</b><br></if>
<if D NOT O><b>Device previously in use - will be replaced.</b><br></if>
</td>
</tr>
</sql>
</table>
</form>
<if not found><p>No pending device</p></if>
'END'
