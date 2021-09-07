#!../login/loggedin /bin/csh -f
if(! $?PATH_INFO) setenv PATH_INFO /
if("$PATH_INFO:h" != "") setenv PATH_INFO "$PATH_INFO:h"
sql "$DB" 'UPDATE user SET site=$SET_SITE WHERE user="$USER_ID"'
redirect "$PATH_INFO"
