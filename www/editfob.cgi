#!../login/loggedin /bin/csh -f
setenv XMLSQLDEBUG
can --redirect --organisation='$SESSION_ORGANISATION' editfob
if($status) exit 0
unsetenv CANADOPTFOB
can --organisation='$SESSION_ORGANISATION' adoptfob
if(! $status) setenv CANADOPTFOB

list:
if(! $?PATH_INFO) then
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<table>
<sql table="foborganisation" where="organisation=$SESSION_ORGANISATION"><set found=1>
<tr>
<td><output name=fob href="editfob.cgi/$fob"></td>
</tr>
</sql>
</table>
<if not found><p>No fobs found</p></if>
<if CANADOPTFOB>
<form method=post>
<input name=fob size=14 autofocus placeholder="FOB ID">
<input type=submit value="Adopt" name=ADOPT>
</form><br>
<form method=post>
<select name=device>
<sql table=device where="organisation=$SESSION_ORGANISATION AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set dev=1>
<option value="$device"><output name=description blank=Unnamed> <output name=address></option>
</sql>
</select>
<if dev><input type=submit value="Adopt next fob" name=ADOPTNEXT></if>
<if not dev><p>No admin NFC devices on line.</p></if>
</form>
</if>
'END'
exit 0
endif

edit:
setenv fob "$PATH_INFO:t"
setenv OK `sql "$DB" 'SELECT COUNT(*) FROM fob WHERE fob="$fob"'`
if("$OK" == 0 || "$OK" == "" || "$OK" == NULL) then
	setenv MSG "Sorry, fob not found, ensure it is provisioned"
endif
xmlsql -C -d "$DB" head.html - foot.html << 'END'

'END'
