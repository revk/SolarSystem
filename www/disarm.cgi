#!../login/loggedin /bin/csh -f
if(! $?PATH_INFO) setenv PATH_INFO /$USER_SITE
setenv site "$PATH_INFO:t"
can --redirect --site='$site' disarm
if($status) exit 0
source ../setcan

if($?DISARM) then
	if($?disarm) then
		setenv MSG `message --site="$site" --disarm="$disarm"`
		if("$MSG" != "") goto done
	endif
	redirect /
	exit 0
endif

done:
source ../types
xmlsql -C -d "$DB" head.html - foot.html << END
<sql table=site where='site=$site'>
<h1>🔕 Disarming for <output name=sitename></h1>
<form method=post>
<table border=1>
<tr>
<th></th><th>Disarm</th>
<th>Areas</th>
</tr>
<sql table=area where="site=$site">
<set s='background:green;'><if armed=*\$tag><set s='background:yellow;'></if><if alarm=*\$tag><set s='background:red;'></if>
<tr style="\$s">
<th><label for=\$tag><output name=tag></label></th>
<td><input type=checkbox value=\$tag name=disarm id=\$tag></td>
<td><label for=\$tag><output name=areaname></label></td>
</tr>
</sql>
</table>
<input type=submit value="Disarm" name=DISARM>
</form>
</sql>
END
