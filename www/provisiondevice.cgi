#!../login/loggedin /bin/csh -f
can --redirect --site='$USER_SITE' admin
if($status) exit 0
if($?PROVISION) then
	if("$deport" != "") then
		setenv MSG `message --pending="$PROVISION" --deport="$deport"`
		if(! $status) then
			redirect / Deported
			exit 0
		endif
	else
		if("$pcb" == 0) then
			setenv MSG "Pick PCB to provision"
			goto done
		endif
		if("$aid" == "") then
			setenv MSG "Pick AID"
		endif
		setenv authenticated `sql "$DB" 'SELECT authenticated FROM pending WHERE pending="$PROVISION"'`
		setenv nfc `sql "$DB" 'SELECT IF(nfctx="-","false","true") FROM pcb WHERE pcb=$pcb'`
		setenv gps `sql "$DB" 'SELECT IF(gpstx="-","false","true") FROM pcb WHERE pcb=$pcb'`
		sql "$DB" 'INSERT INTO device SET device="$PROVISION",pcb="$pcb",organisation="$USER_ORGANISATION",site="$USER_SITE",aid="$aid",nfc="$nfc",gps="$gps",devicename="$devicename",outofservice="true",upgrade=NOW() ON DUPLICATE KEY UPDATE pcb="$pcb",organisation="$USER_ORGANISATION",site="$USER_SITE",aid="$aid",nfc="$nfc",gps="$gps",devicename="$devicename",outofservice="true",upgrade=NOW()'
		sql "$DB" 'DELETE FROM devicegpio WHERE device="$PROVISION"'
		sql "$DB" 'INSERT INTO devicegpio (device,gpio,type,name,hold,pulse,invert,func) SELECT "$PROVISION",gpio,inittype,initname,inithold,initpulse,initinvert,initfunc FROM gpio WHERE pcb=$pcb'
		# device count change
		sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
		message --poke
		if("$authenticated" == "true")then
			setenv MSG `message --pending="$PROVISION" --command="restart"`
		else
			setenv MSG `message --pending="$PROVISION" --provision --pending="$PROVISION" --aid="$aid"`
		endif
		if(! $status) then
			redirect / Provisioned
			exit 0
		endif
	endif
endif
done:
setenv pcb `sql "$DB" 'SELECT pcb FROM device WHERE device="$device"'`
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Provision device for this site</h1>
<form style="display:inline;" method=post>
PCB:<select name=pcb><option value=0>-- Pick PCB --</option><sql table=pcb order=pcbname ><option value=$pcb><output name=pcbname blank="Unnamed"></option></sql></select>
Note that the device will have WiFi set for the selected site (<b><sql table=site where="site='$USER_SITE'"><output name=wifissid></sql></b>).<br>
AID:<select name=aid><sql table=aid WHERE="site=$USER_SITE"><option value=$aid><output name=aidname></option></sql></select><br>
Name:<input name=devicename size=20 autofocus><br>
Deport:<input name=deport size=20 placeholder='MQTT server'><br>
<table border=1>
<sql select="pending.*,device.device AS D,device.online AS O" table="pending LEFT JOIN device ON (pending=device)" order="pending.online" WHERE="pending.online<NOW()"><set found=1>
<tr>
<td>
<input type=submit value="Provision" name="PROVISION" onclick="this.value='$pending';">
</td>
<td><output name=online></td>
<td><output name=pending></td>
<td><output name=address></td>
<td><output name=version></td>
<td><output name=flash></td>
<td>
<if authenticated=true><b>Device is authenticated.</b><br></if>
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
