#!../login/loggedin /bin/csh -fx
setenv XMLSQLDEBUG
can --redirect --site="$SESSION_SITE" editaccess
if($status) exit 0

source ../setcan
if($?access) then
	if("$access" == 0) then
		setenv access `sql -i "$DB" 'INSERT INTO access SET organisation=$SESSION_ORGANISATION,site=$SESSION_SITE'`
	endif
	can --redirect --access="$access" editaccess
	if($status) exit 0
	setenv allow "accessname clock log count commit open arm"
	if(! $?clock) setenv clock false
	if(! $?override) setenv override false
	if(! $?log) setenv log false
	if(! $?count) setenv count false
	if(! $?commit) setenv commit false
	if($?ADMINORGANISATION) setenv allow "$allow override"
	sqlwrite -v -o -n "$DB" access $allow
	setenv MSG "Updated"
	goto list
endif

if(! $?PATH_INFO) then
list:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Access classes</h1>
<table>
<sql table=access where="site=$SESSION_SITE">
<tr>
<td><output name=accessname blank="Unnanmed" href="editaccess.cgi/$access"></td>
<td>
<if clock=true>Time override</if>
<if override=true><b>OVERRIDE</b></if>
<if log=true>Fob log</if>
<if count=true>Fob count</if>
<if commit=true>Fob commit</if>
</td>
</tr>
</sql>
</table>
<p><a href="/editaccess.cgi/0">New access</p></p>
'END'
exit 0
endif

edit:
setenv access "$PATH_INFO:t"
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Access</h1>
<form method=post action=/editaccess.cgi>
<sql table=access key=access><input name=access type=hidden>
<table>
<tr><td>Name</td><td><input name=accessname size=40 autofocus></td></tr>
<tr><td>Open</td><td><sql select="tag" table=area where="site=$site"><label for=open$tag><output name=tag>:</label><input id=open$tag name=open type=checkbox value=$tag></sql></td></tr>
<tr><td>Arm/disarm</td><td><sql select="tag" table=area where="site=$site"><label for=arm$tag><output name=tag>:</label><input id=arm$tag name=arm type=checkbox value=$tag></sql></td></tr>
<tr><td><input type=checkbox id=clock name=clock value=true></td><td><label for=clock>Ignore time restrictions if clock not set.</label></td></tr>
<tr><td><input type=checkbox id=log name=log value=true></td><td><label for=log>Try to log access to fob.</label></td></tr>
<tr><td><input type=checkbox id=count name=count value=true></td><td><label for=count>Try to count access on fob.</label></td></tr>
<tr><td><input type=checkbox id=commit name=commit value=true></td><td><label for=commit>Commit changes (log/count) before allowing access (slower).</label></td></tr>
<IF ADMINORGANISATION><tr><td><input type=checkbox id=override name=override value=true></td><td><label for=override>Open door in all cases.</label></td></tr></if>
</table>
<input type=submit value="Update">
</sql>
</form>
<p>TODO expiry logic</p>
<p>TODO time of day logic</p>
'END'
