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
if($?description) then
 if($organisation == 0) then
  setenv organisation `sql -i "$DB" 'INSERT INTO organisation SET organisation=0'`
  sql "$DB" 'INSERT INTO class SET description="Admin",admin="true",organisation=$organisation'
  sql "$DB" 'INSERT INTO class SET description="Staff",organisation=$organisation'
  sql "$DB" 'INSERT INTO class SET description="Contractor",organisation=$organisation'
  sql "$DB" 'INSERT INTO class SET description="Visitor",organisation=$organisation'
  setenv site `sql -i "$DB" 'INSERT INTO site SET description="Main building",organisation=$organisation'`
  sql "$DB" 'INSERT INTO area SET site=$site,area="A",description="Main building"'
  sql "$DB" 'UPDATE session SET organisation=$organisation,site=$site WHERE session="$ENVCGI"'
 endif
 sqlwrite -o "$DB" organisation organisation description
 if($?adduser && $NOTADMIN == 0) then
  if("$adduser" != "") then
   setenv user `sql "$DB" 'SELECT user FROM user WHERE email="$adduser"'`
   if("$user" == NULL || "$user" == "") then
    setenv MSG "Unknown user";
    goto done
   endif
   if("$class" == "0") then
    setenv MSG "Pick a class"
    goto done
   endif
   sql "$DB" 'INSERT INTO userorganisation SET user=$user,organisation=$organisation,class=$class'
  endif
 endif
 echo "Location: ${ENVCGI_SERVER}"
 echo ""
 exit 0
endif
done:
echo "Content-Type: text/html"
echo ""
xmlsql -d "$DB" head.html - foot.html << 'END'
<h1>Organisation</h1>
<form method=post>
<sql table=organisation key=organisation>
<table>
<tr><td>Name</td><td><input name=description size=40 autofocus></td></tr>
<IF NOTADMIN=0 NOT organisation=0>
<tr><td>Add user</td><td><input name=adduser type=email size=40></td></tr>
<tr><td>Class</td><td><select name=class><option value='0'>-- Pick class --</option>
<sql table=class WHERE="organisation=$organisation">
<option value="$class"><output name=description blank="Unspecified"></option>
</sql>
</select></td></tr>
</IF>
</table>
</sql>
<input type=submit value="Update">
<IF NOT organisation=0 USER_ADMIN><input type=submit value="Delete" name=DELETE></IF>
</form>
</sql>
'END'
