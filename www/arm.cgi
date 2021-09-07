#!../login/loggedin /bin/csh -f
setenv site "$PATH_INFO:t"
can --redirect --site='$site' arm
if($status) exit 0
source ../setcan

if($?ARM) then
	if($?arm) then
		setenv MSG `message --site="$site" --arm="$arm"`
		if("$MSG" != "") goto done
	endif
	redirect /
	exit 0
endif

if($?STRONGARM) then
	can --redirect --site='$site' strongarm
	if($status) exit 0
	if($?arm) then
		setenv MSG `message --site="$site" --strongarm="$arm"`
		if("$MSG" != "") goto done
	endif
	redirect /
	exit 0
endif

done:
source ../types
xmlsql -C -d "$DB" head.html - foot.html << END
<sql table=site where='site=$site'>
<h1>🔔 Arming for <output name=sitename></h1>
<form method=post>
<table border=1>
<tr>
<th></th><th>Arm</th>
<th>Areas</th>
</tr>
<sql table=area where="site=$site">
<set s='background:green;'><if armed=*\$tag><set s='background:yellow;'></if><if alarm=*\$tag><set s='background:red;'></if>
<tr style="\$s">
<th><label for=\$tag><output name=tag></label></th>
<td><input type=checkbox value=\$tag name=arm id=\$tag></td>
<td><label for=\$tag><output name=areaname></label></td>
</tr>
</sql>
</table>
<input type=submit value="Arm" name=ARM>
<IF CANSTRONGARM><input type=submit value="Force" name=STRONGARM></if>
</form>
</sql>
END
