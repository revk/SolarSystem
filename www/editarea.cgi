#!../login/loggedin /bin/csh -f
if(! $?PATH_INFO) setenv PATH_INFO /$USER_SITE
setenv site "$PATH_INFO:t"
can --redirect --site='$site' editarea
if($status) exit 0
source ../types
source ../setcan
setenv MIX "1 2 3 4 5"

if($?AREAA) then #save
	foreach a ($AREALIST)
		setenv A "$a"
		setenv D `printenv "AREA$A"`
		if("$D" == "") then
			sql "$DB" 'DELETE FROM area WHERE site=$site AND tag="$A"'
		else
			sql "$DB" 'REPLACE INTO area SET organisation=$USER_ORGANISATION,site=$site,tag="$A",areaname="$D"'
		endif
	end
        sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
	set allow=""
	foreach s ($MIX)
		set allow="$allow mixand$s mixset$s"
	end
	sqlwrite -qon "$DB" site site="$site" $allow
        sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$site'
	message --poke
	redirect / Updated
	exit 0
endif

done:
xmlsql -C -d "$DB" head.html - foot.html << END
<sql table=site WHERE="site=$site">
<h1>🔠 Manage areas for <output name=sitename></h1>
<form method=post>
<table border=1>
<eval C=3><for SPACE S="$MIX"><eval C=\$C+3></for>
<tr><th colspan=\$C>IoT messages</th></tr>
<tr>
<th>Area</th><th>Name</th>
<for SPACE S="$MIX">
<eval N=$AREACOUNT+1>
<th rowspan=\$N>&nbsp;</th>
<th>And</th><th>Set</th>
</for>
</tr>
<sql table=area WHERE="site=$site"><set "AREA\$tag"="\$areaname"></sql>
<for space A="$AREALIST">
<tr>
<td><output name=A></td>
<td><input name="AREA\$A" size=20></td>
<for SPACE S="$MIX">
<td><input type=checkbox name=mixand\$S value=\$A></td>
<td><input type=checkbox name=mixset\$S value=\$A></td>
</for>
</tr>
</for>
</table>
<input type=submit value="Update">
</form>
</sql>
END
