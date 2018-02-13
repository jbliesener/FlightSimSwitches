# FlightSimSwitches for X-Plane and Teensy

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
  requires commands to control a certain feature.** See the
  [specific section in the Wiki](https://github.com/jbliesener/FlightSimSwitches/wiki/04.-Commands-vs.-Datarefs-(the-most-frequent-issue))
  for additional explanations.
* Repeated, high frequency dataref write operations or commands not only consume
  bandwidth on the USB bus, but also can overload and consequently crash X-Plane
  or plugins like Lua or Python. **Only set datarefs or send commands when a
  switch actually changes**.
* No rule without an exception: You **must** set **all** switch values right after
  a plane is loaded in order to synchronize the plane with your real hardware.
* Some switches in X-Plane are actually controlled through "*up*" and
  "*down*" commands, that require a dataref for tracking. That's when things
  really start to get complicated. Check the specific
  [wiki page on the subject](https://github.com/jbliesener/FlightSimSwitches/wiki/03.04.-FlightSimUpDownCommandSwitch)
* When you have LOTS of switches (more than 20 or so), you may not have enough
  pins on your Teensy to connect them all. A [switch matrix](https://github.com/jbliesener/FlightSimSwitches/wiki/06.-Switch-matrices) is a
  common and proven technique to connect and query many switches to a
  microcontroller. However, it requires additional software to be used.
* Debug output: Sometime you don't know what's wrong with your sketch.
  [Debug output](https://github.com/jbliesener/FlightSimSwitches/wiki/05.-Debugging-(and-some-words-about-Serial.print)) helps you to find and resolve the problem

This library addresses all the issues mentioned above, plus some more. It is
licensed under LGPL3 and can be used in commercial projects, according to LGPL3
terms.

Check the Wiki at https://github.com/jbliesener/FlightSimSwitches/wiki for
additional docs, examples and FAQ
