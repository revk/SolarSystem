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
	setenv allow "fobname expires"
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

if(! $?PATH_INFO) then
list:
setenv XMLSQLDEBUG
echo "Content-encoding: none" # so no deflating and hence no caching for interactive status
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<if CANADOPTFOB>
<sql table="device LEFT JOIN aid USING (aid)" where="device.site=$USER_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set found=1></sql>
<if found>
<h2>Adopting a fob</h1>
<form name=f method=post style="display:inline;">
<select name=device>
<sql table="device LEFT JOIN aid USING (aid)" where="device.site=$USER_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')">
<option value="$device"><output name=aidname>:<output name=devicename blank="$device"></option>
</sql>
</select>
<input type=submit name=IDENTIFY value="Read fob ID">
<input type=hidden name=fob>
<if not IDENTIFY fob not fob="">
<set mem><set capacity><sql table=fob where="fob='$fob'"><set foundfob=1><if mem><set mem="$mem"></if><if capacity><set capacity="$capacity"></if></sql>
<if not foundfob><p>Sorry, fob <output name=fob> is not recognised by the system. Any fob you wish to use must first be provisioned by the system administrator before it can be adopted. Please contact the system administrator <sql table=user where="admin='true'" limit=1 order="rand()"><output name=username></sql> for more information.</p></if>
<if else>
<h2>Fob <output name=fob href="editfob.cgi/$fob"></h2>
<if IDENTIFIED><sql table=foborganisation where="fob='$fob' AND organisation=$USER_ORGANISATION"><set fobname="$fobname"></sql></if>
<sql table="fobaid LEFT JOIN aid USING (aid) LEFT JOIN access USING (access)" WHERE="fob='$fob' AND organisation=$USER_ORGANISATION">
<p>Already adopted for <output name=aidname> <output name=accessname></p>
</sql>
<input name=fobname size=15 maxlength=15 placeholder="Name" autofocus><if mem> <output name=mem type=mebi>B free</if><br>
<select name=aid><sql table=aid where='site=$USER_SITE'><option value=$aid><output name=aidname></option></sql></select>
<select name=access><sql table=access where='site=$USER_SITE'><option value=$access><output name=accessname></option></sql></select>
<input name=ADOPT type=submit value="Adopt fob">
<if USER_ADMIN=true><input name=FORMAT type=submit value="Format"></if>
</if>
</if>
<if device><ul id=status></ul></if>
</form>
</if>
</if>

<h2>Fobs</h2>
<table>
<sql table="foborganisation LEFT JOIN fobaid USING (fob) LEFT JOIN aid USING (aid) LEFT JOIN access USING (access)" where="foborganisation.organisation=$USER_ORGANISATION AND aid.organisation=$USER_ORGANISATION" group="fob" order="max(adopted) DESC" select="*,count(aid) as N, sum(if(adopted IS NULL AND aid.aid IS NOT NULL,1,0)) AS W,sum(if(override='true',1,0)) AS O">
<if not found><set found=1><tr>
<th>Fob</th><th>Expiry</th><th>Name</th><th>Notes</th>
</tr></if>
<tr>
<td><output name=fob href="editfob.cgi/$fob"></td>
<td><output name=expires></td>
<td><output name=fobname></td>
<td><if not N=1><output name=N 0=No> AIDs</if><if N=1><sql table=site where="site=$site"><output name=sitename></sql>: <output name=aidname> (<output name=accessname>)</if></td>
</tr>
</sql>
</table>
<if found><set found></if><if else><p>No fobs found</p></if>
'END'

if($?IDENTIFY) then
	can --redirect --organisation='$USER_ORGANISATION' adoptfob
	if($status) exit 0
	echo "<script>e=document.createElement('li');e.textContent='Waiting for fob';document.getElementById('status').append(e);</script>"
	setenv fob `message --device="$device" --fob-identify --silent`
	if("$fob" == "") then
		echo "<script>e=document.createElement('li');e.textContent='Failed';document.getElementById('status').append(e);</script>"
	else
		echo "<script>f.fob.value='$fob';e=document.createElement('input');e.type='hidden';e.name='IDENTIFIED';f.append(e);f.submit();</script>"
	endif
	exit 0
endif
if($?ADOPT) then
	can --redirect --organisation='$USER_ORGANISATION' adoptfob
	if($status) exit 0
	message --device="$device" --fob-adopt="$fob" --aid="$aid" --access="$access" --status=status --silent --fob-name="$fobname" --organisation="$USER_ORGANISATION"
	exit 0
endif
if($?FORMAT) then
	can --redirect admin
	if($status) exit 0
	if($?hardformat) then
		message --device="$device" --fob-format="$fob" --status=status --silent
	else
		message --device="$device" --fob-format="$fob" --status=status --silent
	endif
	exit 0
endif

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
<h1>Fob <output name=fob></h1>
<form method=post action=/editfob.cgi><input name=fob type=hidden>
<sql table=foborganisation where="fob='$fob' AND organisation=$USER_ORGANISATION">
<table>
<tr><td>Name</td><td colspan=2><input name=fobname size=15 maxlength=15 autofocus></td></tr>
<tr><td>Expiry</td><td colspan=2><input name=expires id=expires type=datetime-local><input type=button onclick='document.getElementById("expires").value="";' value="No expiry"></td></tr>
<if blocked><tr><td>Block</td><td>Access blocked <output name=blocked> <if blocked and confirmed>(confirmed <output name=confirmed>)</if></td></tr></if>
<sql table="fobaid LEFT JOIN aid USING (aid) LEFT JOIN site USING (site)" where="fob='$fob' AND aid.organisation=$USER_ORGANISATION" order=sitename,aidname><set "access$aid"="$access">
<tr>
<td><input type=hidden name=aids value="$aid"><output name=sitename></td>
<td><select name="access$aid"><option value=''>No access</option><sql table=access where="site=$site"><option value="$access"><output name=accessname></option></sql></select></td><td><output name=aidname></td>
</tr>
</sql>
</table>
<input type=submit value="Update"><if blocked><input type=SUBMIT name=UNBLOCK Value="Unblock"></if><if else><input type=SUBMIT name=BLOCK Value="Block"></if>
<input type=submit name="DELETE" value="Delete">
</sql>
</form>
'END'


