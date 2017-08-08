FAB_LED Rainbow Letter Sign Sample Program
==========================================

The letters are cut out of wood, and uses WS2811 30mm LED bulbs/pucks which each contain three 5050 LEDs, and consume 0.78Watt at 12V.  There are 3 letters with each usin 26,36 and 42 pixels.

The program is self explanatory with a header that describes the modes.

I will describe verbally the build for the letters:

The letters are made of particle board cut with a jigsaw. The width of the letter legs is 20cm. Because the wood is flimsy, I glued with Elmer's wood glue 1" or 1 1/2" sticks in the back.

The pixels are separated by bout 10cm from each other, and are organised in two rows, 5cm from the border. They are screwed to the board with 1/2" screws. The screws stick out a bit and their tip is hot glued for safety.

The LED wires come through the back via drilled holes. To make a small hole, the power connectors are soldered last. The wires are soldered as needed, using shrink wrap for each electric wire, and then a larger shrink wrap around all the wires to keep stuff more robust. Sinc ethey come in set of 20, I had to do some splicing. The wire holes are filled with hot glue to protect it a bit from tugging. When turning corners, the LEDs were turned 45 degrees to reduce the wire tugging.

The letters are covered with fur that is stapled on after running the LEDs. To install, the fur was cut to match the letter (this could be done after too. It is laid over the letter and its LEDs. For each LED, I find the center of it on the fur and make a small notch circle with a pair of scisors of about 1/2". To avoid stuff slipping ,  did LEDs at the edges, making sure the edge lines up and the fur is taught. I then squeeze the LED trough that tiny hole, which makes it stay rather snug. Repeat 104 times... Once all the LEDs are passed through and visible, I staple the fur in place after dealing with the LED wires. I mostly staple the surround to avoid flopping fabric, but also stapled near some LEDs to keep the fur in the middle sturdy.

The letters are connected via 3-wire extensions that can be run in the back, using standard 3-pin LED connectors.

The power supply uses a standard cylindrical connector, 5A/12V, waterproof from recycled electronics.

The power connector to the power supply is soldered to a 12V-to-5V buck converter wired to the AtTiny85 microcontroller. The power is soldered from there along with the data pin to the 3-pin LED connector. All that is shrink wrapped and epoxy'ed to resist the weather.

FAB_LED library
===============

This program uses the Fast Arduino Bitbang (FAB) LED library.

This library is meant to be a very compact, fast library to drive your addressable LEDs.

* Install from FAB_LED in [GIT](https://github.com/sonyhome/FAB_LED)
* Blog entry for FAB_LED on [Wordpress](https://dntruong.wordpress.com/2016/03/23/my-ws2812b-library-with-palettes)
* Forum entry for FAB_LED on [Arduino.cc](http://forum.arduino.cc/index.php?topic=392074.0)
