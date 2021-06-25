#!../login/loggedin /bin/csh -f
can --redirect admin
if($status) exit 0
echo "Content-Type: text/html"
echo ""
xmlsql -d SS head.html - foot.html << 'END'
<h1>Organisations</h1>
<if USER_ADMIN>
<table>
<sql table=organisation>
<tr><td><output name=description blank="Un-named" href="editorganisation.cgi/$organisation"></td></tr>
</sql>
</table>
<a href=editorganisation.cgi/0>New organisation</a>
</if><if else>
<table>
<sql table="userorganisation LEFT JOIN organisation USING (organisation)" WHERE="user=$USER_ID">
<tr><td><output name=description blank="Un-named" href="editorganisation.cgi/$organisation"></td></tr>
</sql>
</table>
</if>
'END'
