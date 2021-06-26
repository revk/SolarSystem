#!../login/logincheck --query /bin/csh -f
setenv USERNAME `weblink --check`
if($status) then
	setenv MSG "The link you have used is not valid or has expired, sorry"
	source login.cgi
	exit 0
endif
sql "$DB" 'UPDATE user SET hash=NULL WHERE email="$USERNAME"'
dologin --redirect --force

