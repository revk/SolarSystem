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

function send(o)
{
	var a={position:[{type:o.className,id:o.tag}]};
	if(o.x)a.position[0].x=o.x;
	if(o.y)a.position[0].y=o.y;
	if(o.a)a.position[0].a=o.a;
	if(o.t)a.position[0].t=o.t;
	ws.send(JSON.stringify(a));
}

function dodrop(e)
{
	o=document.getElementById(e.dataTransfer.getData("text"));
	if(o)
	{
		o.x=e.x-o.ox;
		o.y=e.y-o.oy;
		console.log(e);
		o.style.position="absolute";
		o.style.left=o.x+"px";
		o.style.top=o.y+"px";
		if('a' in o)x.style.transform="rotate("+(o.a)+"deg)";
		send(o);
	}
}

function iconmenu(menu,e,o)
{
	console.log(menu);
	console.log(e);
	var x=document.createElement("div");
	x.className="menu";
	x.style.position="absolute";
	x.style.left=(e.x-10)+"px";
	x.style.top=(e.y-10)+"px";
	x.onmouseleave=function(){this.parentNode.removeChild(this);}
	document.getElementById("floorplan").appendChild(x);
	menu.forEach(function(m){
		var l=document.createElement("div");
		l.className="menuitem";
		l.textContent=m;
		l.onclick=function(){
			o.t=l.textContent;
			send(o);
			x.onmouseleave=null;
			x.parentNode.removeChild(x);
		};
		x.appendChild(l);
	});
	{
		var l=document.createElement("div");
		l.className="menuitem";
		l.textContent="↻";
		l.onclick=function(){
			if(!o.a)o.a=0;
			o.a+=45;
			if(o.a>=360)o.a-=360;
			send(o);
			x.onmouseleave=null;
			x.parentNode.removeChild(x);
		};
		x.appendChild(l);
	}
	{
		var l=document.createElement("div");
		l.className="menuitem";
		l.textContent="⟲";
		l.onclick=function(){
			if(!o.a)o.a=0;
			o.a-=45;
			if(o.a<0)o.a+=360;
			send(o);
			x.onmouseleave=null;
			x.parentNode.removeChild(x);
		};
		x.appendChild(l);
	}
}

function newobj(o,type,state,classes,menu)
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
		if(menu&&engineering)x.oncontextmenu=function(e){iconmenu(menu,e,this);return false;}
		document.getElementById("floorplan").appendChild(x);
	}
	if('x' in o || 'y' in o)x.style.position="absolute";
	if('x' in o)x.style.left=(x.x=o.x)+"px";
	if('y' in o)x.style.top=(x.y=o.y)+"px";
	if('a' in o)x.style.transform="rotate("+(x.a=o.a)+"deg)";
	if('t' in o)x.t=o.t;
	s="floorplan-"+(x.t?x.t:type)+"-"+state+".svg";
	if(x.src!=s)x.src=s;
	s=type;
	if(classes)s=s+" "+classes;
	if(x.className!=s)x.className=s;
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
				x=newobj(d,"door",d.state,(d.state=="TAMPER")?"tamper":(d.state=="FAULT")?"fault":null,[]);
				if(!x.onclick) x.onclick=function()
				{
					var a={door:[{id:this.tag}]};
					ws.send(JSON.stringify(a));
				}
			});
		        if(o.input)o.input.forEach(function(i)
			{
				x=newobj(i,"input",(i.active?"active":"idle"),i.tamper?"tamper":i.fault?"fault":"",["input","pir","reed"]);
			});
		        if(o.output)o.output.forEach(function(o)
			{
				x=newobj(o,"output",(o.active?"active":"idle"),o.tamper?"tamper":o.fault?"fault":"",["output","lock","bell"]);
			});
	}
	x=document.getElementById("floorplan");
	x.ondragover=allowdrop;
	x.ondrop=dodrop;
}

window.onload=wsconnect;
