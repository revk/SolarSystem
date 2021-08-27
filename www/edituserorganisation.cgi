#!../login/loggedin /bin/csh -f
unset user # clash
can --redirect --organisation='$USER_ORGANISATION' edituser
if($status) exit 0
source ../setcan
# save
if($?userorganisationname) then
	if($status) exit 0
	setenv allow "userorganisationname"
	if(! $?admin) setenv admin false
	if(! $?caneditorganisation) setenv caneditorganisation false
	if(! $?caneditaccess) setenv caneditaccess false
	if(! $?caneditsite) setenv caneditsite false
	if(! $?caneditdevice) setenv caneditdevice false
	if(! $?caneditfob) setenv caneditfob false
	if(! $?canadoptfob) setenv canadoptfob false
	if(! $?canedituser) setenv canedituser false
	if(! $?caneditarea) setenv caneditarea false
	if(! $?canarm) setenv canarm false
	if(! $?candisarm) setenv candisarm false
	if(! $?canunlock) setenv canunlock false
	if(! $?canviewlog) setenv canviewlog false
	if(! $?canapi) setenv canapi false
	if(! $?apiexpires) setenv apiexpires false
	if($?ADMINORGANISATION) setenv allow "$allow admin"
	if($?CANEDITORGANISATION) setenv allow "$allow caneditorganisation"
	if($?CANEDITACCESS) setenv allow "$allow caneditaccess"
	if($?CANEDITSITE) setenv allow "$allow caneditsite"
	if($?CANEDITDEVICE) setenv allow "$allow caneditdevice"
	if($?CANEDITFOB) setenv allow "$allow caneditfob"
	if($?CANADOPTFOB) setenv allow "$allow canadoptfob"
	if($?CANEDITUSER) setenv allow "$allow canedituser"
	if($?CANEDITAREA) setenv allow "$allow caneditarea"
	if($?CANARM) setenv allow "$allow canarm"
	if($?CANDISARM) setenv allow "$allow candisarm"
	if($?CANUNLOCK) setenv allow "$allow canunlock"
	if($?CANVIEWLOG) setenv allow "$allow canviewlog"
	if($?CANAPI) setenv allow "$allow canapi apiexpires"
	sqlwrite -qon "$DB" userorganisation $allow
endif
if($?DELETE) then
	if(! $?SURE) then
		setenv MSG "Click to say you are sure"
		goto edit
	endif
	sql "$DB" 'DELETE FROM userorganisation WHERE user=$user AND organisation=$USER_ORGANISATION'
endif
if($?NEW) then
	setenv user `sql "$DB" 'SELECT user FROM user WHERE email="$NEW"'`
	if("$user" == "" || "$user" == NULL) then
		setenv MSG "User not found, have them register first"
		goto list
	endif
	sql "$DB" 'INSERT INTO userorganisation SET organisation=$USER_ORGANISATION,user=$user'
	redirect "edituserorganisation.cgi/$user"
	exit 0
endif
list:
if(! $?PATH_INFO) then
setenv XMLSQLDEBUG
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Manage users</h1>
<form method=post style='inline'>
<table>
<sql select="*,user.admin AS A" table="userorganisation LEFT JOIN user USING (user)" WHERE="userorganisation.organisation=$USER_ORGANISATION"><set found=1>
<tr>
<td>
<if A=true><b>System Admin</b></if>
<if admin=true>Admin</if>
<if canapi=true>API</if>
</td>
<td><output name=email href="/edituserorganisation.cgi/$user"></td>
<td><output name=username></td>
<td><output name=userorganisationname></td>
</tr>
</sql>
<tr>
<td><input type=submit value="Add user"></td>
<td colspan=3><input name=NEW type=email size=40 placeholder="Email address of user" autofocus></td>
</tr>
</table>
</form>
<if not found><p>No users found</p></if>
'END'
exit 0
endif
# edit class
setenv user "$PATH_INFO:t"
edit:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Edit user</h1>
<sql table="userorganisation LEFT JOIN user USING (user)"  where="user=$user AND userorganisation.organisation=$USER_ORGANISATION">
<form method=post action="/edituserorganisation.cgi"><input type=hidden name=user><input type=hidden name=organisation>
<table>
<tr><td>Name</td><td><output name=username></td></tr>
<tr><td>Email</td><td><output name=email></td></tr>
<tr><td>Job title</td><td><input name=userorganisationname size=40 autofocus></td></tr>
<if ADMINORGANISATION><tr><td><input type=checkbox id=admin name=admin value=true></td><td><label for=admin>Admin for organisation (can do all of these)</label></td></tr></if>
<if CANEDITORGANISATION><tr><td><input type=checkbox id=caneditorganisation name=caneditorganisation value=true></td><td><label for=caneditorganisation>Can edit organisation</label></td></tr></if>
<if CANEDITACCESS><tr><td><input type=checkbox id=caneditaccess name=caneditaccess value=true></td><td><label for=caneditaccess>Can edit access</label></td></tr></if>
<if CANEDITSITE><tr><td><input type=checkbox id=caneditsite name=caneditsite value=true></td><td><label for=caneditsite>Can edit site</label></td></tr></if>
<if CANEDITDEVICE><tr><td><input type=checkbox id=caneditdevice name=caneditdevice value=true></td><td><label for=caneditdevice>Can edit device</label></td></tr></if>
<if CANEDITFOB><tr><td><input type=checkbox id=caneditfob name=caneditfob value=true></td><td><label for=caneditfob>Can edit fob</label></td></tr></if>
<if CANADOPTFOB><tr><td><input type=checkbox id=canadoptfob name=canadoptfob value=true></td><td><label for=canadoptfob>Can adopt fob</label></td></tr></if>
<if CANEDITUSER><tr><td><input type=checkbox id=canedituser name=canedituser value=true></td><td><label for=canedituser>Can edit user</label></td></tr></if>
<if CANEDITAREA><tr><td><input type=checkbox id=caneditarea name=caneditarea value=true></td><td><label for=caneditarea>Can edit area</label></td></tr></if>
<if CANARM><tr><td><input type=checkbox id=canarm name=canarm value=true></td><td><label for=canarm>Can arm</label></td></tr></if>
<if CANDISARM><tr><td><input type=checkbox id=candisarm name=candisarm value=true></td><td><label for=candisarm>Can disarm</label></td></tr></if>
<if CANUNLOCK><tr><td><input type=checkbox id=canunlock name=canunlock value=true></td><td><label for=canunlock>Can unlock</label></td></tr></if>
<if CANVIEWLOG><tr><td><input type=checkbox id=canviewlog name=canviewlog value=true></td><td><label for=canviewlog>Can view logs</label></td></tr></if>
<if CANAPI><tr><td><input type=checkbox id=canapi name=canapi value=true></td><td><label for=canapi>Can access API</label></td></tr></if>
<if CANAPI canapi=true><tr><td><input type=checkbox id=apiexpires name=apiexpires value=true></td><td><label for=apiexpires>Can API set fob expires</label></td></tr></if>
</table>
<input type=submit value="Update">
<input type=submit name=DELETE value="Delete"><input type=checkbox name=SURE>
</form>
</sql>
'END'
