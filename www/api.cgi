#!../login/loggedin --http-auth /bin/csh -f
can --redirect --organisation='$SESSION_ORGANISATION' api
if($status) exit 0

setenv MSG `../message --api="$USER_ID" --stdin`
if(! $status) then
	echo "Status: 204"
	echo ""
	exit 0
endif
echo "Status: 500"
echo "Content-Type: text/plain"
echo ""
echo "$MSG"
