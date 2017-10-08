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
			l=document.createElement("img");
			l.className="keypad";
			l.src="keypad.png";
			x.append(l);
			l=document.createElement("div");
			l.className="keypadline1";
			x.append(l);
			l=document.createElement("div");
			l.className="keypadline2";
			x.append(l);
			l=document.createElement("div");
			l.className="keypadlabel";
			l.textContent=k.id;
			x.append(l);
			document.getElementById("keypads").append(x);
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
