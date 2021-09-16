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
setenv XMLSQLDEBUG
xmlsql -C -d "$DB" head.html - foot.html << END
<sql table=site where='site=$site'>
<h1>🔕 Disarming for <output name=sitename></h1>
<form method=post>
<table border=1>
<tr>
<th></th><th>Disarm</th>
<th>Areas</th>
</tr>
<for space A="$AREALIST"><set areaname><sql table=area where="tag='\$A' AND site=\$site"><set areaname="\$areaname"></sql><if not areaname armed=*\$A><set areaname="?"></if>
<if areaname>
<set s='background:green;'><if armed=*\$A><set s='background:yellow;'></if><if alarm=*\$A><set s='background:red;'></if>
<tr style="\$s">
<th><label for=\$A><output name=A></label></th>
<td><input type=checkbox value=\$A name=disarm id=\$A></td>
<td><label for=\$A><output name=areaname></label></td>
</tr>
</if>
</for>
</table>
<input type=submit value="Disarm" name=DISARM>
</form>
</sql>
END
