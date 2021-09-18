#!../login/loggedin /bin/csh -f
if($?PATH_INFO) then
        setenv for `echo "$PATH_INFO:t"|sed 's/[^0-9A-Z]//g'`
	setenv WHERE "AND (device='$for' OR fob='$for')"
else
	setenv WHERE ""
endif
can --redirect --organisation='$USER_ORGANISATION' viewlog
if($status) exit 0
source ../setcan

xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>🗒 Logs<if for> for <sql table=device where='device="$for"'><output name=devicename></sql><sql table=foborganisation where='organisation="$USER_ORGANISATION" AND fob="$for"'><output name=fobname></sql></if></h1>
<table style="white-space:nowrap;">
<sql table='event LEFT JOIN device USING (device)' WHERE='site=$USER_SITE $WHERE' ORDER='event DESC'>
<tr>
<td><output name=logged type=recent></td>
<if not for or not for='\$device'><td><output name=devicename></td></if>
<td><output name=suffix></td>
<td><output name=data></td>
</tr>
</sql>
</table>
'END'

