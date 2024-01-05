#!../login/loggedin /bin/csh -f
can --redirect --site='$USER_SITE' admin
if($status) exit 0
source ../setcan
if(! $?PATH_INFO) then
	redirect /
	exit 0
endif
setenv device "$PATH_INFO:t"
if(`sql "$DB" 'SELECT count(*) FROM pending WHERE pending="$device"'` == 0 && `sql "$DB" 'SELECT count(*) FROM device WHERE device="$device"'` == 0) then
	redirect / "Device $device not online"
	exit 0
endif

if($?DEPORT) then
	setenv MSG `message --pending="$device" --deport="$deport"`
	if(! $status) then
		redirect / Deported
		exit 0
	endif
endif
if($?PROVISION) then
	if("$pcb" == 0) then
		setenv MSG "Pick PCB to provision"
		goto done
	endif
	if("$aid" == "") then
		setenv MSG "Pick AID"
	endif
	setenv authenticated `sql "$DB" 'SELECT authenticated FROM pending WHERE pending="$device"'`

	../sstool --provision-device="$device" --aid="$aid" --device-name="$devicename" --copy-device="$copydevice" --pcb="$pcb"

	message --poke
	if("$authenticated" == "true") then
		setenv MSG `message --pending="$device" --command="restart"`
	else
		setenv MSG `message --pending="$device" --provision --pending="$device" --aid="$aid"`
	endif
	if(! $status) then
		redirect / Provisioned
		exit 0
	endif
endif
done:
setenv pcb `sql "$DB" 'SELECT pcb FROM device WHERE device="$device"'`
setenv devicename `sql "$DB" 'SELECT devicename FROM device WHERE device="$device"'`
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Provision <output name=device> for this site</h1>
<p>Note that the device will have WiFi set for the selected site (<b><sql table=site where="site='$USER_SITE'"><output name=wifissid></sql></b>).</p>
<form style="display:inline;" method=post>
<table>
<tr><td>PCB</td><td><select name=pcb><option value=0>-- Pick PCB --</option><sql table=pcb order=pcbname ><option value=$pcb><output name=pcbname blank="Unnamed"></option></sql></select></td></tr>
<tr><td>Copy</td><td><select name=copydevice><option value=''>Copy device</option><sql table="device LEFT JOIN pcb USING (pcb)" WHERE="site=$USER_SITE"><option value=$device><output name=pcbname>: <output name=devicename></option></sql></select></td></tr>
<tr><td>AID</td><td><select name=aid><sql table=aid WHERE="site=$USER_SITE"><option value=$aid><output name=aidname></option></sql></select></td></tr>
<tr><td>Name</td><td><input name=devicename size=20 autofocus></td></tr>
<tr><td><input type=submit name=PROVISION value="Provision"></td></tr>
</table>
<p>Alternatively this device can be deported to another server.</p>
<table>
<tr><td>Deport</td><td><input name=deport size=20 placeholder='MQTT server'></td></tr>
<tr><td><input type=submit name=DEPORT value="Deport"></td></tr>
</table>
</form>
'END'
