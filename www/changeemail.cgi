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
setenv C `sql -c SS 'UPDATE user SET email="$USERNAME" WHERE user=$user'`
if($status || "$C" == 0) then
	echo "Location: ${ENVCGI_SERVER}login.cgi?FAIL=Cannot+update+email"
	echo ""
	exit 0
endif
dologin --force --silent
if($status) then
	echo "Location: ${ENVCGI_SERVER}login.cgi"
else
	echo "Location: $ENVCGI_SERVER?FAIL=Updated"
endif
echo ""
