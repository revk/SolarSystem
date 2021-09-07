#!../login/loggedin /bin/csh -f
can --redirect admin
if($status) exit 0
source ../setcan

if($?DELETE) then
	if(! $?SURE) then
		setenv MSG "Tick to say you are sure"
		goto done
	endif
	setenv C `sql -ct "$DB" 'DELETE FROM gpio WHERE pcb=$pcb' 'DELETE FROM pcb WHERE pcb=$pcb'`
	if("$C" == "" || "$C" == "0") then
		setenv MSG "Cannot delete as in use"
		goto done
	endif
	unsetenv pcb
	setenv MSG Deleted
	goto done
endif
if($?COPY) then
	setenv C `sql "$DB" 'SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name="pcb" and column_name<>"pcb" AND column_name<>"pcbname"'|sed '2,$s/^/,/'`
	setenv N `sql -i "$DB" 'INSERT INTO pcb (pcb,pcbname,'"$C"') SELECT 0,concat(pcbname," copy"),'"$C"' FROM pcb WHERE pcb=$pcb'`
	setenv C `sql "$DB" 'SELECT column_name FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name="gpio" and column_name<>"gpio" AND column_name<>"pcb"'|sed '2,$s/^/,/'`
	sql "$DB" 'INSERT INTO gpio (gpio,pcb,'"$C"') SELECT 0,$N,'"$C"' FROM gpio WHERE pcb=$pcb'
	setenv pcb "$N"
	goto done
endif
if($?pcbname) then # save
	set changed=0
	if($pcb == 0) then
		setenv pcb `sql -i "$DB" 'INSERT INTO pcb SET pcb=0'`
	endif
	@ changed = $changed + `sqlwrite -c "$DB" pcb`
	set gpio=(`printenv gpio|sed 's/[^0-9 	]//g'`)
	set pin=(`printenv pin|sed 's/[^-0-9 	]//g'`)
	set io=(`printenv io|sed 's/[^-0-9A-Z 	]//g'`)
	set inittype=(`printenv inittype|sed 's/[^-0-9A-Z 	]//g'`)
	setenv set ""
	foreach n ($gpio)
	setenv n "$n"
	setenv g "$pin[1]"
	shift pin
	setenv t "$io[1]"
	shift io
	setenv i "$inittype[1]"
	shift inittype
	if("$n" == "0") then
		if("$g" != "-") then
			if("$initname" == "") then
				setenv MSG "Specify pin name"
				goto done
			endif
			@ changed = $changed + 1
			if("$inithold" == "")setenv inithold 0
			if("$initpulse" == "")setenv initpulse 0
			if("$value0" == "")setenv value0 Low
			if("$value1" == "")setenv value1 High
			if(! $?initinvert) setenv initinvert false
			set n=`sql -i "$DB" 'INSERT INTO gpio SET pcb="$pcb",pin="$g",io="$t",inittype="$i",initname="$initname",inithold=$inithold,initpulse=$initpulse,initinvert="$initinvert",value0="$value0",value1="$value1"'`
		endif
	else
		if("$g" == "-") then
			@ changed = $changed + `sql -c "$DB" 'DELETE FROM gpio WHERE gpio="$n" AND pcb="$pcb"'`
		else
			setenv name `printenv "initname$n"`
			setenv hold `printenv "inithold$n"`
			setenv pulse `printenv "initpulse$n"`
			setenv invert `printenv "initinvert$n"`
			setenv v0 `printenv "value0$n"`
			if("$v0" == "")setenv v0 Low
			setenv v1 `printenv "value1$n"`
			if("$v1" == "")setenv v1 High
			if("$invert" == "") setenv invert false
			@ changed = $changed + `sql -c "$DB" 'UPDATE gpio SET pin="$g",io="$t",inittype="$i",initname="$name",inithold=$hold,initpulse=$pulse,initinvert="$invert",value0="$v0",value1="$v1" WHERE gpio="$n" AND pcb="$pcb" AND (pin<>"$g" OR io<>"$t" OR inittype<>"$i" OR initname<>"$name" OR inithold<>$hold OR initpulse<>$pulse OR initinvert<>"$invert" OR value0<>"$v0" OR value1<>"$v1")'`
		endif
	endif
	setenv set "$set,$n"
	end
	@ changed = $changed + `sql -c "$DB" 'DELETE FROM gpio WHERE pcb=$pcb AND gpio NOT IN ($,set)'`
	sql "$DB" 'UPDATE device SET poke=NOW() WHERE pcb=$pcb'
       	message --poke
	if($changed) goto done
	redirect editpcb.cgi
	exit 0
endif
done:
source ../types
if($?PATH_INFO) then
	setenv pcb "$PATH_INFO:t"
