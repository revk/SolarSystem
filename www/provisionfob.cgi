#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' admin
if($status) exit 0

done:
xmlsql -C -d "$DB" head.html - << 'END'
<h1>Provision fob</h1>
<form method=post>
<select name=aid><option value=''>-- Just provision --</option>
<if SESSION_SITE><sql table=aid where="site=$SESSION_SITE"><option value="$aid"><output name=aidname></option></sql></if>
</select>
<select name=device><sql table=device where="nfctrusted='true' AND online IS NOT NULL"><set found=1><option value="$device"><output name=devicename blank="Unnamed"> <output name=address></option></sql></select>
<if found><set found><input type=submit value="Provision"></if>
<if else><p>No devices on line for provisioning</p></if>
</form>
<if device>Provisioning<pre></if>
'END'
if($?device) then
	message --device="$device" --aid="$aid" --fob-provision
endif
xmlsql foot.html

