#!../login/loggedin /bin/csh -fx
if($?PATH_INFO) then
        setenv fordevice `echo "$PATH_INFO:t"|sed 's/[^0-9A-Z]//g'`
	setenv WHERE "AND device='$fordevice'"
else
	setenv WHERE ""
endif
can --redirect --organisation='$USER_ORGANISATION' viewlog
if($status) exit 0

xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Logs<if fordevice> for <sql table=device where='device="$fordevice"'><output name=devicename></sql></if></h1>
<table style="white-space:nowrap;">
<sql table='event LEFT JOIN device USING (device)' WHERE='site=$USER_SITE $WHERE' ORDER='event DESC'>
<tr>
<td><output name=logged type=recent></td>
<if not fordevice><td><output name=devicename></td></if>
<td><output name=suffix></td>
<td><output name=data></td>
</tr>
</sql>
</table>
'END'

