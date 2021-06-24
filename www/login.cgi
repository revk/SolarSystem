#!../login/logincheck /bin/csh
if($?FORGOT) then
	# TODO
	exit 0
endif
if($?NEW) then
	# TODO
	exit 0
endif
if($?USERNAME) then
	dologin --redirect
	exit 0
endif
echo "Content-Type: text/html"
echo ""
/projects/tools/bin/xmlsql head.html - foot.html << END
<h1>Access.me.uk</h1>
<p>Solar System security management portal.</p>
<if FAIL><p><b><output name=FAIL></b></p></if>
<form method=post action=login.cgi>
	<table>
		<tr><td>Email</td><td><input size=40 type=email name=USERNAME autofocus></td></tr>
		<tr><td>Password</td><td><input size=40 type=password name=PASSWORD></td></td>
	</table>
	<input type=submit value="Login" name=LOGIN>
	<input type=submit value="Forgotten password" name=FORGET>
	<input type=submit value="Register" name=NEW>
</form>
END
