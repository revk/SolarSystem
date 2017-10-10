// Floorplan

var backoff=1;
var ws=null;
function wsconnect()
{
	//document.getElementById("status").textContent="Connecting";
	ws = new WebSocket((window.location.protocol=="https:"?"wss://":"ws://")+window.location.host+window.location.pathname);
	ws.onopen=function()
	{
		        backoff=1;
	}
	ws.onclose=function()
	{
		        backoff=backoff*2;
		        setTimeout(wsconnect,100*backoff);
	}
	ws.onmessage=function(event)
	{
		        o=JSON.parse(event.data);
		        if(o.door)o.door.forEach(function(d)
			{
				x=document.getElementById(d.id);
				if(!x)
				{
					x=document.createElement("img");
					x.id=d.id;
					x.className="door";
					x.draggable=true;
					x.ondblclick=function()
					{
						var a={door:[{id:this.id}]};
						ws.send(JSON.stringify(a));
					}
					document.getElementById("floorplan").appendChild(x);
				}
				x.src="floorplan-door-"+d.state+".svg";
				x.title=d.name?d.name:d.id;
			});
		        if(o.input)o.input.forEach(function(i)
			{
				x=document.getElementById(i.id);
				if(!x)
				{
					x=document.createElement("img");
					x.id=i.id;
					x.className="input";
					x.draggable=true;
					document.getElementById("floorplan").appendChild(x);
				}
				x.src="floorplan-input-"+(i.tamper?"tamper":i.fault?"fault":i.active?"active":"idle")+".svg";
				x.title=i.name?i.name:i.id;
			});
	}
}

window.onload=wsconnect;
