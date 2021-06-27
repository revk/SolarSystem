#!../login/loggedin /bin/csh -f
can --redirect --site='$SESSION_SITE' editarea
if($status) exit 0
source ../types

if($?AREAA) then #save
	foreach a ($AREALIST)
		setenv A "$a"
		setenv D `printenv "AREA$A"`
		if("$D" == "") then
			sql "$DB" 'DELETE FROM area WHERE site=$SESSION_SITE AND area="$A"'
		else
			sql "$DB" 'REPLACE INTO area SET  site=$SESSION_SITE,area="$A",description="$D"'
		endif
	end
	echo "Location: $ENVCGI_SERVER?MSG=Updated"
	echo ""
	exit 0
endif

done:
echo "Content-Type: text/html"
echo ""
xmlsql -d "$DB" head.html - foot.html << END
<form method=post>
<table>
<sql table=area WHERE="site=$SESSION_SITE"><set "AREA\$area"="\$description"></sql>
<for space A="$AREALIST">
<tr>
<td><output name=A></td>
<td><input name="AREA\$A" size=20></td>
<tr>
</for>
</table>
<input type=submit value="Update">
</form>
END
