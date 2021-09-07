#!../login/loggedin /bin/csh -f
unset user # csh
setenv organisation "$PATH_INFO:t"
can --redirect --organisation=$organisation editorganisation
if($status) exit 0
source ../setcan
can --organisation=$organisation admin
setenv NOTADMIN $status
if($?DELETE) then
	if(! $?SURE) then
		setenv MSG "Are you sure?"
		goto done
	endif
	sql "$DB" 'UPDATE session set organisation=NULL,site=NULL WHERE organisation=$organisation'
	setenv C `sql -c "$DB" 'DELETE FROM organisation WHERE organisation=$organisation'`
	if("$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use - you need to delete stuff in organiastion first"
		goto done
	endif
	redirect / Deleted
	exit 0
endif
if($?organisationname) then
	if($organisation == 0) then
		setenv organisation `sql -i "$DB" 'INSERT INTO organisation SET organisation=0'`
		setenv site `sql -i "$DB" 'INSERT INTO site SET sitename="Main building",organisation=$organisation,wifissid="SolarSystem",wifipass="security"'`
		sql "$DB" 'INSERT INTO access SET organisation=$organisation,site="$site",accessname="Default access"'
		sql "$DB" 'INSERT INTO area SET organisation=$organisation,site=$site,tag="A",areaname="Main building"'
		setenv aid `makeaid --site="$site"`
		sql "$DB" 'UPDATE session SET organisation=$organisation,site=$site WHERE session="$ENVCGI"'
		sqlwrite -qon "$DB" organisation organisationname
		redirect "editorganisation.cgi/$organisation"
		exit 0
	endif
	sqlwrite -o "$DB" organisation organisationname
	redirect /
	exit 0
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1><if organisation=0>🆕 </if>🏢 Organisation</h1>
<form method=post>
<sql table=organisation key=organisation>
<table>
<tr><td>Name</td><td><input name=organisationname size=40 autofocus></td></tr>
</table>
</sql>
<input type=submit value="Update">
<IF NOT organisation=0 USER_ADMIN=true><input type=submit value="Delete" name=DELETE><input type=checkbox name=SURE></IF>
</form>
</sql>
'END'
