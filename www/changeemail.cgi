#!../login/logincheck --query /bin/csh -f
setenv LINK `weblink --check`
if($status) then
        setenv FAIL "The link you have used is not valid or has expired, sorry"
        source login.cgi
        exit 0
endif
unset user
setenv user "$LINK:e"
setenv USERNAME "$LINK:r"
setenv C `sql -c "$DB" 'UPDATE user SET email="$USERNAME" WHERE user=$user'`
if($status || "$C" == 0) then
	login/redirect login.cgi "Cannot update email"
	exit 0
endif
dologin --force --silent
if($status) then
	login/redirect login.cgi
else
	login/redirect / Updated
endif
