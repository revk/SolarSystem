#!../login/loggedin /bin/csh -fx
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
 set gpio=(`printenv gpio|sed 's/^-[0-9] //g'`)
 set gpiotype=(`printenv gpiotype|sed 's/^-[0-9] //g'`)
 set n=0
 setenv set ""
 foreach g ($gpio)
	setenv g "$g"
	setenv t "$gpiotype[1]"
	shift gpiotype
	if("$g" == 0) continue
	setenv c `sql "$DB" 'SELECT COUNT(*) FROM pcbgpio WHERE pcb=$pcb AND gpio="$g" AND type="$t"'`
	if("$c" == "0" || "$c" == "") then
		@ n = $n + `sql -c "$DB" 'REPLACE INTO pcbgpio SET pcb=$pcb,gpio="$g",type="$t",pinname="$pinname"'`
	endif
	setenv set "$set,$g"
 end
 @ n = $n + `sql -c "$DB" 'DELETE FROM pcbgpio WHERE pcb=$pcb AND gpio NOT IN ($,set)'`
 unsetenv gpio
 unsetenv gpiotype
 unsetenv pinname
 if($n)goto done
 echo "Location: ${ENVCGI_SERVER}/editpcb.cgi"
 echo ""
 exit 0
endif
done:
echo "Content-Type: text/html"
echo ""
setenv PICKGPIO '<option value="">-- None --</option><for space G="2 4 5 12 13 14 15 16 17 18 19 21 22 23 25 26 27 32 33 34 35 36 39"><option value=\$G><output name=G></option><option value=-\$G><output name=G> (inverted)</option></for>'
setenv PICKNFCGPIO '<option value="">-- None --</option><for space G="30 31 32 33 34 35 71 72"><option value=\$G><output name=G></option><option value=-\$G><output name=G> (inverted)</option></for>'
setenv PICKTYPE	'<option value=IO>Input/Output</option><option value=I>Input</option><option value=O>Output</option>'
if($?PATH_INFO) then
	setenv pcb "$PATH_INFO:t"
endif
xmlsql -d "$DB" head.html - foot.html << END
<h1>PCB template</h1>
<if not pcb>
<table>
<sql table=pcb order=description>
<tr>
<td><output name=description blank="Unspecified" missing="?"  href="/editpcb.cgi/\$pcb"></td>
</tr>
</sql>
</table>
<a href="/editpcb.cgi/0">New PCB</a>
</if><if else>
<form method=post action=/editpcb.cgi><input type=hidden name=pcb>
<sql table=pcb key=pcb>
<table>
<tr><td>Name</td><td><input name=description ize=40 autofocus></td></tr>
<tr><td><select name=tamper>$PICKGPIO</select></td><td>GPIO Controller Tamper</td></tr>
<tr><td><select name=blink>$PICKGPIO</select></td><td>GPIO Controller LED</td></tr>
<tr><td><select name=nfcpower>$PICKGPIO</select></td><td>GPIO NFC Power</td></tr>
<tr><td><select name=nfctx>$PICKGPIO</select></td><td>GPIO NFC Tx</td></tr>
<tr><td><select name=nfcrx>$PICKGPIO</select></td><td>GPIO NFC Rx</td></tr>
<tr><td><select name=nfcred>$PICKNFCGPIO</select></td><td>PN532 NFC red LED</td></tr>
<tr><td><select name=nfcamber>$PICKNFCGPIO</select></td><td>PN532 NFC amber LED</td></tr>
<tr><td><select name=nfcgreen>$PICKNFCGPIO</select></td><td>PN532 NFC green LED</td></tr>
<tr><td><select name=nfccard>$PICKNFCGPIO</select></td><td>PN532 NFC LED to blink for card</td></tr>
<tr><td><select name=nfctamper>$PICKNFCGPIO</select></td><td>PN532 NFC Tamper button</td></tr>
<tr><td><select name=nfcbell>$PICKNFCGPIO</select></td><td>PN532 NFC Bell input</td></tr>
<sql table=pcbgpio where="pcb=\$pcb" order=type>
<tr><td><select name=gpio>$PICKGPIO</select></td><td><select name=gpiotype>$PICKTYPE</select> <output name=pinname></td></tr></td>
</sql>
<tr><td><select name=gpio>$PICKGPIO</select></td><td><select name=gpiotype>$PICKTYPE</select> <input name=pinname size=10 placeholder='New pin'></td></tr></td>
</table>
</sql>
<input type=submit value="Update">
<IF not pcb=0><input type=submit value="Delete" name=DELETE></if>
</form>
</if>
</sql>
END
