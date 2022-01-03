# M5Atom Motion LEGO control

## Supported hardware

[M5Atom](https://docs.m5stack.com/en/core/atom_lite) with [Motion](https://docs.m5stack.com/en/atom/atom_motion) base is supported. Could be ported to another ESP32 board with motor and servo support.

## Features

Switch program by long pressing the M5Atom button, current program is indicated by the LED color.

1. Green: motors on, servos cycle full range; short-press to toggle motors' direction
2. Yellow:  motors on, servos cycle full range; short-press to toggle motors' direction
3. Blue: control two wheeled robot/car (one motor for each wheel) with Wiimote

Last selected program number is stored in non-volatile memory in order to continue with the program after powerdown-powerup cycle.

## Wiimote controls

Wiimote is expected to be held horizontaly, ie. Buttons 1 is leftwards and Button 2 is rightwards.

* D-pad right (oriented forwards) = forward
* D-pad left (oriented backwards) = reverse
* Button 1 = left
* Button 2 = right
* Button A = toggle servo position (full cycle)

All combinations of forward/reverse and left/right are supported, including "pirouette".

Wiimote is required to be paired after every power-on by pressing both 1 and 2 buttons on Wiimote (ESP32Wiimote library limitation, see [ESP32Wiimote usage](https://github.com/hrgraf/ESP32Wiimote#usage)).

## Supported IDEs

This repository is intended to be compatible with both Arduino IDE and PlatformIO IDE. See [platformio.ini](platformio.ini) for required libraries (lib_deps) to be manually added to Arduino IDE. PlatformIO IDE will download libraries automatically.