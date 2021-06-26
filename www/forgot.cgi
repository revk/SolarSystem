#!../login/logincheck --query /bin/csh -fx
setenv USERNAME `weblink --check`
if($status) then
	setenv FAIL "The link you have used is not valid or has expired, sorry"
	source login.cgi
	exit 0
endif
sql -dSS 'UPDATE user SET hash=NULL WHERE email="$USERNAME"'
dologin --redirect --force

