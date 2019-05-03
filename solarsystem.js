// SolarSytem Javascript

// Set up web socket connection
var backoff=1;
var ws=null;
function wsconnect()
{
ws = new WebSocket((window.location.protocol=="https:"?"wss://":"ws://")+window.location.host+window.location.pathname);
ws.onopen=function()
{
	document.getElementById("main").style.background="none";
	backoff=1;
}
ws.onclose=function()
{
	document.getElementById("main").style.background="yellow";
	if(backoff<10000)backoff=backoff*2;
	setTimeout(wsconnect,100*backoff);
}
ws.onerror=function()
{
	document.getElementById("main").style.background="red";
}
function io(dir,e)
{
	dev=document.getElementById(e.dev);
	if(!dev)
	{ // New device
			dev=document.createElement("table");
			dev.id=e.dev;
			dev.className=e.type;
			r=document.createElement("tr");
			dev.appendChild(r);
			c=document.createElement("th");
			c.textContent=e.dev;
			c.colSpan="2";
			r.appendChild(c);
			document.getElementById("io").appendChild(dev);
	}
	if(!dir)return dev; // Device level
	d=document.getElementById(dir+e.id);
	if(!d)
	{ // New port
		r=document.createElement("tr");
		c=document.createElement("td");
		c.textContent=e.port;
		r.appendChild(c);
		d=document.createElement("td");
		d.id=dir+e.id;
		r.appendChild(d);
		dev.appendChild(r);
	}
	if(d)
	{
		d.textContent=(e.name?e.name:e.id);
		d.title=e.id;
	}
	return d;
}
ws.onmessage=function(event)
{
	o=JSON.parse(event.data);
	if(o.group)o.group.forEach(function(g)
	{
		x=document.getElementById("group"+g.id);
		if(!x)
		{
			x=document.createElement("div");
			x.id="group"+g.id;
			x.className="group";
			document.getElementById("groups").appendChild(x);
			l=document.createElement("img");
			l.className="groupimg";
			l.groupid=g.id;
			l.onclick=function()
			{
				var a;
				if(this.getAttribute("data-fault_latch")
					|| this.getAttribute("data-tamper_latch")
					|| this.getAttribute("data-intruder_latch")
				)a={reset:[this.groupid]};
				else if(this.getAttribute("data-set")||this.getAttribute("data-arm")) a={disarm:[this.groupid]};
				else a={arm:[this.groupid]};
				ws.send(JSON.stringify(a));
			}
			x.appendChild(l);
			l=document.createElement("div");
			l.className="groupid";
			l.textContent=g.id;
			x.appendChild(l);
			if(g.name)
			{
				l=document.createElement("div");
				l.className="groupname";
				l.textContent=g.name;
				x.appendChild(l);
			}
		}
	});
	function groupstate(x)
	{
		if(x.getAttribute("data-set"))x.src="groupSET.svg";
		else if(x.getAttribute("data-arm"))x.src="groupARM.svg";
		else if(x.getAttribute("data-fault_latch"))x.src="groupFAULT.svg";
		else if(x.getAttribute("data-intruder_latch"))x.src="groupINTRUDER.svg";
		else if(x.getAttribute("data-tamper_latch"))x.src="groupTAMPER.svg";
		else x.src="groupUNSET.svg";
	}
	if(o.clr)for(var s in o.clr)
	{
		o.clr[s].forEach(function(g)
			{
			x=document.getElementById("group"+g).children[0];
			x.setAttribute("data-"+s,"");
			groupstate(x);
			});
	};
	if(o.set)for(var s in o.set)
	{
		o.set[s].forEach(function(g)
			{
			x=document.getElementById("group"+g).children[0];
			x.setAttribute("data-"+s,1);
			groupstate(x);
			});
	};
	if(o.keypad)o.keypad.forEach(function(k)
	{
		x=document.getElementById(k.id);
		if(!x)
		{
			x=document.createElement("div");
			x.id=k.id;
			x.className="keypad";
			x.title=k.id;
			document.getElementById("keypads").appendChild(x);
			l=document.createElement("img");
			l.className="keypad";
			l.src="keypad.png";
			x.appendChild(l);
			l=document.createElement("pre");
			l.className="keypadline1";
			x.appendChild(l);
			l=document.createElement("pre");
			l.className="keypadline2";
			x.appendChild(l);
			l=document.createElement("div");
			l.className="keypadlabel";
			l.textContent=(k.name?k.name:k.id);
			x.appendChild(l);
			l=document.createElement("div");
			l.className="keypadbuttons";
			x.appendChild(l);
			keys=['1','2','3','A','4','5','6','B','7','8','9','ent','*','0','#','esc'];
			for(var n=0;n<16;n++)
			{
				b=document.createElement("button");
				b.className="keypadbutton";
				b.style.left=((n%4)*25)+"%";
				b.style.top=(Math.floor(n/4)*25)+"%";
				b.key=keys[n];
				b.onclick=function()
				{
					var a={keypad:[{id:k.id,key:this.key}]};
					ws.send(JSON.stringify(a));
				}
				l.appendChild(b);
			}
		}
		x.children[1].textContent=k.line[0];
		x.children[2].textContent=k.line[1];
	});
	if(o.door)o.door.forEach(function(d)
	{
		x=document.getElementById(d.id);
		if(!x)
		{
			x=document.createElement("div");
			x.id=d.id;
			x.className="door";
			x.title=d.id;
			document.getElementById("doors").appendChild(x);
			l=document.createElement("div");
			l.className="doorlabel";
			l.textContent=d.name;
			x.appendChild(l);
			l=document.createElement("img");
			l.className="doorimg";
			l.textContent=d.name;
			l.doorid=d.id;
			l.onclick=function()
			{
				var a={door:[{id:this.doorid}]};
				ws.send(JSON.stringify(a));
			}
			x.appendChild(l);
			l=document.createElement("div");
			l.className="doorstate";
			l.textContent=d.name;
			x.appendChild(l);
		}
		x.children[1].src="door"+d.state+".png";
		x.children[2].textContent=d.state;
	});
	if(o.device)o.device.forEach(function(o)
	{	// Log updates
		x=io(false,o);
		if(!x.className) x.className=(o.active?"deviceactive":"deviceidle");
	});
	if(o.output)o.output.forEach(function(o)
	{	// Log updates
		x=io("O",o);
		x.className=(o.active?"outputactive":"outputidle");
	});
	if(o.input)o.input.forEach(function(i)
	{
		x=io("I",i);
		x.className="input"+(i.tamper?"tamper":i.fault?"fault":i.active?"active":"idle");
	});
}
}

window.onload=wsconnect;

