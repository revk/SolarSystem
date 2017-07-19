# SolarSystem
A complete alarm panel using devices compatible wot Honeywell/Galaxy RS485 buses

There are two main parts - galaxybus and alarmpanel. The galaxy bus does the low level polling and device management but also includes the low level door control logic.

The alarm panel provides the higher level logic.

Over time I expect to add more layers with websocket based browser monitoring and control, and config edit systems.
