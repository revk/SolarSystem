#!../login/loggedin /bin/csh -f
sql "$DB" 'UPDATE session SET organisation=$SET_ORGANISATION WHERE session="$ENVCGI"'
echo "Location: $ENVCGI_SERVER"
echo ""
