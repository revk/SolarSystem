#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' --site='$SESSION_SITE' editfob
if($status) exit 0
unsetenv CANADOPTFOB
can --organisation='$SESSION_ORGANISATION' adoptfob
if(! $status) setenv CANADOPTFOB

if($?ADOPTNEXT) then
	sql "$DB" 'UPDATE device SET adoptnext="true" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
	setenv MSG "Device set to adopt next reader"
endif
if($?FORMATNEXT) then
	sql "$DB" 'UPDATE device SET formatnext="true" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
	setenv MSG "Device set to adopt next reader"
endif
if($?CANCEL) then
	sql "$DB" 'UPDATE device SET adoptnext="false",formatnext="false" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
	setenv MSG "Auto fob canceled"
endif
if($?ADOPT) then
	setenv OK `sql "$DB" 'SELECT COUNT(*) FROM fob WHERE fob="$fob"'`
	if("$OK" == 0 || "$OK" == "" || "$OK" == NULL) then
		setenv MSG "Sorry, fob not found, ensure it is provisioned"
		goto list
	endif
	sql "$DB" 'INSERT IGNORE INTO foborganisation SET fob="$fob",organisation="$SESSION_ORGANISATION"'
	sql "$DB" 'INSERT  INTO fobaid SET fob="$fob",aid="$SESSION_SITE"'
	setenv MSG "Fob is set up to be adopted"
	unsetenv fob
	goto list
endif

if(! $?PATH_INFO) then
list:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Fobs</h1>
<if CANADOPTFOB>
<h2>Adopting a provisioned fob</h1>
<p>Fobs used on the system must have been provisioned.</p>
<form method=post style="display:inline;">
<select name=aid><sql table=aid where="site=$SESSION_SITE"><option value="$aid"><output name=aidname></option></sql><input name=fob size=14 autofocus placeholder="FOB ID">
<input type=submit value="Adopt" name=ADOPT> Adopt a specific FOB.
</form><br>
<form method=post style="display:inline;">
<select name=device>
<sql table="device LEFT JOIN aid USING (site)" where="site=$SESSION_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set found=1>
<option value="$device"><output name=aidname>:<output name=devicename blank=Unnamed> <output name=address></option>
</sql>
</select>
<if found><set found><input type=submit value="Adopt next fob" name=ADOPTNEXT><if USER_ADMIN><input type=submit value="Format next fob" name=FORMATNEXT></if></if>
<if else><p>No admin NFC devices on line.</p></if>
</form>
<table>
<sql table=device where="site=$SESSION_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true') AND (`adoptnext`='true' OR `formatnext`='true')">
<if not found><set found=1><tr><th colspan=3>Devices waiting to auto handle a fob.</th></tr></if>
<tr>
<td><output name=aidname></td>
<td><form method=post style="display:inline;"><input name=device type=hidden><input type=submit name=CANCEL value="Cancel"></form></td>
<td><output name=devicename blank=Unnamed> <output name=address></td>
<td><if formatnext=true>FORMAT NEXT CARD</if></td>
</tr>
</sql>
</table>
<if found><set found></if><if else><p>No devices set to auto adopt fobs.</p></if>
</if>
<table>
<sql table="foborganisation LEFT JOIN fobaid USING (fob) LEFT JOIN aid USING (aid)" where="organisation=$SESSION_ORGANISATION" order="adopted DESC">
<if not found><set found=1><tr><th>Fobs</th></tr></if>
<tr>
<td><output name=fob href="editfob.cgi/$fob"></td>
<td><output name=aidname></td></td>
<td><output name=devicename blank="Unnamed"></td>
<td><if not adopted>Waiting to be adopted</if></td>
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

'END'
