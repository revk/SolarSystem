#!../login/loggedin /bin/csh -f
can --redirect --organisation='$USER_ORGANISATION' --site='$USER_SITE' adoptfob
if($status) exit 0
source ../setcan

echo "Content-encoding: none" # so no deflating and hence no caching for interactive status
xmlsql -C -d "$DB" head.html - foot.html << 'END'
<if CANADOPTFOB>
<sql table="device LEFT JOIN aid USING (aid)" where="device.site=$USER_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')"><set found=1></sql>
<if found>
<h2>Adopting a fob</h1>
<form name=f method=post style="display:inline;">
<select name=device>
<sql table="device LEFT JOIN aid USING (aid)" where="device.site=$USER_SITE AND online IS NOT NULL AND (nfctrusted='true' OR nfcadmin='true')">
<option value="$device"><output name=aidname>:<output name=devicename blank="$device"></option>
</sql>
</select>
<input type=submit name=IDENTIFY value="Read fob ID">
<input type=hidden name=fob>
<if not IDENTIFY fob not fob="">
<set mem><set capacity><sql table=fob where="fob='$fob'"><set foundfob=1><if mem><set mem="$mem"></if><if capacity><set capacity="$capacity"></if></sql>
<if not foundfob><p>Sorry, fob <output name=fob> is not recognised by the system. Any fob you wish to use must first be provisioned by the system administrator before it can be adopted. Please contact the system administrator <sql table=user where="admin='true'" limit=1 order="rand()"><output name=username></sql> for more information.</p></if>
<if else>
<h2>Fob <output name=fob href="editfob.cgi/$fob"></h2>
<if IDENTIFIED><sql table=foborganisation where="fob='$fob' AND organisation=$USER_ORGANISATION"><set fobname="$fobname"></sql></if>
<sql table="fobaid LEFT JOIN aid USING (aid) LEFT JOIN access USING (access)" WHERE="fob='$fob' AND organisation=$USER_ORGANISATION">
<p>Already adopted for <output name=aidname> <output name=accessname></p>
</sql>
<input name=fobname size=15 maxlength=15 placeholder="Name" autofocus><if mem> <output name=mem type=mebi>B free</if><br>
<select name=aid><sql table=aid where='site=$USER_SITE'><option value=$aid><output name=aidname></option></sql></select>
<select name=access><sql table=access where='site=$USER_SITE'><option value=$access><output name=accessname></option></sql></select>
<input name=ADOPT type=submit value="Adopt fob">
<if USER_ADMIN=true><input name=FORMAT type=submit value="Format"></if>
</if>
</if>
<if device><ul id=status></ul></if>
</form>
</if>
</if>
'END'

if($?IDENTIFY) then
	echo "<script>e=document.createElement('li');e.textContent='Waiting for fob';document.getElementById('status').append(e);</script>"
	setenv fob `message --device="$device" --fob-identify --silent`
	if("$fob" == "") then
		echo "<script>e=document.createElement('li');e.textContent='Failed';document.getElementById('status').append(e);</script>"
	else
		echo "<script>f.fob.value='$fob';e=document.createElement('input');e.type='hidden';e.name='IDENTIFIED';f.append(e);f.submit();</script>"
	endif
	exit 0
endif
if($?ADOPT) then
	message --device="$device" --fob-adopt="$fob" --aid="$aid" --access="$access" --status=status --silent --fob-name="$fobname" --organisation="$USER_ORGANISATION"
	exit 0
endif
if($?FORMAT) then
	can --redirect admin
	if($status) exit 0
	if($?hardformat) then
		message --device="$device" --fob-format="$fob" --status=status --silent
	else
		message --device="$device" --fob-format="$fob" --status=status --silent
	endif
	exit 0
endif
