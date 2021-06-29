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
	echo "Location: ${ENVCGI_SERVER}?MSG=Deleted"
	echo ""
	exit 0
endif
if($?sitename) then
	if($site == 0) then
		setenv site `sql -i "$DB" 'INSERT INTO site SET organisation=$SESSION_ORGANISATION,site=0'`
		sql "$DB" 'INSERT INTO access SET site="$site",accessname="Default access"'
		sql "$DB" 'INSERT INTO area SET site=$site,area="A",areaname="Main building"'
		sql "$DB" 'UPDATE session SET organisation=$SESSION_ORGANISATION,site=$site WHERE session="$ENVCGI"'
	endif
	sqlwrite -o "$DB" site sitename wifissid wifipass
	echo "Location: ${ENVCGI_SERVER}"
	echo ""
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
</table>
</sql>
<input type=submit value="Update">
<IF NOT site=0 USER_ADMIN><input type=submit value="Delete" name=DELETE><input type=checkbox name=SURE title='Tick this to say you are sure'></IF>
</form>
</sql>
'END'
