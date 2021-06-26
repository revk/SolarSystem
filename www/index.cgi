#!../login/loggedin --query /bin/csh -f
echo "Content-Type: text/html"
echo ""
source ../setses
source ../setcan
/projects/tools/bin/xmlsql -d "$DB" head.html - foot.html << 'END'
<IF SESSION_ORGANISATION>
<form style='display:inline-block;' name=site method=post action=setsite.cgi>
<if SESSION_SITE><set SET_SITE=$SESSION_SITE></if>
<select name=SET_SITE onchange='site.submit()'>
<IF NOT SESSION_SITE><option value=0>-- Select site --</option></if>
<sql table="site LEFT JOIN organisation USING (organisation)" WHERE="organisation=$SESSION_ORGANISATION" ORDER="site.description">
<option value=$site><output name=description></option>
</sql>
</select>
</form>
</if>
<ul>
<if USER_ADMIN>
<li><a href="editorganisation.cgi/0">New organisation</a></li>
</if>
<if SESSION_ORGANISATION>
<if CANEDITORGANISATION><li><a href="editorganisation.cgi/$SESSION_ORGANISATION">Edit organisation</a></li></if>
<if CANEDITCLASS><li><a href="editclass.cgi">Edit classes</a></li></if>
<if CANEDITUSER><li><a href="edituser.cgi">Edit users</a></li></if>
<if CANEDITFOB><li><a href="editfob.cgi">Edit fobs</a></li></if>
<if USER_ADMIN><li><a href="editsite.cgi/0">New site</a></li></if>
</if>
<if SESSION_SITE>
<if CANEDITSITE><li><a href="editsite.cgi/$SESSION_SITE">Edit site</a></li></if>
<if CANEDITDEVICE><li><a href="editdevice.cgi">Edit devices</a></li></if>
<if CANEDITAREA><li><a href="editarea.cgi">Edit area</a></li></if>
</if>
<IF USER_ADMIN><sql table=pending limit=1><li><a href="provisiondevice.cgi">Provision new device</a></li></sql></if>
<IF USER_ADMIN><sql table=device where="online is not null AND trusted='true'" limit=1><li><a href="provisionfob.cgi">Provision new fob</a></li></sql></if>
</ul>
<pre>
'END'
