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
setenv GPIO "2 4 5 12 13 14 15 16 17 18 19 21 22 23 25 26 27 32 33 34 35 36 39"
setenv NFCGPIO "30 31 32 33 34 35 71 72"
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
<tr><td><select name=nfcpower><option value=''>-- None --</option><for space G="$GPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>GPIO NFC Power</td></tr>
<tr><td><select name=nfctx><option value=''>-- None --</option><for space G="$GPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>GPIO NFC Tx</td></tr>
<tr><td><select name=nfcrx><option value=''>-- None --</option><for space G="$GPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>GPIO NFC Rx</td></tr>
<tr><td><select name=nfcred><option value=''>-- None --</option><for space G="$NFCGPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>PN532 NFC Red LED</td></tr>
<tr><td><select name=nfcamber><option value=''>-- None --</option><for space G="$NFCGPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>PN532 NFC Amber LED</td></tr>
<tr><td><select name=nfcgreen><option value=''>-- None --</option><for space G="$NFCGPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>PN532 NFC Green LED</td></tr>
<tr><td><select name=nfccard><option value=''>-- None --</option><for space G="$NFCGPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>PN532 NFC LED to blink for card</td></tr>
<tr><td><select name=nfctamper><option value=''>-- None --</option><for space G="$NFCGPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>PN532 NFC Tamper button</td></tr>
<tr><td><select name=nfcbell><option value=''>-- None --</option><for space G="$NFCGPIO"><option value=$G><output name=G></option><option value=-$G><output name=G> (inverted)</option></for></select></td><td>PN532 NFC Bell input</td></tr>
</table>
</sql>
<input type=submit value="Update">
<IF not pcb=0><input type=submit value="Delete" name=DELETE></if>
</form>
</if>
</sql>
'END'
