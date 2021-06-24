#!../login/loggedin /bin/csh
setenv Z `sql -dSS 'SELECT COUNT(*) FROM user WHERE email="$USER_ID" AND hash=NULL'`
if($?NEWPASSWORD) then
	if("$Z" == 1) then
		setenv FAIL `changepassword`
	else
		setenv FAIL `changepassword`
	endif
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
/projects/tools/bin/xmlsql head.html - foot.html << END
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
END
