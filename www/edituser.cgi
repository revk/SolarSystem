#!../login/loggedin /bin/csh -f
unset user # clash with csh variable, duh
if(! $?user) setenv user "$USER_ID"	# Edit self
can --redirect --user="$user" edituser
echo "Content-Type: text/html"
echo ""
if($?email) then # save

echo "Location: $ENVCGI_SERVER"
echo ""
exit 0
endif
setenv XMLSQLDEBUG
xmlsql -d SS head.html - foot.html << 'END'
<h1>Edit user</h1>
<sql table=user where="user=$user">
<form method=post>
<table>
<tr><td>Email</td><td><input name=email type=email size=40></td></tr>
<tr><td>Name</td><td><input name=description size=40></td></tr>
</table>
<input type=submit value="Update">
</form>
</sql>
'END'
