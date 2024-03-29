#!../login/loggedin --query /bin/csh -f
unset user # clash with csh variable, duh
if(! $?user) setenv user "$USER_ID"	# Edit self
can --redirect --user="$user" edituser
if($status) exit 0
source ../setcan
if($?email) then # save
	# update other fields... not email
	sqlwrite -qon "$DB" user user="$user" user username
	# check email change as special handling
	setenv was `sql "$DB" 'SELECT email FROM user WHERE user=$user'`
	if("$email" != "$was") then
		setenv LINK `weblink --make="$email"."$user"`
		email --subject="Change of email address" --to="$email" --name="$SERVER_NAME" --from=noreply@"$SERVER_NAME" << END
You have asked to change your email address from $was.

To confirm this change, click this link.

${ENVCGI_SERVER}changeemail.cgi?$LINK
END
		email --subject="Change of email address" --to="$was" --name="$SERVER_NAME" --from=noreply@"$SERVER_NAME" << END
You have asked to changed your email addess to $email.

You have been sent a link to confirm the change.
Check your email for that account and click the link.
END
		redirect / "An email address has been sent to $email, please check and follow link."
		exit 0
	endif
	echo "Location: $ENVCGI_SERVER?MSG=Updated"
	echo ""
	exit 0
endif
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<h1>📇 Your details</h1>
<sql table=user where="user=$user">
<form method=post><input type=hidden name=user>
<table>
<tr><td>Email</td><td><input name=email type=email size=40></td></tr>
<tr><td>Name</td><td><input name=username size=40 autofocus></td></tr>
</table>
<input type=submit value="Update">
</form>
</sql>
<a href="/changepassword.cgi">Change password</a>
'END'
