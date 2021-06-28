#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' editclass
if($status) exit 0
source ../setcan
# save
if($?description) then
	can --redirect --organisation='$SESSION_ORGANISATION' --class="$class"
	if($status) exit 0
	setenv allow "description"
	if(! $?admin) setenv admin false
	if(! $?caneditorganisation) setenv caneditorganisation false
	if(! $?caneditclass) setenv caneditclass false
	if(! $?caneditsite) setenv caneditsite false
	if(! $?caneditdevice) setenv caneditdevice false
	if(! $?caneditfob) setenv caneditfob false
	if(! $?canedituser) setenv canedituser false
	if(! $?caneditarea) setenv caneditarea false
	if($?ADMINORGANISATION) setenv allow "$allow admin"
	if($?CANEDITORGANISATION) setenv allow "$allow caneditorganisation"
	if($?CANEDITCLASS) setenv allow "$allow caneditclass"
	if($?CANEDITSITE) setenv allow "$allow caneditsite"
	if($?CANEDITDEVICE) setenv allow "$allow caneditdevice"
	if($?CANEDITFOB) setenv allow "$allow caneditfob"
	if($?CANEDITUSER) setenv allow "$allow canedituser"
	if($?CANEDITAREA) setenv allow "$allow caneditarea"
	sqlwrite -o "$DB" class $allow
endif
if(! $?PATH_INFO) then
# list classes
xmlsql -C -d "$DB" head.html - foot.html << 'END'
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
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<sql table=class KEY=class>
<form method=post action="/editclass.cgi"><input type=hidden name=class>
<table>
<tr><td>Name</td><td><input name=description size=40 autofocus></td></tr>
<if ADMINORGANISATION><tr><td><input type=checkbox id=admin name=admin value=true></td><td><label for=admin>Admin for organisation</lable></td></tr></if>
<if CANEDITORGANISATION><tr><td><input type=checkbox id=caneditorganisation name=caneditorganisation value=true></td><td><label for=caneditorganisation>Can edit organisation</lable></td></tr></if>
<if CANEDITCLASS><tr><td><input type=checkbox id=caneditclass name=caneditclass value=true></td><td><label for=caneditclass>Can edit class</lable></td></tr></if>
<if CANEDITSITE><tr><td><input type=checkbox id=caneditsite name=caneditsite value=true></td><td><label for=caneditsite>Can edit site</lable></td></tr></if>
<if CANEDITDEVICE><tr><td><input type=checkbox id=caneditdevice name=caneditdevice value=true></td><td><label for=caneditdevice>Can edit device</lable></td></tr></if>
<if CANEDITFOB><tr><td><input type=checkbox id=caneditfob name=caneditfob value=true></td><td><label for=caneditfob>Can edit fob</lable></td></tr></if>
<if CANEDITUSER><tr><td><input type=checkbox id=canedituser name=canedituser value=true></td><td><label for=canedituser>Can edit user</lable></td></tr></if>
<if CANEDITAREA><tr><td><input type=checkbox id=caneditarea name=caneditarea value=true></td><td><label for=caneditarea>Can edit area</lable></td></tr></if>
</table>
<input type=submit value="Update">
</form>
</sql>
'END'
