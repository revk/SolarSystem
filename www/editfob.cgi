#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' --site='$SESSION_SITE' editfob
if($status) exit 0
unsetenv CANADOPTFOB
can --organisation='$SESSION_ORGANISATION' adoptfob
if(! $status) setenv CANADOPTFOB

if($?fobname) then
	set aids=(`printenv aids|sed 's/[^0-9A-F	]//g'`)
	foreach a ($aids)
		setenv A "$a"
		can --redirect --aid="$A" editfob
		if($status) exit 0
		setenv access `printenv "access$A"`
		if("$access" == "") then
			setenv C `sql -c "$DB" 'DELETE FROM fobaid WHERE  fob="$fob" AND aid="$A"'`
			if("$C" == 1) sql "$DB" 'UPDATE device SET device.poke=NOW() WHERE aid="$A"'
		else
			setenv C `sql -c "$DB" 'INSERT IGNORE INTO fobaid SET fob="$fob",aid="$A"'`
			if("$C" == 1) sql "$DB" 'UPDATE device SET device.poke=NOW() WHERE aid="$A"'
			sql "$DB" 'UPDATE fobaid SET access=$access WHERE fob="$fob" AND aid="$A"'
		endif
	end
	setenv organisation "$SESSION_ORGANISATION"
	setenv allow "fobname expires"
	if($?BLOCK) setenv blocked "`date +'%F %T'`"
	if($?BLOCK) setenv allow "$allow confirmed"
	if($?UNBLOCK||$?BLOCK) then
		setenv allow "$allow blocked"
		sql "$DB" 'UPDATE device SET device.poke=NOW() WHERE organisation="$SESSION_ORGANISATION"'
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
echo "Content-encoding: none" # so no deflating and hence no caching for interactive status
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Fobs</h1>
<if CANADOPTFOB>
<h2>Adopting a provisioned fob</h1>
<p>Fobs used on the system must have been provisioned.</p>
<form name=f method=post style="display:inline;">
<select name=device>
<sql table="device LEFT JOIN aid USING (aid)" where="device.site=$SESSION_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set found=1>
<option value="$device"><output name=aidname>:<output name=devicename blank="$device"></option>
</sql>
</select>
<input type=submit name=IDENTIFY value="Read fob ID">
<input type=hidden name=fob>
<if not IDENTIFY fob not fob=""><h2>Fob <output name=fob href="editfob.cgi/$fob"><sql table=foborganisation where="fob='$fob' AND organisation=$SESSION_ORGANISATION"> <output name=fobname></sql></h2>
<sql table="fobaid LEFT JOIN aid USING (aid) LEFT JOIN access USING (access)" WHERE="fob='$fob'">
<p>Already adopted for <output name=aidname> <output name=accessname></p>
</sql>
<select name=aid><sql table=aid where='site=$SESSION_SITE'><option value=$aid><output name=aidname></option></sql></select>
<select name=access><sql table=access where='site=$SESSION_SITE'><option value=$access><output name=accessname></option></sql></select>
<input name=ADOPT type=submit value="Adopt fob">
<if USER_ADMIN=true><input name=FORMAT type=submit value="Format"></if>
</if>
<if device><ul id=status></ul></if>
</form>
</if>

<h2>Fobs</h2>
<table>
<sql table="foborganisation LEFT JOIN fobaid USING (fob) LEFT JOIN aid USING (aid) LEFT JOIN access USING (access)" where="foborganisation.organisation=$SESSION_ORGANISATION" group="fob" order="max(adopted) DESC" select="*,count(aid) as N, sum(if(adopted IS NULL AND aid.aid IS NOT NULL,1,0)) AS W,sum(if(override='true',1,0)) AS O">
<if not found><set found=1><tr>
<th>Fob</th><th>Expiry</th><th>Name</th><th>Notes</th>
</tr></if>
<tr>
<td><output name=fob href="editfob.cgi/$fob"></td>
<td><output name=expires></td>
<td><output name=fobname></td>
<td><if not N=1><output name=N 0=No> AIDs</if><if else not N=0><output name=aidname> (<output name=accessname>) </if><if not W=0><output name=W 1=""> waiting to be adopted. </if><if blocked><b>BLOCKED<if confirmed>(confirmed) </if></b></if><if NOT O=0><b><ouput name=O 1=""> has override.</b></if></td>
</tr>
</sql>
</table>
<if found><set found></if><if else><p>No fobs found</p></if>
'END'

if($?IDENTIFY) then
	can --redirect --organisation='$SESSION_ORGANISATION' adoptfob
	if($status) exit 0
	echo "<script>e=document.createElement('li');e.textContent='Waiting for fob';document.getElementById('status').append(e);</script>"
	setenv fob `message --device="$device" --fob-identify --silent`
	if("$fob" == "") then
		echo "<script>e=document.createElement('li');e.textContent='Failed';document.getElementById('status').append(e);</script>"
	else
		echo "<script>f.fob.value='$fob';f.submit();</script>"
	endif
	exit 0
endif
if($?ADOPT) then
	can --redirect --organisation='$SESSION_ORGANISATION' adoptfob
	if($status) exit 0
	message --device="$device" --fob-adopt="$fob" --aid="$aid" --access="$access" --status=status --silent
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
<sql table=foborganisation where="fob='$fob'">
<table>
<tr><td>Name</td><td colspan=2><input name=fobname size=15 maxlength=15 autofocus></td></tr>
<tr><td>Expiry</td><td colspan=2><input name=expires id=expires type=datetime-local><input type=button onclick='document.getElementById("expires").value="";' value="No expiry"></td></tr>
<if blocked><tr><td>Block</td><td>Access blocked <output name=blocked> <if blocked and confirmed>(confirmed <output name=confirmed>)</if></td></tr></if>
<sql table="aid" where="site='$SESSION_SITE'" order=aidname><set adopted><set access$aid><sql table=fobaid where="fob='$fob' AND aid='$aid'"><set access$aid=$access><if adopted><set adopted="$adopted"></if></sql>
<tr>
<td><input type=hidden name=aids value="$aid"><output name=aidname></td>
<td><select name="access$aid"><option value=''>No access</option><sql table=access where="site=$site"><option value="$access"><output name=accessname></option></sql></select></td>
<td><if access$aid AND not adopted>Waiting to be adopted</if></td>
</tr>
</sql>
</table>
<input type=submit value="Update"><if blocked><input type=SUBMIT name=UNBLOCK Value="Unblock"></if><if else><input type=SUBMIT name=BLOCK Value="Block"></if>
</sql>
</sql>
</form>
'END'


