#!../login/loggedin /bin/csh -f
sql "$DB" 'UPDATE session SET organisation=$SET_ORGANISATION,site=NULL WHERE session="$ENVCGI"'
echo "Location: $ENVCGI_SERVER"
echo ""
