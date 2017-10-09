// Floorplan

var backoff=1;
var ws=null;
function wsconnect()
{
	//document.getElementById("status").textContent="Connecting";
	ws = new WebSocket((window.location.protocol=="https:"?"wss://":"ws://")+window.location.host+window.location.pathname);
	ws.onopen=function()
	{
		        document.getElementById("floorplan").style.background="none";
		        backoff=1;
	}
	ws.onclose=function()
	{
		        document.getElementById("floorplan").style.background="yellow";
		        backoff=backoff*2;
		        setTimeout(wsconnect,100*backoff);
	}
	ws.onerror=function()
	{
		        document.getElementById("floorplan").style.background="red";
	}
	ws.onmessage=function(event)
	{
		        o=JSON.parse(event.data);
	}
}

window.onload=wsconnect;
