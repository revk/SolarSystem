#!../login/loggedin /bin/csh -f
can --redirect --organisation='$USER_ORGANISATION' admin
if($status) exit 0
source ../setcan

done:
echo "Content-encoding: none" # so no deflating and hence no caching for interactive status
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>Provision fob</h1>
<form method=post>
Device:<select name=device><sql table=device where="nfctrusted='true' AND online IS NOT NULL"><set found=1><option value="$device"><output name=devicename blank="Unnamed"> <output name=address></option></sql></select>
<if found><set found><input type=submit value="Provision fob"></if>
<if else><p>No devices on line for provisioning</p></if>
</form>
<if device><ul id=status></ul></if>
'END'
if($?device) then
	message --device="$device" --fob-provision --status=status --silent
endif

