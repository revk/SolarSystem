#!../login/loggedin /bin/csh -f
unset user # csh
setenv organisation "$PATH_INFO:t"
can --redirect --organisation=$organisation editorganisation
if($status) exit 0
can --organisation=$organisation admin
setenv NOTADMIN $status
if($?DELETE) then
 sql "$DB" 'UPDATE session set organisation=NULL,site=NULL WHERE organisation=$organisation'
 setenv C `sql -c "$DB" 'DELETE FROM organisation WHERE organisation=$organisation'`
 if("$C" == "" || "$C" == "0") then
  setenv MSG "Cannot delete as in use"
  goto done
 endif
 echo "Location: ${ENVCGI_SERVER}?MSG=Deleted"
 echo ""
 exit 0
endif
if($?organisationname) then
 if($organisation == 0) then
  setenv organisation `sql -i "$DB" 'INSERT INTO organisation SET organisation=0'`
  setenv site `sql -i "$DB" 'INSERT INTO site SET description="Main building",organisation=$organisation,wifissid="SolarSystem",wifipass="security"'`
  sql "$DB" 'INSERT INTO access SET site="$site",accessname="Default access"'
  sql "$DB" 'INSERT INTO area SET site=$site,tag="A",description="Main building"'
  setenv aid `makeaid --organisation="$organisation"`
  sql "$DB" 'UPDATE session SET organisation=$organisation,site=$site WHERE session="$ENVCGI"'
 endif
 sqlwrite -o "$DB" organisation organisationname
 echo "Location: ${ENVCGI_SERVER}"
 echo ""
 exit 0
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Organisation</h1>
<form method=post>
<sql table=organisation key=organisation>
<table>
<tr><td>Name</td><td><input name=organisationname size=40 autofocus></td></tr>
</table>
</sql>
<input type=submit value="Update">
<IF NOT organisation=0 USER_ADMIN><input type=submit value="Delete" name=DELETE></IF>
</form>
</sql>
'END'
