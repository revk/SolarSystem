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
	sql "$DB" 'UPDATE session set site=NULL WHERE site=$site'
	setenv C `sql -c "$DB" 'DELETE FROM site WHERE site=$site'`
	if("$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use"
		goto done
	endif
	../login/redirect / Deleted
	exit 0
endif
if($?sitename) then
	if($site == 0) then
		setenv site `sql -i "$DB" 'INSERT INTO site SET organisation=$SESSION_ORGANISATION,site=0'`
		sql "$DB" 'INSERT INTO access SET organisation=$SESSION_ORGANISATION,site="$site",accessname="Default access"'
		sql "$DB" 'INSERT INTO area SET organisation=$SESSION_ORGANISATION,site=$site,area="A",areaname="Main building"'
		sql "$DB" 'UPDATE session SET organisation=$SESSION_ORGANISATION,site=$site WHERE session="$ENVCGI"'
	endif
	if(! $?nomesh) setenv nomesh false
	if(! $?ioteventarm) setenv ioteventarm false
	sqlwrite -qon "$DB" site sitename wifissid wifipass iothost nomesh smsuser smspass armcancel alarmdelay alarmhold ioteventarm
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
	message --poke
	../login/redirect /
	exit 0
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Site</h1>
<form method=post>
<sql table=site key=site>
<table>
<tr><td>Name</td><td><input name=sitename size=40 autofocus></td></tr>
<tr><td>WiFi SSID</td><td><input name=wifissid size=40 value="SolarSystem"></td></tr>
<tr><td>WIFi pass</td><td><input name=wifipass size=40 value="security"></td></tr>
<tr><td><input type=checkbox id=nomesh name=nomesh value=true></td><td><label for=nomesh>Don't use mesh wifi on site (i.e. only access control).</label></td></tr>
<tr><td>IoT MQTT</td><td><input name=iothost size=40></td></tr>
<tr><td><input type=checkbox id=ioteventarm name=ioteventarm value=true></td><td><label for=ioteventarm>Log arm/disarm events to IoT.</label></td></tr>
<tr><td>SMS Username</td><td><input name=smsuser size=40></td></tr>
<tr><td>SMS Password</td><td><input name=smspass size=40></td></tr>
<tr><td>Arm-Cancel</td><td><input name=armcancel size=3> seconds (timeout before cancelling arming)</td></tr>
<tr><td>Alarm-Delay</td><td><input name=alarmdelay size=3> seconds (timeout before alarm triggers)</td></tr>
<tr><td>Alarm-Hold</td><td><input name=alarmhold size=3> seconds (timeout before alarm cancels after last trigger)</td></tr>
</table>
</sql>
<input type=submit value="Update">
<IF NOT site=0 USER_ADMIN=true><input type=submit value="Delete" name=DELETE><input type=checkbox name=SURE title='Tick this to say you are sure'></IF>
</form>
</sql>
'END'
