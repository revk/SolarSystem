#!../login/loggedin --query /bin/csh -f
unset user # clash with csh variable, duh
if(! $?user) setenv user "$USER_ID"	# Edit self
can --redirect --user="$user"
if($?email) then # save
	# update other fields... not email
	sqlwrite -o SS user user="$user" user description
	# check email change as special handling
	setenv was `sql SS 'SELECT email FROM user WHERE user=$user'`
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
		echo "Location: $ENVCGI_SERVER?FAIL=An+email+has+been+sent,+check+it+and+click+link."
		echo ""
		exit 0
	endif
	echo "Location: $ENVCGI_SERVER?FAIL=Updated"
	echo ""
	exit 0
endif
echo "Content-Type: text/html"
echo ""
xmlsql -d SS head.html - foot.html << 'END'
<h1>Edit user</h1>
<if FAIL><p class=error><output name=FAIL></p></if>
<sql table=user where="user=$user">
<form method=post><input type=hidden name=user>
<table>
<tr><td>Email</td><td><input name=email type=email size=40></td></tr>
<tr><td>Name</td><td><input name=description size=40></td></tr>
</table>
<input type=submit value="Update">
</form>
</sql>
'END'
