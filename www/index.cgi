#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
if(! $?SESSION_ORGANISATION) then
endif

/projects/tools/bin/xmlsql -d SS head.html - foot.html << END
<if USER_ADMIN>
<h2>Admin functions</h2>
<a href="listorganisation.cgi">Organisation</a>
<hr/>
</if>
<p>This is a work in progress... Talk to RevK for more details.</p>
<pre>
END
