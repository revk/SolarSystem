// SolarSytem Javascript

window.onload=function()
{
var ws = new WebSocket("ws://"+window.location.host+"/");
ws.onopen=function()
{
	console.log("ws open");
}
ws.onclose=function()
{
	console.log("ws close");
}
ws.onerror=function()
{
	console.log("ws error");
}
ws.onmessage=function(event)
{
	console.log(JSON.parse(event.data));
}
}
