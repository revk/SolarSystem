#!../login/loggedin /bin/csh -f
if($?PATH_INFO) then
	setenv site "$PATH_INFO:t"
else
	setenv site "$USER_SITE"
endif
can --redirect --site='$site' admin
if($status) exit 0
source ../setcan

if($?aids || $?NEW) then #save
        set aids=(`printenv aids|sed 's/[^0-9A-F	]//g'`)
	foreach a ($aids)
		setenv A "$a"
		setenv D `printenv "NAME$A"`
		setenv R `printenv "ROLLOVER$A"`
		if("$D" == "") then
			sql "$DB" 'DELETE FROM aid WHERE site=$site AND aid="$A"'
		else
			sql "$DB" 'INSERT IGNORE INTO aid SET organisation=$USER_ORGANISATION,site=$site,aid="$A",aidname="$D"'
			sql "$DB" 'UPDATE aid SET aidname="$D" WHERE organisation=$USER_ORGANISATION AND site=$site AND aid="$A"'
		endif
		if("$R" != "") then
			setenv MSG `message --rollover="$A"`
		endif
	end
	if($?USER_ADMIN) then
		if("$NEW" != "") then
			setenv A `makeaid --site=$site --name="$NEW"`
		endif
	endif
	redirect / Updated
	exit 0
endif

xmlsql -C -d "$DB" head.html - foot.html << 'END'
<sql table=site where="site=$site">
<h1>🔢 Manage AIDs for <output name=sitename></h1>
<p>You can add more AIDs if needed for distinct security areas, but this is not usually necessary.</p>
<form method=post>
<table>
<tr><th>AID</th><th>Keys</th><th>Fobs</th></tr>
<sql table=aid WHERE="site=$site">
<tr>
<td><input type=hidden name=aids value="$aid"><input name="NAME$aid" size=20 value="$aidname"></td>
<td title="Key rollover"><tt><b><output name=ver1></b></tt><input type=checkbox name="ROLLOVER$aid" id="ROLLOVER$aid"><label for="ROLLOVER$aid"><if ver2><br><tt><output name=ver2></tt></if><if ver3><br><tt><output name=ver3></tt></if></label></td>
<td>
<sql table='fobaid' where='ver IS NOT NULL AND aid="$aid"' group='ver' select='ver,count(*) AS N' order='if(ver="$ver1",1,if(ver="$ver2",2,if(ver="$ver3",3,4)))'>
<tt><output name=ver></tt> <output name=N><if ver="$ver1"> Current</if><if ver="$ver2" or ver="$ver3"> Old</if><if not ver="$ver1" not ver="$ver2" not ver="$ver3"> (obsolete)</if><br>
</sql>
</td>
</tr>
</sql>
<if USER_ADMIN=true><tr><td><input name=NEW></td><td colspan=2>New AID</td></tr></if>
</table>
<input type=submit value="Update">
<if not USER_ADMIN=true><p>If you need an additional AID, please contact the system controller.</p></if>
</form>
</sql>
<p>Each AID has a current key, and up to two old keys (each with a random two digit code). Fobs will update to the current one on use. You can roll over to create a new key at any time using the checkbox, but all fobs will be have to be used at some point to update. Once a fob is too old it will not work unless re-adopted.</p>
'END'
