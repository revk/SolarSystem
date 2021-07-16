#!../login/loggedin /bin/csh -f
can --redirect --site="$SESSION_SITE" editaccess
if($status) exit 0

source ../setcan
setenv days "sun mon tue wed thu fri sat"

if($?DELETE) then
	if(! $?SURE) then
		setenv MSG "Are you sure?"
		goto edit
	endif
	can --redirect --access="$access" admin
	if($status) exit 0
	setenv C `sql -c "$DB" 'DELETE FROM access WHERE access=$access'`
	if("$C" == "" || "$C" == "0") then
                setenv MSG "Cannot delete as in use"
                goto done
        endif
        ../login/redirect editaccess.cgi
        exit 0
endif
if($?access) then
	if("$access" == 0) then
		setenv access `sql -i "$DB" 'INSERT INTO access SET organisation=$SESSION_ORGANISATION,site=$SESSION_SITE'`
	endif
	can --redirect --access="$access" editaccess
	if($status) exit 0
	setenv allow "accessname clock log count armlate commit enter arm disarm expiry"
	foreach day ($days)
		setenv allow "$allow ${day}from ${day}to"
	end
	if(! $?clock) setenv clock false
	if(! $?override) setenv override false
	if(! $?log) setenv log false
	if(! $?count) setenv count false
	if(! $?commit) setenv commit false
	if(! $?armlate) setenv armlate false
	if($?ADMINORGANISATION) setenv allow "$allow override"
	sqlwrite -qon "$DB" access $allow
	../login/redirect editaccess.cgi
	exit 0
endif

if(! $?PATH_INFO) then
list:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Access classes</h1>
<table>
<tr><th>Access</th><th>Notes</th><th>Expiry</th><th>Sunday</th><th>Monday</th><th>Tuesday</th><th>Wednesday</th><th>Thursday</th><th>Friday</th><th>Saturday</th><th>Enter</th><th>Arm</th><th>ForceArm</th><th>Disarm</th></tr>
<sql table=access where="site=$SESSION_SITE">
<tr>
<td><output name=accessname missing="Unnamed" blank="Unnanmed" href="editaccess.cgi/$access"></td>
<td>
<if override=true><b>OVERRIDE</b></if>
<if armlate=true>Late arm allowed</if>
<if clock=true>Time override if no clock</if>
<if log=true>Fob log</if>
<if count=true>Fob count</if>
<if commit=true>Fob commit</if>
</td>
<td><if expiry=0>No auto expiry</if><if else><output name=expiry> day<if not expiry=1>s</if> auto expiry</if></td>
<for space day="$days">
<td><output type="%H:%M" name=${day}from>-<output name=${day}to type=%H:%M></td>
</for>
<td><output name=enter></td>
<td><output name=arm></td>
<td><output name=forcearm></td>
<td><output name=disarm></td>
</tr>
</sql>
</table>
<p><a href="/editaccess.cgi/0">New access</p></p>
'END'
exit 0
endif

setenv access "$PATH_INFO:t"
edit:
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Access</h1>
<form method=post action=/editaccess.cgi>
<sql table=access key=access><input name=access type=hidden>
<table>
<tr><td>Name</td><td><input name=accessname size=40 autofocus></td></tr>
<tr><td>Enter</td><td><sql select="tag" table=area where="site=$site"><label for=enter$tag><output name=tag>:</label><input id=enter$tag name=enter type=checkbox value=$tag></sql></td></tr>
<tr><td>Arm</td><td><sql select="tag" table=area where="site=$site"><label for=arm$tag><output name=tag>:</label><input id=arm$tag name=arm type=checkbox value=$tag></sql></td></tr>
<tr><td>ForceArm</td><td><sql select="tag" table=area where="site=$site"><label for=forcearm$tag><output name=tag>:</label><input id=forcearm$tag name=forcearm type=checkbox value=$tag></sql></td></tr>
<tr><td>Disarm</td><td><sql select="tag" table=area where="site=$site"><label for=disarm$tag><output name=tag>:</label><input id=disarm$tag name=disarm type=checkbox value=$tag></sql></td></tr>
<tr><td><input type=checkbox id=clock name=clock value=true></td><td><label for=clock>Ignore time restrictions if clock not set.</label></td></tr>
<tr><td><input type=checkbox id=log name=log value=true></td><td><label for=log>Try to log access to fob.</label></td></tr>
<tr><td><input type=checkbox id=count name=count value=true></td><td><label for=count>Try to count access on fob.</label></td></tr>
<tr><td><input type=checkbox id=commit name=commit value=true></td><td><label for=commit>Commit changes (log/count) before allowing access (slower).</label></td></tr>
<tr><td><input type=checkbox id=armlate name=armlate value=true></td><td><label for=armlate>Allow arming out of hours.</label></td></tr>
<IF ADMINORGANISATION><tr><td><input type=checkbox id=override name=override value=true></td><td><label for=override>Open door in all cases.</label></td></tr></if>
<tr><td>Auto expire</td><td><input name=expiry size=3>Days</td></tr>
<for space day="$days">
<tr><td><output name=day sun=Sunday mon=Monday tue=Tuesday wed=Wednesday thu=Thursday fri=Friday sat=Saturday></td><td><input name="${day}from" type=time>-<input name="${day}to" type=time></td></tr>
</for>
</table>
<input type=submit value="Update"><if ADMINORGANISATION><input type=submit name=DELETE Value="Delete"><input name=SURE type=checkbox></if>
</sql>
</form>
'END'
