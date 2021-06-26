#!../login/loggedin /bin/csh -f
can --redirect admin
if($status) exit 0
if($?pending) then
	setenv MSG "Provisioning..."
endif
done:
echo "Content-Type: text/html"
echo ""
xmlsql -d "$DB" head.html - foot.html << 'END'
<h1>Provision device</h1>
<table border=1>
<sql table=pending order=online><set found=1>
<tr>
<td><form style="display:inline;" method=post><input type=hidden name=pending><input type=submit value="Provision"></form></td>
<td><output name=online></td>
<td><output name=pending></td>
<td><output name=address></td>
<td><output name=version></td>
<td><output name=flash></td>
<td><if secureboot=false><b>Not secure boot. </b></if><if encryptednvs=false><b>Not encrypted NVS. </b></if></td>
</tr>
</sql>
</table>
<if not found><p>No pending device</p></if>
'END'
