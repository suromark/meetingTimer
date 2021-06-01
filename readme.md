# meetingTimer

The meetingTimer by suromark. It's a countdown / stopwatch display. Uses MAX7219 8x8 LED displays driven by an ESP8266 microcontroller. It uses PlatformIO and various libraries from that software ecosystem (check their respective license info).

This is a hobby project, a learning-by-doing experience, and it does not come with any warranty or usefulness for a particular purpose.

It's also most likely a hodgepodge of coding sins, but hey ... everyone has to start somewhere.

## Initial setup/configuration

**NOTE: The orient.txt file that determines the matrix orientation is stored in the SPIFFS filesystem, but I didn't bother to add an integrated first-run setup routine since I knew I'd flash its contents via PlatformIO anyway - so please do not just flash the program binary but also run PlatformIO's "Upload File System image" to make sure the Flash memory of the ESP is properly initialized.**

- The orientation of the dot matrix displays is stored in the file data/orient.txt
- Use characters 0...3 to set the rotation. This depends on the modules you bought. I have encountered two variations of the 4-in-a-row type that require either 1 or 3 for the display to appear right side up.
- Each char in the file corresponds to a 8x8 matrix, in order of wiring. So the first char is the first matrix, 2nd is 2nd, and so on. 
- If there are more displays than characters in the file, the extra displays will be initialized with a default rotation of 1 (see myglobals.h -> panelRotation to alter that value);
- If there are more characters in the file than displays, the extra characters will be ignored.
- Limit is defined in myglobals.h -> numberOfHorizontalDisplays and numberOfVerticalDisplays

## Operation

- After power on, the display always defaults to 0:00 and stopwatch mode
- Short press ( < 1 sec ) the button on D3 to start/stop the timer
- Long press ( +1 sec ) the button to reset the counter to 0:00 and enter programming mode.
- Programming mode allows to cycle once through all digits (long press) and to cycle through 0-9 (or 0-5) for each digit. Once a time unequal to 0:00 is set, the mode is "countdown".
- It'll stop if it reaches 0:00 and will flash. Short press to cancel flash and start the stopwatch ("times up, entering overtime")


## Bill of materials

- 1x Wemos D1 mini ESP-8266 microcontroller
- 1x MAX7219 8x8 LED Matrix strip module (4 in a row pre-connected on a PCB)
- Push Button (Normally Open type)
- Alternative: a touch panel module, and a NPN transistor on its output to act as pulldown on D3
- Resistors 560 Ω (optional, to protect the ESP I/O pins against accidental reverse flow)
- Resistors 40-68 kΩ (optional, to modify the LED matrix current from its far too high default configuration)
- Various wires and connectors (if desired)
- Power supply 5 volts, max. ampere rating depends on whether you've throttled the MAX7219 modules with the resistors or not ...  

## Wire connections

as seen from Wemos D1:

- D3 -> R560 -> Push Button -> GND
- D4 (internal LED)
- D5 -> R560 -> first Matrix CLK
- D6 (MISO, not used here)
- D7 -> R560 -> first Matrix DI
- D8 -> R560 -> first Matrix CS
- GND -> Matrix GND, RTC GND
- 5V -> Matrix VCC, RTC VCC

The matrix modules get wired in series. The VCC and GND lines effectively run parallel through all modules, while each module's last DOUT / CS / CLK pin gets connected to the next module's first DIN / CS / CLK pin.

I recommend feeding the 5V power around the center of the matrix module chain to minimize line length losses, and to not run the modules at full brightness if you haven't done the hardware hack below:

**NOTE: Be aware that full brightness of a single low-budget 4 x 8 x 8 red LED board configured with 10kΩ can exceed 1 A consumption (most of which will be converted to useless heat since it's overdriving the LEDs) so a test run of all 3 will overwhelm the usual 5V / 2A chargers. They *should* shut down but ... no guarantee**

I do recommend (if you have the patience, equipment and soldering skill for fiddling with SMD components) to replace the default 10kΩ current setting resistors of the MAX7219 modules with 40-68 kΩ ones that will significantly lower both heat and power demand of the LED display ...

![Replaced resistor](/docs/img/2019-09-29_181650_IMG_web.jpg)