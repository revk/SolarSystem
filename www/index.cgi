#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
source ../setses
source ../setcan
/projects/tools/bin/xmlsql -d "$DB" head.html - foot.html << 'END'
<ul>
<if USER_ADMIN>
<li><a href="editorganisation.cgi/0">New organisation</a></li>
</if>
<if SESSION_ORGANISATION>
<if CANEDITORGANISATION><li><a href="editorganisation.cgi/$SESSION_ORGANISATION">Edit organisation</a></li></if>
<if CANEDITCLASS><li><a href="editclass.cgi">Edit classes</a></li></if>
<if CANEDITUSER><li><a href="edituser.cgi">Edit users</a></li></if>
<if CANEDITSITE><li><a href="editsite.cgi">Edit sites</a></li></if>
<if CANEDITDEVICE><li><a href="editdevice.cgi">Edit devices</a></li></if>
<if CANEDITFOB><li><a href="editfob.cgi">Edit fobs</a></li></if>
<if CANEDITAREA><li><a href="editarea.cgi">Edit area</a></li></if>
</if>
</ul>
<pre>
'END'
