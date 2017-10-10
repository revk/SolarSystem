// Floorplan

var backoff=1;
var ws=null;
var engineering=false;
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
		var a={position:[{type:o.className,id:o.tag,x:(e.x-o.ox),y:(e.y-o.oy)}]};
		ws.send(JSON.stringify(a));
		// TODO allow rotation - how
		// TODO allow picking icon (drop on icon to use?)
	}
}
function newobj(o,type,state)
{
	x=document.getElementById(type+o.id);
	if(!x)
	{
		x=document.createElement("img");
		x.id=type+o.id;
		x.tag=o.id;
		x.className=type;
		x.draggable=engineering;
		x.ondragstart=startdrop;
		x.title=o.name?o.name:o.id;
		if(o.x)
		{
			x.style.position="absolute";
			x.style.left=(o.x)+"px";
			x.style.top=(o.y)+"px";
			if('a' in o)x.style.transform="rotate("+(o.a)+"deg)";
		}
		document.getElementById("floorplan").appendChild(x);
	}
	x.src="floorplan-"+type+"-"+state+".svg";
	return x;
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
			if(o.set&&o.set.engineering)engineering=true;
		        if(o.door)o.door.forEach(function(d)
			{
				x=newobj(d,"door",d.state);
				if(!x.onclick) x.onclick=function()
				{
					var a={door:[{id:this.tag}]};
					ws.send(JSON.stringify(a));
				}
			});
		        if(o.input)o.input.forEach(function(i)
			{
				x=newobj(i,"input",(i.tamper?"tamper":i.fault?"fault":i.active?"active":"idle"));
			});
		        if(o.output)o.output.forEach(function(o)
			{
				x=newobj(o,"output",(o.active?"active":"idle"));
			});
	}
	x=document.getElementById("floorplan");
	x.ondragover=allowdrop;
	x.ondrop=dodrop;
}

window.onload=wsconnect;
