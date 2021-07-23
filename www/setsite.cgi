#!../login/loggedin /bin/csh -f
sql "$DB" 'UPDATE session SET site=$SET_SITE WHERE session="$ENVCGI"'
redirect /
