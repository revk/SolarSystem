// SolarSytem Javascript

window.onload=function()
{
// Set up web socket connection
var ws = new WebSocket((window.location.protocol=="https:"?"wss://":"ws://")+window.location.host+"/");
ws.onopen=function()
{
	document.getElementById("status").textContent="Connected";
}
ws.onclose=function()
{
	document.getElementById("status").textContent="DISCONNECTED";
	setTimeout(function(){window.location.reload(true)},1000);
}
ws.onerror=function(event)
{
	document.getElementById("status").textContent="Error: "+event.data;
}
ws.onmessage=function(event)
{
	o=JSON.parse(event.data);
	if(o.keypad)o.keypad.forEach(function(k){
		x=document.getElementById(k.id);
		if(!x)
		{
			x=document.createElement("div");
			x.id=k.id;
			x.className="keypad";
			document.getElementById("keypads").appendChild(x);
			l=document.createElement("img");
			l.className="keypad";
			l.src="keypad.png";
			x.appendChild(l);
			l=document.createElement("div");
			l.className="keypadline1";
			x.appendChild(l);
			l=document.createElement("div");
			l.className="keypadline2";
			x.appendChild(l);
			l=document.createElement("div");
			l.className="keypadlabel";
			l.textContent=k.id;
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
	if(o.input)
	{	// Input updates
	}
	if(o.log)
	{	// Log updates
	}
}
}
