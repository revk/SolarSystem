#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
if(! $?SESSION_ORGANISATION && ! $?USER_ADMIN) then
	setenv C `sql "$DB" 'SELECT COUNT(*) FROM userorganisation WHERE user=$USER_ID'`
	if("$C" == 1) then
		setenv SESSION_ORGANISATION `sql "$DB" 'SELECT organisation FROM userorganisation WHERE user=$USER_ID'`
	endif
endif

if($?SESSION_ORGANISATION) then
can --organisation=$SESSION_ORGANISATION editoriganisation
setenv CANTEDITORGANISATION $status
can --organisation=$SESSION_ORGANISATION editclass
setenv CANTEDITCLASS $status
can --organisation=$SESSION_ORGANISATION editsite
setenv CANTEDITSITE $status
can --organisation=$SESSION_ORGANISATION editdevice
setenv CANTEDITDEVICE $status
can --organisation=$SESSION_ORGANISATION editfob
setenv CANTEDITFOB $status
can --organisation=$SESSION_ORGANISATION edituser
setenv CANTEDITUSER $status
endif

/projects/tools/bin/xmlsql -d "$DB" head.html - foot.html << 'END'
<ul>
<if USER_ADMIN>
<li><a href="editorganisation.cgi/0">New organisation</a></li>
</if>
<if SESSION_ORGANISATION>
<if CANTEDITORGANISATION=0><li><a href="editorganisation.cgi/$SESSION_ORGANISATION">Edit organisation</a></li></if>
<if CANTEDITCLASS=0><li><a href="editclass.cgi">Edit class</a></li></if>
<if CANTEDITSITE=0><li><a href="editsite.cgi">Edit sites</a></li></if>
<if CANTEDITUSER=0><li><a href="edituser.cgi">Edit users</a></li></if>
<if CANTEDITDEVICE=0><li><a href="editdevice.cgi">Edit devices</a></li></if>
<if CANTEDITFOB=0><li><a href="editfob.cgi">Edit fobs</a></li></if>
</if>
</ul>
<pre>
'END'
