#!../login/loggedin /bin/csh -f
setenv site "$PATH_INFO:t"
can --redirect --site='$site' admin
if($status) exit 0

if($?aids || $?NEW) then #save
        set aids=(`printenv aids|sed 's/[^0-9A-F	]//g'`)
	foreach a ($aids)
		setenv A "$a"
		setenv D `printenv "NAME$A"`
		if("$D" == "") then
			sql "$DB" 'DELETE FROM aid WHERE site=$site AND aid="$A"'
		else
			sql "$DB" 'INSERT IGNORE INTO aid SET organisation=$USER_ORGANISATION,site=$site,aid="$A",aidname="$D"'
			sql "$DB" 'UPDATE aid SET aidname="$D" WHERE organisation=$USER_ORGANISATION AND site=$site AND aid="$A"'
		endif
	end
	if($?USER_ADMIN) then
		if("$NEW" != "") then
			setenv A `makeaid --site=$site --name="$NEW"`
		endif
	endif
	redirect / Updated
	exit 0
endif

xmlsql -C -d "$DB" head.html - foot.html << 'END'
<sql table=site where="site=$site">
<h1>Manage AIDs for <output name=sitename></h1>
<form method=post>
<table>
<sql table=aid WHERE="site=$site">
<tr>
<td><input type=hidden name=aids value="$aid"><input name="NAME$aid" size=20 value="$aidname"></td>
</tr>
</sql>
<if USER_ADMIN=true><tr><td><input name=NEW> New AID</if>
</table>
<input type=submit value="Update">
<if not USER_ADMIN=true><p>If you need an additional AID, please contact the system controller.</p></if>
</form>
</sql>
'END'
