#!../login/loggedin /bin/csh -f
can --redirect --site='$SESSION_SITE' admin
if($status) exit 0

if($?aids) then #save
        set aids=(`printenv aids|sed 's/[^0-9A-F	]//g'`)
	foreach a ($aids)
		setenv A "$a"
		setenv D `printenv "NAME$A"`
		if("$D" == "") then
			sql "$DB" 'DELETE FROM aid WHERE site=$SESSION_SITE AND aid="$A"'
		else
			sql "$DB" 'REPLACE INTO aid SET organisation=$SESSION_ORGANISATION,site=$SESSION_SITE,aid="$A",aidname="$D"'
		endif
	end
	if($?USER_ADMIN) then
		if("$NEW" != "") then
			setenv A `makeaid --site=$SESSION_SITE --name="$NEW"`
		endif
	endif
	../login/redirect / Updated
	exit 0
endif

xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>AIDs in use</h1>
<form method=post>
<table>
<sql table=aid WHERE="site=$SESSION_SITE">
<tr>
<td><input type=hidden name=aids value="$aid"><input name="NAME$aid" size=20 value="$aidname"></td>
</tr>
</sql>
<if USER_ADMIN><tr><td><input name=NEW> New AID</if>
</table>
<input type=submit value="Update">
<if not USER_ADMIN><p>If you need an additional AID, please contact the system controller.</p></if>
</form>
'END'
