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
	if($?CANEDITORGANISATION) setenv allow "$allow caneditorganisation"
	if($?CANEDITCLASS) setenv allow "$allow caneditclass"
	if($?CANEDITSITE) setenv allow "$allow caneditsite"
	if($?CANEDITDEVICE) setenv allow "$allow caneditdevice"
	if($?CANEDITFOB) setenv allow "$allow caneditfob"
	if($?CANEDITUSER) setenv allow "$allow canedituser"
	if($?CANEDITAREA) setenv allow "$allow caneditarea"
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
<if CANEDITORGANISATION><tr><td>Can edit organisation</td><td><select name=caneditorganisation><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
<if CANEDITCLASS><tr><td>Can edit class</td><td><select name=caneditclass><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
<if CANEDITSITE><tr><td>Can edit site</td><td><select name=caneditsite><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
<if CANEDITDEVICE><tr><td>Can edit device</td><td><select name=caneditdevice><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
<if CANEDITFOB><tr><td>Can edit fob</td><td><select name=caneditfob><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
<if CANEDITUSER><tr><td>Can edit user</td><td><select name=canedituser><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
<if CANEDITAREA><tr><td>Can edit area</td><td><select name=caneditarea><option value=false>No</option><option value=true>Yes</option></select></td></tr></if>
</table>
<input type=submit value="Update">
</form>
</sql>
'END'
