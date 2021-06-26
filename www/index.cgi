#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
if(! $?SESSION_ORGANISATION) then
	setenv C `sql "$DB" 'SELECT COUNT(*) FROM userorganisation WHERE user=$USER_ID'`
	if("$C" == 1) then
		setenv SESSION_ORGANISATION `sql "$DB" 'SELECT organisation FROM userorganisation WHERE user=$USER_ID'`
	endif
endif

/projects/tools/bin/xmlsql -d "$DB" head.html - foot.html << END
<if USER_ADMIN>
<h2>Admin functions</h2>
<a href="listorganisation.cgi">Organisation</a>
<hr/>
</if>
<pre>
END
