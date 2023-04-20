#!../login/loggedin /bin/csh -f
can --redirect --organisation='$USER_ORGANISATION' --site='$USER_SITE' editfob
if($status) exit 0
source ../setcan

if($?DELETE) then
	sql "$DB" 'DELETE FROM fobaid WHERE fob="$fob" AND aid IN (SELECT aid from aid WHERE organisation="$USER_ORGANISATION")'
	sql "$DB" 'DELETE FROM foborganisation WHERE fob="$fob" AND organisation="$USER_ORGANISATION"'
	redirect /
	exit 0
endif

if($?expires) then
	set aids=(`printenv aids|sed 's/[^0-9A-F	]//g'`)
	foreach a ($aids)
		setenv A "$a"
		can --redirect --aid="$A" editfob
		if($status) exit 0
		setenv access `printenv "access$A"`
		if("$access" == "") then
			setenv C `sql -c "$DB" 'DELETE FROM fobaid WHERE fob="$fob" AND aid="$A"'`
		else
			setenv C `sql -c "$DB" 'INSERT IGNORE INTO fobaid SET fob="$fob",aid="$A"'`
			sql "$DB" 'UPDATE fobaid SET access=$access WHERE fob="$fob" AND aid="$A"'
		endif
	end
	setenv organisation "$USER_ORGANISATION"
	setenv allow "fobname fobsms expires"
	if($?BLOCK) setenv blocked "`date +'%F %T'`"
	if($?BLOCK) setenv allow "$allow confirmed"
	if($?UNBLOCK||$?BLOCK) then
		setenv allow "$allow blocked"
		sql "$DB" 'UPDATE device SET device.poke=NOW() WHERE organisation="$USER_ORGANISATION"'
	endif
	setenv expires "${expires}Z"
	if("$expires" == "Z") unsetenv expires
	sqlwrite -qon "$DB" foborganisation fob organisation $allow
	message --poke
	redirect editfob.cgi
	exit 0
endif
setenv XMLSQLDEBUG
if(! $?PATH_INFO) then
list:
setenv XMLSQLDEBUG
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>🔑 Fobs</h1>
<table>
<set last>
<sql table="foborganisation LEFT JOIN fobaid USING (fob) LEFT JOIN aid USING (aid)" group="fob" order="max(lastused) DESC,max(adopted) DESC,fob DESC" where="foborganisation.organisation=$USER_ORGANISATION">
<sql table="foborganisation LEFT JOIN fobaid USING (fob) LEFT JOIN aid USING (aid) LEFT JOIN fob USING (fob) LEFT JOIN site USING (site) LEFT JOIN access USING (access)" where="foborganisation.organisation=$USER_ORGANISATION AND (aid.site IS NULL OR aid.site=$USER_SITE) AND fob.fob='$fob'" order="aid">
<if not found><set found=1><tr>
<th>Fob</th>
<th>Free</th>
<th>Name</th>
<th>SMS</th>
<th>Last</th>
<th>Expiry</th>
<th colspan=2>Notes</th>
</tr></if>
<tr>
<if last="$fob"><td colspan=4></if>
<if else><set last="$fob">
<td><output name=fob href="editfob.cgi/$fob"></td>
<td align=right><if mem><output name=mem></if></td>
<td><output name=fobname></td>
<td><output name=fobsms></td>
</if>
<td align=right><output name=lastused type=recent></td>
<td align=right><output name=expires type=recent></td>
<td>
<if site><output name=sitename>: <output name=aidname> (<output name=accessname missing="?">)</if><if else>No access</if>
</td>
<if ver="$ver2" or ver="$ver3"><td><b>Old key <tt><output name=ver></tt></b></td></if>
<if ver not ver="$ver1" not ver="$ver2" not ver="$ver3"><td><b>Obsolete key <tt><output name=ver></tt></b></td></if>
<if not ver><td><i>Unknown key version</i></td></if>
<if blocked><b>Blocked</b></if>
</tr>
</sql>
</sql>
</table>
<if found><set found></if><if else><p>No fobs found</p></if>
'END'
exit 0
endif

edit:
setenv fob "$PATH_INFO:t"
setenv OK `sql "$DB" 'SELECT COUNT(*) FROM fob WHERE fob="$fob"'`
if("$OK" == 0 || "$OK" == "" || "$OK" == NULL) then
	setenv MSG "Sorry, fob not found, ensure it is provisioned"
	goto list
endif
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>🔑 Fob <output name=fob></h1>
<if fob CANVIEWLOG><p><a href="/log.cgi/$fob">View logs</a></p></if>
<form method=post action=/editfob.cgi><input name=fob type=hidden>
<sql table="foborganisation LEFT JOIN fob using (fob)" where="fob='$fob' AND organisation=$USER_ORGANISATION">
<table>
<tr><td>Name</td><td colspan=2><input name=fobname size=15 maxlength=15 autofocus></td></tr>
<tr><td>Mobile</td><td colspan=2><input name=fobsms size=15 maxlength=15 autofocus></td></tr>
<tr><td>Expiry</td><td colspan=2><input name=expires id=expires type=datetime-local><input type=button onclick='document.getElementById("expires").value="";' value="No expiry"></td></tr>
<if capacity><tr><td>Capacity</td><td><output name=capacity> bytes</td></tr></if>
<if mem><tr><td>Free</td><td><output name=mem> bytes</td></tr></if>
<if blocked><tr><td>Block</td><td>Access blocked <output name=blocked> <if blocked and confirmed>(confirmed <output name=confirmed>)</if></td></tr></if>
<tr><th>Site</th><th>Access</th><th>Adopted</th><th>First in day</th><th>Last used</th><th>AID</th><th>Key</th></tr>
<sql table="fobaid LEFT JOIN aid USING (aid) LEFT JOIN site USING (site)" where="fob='$fob' AND aid.organisation=$USER_ORGANISATION" order=sitename,aidname><set "access$aid"="$access">
<tr>
<td><input type=hidden name=aids value="$aid"><output name=sitename></td>
<td><select name="access$aid"><option value=''>No access</option><sql table=access where="site=$site"><option value="$access"><output name=accessname></option></sql></select></td><td align=right><output name=adopted type=recent></td><td align=right><output name=firstinday type=recent></td><td align=right><output name=lastused type=recent></td><td><output name=aidname></td><td><tt><output name=ver></tt></td>
</tr>
</sql>
</table>
<input type=submit value="Update"><if blocked><input type=SUBMIT name=UNBLOCK Value="Unblock"></if><if else><input type=SUBMIT name=BLOCK Value="Block"></if>
<input type=submit name="DELETE" value="Delete">
</sql>
</form>
'END'


