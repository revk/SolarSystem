#!../login/loggedin /bin/csh -fx
unset user # csh
setenv organisation "$PATH_INFO:t"
can --redirect --organisation=$organisation editorganisation
if($status) exit 0
can --organisation=$organisation admin
setenv NOTADMIN $status
if($?DELETE) then
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
   goto done
  endif
 endif
 echo "Location: ${ENVCGI_SERVER}listorganisation.cgi"
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
<IF not organisation=0>
<h2>Users</h2>
<table>
<sql select="*,class.description AS C" table="userorganisation LEFT JOIN user USING (user) LEFT JOIN class ON userorganisation.class=class.class" where="userorganisation.organisation=$organisation">
<tr><td><output name=C></td><td><output name=description blank="Unnamed"></td></tr>
</sql>
</table>
</if>
</sql>
'END'
