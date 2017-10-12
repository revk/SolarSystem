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
	this.lx=e.layerX;
	this.ly=e.layerY;
	console.log(e);
}

function send(o)
{
	var a={position:[{type:o.className,id:o.tag}]};
	if('posx' in o)a.position[0].x=o.posx;
	if('posy' in o)a.position[0].y=o.posy;
	if('posa' in o)a.position[0].a=o.posa;
	if('post' in o)a.position[0].t=o.post;
	ws.send(JSON.stringify(a));
}

function dodrop(e)
{
	o=document.getElementById(e.dataTransfer.getData("text"));
	if(o)
	{
		o.posx=(e.x-o.lx);
		o.posy=(e.y-o.ly);
		o.style.position="absolute";
		o.style.left=o.posx+"px";
		o.style.top=o.posy+"px";
		send(o);
	}
}

function iconmenu(menu,e,o)
{
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
			o.post=l.textContent;
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
			if(!o.posa)o.posa=0;
			o.posa=(o.posa+45);
			if(o.posa>=360)o.posa=o.posa-360;
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
			if(!o.posa)o.posa=0;
			o.posa=o.posa-45;
			if(o.posa<0)o.posa=o.posa+360;
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
		x.title=(o.name?o.name:o.id);
		if(menu&&engineering)x.oncontextmenu=function(e){iconmenu(menu,e,this);return false;}
		document.getElementById("floorplan").appendChild(x);
	}
	if('x' in o || 'y' in o)x.style.position="absolute";
	if('x' in o)x.style.left=(x.posx=o.x)+"px";
	if('y' in o)x.style.top=(x.posy=o.y)+"px";
	if('a' in o)x.style.transform="rotate("+(x.posa=o.a)+"deg)";
	if('t' in o)x.post=o.t;
	s="floorplan-"+(x.post?x.post:type)+"-"+state+".svg";
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
		        if(backoff<10000)backoff=backoff*2;
		        setTimeout(wsconnect,100*backoff);
	}
	ws.onmessage=function(event)
	{
		        o=JSON.parse(event.data);
			if(o.set&&o.set.engineering)engineering=true;
		        if(o.door)o.door.forEach(function(d)
			{
				x=newobj(d,"door",d.state,(d.state=="TAMPER")?"tamper":(d.state=="FAULT")?"fault":null,["door","door2"]);
				if(!x.onclick) x.onclick=function()
				{
					var a={door:[{id:this.tag}]};
					ws.send(JSON.stringify(a));
				}
			});
		        if(o.input)o.input.forEach(function(i)
			{
				x=newobj(i,"input",(i.active?"active":"idle"),i.tamper?"tamper":i.fault?"fault":"",["input","pir","reed","button","glass","smoke"]);
				if(!engineering)x.style.pointerEvents=null;
			});
		        if(o.output)o.output.forEach(function(o)
			{
				x=newobj(o,"output",(o.active?"active":"idle"),o.tamper?"tamper":o.fault?"fault":"",["output","lock","bell","light","beep"]);
				if(!engineering)x.style.pointerEvents=null;
			});
	}
	x=document.getElementById("floorplan");
	x.ondragover=allowdrop;
	x.ondrop=dodrop;
}

window.onload=wsconnect;
