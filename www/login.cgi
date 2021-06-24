#!../login/logincheck /bin/csh -f
if($?FORGOT || $?NEW) then
	if("$USERNAME" == "") then
		setenv FAIL "Specify email address"
		goto done
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
	setenv FAIL "We have emailed $USERNAME, check your email and follow the link"
	if("$PASSWORD" != "") setenv FAIL "$FAIL. You will be asked a password when you follow the link."
	unsetenv PASSWORD
	goto done
	exit 0
endif
if($?PASSWORD) then
	dologin --redirect
	exit 0
endif
done:
echo "Content-Type: text/html"
echo ""
/projects/tools/bin/xmlsql head.html - foot.html << END
<h1>Access.me.uk</h1>
<if FAIL><p><b><output name=FAIL></b></p></if>
<form method=post action=login.cgi>
	<table>
		<tr><td>Email</td><td><input size=40 type=email name=USERNAME autofocus></td></tr>
		<tr><td>Password</td><td><input size=40 type=password name=PASSWORD></td></td>
	</table>
	<input type=submit value="Login" name=LOGIN>
	<input type=submit value="Forgotten password" name=FORGOT>
	<input type=submit value="Register as new user" name=NEW>
</form>
END
