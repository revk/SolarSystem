# SolarSystem
A complete alarm panel using devices compatible with Honeywell/Galaxy RS485 buses.
Also works with a number of WiFi connected devices, designs and code for which are included.

There are two main parts - galaxybus and alarmpanel. The galaxybus does the low level polling and device management but also includes the low level door control logic.

The alarm panel provides the higher level logic.

Build and set up:-

You need to also get projects AXL, Dataformat, and websocket.
The make file assumes these are in ../Dataformat, ../AXL and ../websocket, and they have been built

Then just use make, this creates galaxybus.o and alarmpanel. The alarmpanel is the code.

You then need a config, this is an example is included as panel.xml.

You run with the xml as the argumet. I ran as root, but with suitable permissions on /dev/ttyUSB* and /sys/bus/usb-serial/devices/ttyUSB*/latency_timer you can run as non root.

I have something to keep it running if it stops, I think linux / debian on a pi makes that pretty easy these days too.

By the way, you can get industrial grade SD cards for your Pi, and that is probably a good idea.

You should also run a TLS MQTT server (mosquitto) with username and password for wifi connected devices.
