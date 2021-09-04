#!../login/loggedin /bin/csh -f
setenv USER_SITE `sql "$DB" 'SELECT site FROM site WHERE organisation=$SET_ORGANISATION ORDER BY site LIMIT 1'`
sql "$DB" 'UPDATE user SET organisation=$SET_ORGANISATION,site=$USER_SITE WHERE user="$USER_ID"'
redirect "$page"
