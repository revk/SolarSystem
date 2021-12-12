#!../login/loggedin --insecure-post --http-auth /bin/csh -f

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

setenv USER_ORGANISATION "$PATH_INFO:t"

can --organisation='$USER_ORGANISATION' api
if($status) then
	echo "Status: 403"	# Must be an API allowed user
	echo ""
	exit 0
endif

setenv MSG `../message --api="$USER_ID" --organisation="$USER_ORGANISATION" --stdin`
if(! $status) then
	if("$MSG" != "") then
		echo "Content-Type: application/json"	# JSON response
		echo ""
		echo "$MSG"
		exit 0
	endif
	echo "Status: 204"	# No response
	echo ""
	exit 0
endif

echo "Status: 500"
echo "Content-Type: text/plain"
echo ""
echo "$MSG"
