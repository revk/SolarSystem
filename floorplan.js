// Floorplan

var backoff=1;
var ws=null;
function allowdrop(e)
{
	e.preventDefault();
}
function startdrop(e)
{
	e.dataTransfer.setData("text", e.target.id);
	this.ox=e.offsetX;
	this.oy=e.offsetY;
}
function dodrop(e)
{
	o=document.getElementById(e.dataTransfer.getData("text"));
	if(o)
	{
		console.log(e);
		o.style.position="absolute";
		o.style.left=(e.x-o.ox)+"px";
		o.style.top=(e.y-o.oy)+"px";
		// TODO reporting back position to permanently store
		// TODO allow rotation - how
		// TODO allow picking icon (drop on icon to use?)
	}
}
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
					x.ondragstart=startdrop;
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
				x=document.getElementById("input"+i.id);
				if(!x)
				{
					x=document.createElement("img");
					x.id="input"+i.id;
					x.className="input";
					x.draggable=true;
					x.ondragstart=startdrop;
					x.title=i.name?i.name:i.id;
					document.getElementById("floorplan").appendChild(x);
				}
				x.src="floorplan-input-"+(i.tamper?"tamper":i.fault?"fault":i.active?"active":"idle")+".svg";
			});
		        if(o.output)o.output.forEach(function(o)
			{
				x=document.getElementById("output"+o.id);
				if(!x)
				{
					x=document.createElement("img");
					x.id="output"+o.id;
					x.className="output";
					x.draggable=true;
					x.ondragstart=startdrop;
					x.title=o.name?o.name:o.id;
					document.getElementById("floorplan").appendChild(x);
				}
				x.src="floorplan-output-"+(o.active?"active":"idle")+".svg";
			});
	}
	x=document.getElementById("floorplan");
	x.ondragover=allowdrop;
	x.ondrop=dodrop;
}

window.onload=wsconnect;
