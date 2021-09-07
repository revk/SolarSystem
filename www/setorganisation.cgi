#!../login/loggedin /bin/csh -f
if(! $?PATH_INFO) setenv PATH_INFO /
if("$PATH_INFO:h" != "") setenv PATH_INFO "$PATH_INFO:h"
setenv USER_SITE `sql "$DB" 'SELECT site FROM site WHERE organisation=$SET_ORGANISATION ORDER BY site LIMIT 1'`
sql "$DB" 'UPDATE user SET organisation=$SET_ORGANISATION,site=$USER_SITE WHERE user="$USER_ID"'
redirect "$page"
