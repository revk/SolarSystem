#!../login/loggedin --query /bin/csh -f
setenv Z `sql "$DB" 'SELECT COUNT(*) FROM user WHERE user="$USER_ID" AND hash IS NULL'`
if($?NEWPASSWORD) then
	if("$Z" == 1) then
		setenv MSG `changepassword --force`
	else
		setenv MSG `changepassword`
	endif
	if(! $status) then
		login/redirect edituser.cgi Updated
		exit 0
	endif
	unsetenv OLDPASSWORD
	unsetenv NEWPASSWORD
endif
setenv NEWPASSWORD `password`
xmlsql -C -d "$DB" head.html - foot.html << END
<p>We suggest a new password for you, but you can enter one of your choice if you prefer.</p>
<form method=post action=changepassword.cgi>
	<table>
		<IF Z=0><tr><td>Old password</td><td><input size=40 type=password name=OLDPASSWORD autofocus></td></tr></if>
		<tr><td>New password</td><td><input size=40 name=NEWPASSWORD></td></td>
	</table>
	<input type=submit value="Change" name=CHANGE>
</form>
END
