#!../login/loggedin --query /bin/csh -f
source ../setses
source ../setcan
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<ul>
<if USER_ADMIN>
<li><a href="editorganisation.cgi/0">New organisation</a></li>
</if>
<if SESSION_ORGANISATION>
<if CANEDITORGANISATION><li><a href="editorganisation.cgi/$SESSION_ORGANISATION">Edit organisation</a></li></if>
<if CANEDITUSER><li><a href="edituserorganisation.cgi">Edit users</a></li></if>
<if CANEDITFOB><li><a href="editfob.cgi">Edit fobs</a></li></if>
<if ADMINORGANISATION><li><a href="editaid.cgi">Edit AID</a></li></if>
<if USER_ADMIN><li><a href="editsite.cgi/0">New site</a></li></if>
</if>
<if SESSION_SITE>
<if CANEDITSITE><li><a href="editsite.cgi/$SESSION_SITE">Edit site</a></li></if>
<if CANEDITDEVICE><li><a href="editdevice.cgi">Edit devices</a></li></if>
<if CANEDITAREA><li><a href="editarea.cgi">Edit area</a></li></if>
<IF USER_ADMIN><sql table=pending limit=1 WHERE="online<NOW()"><li><a href="provisiondevice.cgi">Provision new device</a></li></sql></if>
</if>
<IF USER_ADMIN><sql table=device where="online is not null AND trusted='true'" limit=1><li><a href="provisionfob.cgi">Provision new fob</a></li></sql></if>
<IF USER_ADMIN><li><a href="editpcb.cgi">Edit PCB</a></li></if>
</ul>
<pre>
'END'
