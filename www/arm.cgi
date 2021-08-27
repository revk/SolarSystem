#!../login/loggedin /bin/csh -f
can --redirect --site='$USER_SITE' arm
if($status) exit 0

if($?ARM) then
	if($?arm) then
		setenv MSG `message --site="$USER_SITE" --arm="$arm"`
		if("$MSG" != "") goto done
	endif
	redirect /
	exit 0
endif

if($?STRONGARM) then
	can --redirect --site='$USER_SITE' strongarm
	if($status) exit 0
	if($?arm) then
		setenv MSG `message --site="$USER_SITE" --strongarm="$arm"`
		if("$MSG" != "") goto done
	endif
	redirect /
	exit 0
endif

done:
source ../types
source ../setcan
xmlsql -C -d "$DB" head.html - foot.html << END
<h1>Arming</h1>
<sql table=site where='site=$USER_SITE'>
<form method=post>
<table border=1>
<tr>
<th></th><th>Arm</th>
<th>Areas</th>
</tr>
<sql table=area where="site=$USER_SITE">
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
