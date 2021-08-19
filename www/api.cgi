#!../login/loggedin --insecure-post --http-auth /bin/csh -fx
echo "Content-Type: text/plain"
echo ""
printenv

if(! $?PATH_INFO) then
	echo "Status: 404"	# Organisation after API
	echo ""
	exit 0
endif
if("$REQUEST_METHOD" != "POST") then
	echo "Status: 405"	# Must be post
	echo ""
	exit 0
endif
if(! $?CONTENT_TYPE) setenv CONTENT_TYPE
if("$CONTENT_TYPE" != "application/json") then
	echo "Status: 415"	# Must be JSON
	echo ""
	exit 0
endif
setenv SESSION_ORGANISATION "$PATH_INFO:t"

can --organisation='$SESSION_ORGANISATION' api
if($status) then
	echo "Status: 403"
	echo ""
	exit 0
endif

setenv MSG `../message --api="$USER_ID" --organisation="$SESSION_ORGANISATION" --stdin`
if(! $status) then
	if("$MSG" != "") then
		echo "Content-Type: application/json";
		echo ""
		echo "$MSG"
		exit 0
	endif
	echo "Status: 204"
	echo ""
	exit 0
endif
echo "Status: 500"
echo "Content-Type: text/plain"
echo ""
echo "$MSG"
