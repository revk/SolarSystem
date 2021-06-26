#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' editclass
if($status) exit 0
source ../setcan
# save
if($?description) then
	can --redirect --organisation='$SESSION_ORGANISATION' --class="$class"
	if($status) exit 0
	setenv allow "description"
	if($?ADMINORGANISATION) setenv allow "$allow admin"
	sqlwrite -v -o "$DB" class $allow
endif
echo "Content-Type: text/html"
echo ""
if(! $?PATH_INFO) then
# list classes
xmlsql -d "$DB" head.html - foot.html << 'END'
<table>
<sql table=class WHERE="organisation=$SESSION_ORGANISATION">
<tr>
<td><if admin=true>Admin</if></td>
<td><output name=description blank="Blank" href="/editclass.cgi/$class"></td>
</tr>
</sql>
</table>
'END'
endif
# edit class
setenv class "$PATH_INFO:t"
if("$class" == "0") then # create
	can --organisation='$SESSION_ORGANISATION' admin
	if($status) exit 0
else # edit
	can --organisation='$SESSION_ORGANISATION' --class="$class"
	if($status) exit 0
endif
xmlsql -d "$DB" head.html - foot.html << 'END'
<sql table=class KEY=class>
<form method=post action="/editclass.cgi"><input type=hidden name=class>
<table>
<tr><td>Name</td><td><input name=description size=40 autofocus></td></tr>
<if ADMINORGANISATION><tr><td>Admin</td><td><select name=admin><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>

</table>
<input type=submit value="Update">
</form>
</sql>
'END'
