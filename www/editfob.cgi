#!../login/loggedin /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' editfob
if($status) exit 0

done:
xmlsql -C -d "$DB" head.html - foot.cgi << 'END'

'END'


