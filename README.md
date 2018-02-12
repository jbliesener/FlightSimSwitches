# FlightSimSwitches

For X-Plane cockpit builders that use the Teensy microcontroller and want to
handle switches and pushbuttons, this library provides a set of objects that
make the task very easy.

The [Teensy](https://www.pjrc.com) microcontroller is a high performance
substitute for [Arduino](https://www.arduino.cc) hardware. Furthermore, it comes
with a programming interface for the [X-Plane](https://www.x-plane.com) Flight
simulator. The Teensy interface is very easy to use, please check the original
documentation at https://www.pjrc.com/teensy/td_flightsim.html.

However, even with the intuitive Teensy programming model, correctly handling
cockpit switches in X-Plane is not as easy as it seems. Having built some
cockpits, and helped to build on others, I've learned a number of things that
should be observed:

* Hardware switches must be debounced to work correctly. See https://www.allaboutcircuits.com/technical-articles/switch-bounce-how-to-deal-with-it/
* X-Plane can be controlled through commands or through datarefs. **The most
  common error made is trying to write to a dataref, when X-Plane actually
  requires commands to control a certain feature.** See the specific section
  below for additional explanations.
* Repeated, high frequency dataref write operations or commands not only consume
  bandwidth on the USB bus, but also can overload and consequently crash X-Plane
  or plugins like Lua or Python. **Only set datarefs or send commands when a
  switch actually changes**.
* No rule without an exception: You **must** set **all** switch values right after
  a plane is loaded in order to synchronize the plane with your real hardware.
* Some switches in X-Plane are actually controlled through "*up*" and
  "*down*" commands, that require a dataref for tracking. That's when things
  really start to get complicated. See below...
* When you have LOTS of switches (more than 20 or so), you may not have enough
  pins on your Teensy to connect them all. A switch matrix (see below) is a
  common and proven technique to connect and query many switches to a
  microcontroller. However, it requires additional software to be used.

This library supports all the issues mentioned above, plus some more. It is
licensed under LGPL3 and can be used in commercial projects, according to LGPL3
terms.

# How to use

Install the "FlightSimSwitches" library through the Arduino Library manager.
That should automatically include a

```
#include <FlightSimSwitches.h>
```

in your code.

## Instantiating the `FlightSimSwitches` object

Your code needs to define at least one `FlightSimObjects` object. There are two
ways to do this. You can either provide all parameters on the declaration of
the object or specify them one by one in the `setup()` function.

To specify all details on declaration of the `FlightSimSwitches` object, use
this template:

```
const uint8_t  INPUT_PINS[] = {2, 5, 6, 7};   // Put the pin numbers of your input pins here
const uint8_t  ROW_COUNT = 1;                 // Only used for switch matrix. Leave 1 for
                                              // switches directly connected to input pins
const uint8_t  OUTPUT_PINS[] = {};            // Output pins for switch matrix. Leave empty for
                                              // switches directly connected to input pins
const bool     ACTIVE_LOW = true;             // Do the switches connect to GND (true) or to VCC (false)
const uint32_t SCAN_RATE = DEFAULT_SCAN_RATE; // How fast should the switches be scanned? Default scan
                                              // rate is every 15 ms.
const uint8_t  INPUT_COUNT = sizeof(INPUT_PINS) / sizeof(INPUT_PINS[0]);

FlightSimSwitches switches(ROW_COUNT,         // number of rows
                           OUTPUT_PINS,       // pins for row selection
                           INPUT_COUNT,       // number of input pins
                           INPUT_PINS,        // column pins
                           SCAN_RATE,         // scan speed
                           ACTIVE_LOW);
```
Alternatively, it is possible to declare a FlightSimSwitches object without any
parameters and define those in the `setup()` function:

```
FlightSimSwitches switches;

void setup() {
  switches.setNumberOfInputs(4);
  switches.setInputPins(SWITCH_PINS(2, 5, 6, 7));  // Set your input pin numbers here

  ...
}
```

## Starting the switches

You **must** call `FlightSimSwitches.begin()` in your `setup()` function. This
will check the configuration of the object and configure input and output pins
accordingly.
**There is no need to call `pinMode()` for any of the pins, the library does this
automatically**
