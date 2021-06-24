#!../login/logincheck /bin/csh -fx
setenv USERNAME `weblink --check`
if($status) then
	setenv FAIL "The link you have used is not valid or has expired, sorry"
	source login.cgi
	exit 0
endif
setenv C `sql -c -dSS 'INSERT IGNORE INTO user SET email="$USERNAME"'`
dologin --force --redirect
