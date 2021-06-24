#!/projects/github/login/logincheck /bin/csh
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
/projects/tools/bin/xmlsql - << END
<html><head><title>Access.me.uk</title><link rel=stylesheet href=access.css /></head><body>
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
<hr/><address>&copy; Adrian Kennard, Andrews &amp; Arnold Ltd. UK Company 3342760. This is an open source project: <a href="https://github.com/revk/SolarSystem">GitHub</a></address>
</body></html>
END
