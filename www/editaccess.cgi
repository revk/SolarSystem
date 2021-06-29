#!../login/loggedin /bin/csh -f
can --redirect --site="$SESSION_SITE" editaccess
if($status) exit 0

xmlsql -C -d "$DB" head.html - foot.html << 'END'
TODO
'END'
