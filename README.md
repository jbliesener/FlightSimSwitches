# FlightSimSwitches

For X-Plane cockpit builders that use the Teensy microcontroller and want to
handle switches and pushbuttons, this library provides a set of objects that
make the task very easy.

The [Teensy](https://www.pjrc.com) microcontroller is a high performance
substitute for [Arduino](https://www.arduino.cc) hardware. Furthermore, it comes
with a programming interface for the [X-Plane](https://www.x-plane.com) Flight
simulator. The Teensy interface is very easy to use, please check the original
documentation at [https://www.pjrc.com/teensy/td_flightsim.html].

Switches in X-Plane may look trivial, but they aren't. Having built some cockpits,
I've learned a number of things that should be observed:

* X-Plane can be controlled through commands or through datarefs. *The most
  common error made is trying to write to a dataref, when X-Plane actually
  uses commands to control a certain feature.*
* Repeated, high frequency dataref write operations not only consume

The library supports directly connected switches as well as switch matrices. In order to save pins, switch matrices can optionally be connected through multiplexers. \
          Check docs and examples!
