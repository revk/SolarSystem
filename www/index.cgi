#!../login/loggedin /bin/csh -f
echo "Content-Type: text/html"
echo ""
/projects/tools/bin/xmlsql head.html - foot.html << END
<h1>Access.me.uk</h1>
<pre>
`printenv`
END
