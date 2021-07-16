#!../login/loggedin /bin/csh -f
can --redirect --site='$SESSION_SITE' editarea
if($status) exit 0
source ../types

if($?AREAA) then #save
	foreach a ($AREALIST)
		setenv A "$a"
		setenv D `printenv "AREA$A"`
		if("$D" == "") then
			sql "$DB" 'DELETE FROM area WHERE site=$SESSION_SITE AND tag="$A"'
		else
			sql "$DB" 'REPLACE INTO area SET organisation=$SESSION_ORGANISATION,site=$SESSION_SITE,tag="$A",areaname="$D"'
		endif
	end
	sqlwrite -qon "$DB" site site="$SESSION_SITE" engineer
        sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$SESSION_SITE'
	message --poke
	../login/redirect / Updated
	exit 0
endif

done:
xmlsql -C -d "$DB" head.html - foot.html << END
<form method=post>
<sql table=site WHERE="site=$SESSION_SITE">
<table>
<tr><th>Area</th><th>Name</th><th>Engineering mode</th></tr>
<sql table=area WHERE="site=$SESSION_SITE"><set "AREA\$tag"="\$areaname"></sql>
<for space A="$AREALIST">
<tr>
<td><output name=A></td>
<td><input name="AREA\$A" size=20></td>
<td><input name=engineer type=checkbox value=\$A></td>
<tr>
</for>
</table>
</sql>
<input type=submit value="Update">
</form>
END
