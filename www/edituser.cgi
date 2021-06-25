#!../login/loggedin /bin/csh -f
if(! $?U) setenv U "$USER_ID"	# Edit self
can --redirect --user="$U"
echo "Content-Type: text/html"
echo ""
xmlsql -d SS head.html - foot.html << END
<h1>Edit user</h1>

END
