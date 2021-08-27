#!../login/loggedin /bin/csh -f
sql "$DB" 'UPDATE user SET organisation=$SET_ORGANISATION,site=NULL WHERE user="$USER_ID"'
redirect /
