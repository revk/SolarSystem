#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' admin
if($status) exit 0

done:
xmlsql -C -d "$DB" head.html - << 'END'
<h1>Provision fob</h1>
<form method=post>
<select name=device><sql table=device where="nfctrusted='true' AND online IS NOT NULL"><option value="$device"><output name=decsription blank="Unnamed"> <output name=address></option></sql></select>
<input type=submit value="Provision">
</form>
<if device>Provisioning<pre></if>
'END'
if($?device) then
	message --device="$device" --fob-provision
endif
xmlsql foot.html

