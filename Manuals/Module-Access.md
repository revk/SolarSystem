# Access control module

The door control module used with the Solar System alarm panel involves two parts - an outside unit (reader) and an inside unit (controller). You also need an electric lock of some sort (e.g. mag lock), a 12V supply (battery backed up ideally), and typically an exit button. A door closed sensor such as reed switch is usually required as well. If the lock cannot be opened from the inside you may also need some sort of *break glass* emergency exit box in line to the lock power.

The github project includes the software, PCB designs, and the 3D print designs.

## Outside unit (NFC card reader)

The external board is an NFC card reader with *red*, *amber* and *green* status LEDs There is a 3D case design that provides a low profile reader on a wall/door - where ideally the connector wire is in a hold behind the reader. If not, then a 3D printable support is included (may be needed if used on a metal door). The reader also includes a tamber contact if the case is removed. It could also be fitted behind a sheet of glass, but test that it works with your fobs/cards in such case.

The actual contacts on the reader are 0.1" square pins, but typically they would be used with the same mini SPOX plug used in the rest of the system. There are actually 5 pins on the reader - the 5th being a doorbell contact input. If not needed then this 5th pin can be chopped off leaving juse the necessary 4 pins. These 4 pins are connected to the 4 pin header on the controller in the same order.

The reader is a separate GitHub project [PN532](https://github.com/revk/ESP32-PN532).

*Note: This is fitted with a 5 pin 0.1" square pin connector which can be used with either mini SPOX plug, Phœnix Contact PTSM plug, or 0.1" header as needed. Typically Bell is not used, so a 4 pin plug is normal.*

<img src=Round.png width=49%>

## Inside unit (Access2)

The inside unit controls the door access, and is typically small enough to fit in the back of an exit buttonor break glass box, or a shallow surface pattress box. A 3D printed case design is available as well - this is needed for the tamper contact to work.

The board also has a simple RGB LED which can be used to [indicate status](Tech-LED.md).

### Connectors

There have been several generations of connector usage. Older boards may be supplied with mini-SPOX connectors, which need suitable plugs and crimps. Newer boards are supplied with Phœnix Contact PTSM connectors and plugs - these accept solid core wire directly in the plug without the need for a crimp. Either board could be fitted with 0.1" square pin headers instead if needed. The latest designs also have WAGO connectors as an alternative.

The board is provided with 4 or 5 connectors as follows.

<img src=../PCB/Access/Access.png width=49%><img src=../PCB/Access/Access-bottom.png width=49%>

|Connector|Meaning|
|---------|--------|
|Power|This is the power input, with `GND` and `12V`. The board can actually work from `6V` to `40V` but you would normally connected to a 12V DC battery box, which is typically around 13.5V output. This can the same power used for the lock, or a separate power feed if you prefer. WAGO has two 3 connectors for this, older boards have a single 2 way connector.|
|Relay|This can be three way (`GND`, `A`, `B`), or on WAGO boards two way (`A` and `B`). Some boards have a secondary relay as well.|
|NFC|The 4 eway connector goes to the outside NFC reader board, and is `GND`, `5V`, `Tx`, and `Rx`. It connects to the first 4 pins on the NFC reader in the same order. There is also an E-Paper panel with NFC that can be used.|
|Inputs|The 6 way connector is all GPIO pins, but are typically configured to be pairs of `GND` and `input` which go via an external contact such as an exit button, door closed contact, and lock disengaged contact. Their usage is configurable and can allow all 6 to be input pins if required.|

The board may have a USB-C connector as well, or may not, as this is used for programming and debug. If fitted it can be used to flash the board. If not, then there are 5 pads that can be used with an <a href=https://www.tag-connect.com/product/tc2030-usb-nl>TC2030-USB-NL</a> lead.

The WAGO board is the latest design. This is longer but as the conenctors need no plugs can end up taking less space.

<img src=../PCB/AccessWAGO/Access.png width=49%><img src=../PCB/AccessWAGO/Access-bottom.png width=49%>
