#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
/projects/tools/bin/xmlsql -d SS head.html - foot.html << END
<h1>Access.me.uk</h1>
<if FAIL><p class=error><output name=FAIL></p></if>
<if USER_ADMIN>
<h2>Admin functions</h2>
<a href="listorganisation.cgi">Organisation</a>
<hr/>
</if>
<p>This is a work in progress... Talk to RevK for more details.</p>
<pre>
END
