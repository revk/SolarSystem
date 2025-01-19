# Bell box controller

A design is included for a bell box control module - this is specifically tailored to suit installation in a Pyronix bell box with a 2 way and 8 way connector directly mapping to the 10 screw terminals for power, bell, engineering hold off, strobe, backlight disable, tamper, and fault. Some models do not have fault or backlight. A separate 2 pin power connection is also provided allowing power to this unit which is passed on to the bell box.

This can be used as a general purpose unit providing two simple contact inputs (normally tamper and fault) and four outputs that are driven to power supply or pulled to ground. The inputs include ESD protection diodes and resistors.

Power can be 4.5V to 18V but will normally be the 12V required for the bell box. A USB-C connector is provided for debug and programming. An RGB status LED is also included on board.

## GPS

The module can also have a 5 pin header fitted to allow connection to a separate GPS module. This can be conected directly or via a lead to allow more suitable placement. It is used to provide a time reference for the system even when not internet connected.

Again, a 3D case design is availble, but the board itself does not have a tamper contact - it connects to the Tamper in the bell box itself.

<img src=../PCB/Bell/Bell.png width=49%>