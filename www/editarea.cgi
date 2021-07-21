#!../login/loggedin /bin/csh -f
can --redirect --site='$SESSION_SITE' editarea
if($status) exit 0
source ../types
setenv MIX "1 2 3"

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
        sql "$DB" 'UPDATE device SET poke=NOW() WHERE site=$SESSION_SITE'
	set allow=""
	foreach s ($MIX)
		set allow="$allow mixand$s mixset$s mixarm$s mixdisarm$s"
	end
	sqlwrite -qon "$DB" site site="$SESSION_SITE" $allow
	message --poke
	../login/redirect / Updated
	exit 0
endif

done:
xmlsql -C -d "$DB" head.html - foot.html << END
<form method=post>
<sql table=site WHERE="site=$SESSION_SITE">
<table border=1>
<tr>
<th>Area</th><th>Name</th>
<for SPACE S="$MIX">
<eval N=$AREACOUNT+1>
<th rowspan=\$N>&nbsp;</th>
<th>And</th><th>Set</th>
</for>
</tr>
<sql table=area WHERE="site=$SESSION_SITE"><set "AREA\$tag"="\$areaname"></sql>
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
<eval C=2>
<for SPACE S="$MIX">
<tr>
<td colspan=\$C align=right>Arm</td>
<td colspan=3><input name=mixarm\$S size=20></td>
</tr>
<tr>
<td colspan=\$C align=right>Disarm</td>
<td colspan=3><input name=mixdisarm\$S size=20></td>
</tr>
<eval C=\$C+3>
</for>
</table>
</sql>
<input type=submit value="Update">
</form>
END
