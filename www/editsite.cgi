#!../login/loggedin /bin/csh -f
unset user # csh
if(! $?PATH_INFO) setenv PATH_INFO /$USER_SITE
setenv site "$PATH_INFO:t"
can --redirect --site=$site editsite
if($status) exit 0
source ../setcan
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
		sql "$DB" 'UPDATE user SET organisation=$USER_ORGANISATION,site=$site WHERE user="$USER_ID"'
		setenv aid `makeaid --site="$site"`
	endif
	if(! $?nomesh) setenv nomesh false
	if(! $?iotstatesystem) setenv iotstatesystem false
	if(! $?ioteventarm) setenv ioteventarm false
	if(! $?meshlr) setenv meshlr false
	if(! $?debug) setenv debug false
	if($?root) then
		if("$root" == "") unsetenv root
	endif
	sqlwrite -qon "$DB" site sitename wifissid wifipass wifichan iottopic iothost nomesh smsuser smspass smsfrom sms2user sms2pass sms2from toothost tootbearer armcancel armdelay alarmdelay alarmhold debug rollover iotstatesystem ioteventarm smsarm smsarmfail smsdisarm smscancel smsalarm smspanic smsfire engineer smsnumber hookoffline hookfob hookfobdeny hookalarm hookfire hookpanic hookwarning hooktamper hookfault hooktrigger hookinhibit hookarm hookstrong hookdisarm hookarmfail hookopen hookforced hooknotclosed hooknotopen hookpropped hookwrongpin meshlr wifibssid root hookoffline emailfrom hookarm hookstrong hookdisarm hookopen hookarmfail hookforced hookpropped hookwrongpin
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
	message --poke
	redirect /
	exit 0
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1><if site=0>🆕 </if>🏘 Manage site</h1>
<form method=post>
<sql table=site key=site>
<table>
<tr><td>Name</td><td><input name=sitename size=40 autofocus></td></tr>
<tr><td>WiFi SSID</td><td><input name=wifissid size=40 value="SolarSystem"></td></tr>
<tr><td>WIFi pass</td><td><input name=wifipass size=40 value="security"></td></tr>
<tr><td>WIFi chan</td><td><select name=wifichan><option value=0>Auto</option><FOR SPACE C="1 2 3 4 5 6 7 8 9 10 11 12 13"><option value=$C><output name=C></option></FOR></select>
<input name=wifibssid size=12 maxlength=12 placeholder="HEX BSSID">
</td></tr>
<tr><td>Mesh</td><td>
<if not nomesh=true>
Root:<select name=root><option value=''>None</option><sql table=device where="site=$site"><option value="$device"><output name=devicename></option></sql></select>
<input name=meshlr type=checkbox value=true id=meshlr title="Long Range WiFi mode"><label for=meshlr>LR</label>
</if>
<input type=checkbox id=nomesh name=nomesh value=true><label for=nomesh>Non mesh</label>
</td><tr>
<tr><td>IoT MQTT</td><td><input name=iothost size=40></td></tr>
<tr><td>IoT Topic</td><td><input name=iottopic size=16> (for state/SS/.../system messages)</td></tr>
<tr><td>IoT logging</td><td>
<input type=checkbox id=iotstatesystem name=iotstatesystem value=true><label for=iotstatesystem>system</for>
<input type=checkbox id=ioteventarm name=ioteventarm value=true><label for=ioteventarm>arm/trigger/disarm</for>
</td></tr>
<tr><td>Auto-Rollover</td><td><input name=rollover size=3> days (before automatic key rollover)</td></tr>
<tr><td><input type=checkbox id=debug name=debug value=true></td><td><label for=debug>Debug mode (additional logging).</label></td></tr>
<tr><td>Arm-Cancel</td><td><input name=armcancel size=3> seconds (timeout before cancelling arming)</td></tr>
<tr><td>Arm-Delay</td><td><input name=armdelay size=3> seconds (timeout before arm happens)</td></tr>
<tr><td>Alarm-Delay</td><td><input name=alarmdelay size=3> seconds (timeout before alarm triggers)</td></tr>
<tr><td>Alarm-Hold</td><td><input name=alarmhold size=3> seconds (timeout before alarm cancels after last trigger)</td></tr>
<tr><td colspan=2><hr></td></tr>
<tr><td colspan=2>Event hook settings</td></tr>
<tr><td>Bearer</td><td><input name=hookbearer size=40 placeholder="To send on web hooks"> (for web hooks)</td></tr>
<tr><td>Email From</td><td><input name=emailfrom size=40> (for email hooks)</td></tr>
<tr><td>SMS Username</td><td><input name=smsuser size=40> (for sms hooks)</td></tr>
<tr><td>SMS Password</td><td><input name=smspass size=40></td></tr>
<tr><td>SMS From</td><td><input name=smsfrom size=20 maxlength=20> (if carrier allows)</td></tr>
<tr><td>SMS Username</td><td><input name=sms2user size=40> (for sms hooks, number starting *)</td></tr>
<tr><td>SMS Password</td><td><input name=sms2pass size=40></td></tr>
<tr><td>SMS From</td><td><input name=sms2from size=20 maxlength=20> (if carrier allows)</td></tr>
<tr><td>Toot server</td><td><input name=toothost size=50> Mastodon host</td></tr>
<tr><td>Toot bearer</td><td><input name=tootbearer size=50> Authentication</td></tr>
<tr><td colspan=2><hr></td></tr>
<tr><td colspan=2>Event hooks (can be https:// for JSON post, email address, mobile number for SMS, * prefix mobile number for secondary SMS (e.g. set special alert tone for Alarm, etc)</td></tr>
<tr><td>Offline event</td><td><input name=hookoffline size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Fob event</td><td><input name=hookfob size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Fob deny event</td><td><input name=hookfobdeny size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Alarm event</td><td><input name=hookalarm size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Fire event</td><td><input name=hookfire size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Panic event</td><td><input name=hookpanic size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Warning event</td><td><input name=hookwarning size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Tamper event</td><td><input name=hooktamper size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Fault event</td><td><input name=hookfault size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Trigger event</td><td><input name=hooktrigger size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Inhibit event</td><td><input name=hookinhibit size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Arm event</td><td><input name=hookarm size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Strong arm event</td><td><input name=hookstrong size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Disarm event</td><td><input name=hookdisarm size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Arm fail event</td><td><input name=hookarmfail size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Open event</td><td><input name=hookopen size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Forced event</td><td><input name=hookforced size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Not open event</td><td><input name=hooknotopen size=80 placeholder="https://... / email / number"></td></tr>
<tr><td>Not closed event</td><td><input name=hooknotclosed size=80 placeholder="https://... / email / number"> (i.e. unauthorised propped)</td></tr>
<tr><td>Propped event</td><td><input name=hookpropped size=80 placeholder="https://... / email / number"> (i.e. authorised propped)</td></tr>
<tr><td>Wrong PIN event</td><td><input name=hookwrongpin size=80 placeholder="https://... / email / number"> (on keypad)</td></tr>
<tr><td colspan=2><hr></td></tr>
<tr><td>SMS Target</td><td><input name=smsnumber size=20 maxlength=20> (Send for sms selected areas as listed below)</td></tr>
</table>
<table>
<set tags="engineer smsarm smsarmfail smsdisarm smscancel smsalarm smspanic smsfire">
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
