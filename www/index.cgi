#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
/projects/tools/bin/xmlsql head.html - foot.html << END
<h1>Access.me.uk</h1>
<if FAIL><p class=error><output name=FAIL></p></if>
<p>This is a work in progress... Talk to RevK for more details.</p>
<pre>
END
