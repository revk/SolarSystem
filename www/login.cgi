#!../login/logincheck --query /bin/csh -f
if($?FORGOT || $?NEW) then
	if("$USERNAME" == "") then
		redirect login.cgi "Specify email address"
		exit 0
	endif
	setenv LINK `weblink --make='$USERNAME'`
	if($?NEW) then
	email --subject="New user registration" --to="$USERNAME" --name="$SERVER_NAME" --from="noreply@$SERVER_NAME" << END
You have asked to create a new user account.
Please use this link.

${ENVCGI_SERVER}register.cgi?$LINK
END
	else
	email --subject="Forgetting password" --to="$USERNAME" --name="$SERVER_NAME" --from="noreply@$SERVER_NAME" << END
You have requested a password change.
Please use this link.

${ENVCGI_SERVER}forgot.cgi?$LINK
END
	endif
	setenv MSG "We have emailed $USERNAME, check your email and follow the link"
	if("$PASSWORD" != "") setenv MSG "$MSG. You will be asked a password when you follow the link."
	redirect login.cgi "$MSG"
	exit 0
endif
if($?PASSWORD) then
	dologin --redirect
	exit 0
endif
done:
xmlsql -C -d "$DB" head.html - foot.html << END
<form method=post action=login.cgi>
	<table>
		<tr><td>Email</td><td><input autocomplete="username" size=40 type=email name=USERNAME autofocus></td></tr>
		<tr><td>Password</td><td><input autocomplete="current-password" size=40 type=password name=PASSWORD></td></td>
	</table>
	<input type=submit value="Login" name=LOGIN>
	<input type=submit value="Forgotten password" name=FORGOT>
	<input type=submit value="Register as new user" name=NEW>
</form>
END
