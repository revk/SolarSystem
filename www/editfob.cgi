#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' editfob
if($status) exit 0
unsetenv CANADOPTFOB
can --organisation='$SESSION_ORGANISATION' adoptfob
if(! $status) setenv CANADOPTFOB

if($?ADOPTNEXT) then
	sql "$DB" 'UPDATE device SET adoptnext="true" WHERE device="$device" and organisation="$SESSION_ORGANISATION"'
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
	sql "$DB" 'INSERT INTO foborganisation SET fob="$fob",organisation="$SESSION_ORGANISATION"'
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
<input name=fob size=14 autofocus placeholder="FOB ID">
<input type=submit value="Adopt" name=ADOPT> Adopt a specific FOB.
</form><br>
<form method=post style="display:inline;">
<select name=device>
<sql table=device where="organisation=$SESSION_ORGANISATION AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set found=1>
<option value="$device"><output name=description blank=Unnamed> <output name=address></option>
</sql>
</select>
<if found><set found><input type=submit value="Adopt next fob" name=ADOPTNEXT></if>
<if else><p>No admin NFC devices on line.</p></if>
</form>
<table>
<sql table=device where="organisation=$SESSION_ORGANISATION AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true') AND `adoptnext`='true'">
<if not found><set found=1><tr><th colspan=2>Devices waiting to auto adopt a fob.</th></tr></if>
<tr>
<td><form method=post style="display:inline;"><input name=device type=hidden><input type=submit name=CANCEL value="Cancel"></form></td>
<td><output name=description blank=Unnamed> <output name=address></td>
</tr>
</sql>
</table>
<if found><set found></if><if else><p>No devices set to auto adopt fobs.</p></if>
</if>
<table>
<sql table="foborganisation" where="organisation=$SESSION_ORGANISATION">
<if not found><set found=1><tr><th>Fobs</th></tr></if>
<tr>
<td><output name=fob href="editfob.cgi/$fob"></td>
<td><output name=description blank="Unnamed"></td>
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
