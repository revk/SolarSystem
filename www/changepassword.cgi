#!../login/logincheck /bin/csh -f
if(! $?USER_ID) exit 0
setenv Z `sql -c -v -dSS 'SELECT COUNT(*) FROM user WHERE user="$USER_ID" AND hash IS NULL'`
if($?NEWPASSWORD) then
	if("$Z" == 1) then
		setenv FAIL `changepassword --force`
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
setenv NEWPASSWORD `password`
/projects/tools/bin/xmlsql head.html - foot.html << END
<h1>Access.me.uk</h1>
<p>Solar System security management portal.</p>
<if FAIL><p><b><output name=FAIL></b></p></if>
<p>We suggest a new password for you, but you can enter one of your choice if you prefer.</p>
<form method=post action=changepassword.cgi>
	<table>
		<IF Z=0><tr><td>Old password</td><td><input size=40 type=password name=OLDPASSWORD autofocus></td></tr></if>
		<tr><td>New password</td><td><input size=40 name=NEWPASSWORD></td></td>
	</table>
	<input type=submit value="Change" name=CHANGE>
</form>
END
