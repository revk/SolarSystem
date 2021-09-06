#!../login/loggedin /bin/csh -f
unset user # csh
setenv site "$PATH_INFO:t"
can --redirect --site=$site editsite
if($status) exit 0
can --site=$site admin
setenv NOTADMIN $status
if($?DELETE) then
	if(! $?SURE) then
		setenv MSG "Tick to say you are sure"
		goto done
	endif
	setenv C `sql -ct "$DB" 'UPDATE user SET site=NULL WHERE site=$site' 'DELETE FROM site WHERE site=$site'`
	if($status || "$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use"
		goto done
	endif
	../login/redirect / Deleted
	exit 0
endif
if($?sitename) then
	if($site == 0) then
		setenv site `sql -i "$DB" 'INSERT INTO site SET organisation=$USER_ORGANISATION,site=0'`
		sql "$DB" 'INSERT INTO access SET organisation=$USER_ORGANISATION,site="$site",accessname="Default access"'
		sql "$DB" 'INSERT INTO area SET organisation=$USER_ORGANISATION,site=$site,area="A",areaname="Main building"'
		sql "$DB" 'UPDATE session SET organisation=$USER_ORGANISATION,site=$site WHERE session="$ENVCGI"'
		setenv aid `makeaid --site="$site"`
	endif
	if(! $?nomesh) setenv nomesh false
	if(! $?ioteventarm) setenv ioteventarm false
	if(! $?meshlr) setenv meshlr false
	if(! $?debug) setenv debug false
	sqlwrite -qon "$DB" site sitename wifissid wifipass wifichan iothost nomesh smsuser smspass armcancel alarmdelay alarmhold debug iotstatesystem ioteventarm smsarm smsarmfail smsdisarm smsalarm smspanic smsfire engineer smsnumber smsafp1 smsafp2 smsafp3 smsfrom hookbearer hookfob hookalarm hookfire hookpanic meshlr wifibssid root
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
	message --poke
	redirect /
	exit 0
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Manage site</h1>
<form method=post>
<sql table=site key=site>
<table>
<tr><td>Name</td><td><input name=sitename size=40 autofocus></td></tr>
<tr><td>WiFi SSID</td><td><input name=wifissid size=40 value="SolarSystem"></td></tr>
<tr><td>WIFi pass</td><td><input name=wifipass size=40 value="security"></td></tr>
<tr><td>WIFi chan</td><td><select name=wifichan><option value=0>Auto</option><FOR SPACE C="1 2 3 4 5 6 7 8 9 10 11"><option value=$C><output name=C></option></FOR></select>
<input name=wifibssid size=12 maxlength=12 placeholder="HEX BSSID">
</td></tr>
<if not nomesh=true><tr><td>Mesh</td><td>
Root:<select name=root><option value=''>None</option><sql table=device where="site=$site"><option value="$device"><output name=devicename></option></sql></select>
<input name=meshlr type=checkbox value=true id=meshlr title="Long Range WiFi mode"><label for=meshlr>LR</label>
<input type=checkbox id=nomesh name=nomesh value=true><label for=nomesh>Non mesh</label>
</td><tr></if>
<tr><td>IoT MQTT</td><td><input name=iothost size=40></td></tr>
<tr><td>IoT logging</td><td>
<input type=checkbox id=iotstatesystem name=iotstatesystem value=true><label for=iotstatesystem>system</for>
<input type=checkbox id=ioteventarm name=ioteventarm value=true><label for=ioteventarm>arm/trigger/disarm</for>
</td></tr>
<tr><td><input type=checkbox id=debug name=debug value=true></td><td><label for=debug>Debug mode (additional logging).</label></td></tr>
<tr><td>SMS Username</td><td><input name=smsuser size=40></td></tr>
<tr><td>SMS Password</td><td><input name=smspass size=40></td></tr>
<tr><td>SMS Target</td><td><input name=smsnumber size=20 maxlength=20> (Send for sms selected areas as listed below)</td></tr>
<tr><td>SMS Alarm/Fire/Panic</td><td><input name=smsafp1 size=20 maxlength=20> (Sent for any alarm/fire/panic event while on-line)</td></tr>
<tr><td>SMS Alarm/Fire/Panic</td><td><input name=smsafp2 size=20 maxlength=20></td></tr>
<tr><td>SMS Alarm/Fire/Panic</td><td><input name=smsafp3 size=20 maxlength=20></td></tr>
<tr><td>SMS From</td><td><input name=smsfrom size=10 maxlength=10> (if carrier allows)</td></tr>
<tr><td>Arm-Cancel</td><td><input name=armcancel size=3> seconds (timeout before cancelling arming)</td></tr>
<tr><td>Alarm-Delay</td><td><input name=alarmdelay size=3> seconds (timeout before alarm triggers)</td></tr>
<tr><td>Alarm-Hold</td><td><input name=alarmhold size=3> seconds (timeout before alarm cancels after last trigger)</td></tr>
<tr><td colspan=2>Web hooks</td></tr>
<tr><td>Bearer</td><td><input name=hookbearer size=40 placeholder="To send on hooks"></td></tr>
<tr><td>Fob event</td><td><input name=hookfob size=40 placeholder="https://..."></td></tr>
<tr><td>Alarm event</td><td><input name=hookalarm size=40 placeholder="https://..."></td></tr>
<tr><td>Fire event</td><td><input name=hookfire size=40 placeholder="https://..."></td></tr>
<tr><td>Panic event</td><td><input name=hookpanic size=40 placeholder="https://..."></td></tr>
</table>
<p>Area settings</p>
<table border=1>
<set tags="engineer smsarm smsarmfail smsdisarm smsalarm smspanic smsfire">
<tr><th></th>
<for SPACE T="$tags"><th><output name=T></th></for>
<th>Areas</th>
</tr>
<sql table=area where="site=$USER_SITE">
<tr>
<th><output name=tag></th>
<for SPACE T="$tags"><td><input name="$T" type=checkbox value="$tag"></td></for>
<td><output name=areaname></td>
</tr>
</sql>
</table>
</sql>
<input type=submit value="Update">
<IF NOT site=0 USER_ADMIN=true><input type=submit value="Delete" name=DELETE><input type=checkbox name=SURE title='Tick this to say you are sure'></IF>
</form>
</sql>
'END'
