#!/projects/github/login/loggedin /bin/csh
#!../login/loggedin /bin/csh
if($?NEWPASSWORD) then
	setenv FAIL `changepassword`
	if(! $status) then
		echo "Location: $ENVCGI_SERVER"
		echo ""
		exit 0
	endif
	unsetenv OLDPASSWORD
	unsetenv NEWPASSWORD
endif
echo "Content-Type: text/html"
echo ""
/projects/tools/bin/xmlsql - << END
<html><head><title>Access.me.uk</title><link rel=stylesheet href=access.css /></head><body>
<h1>Access.me.uk</h1>
<p>Solar System security management portal.</p>
<if FAIL><p><b><output name=FAIL></b></p></if>
<form method=post action=changepassword.cgi>
	<table>
		<tr><td>Old password</td><td><input size=40 type=password name=OLDPASSWORD autofocus></td></tr>
		<tr><td>New password</td><td><input size=40 type=password name=NEWPASSWORD></td></td>
	</table>
	<input type=submit value="Change" name=CHANGE>
</form>
<hr/><address>&copy; Adrian Kennard, Andrews &amp; Arnold Ltd. UK Company 3342760. This is an open source project: <a href="https://github.com/revk/SolarSystem">GitHub</a></address>
</body></html>
END