endif
unsetenv gpio
unsetenv pin
unsetenv inittype
unsetenv io
unsetenv initname
xmlsql -C -d "$DB" head.html - foot.html << END
<h1>PCB template</h1>
<if not pcb>
<table>
<sql table=pcb order=pcbname>
<tr>
<td><output name=pcbname blank="Unspecified" missing="Unnamed"  href="/editpcb.cgi/\$pcb"></td>
</tr>
</sql>
</table>
<a href="/editpcb.cgi/0">New PCB</a>
</if><if else>
<form method=post action=/editpcb.cgi name=F><input type=hidden name=pcb>
<sql table=pcb key=pcb>
<table>
<tr><td>Name</td><td><input name=pcbname size=40 autofocus></td></tr>
<tr><td><select name=tamper>$GPIONUMPICK</select></td><td>GPIO Controller Tamper</td></tr>
<if ledr=='-' ledg=='-' ledb=='-'><tr><td><select name=leda>$GPIONUMPICK</select></td><td>GPIO Controller LED (amber)</td></tr></if>
<if leda=='-'><tr><td><select name=ledr>$GPIONUMPICK</select></td><td>GPIO Controller LED (red)</td></tr></if>
<if leda=='-'><tr><td><select name=ledg>$GPIONUMPICK</select></td><td>GPIO Controller LED (green)</td></tr></if>
<if leda=='-'><tr><td><select name=ledb>$GPIONUMPICK</select></td><td>GPIO Controller LED (blue)</td></tr></if>
<tr><td><select name=keypadtx onchange='F.submit();'>$GPIONUMPICK</select></td><td>GPIO Keypad Tx</td></tr>
<if not keypadtx=='-'>
<tr><td><select name=keypadrx>$GPIONUMPICK</select></td><td>GPIO Keypad Rx</td></tr>
<tr><td><select name=keypadde>$GPIONUMPICK</select></td><td>GPIO Keypad DE</td></tr>
<tr><td><select name=keypadre>$GPIONUMPICK</select></td><td>GPIO Keypad RE</td></tr>
</if>
<tr><td><select name=nfctx onchange='F.submit();'>$GPIONUMPICK</select></td><td>GPIO NFC Tx</td></tr>
<if not nfctx=='-'>
<tr><td><select name=nfcrx>$GPIONUMPICK</select></td><td>GPIO NFC Rx</td></tr>
<tr><td><select name=nfcpower>$GPIONUMPICK</select></td><td>GPIO NFC Power</td></tr>
<tr><td><select name=nfcred>$GPIONFCPICK</select></td><td>PN532 NFC red LED</td></tr>
<tr><td><select name=nfcamber>$GPIONFCPICK</select></td><td>PN532 NFC amber LED</td></tr>
<tr><td><select name=nfcgreen>$GPIONFCPICK</select></td><td>PN532 NFC green LED</td></tr>
<tr><td><select name=nfccard>$GPIONFCPICK</select></td><td>PN532 NFC LED to blink for card</td></tr>
<tr><td><select name=nfctamper>$GPIONFCPICK</select></td><td>PN532 NFC Tamper button</td></tr>
<tr><td><select name=nfcbell>$GPIONFCPICK</select></td><td>PN532 NFC Bell input</td></tr>
</if>
<sql table=gpio where="pcb=\$pcb" order=io,inittype,initname><set initinvert\$gpio="\$initinvert">
<tr><td><input name=gpio type=hidden><select name=pin>$GPIONUMPICK</select></td><td><input name="initinvert\$gpio" type=checkbox value=true title="Invert"> <input name="value0\$gpio" size=5 value="\$value0" placeholder="0 name"><input name="value1\$gpio" size=5 value="\$value1" placeholder="1 name"> <select name=io>$GPIOIOPICK</select><select name=inittype>$GPIOTYPEPICK</select> <input name="initname\$gpio" value="\$initname" size=10> <input name="inithold\$gpio" size=3 value="\$inithold">ms <input name="initpulse\$gpio" size=5 value="\$initpulse">s/10</td></tr></td>
</sql>
<tr><td><input name=gpio type=hidden value=0><select name=pin>$GPIONUMPICK</select></td><td><input name=initinvert type=checkbox value=true title="Invert"> <input name="value0" size=5 value="" placeholder="0 name"><input name="value1" size=5 value="" placeholder="1 name"> <select name=io>$GPIOIOPICK</select><select name=inittype>$GPIOTYPEPICK</select> <input name=initname size=10 placeholder='New pin'> <input name=inithold size=3 placeholder="Hold">ms <input name=initpulse size=5 placeholder="Pulse">s/10</td></tr></td>
</table>
</sql>
<input type=submit value="Update">
<IF not pcb=0><input type=submit value="Copy" name=COPY></if>
<IF not pcb=0><input type=submit value="Delete" name=DELETE><input type=checkbox name=SURE title='Tick this to say you are sure'></if>
</form>
</if>
</sql>
END
