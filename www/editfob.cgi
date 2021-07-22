#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' --site='$SESSION_SITE' editfob
if($status) exit 0
unsetenv CANADOPTFOB
can --organisation='$SESSION_ORGANISATION' adoptfob
if(! $status) setenv CANADOPTFOB

if($?ADOPTNEXT) then
	sql "$DB" 'UPDATE device SET adoptnext="true" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
	redirect editfob.cgi
	exit 0
endif
if($?FORMATNEXT) then
	sql "$DB" 'UPDATE device SET formatnext="true" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
	redirect editfob.cgi
	exit 0
endif
if($?CANCEL) then
	sql "$DB" 'UPDATE device SET adoptnext="false",formatnext="false" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
	redirect editfob.cgi
	exit 0
endif
if($?ADOPT) then
	setenv OK `sql "$DB" 'SELECT COUNT(*) FROM fob WHERE fob="$fob"'`
	if("$OK" == 0 || "$OK" == "" || "$OK" == NULL) then
		setenv MSG "Sorry, fob not found, ensure it is provisioned"
		goto list
	endif
	sql "$DB" 'INSERT IGNORE INTO foborganisation SET fob="$fob",organisation="$SESSION_ORGANISATION"'
	sql "$DB" 'INSERT INTO fobaid SET fob="$fob",aid="$aid"'
	redirect editfob.cgi
	exit 0
endif
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
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Fobs</h1>
<if CANADOPTFOB>
<h2>Adopting a provisioned fob</h1>
<p>Fobs used on the system must have been provisioned.</p>
<form method=post style="display:inline;">
<select name=aid><sql table=aid where="site=$SESSION_SITE"><option value="$aid"><output name=aidname></option></sql></select><input name=fob size=14 autofocus placeholder="FOB ID">
<input type=submit value="Adopt" name=ADOPT> Adopt a specific FOB.
</form><br>
<form method=post style="display:inline;">
<select name=device>
<sql table="device LEFT JOIN aid USING (aid)" where="device.site=$SESSION_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set found=1>
<option value="$device"><output name=aidname>:<output name=devicename blank="$device"></option>
</sql>
</select>
<if found><set found><input type=submit value="Adopt next fob" name=ADOPTNEXT><if USER_ADMIN=true><input type=submit value="Format next fob" name=FORMATNEXT></if></if>
<if else><p>No admin NFC devices on line.</p></if>
</form>
<table>
<sql table=device where="site=$SESSION_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true') AND (`adoptnext`='true' OR `formatnext`='true')">
<if not found><set found=1><tr><th colspan=3>Devices waiting to auto handle a fob.</th></tr></if>
<tr>
<td><output name=aidname></td>
<td><form method=post style="display:inline;"><input name=device type=hidden><input type=submit name=CANCEL value="Cancel"></form></td>
<td><output name=devicename blank="$device"></td>
<td><if formatnext=true>FORMAT NEXT CARD</if></td>
</tr>
</sql>
</table>
<if found><set found></if><if else><p>No devices set to auto adopt fobs.</p></if>
</if>

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
