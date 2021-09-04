#!../login/loggedin /bin/csh -f
sql "$DB" 'UPDATE user SET site=$SET_SITE WHERE user="$USER_ID"'
redirect "$page"
