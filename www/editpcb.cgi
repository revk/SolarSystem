#!../login/loggedin /bin/csh -f
can --redirect admin
if($status) exit 0

if($?DELETE) then
 setenv C `sql -c "$DB" 'DELETE FROM pcb WHERE pcb=$pcb'`
 if("$C" == "" || "$C" == "0") then
  setenv MSG "Cannot delete as in use"
  goto done
 endif
 unsetenv pcb
 setenv MSG Deleted
 goto done
endif
if($?description) then # save
 if($pcb == 0) then
  setenv pcb `sql -i "$DB" 'INSERT INTO pcb SET pcb=0'`
 endif
 sqlwrite "$DB" pcb
 echo "Location: ${ENVCGI_SERVER}/editpcb.cgi"
 echo ""
 exit 0
endif
done:
echo "Content-Type: text/html"
echo ""
if($?PATH_INFO) then
	setenv pcb "$PATH_INFO:t"
endif
xmlsql -d "$DB" head.html - foot.html << 'END'
<h1>PCB template</h1>
<if not pcb>
<table>
<sql table=pcb order=description>
<tr>
<td><output name=description blank="Unspecified" missing="?"  href="/editpcb.cgi/$pcb"></td>
</tr>
</sql>
</table>
<a href="/editpcb.cgi/0">New PCB</a>
</if><if else>
<form method=post action=/editpcb.cgi><input type=hidden name=pcb>
<sql table=pcb key=pcb>
<table>
<tr><td>Name</td><td><input name=description ize=40 autofocus></td></tr>
</table>
</sql>
<input type=submit value="Update">
<IF not pcb=0><input type=submit value="Delete" name=DELETE></if>
</form>
</if>
</sql>
'END'
