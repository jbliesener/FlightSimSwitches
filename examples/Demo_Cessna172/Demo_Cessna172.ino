// This example shows how to use the FlightSimSwitch library with
// the stock X-Plane Cessna 172 to control taxi and landing lights
// and the Magnetos rotary switch.
//
// Make sure to set the Arduino configuration to "USB Type: Flight Sim Controls",
// otherwise the sketch won't compile.
//
// The following Teensy pins are used. You can connect real hardware
// switches or use a protoboard and a jumper to connect these pins to
// GND
//
// Teensy pin 2: Taxi lights. Pin open: OFF, Pin connected to GND: ON
// Teensy pin 5: Landing lights. Pin open: OFF, Pin connected to GND: ON
// Teensy pins 8-11 Magneto switch. At most one of these pins should be
//                 connected to GND. This will control the magneto switch
//                 in the Cessna as follows:
//                   - All pins open: Off
//                   - Pin 8 to GND: Magnetos R
//                   - Pin 9 to GND: Magnetos L
//                   - Pin 10 to GND: Magnetos BOTH
//                   - Pin 11 to GND: Magnetos START  (DOES NOT WORK YET IN THIS VERSION)
//
// We therefore have the following list of input pins: 2, 5, 8, 9, 10 and 11.
// The first input pin at position 0 of this list is pin 2. The second pin
// at position 1 is pin number 5. Pins 8 to 11 are at positions 2-5 of this
// list.
//
// You can follow the actions of this Sketch on the serial console

#include <FlightSimSwitches.h>

// Declaring the FlightSimSwitches object. This object must always be
// declared BEFORE any specific switch. We can either specify all
// parameters here or configure them later in setup()
// In this case, we specify that we have the six input pins mentioned
// above.
FlightSimSwitches switches(6,SWITCH_PINS(2,5,8,9,10,11));

// Now we configure our individual switches
// The first switch is the taxi light switch. We use the pin POSITION
// in the SWITCH_PINS to specify which pin we use for that switch. DO NOT
// USE THE DIRECT PIN NUMBER HERE!!
// Pin POSITION 0 is the first pin in the array, wich is pin NUMBER 2
FlightSimOnOffCommandSwitch taxiLightSwitch(0);

// Configure the landing lights the same way
// Pin POSITION 1 is the SECOND pin in the list of input pins, which
// is pin NUMBER 5 on the Teensy.
FlightSimOnOffCommandSwitch landingLightSwitch(1);

// The Magnetos switch is more complex. We have a switch with four
// positions and X-Plane wants "up" and "down" commands in order to
// control the setting. The current position is read from a dataref
// and the value is changed with commands. The switch actually has
// five positions, but only four (R, L, BOTH and START) are connected
// to the Teensy. Those four positions correspond with dataref values
// 1 (R), 2 (L), 3 (BOTH) and 4 (START). The fifth position corresponds
// to dataref value 0 and therefore does not need to be specified
// explicitly.
FlightSimUpDownCommandSwitch magnetosSwitch(4,SWITCH_POSITIONS(2,3,4,5),SWITCH_VALUES(1,2,3,4));


void setup() {
  // Serial output stuff, see Wiki (https://www.github.com/jbliesener/FlightSimSwitches/wiki/Serial)
  delay(2000);         // Teensy startup delay
  Serial.begin(9600);
  FlightSim.update();  // Call FlightSim.update to make serial output work. Must not take longer
                       // than 2 seconds for the next call to FlightSim.update()
  Serial.println(__FILE__ ", compiled on " __DATE__ " at "  __TIME__);

  // Set the X-Plane commands to turn the taxi lights on or off
  taxiLightSwitch.setOnOffCommands(
    XPlaneRef("sim/lights/taxi_lights_on"),
    XPlaneRef("sim/lights/taxi_lights_off"));

  // Set the X-Plane commands to turn the landing lights on or off
  landingLightSwitch.setOnOffCommands(
    XPlaneRef("sim/lights/landing_lights_on"),
    XPlaneRef("sim/lights/landing_lights_off"));

  // Set X-Plane datarefs and command for the Magnetos switch
  magnetosSwitch.setDatarefAndCommands(
    XPlaneRef("sim/cockpit2/engine/actuators/ignition_key[0]"),
    XPlaneRef("laminar/c172/ignition_up"),
    XPlaneRef("laminar/c172/ignition_down"));

  // The Magnetos switch on the C172 has a very particular issue:
  // The START position does not lock and immediately jumps back
  // to "BOTH" when triggered through command.once(). It therefore
  // acts like a pushbutton on one position only and must be set
  // with command.begin() and command.end(). The
  // MatrixUpDownCommandSwitch allows to set some positions as
  // pushbuttons that will be handled accordingly.
  // The START position is position index 3 (the fourth position)
  // in the switches position list.
  magnetosSwitch.setPushbuttonPosition(3);

  // provide a little feedback on the serial console
  switches.setDebug(DEBUG_SWITCHES);

  // Do not forget to call FlightSimSwitches.begin()!!
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();

}
