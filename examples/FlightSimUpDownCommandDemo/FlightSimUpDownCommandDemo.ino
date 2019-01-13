#include <FlightSimSwitches.h>

// always declare FlightSimSwitches first
FlightSimSwitches switches;

// Up/down command switch connected to
// pins 3, 4 and 5. When pin 3 is connected to GND,
// "up" or "down" commands are sent until the dataref
// contains the value 2. Connecting pin 4 to GND sends
// "up" or "down" commands until the dataref holds 1.
// Same for pin 5 and value 3. When no pin is connected
// to GND, the object creates "up" or "down" command 
// until the dataref is zero (default value).
// The "up" and "down" commands, as well as the dataref
// will be defined in setup().
FlightSimUpDownCommandSwitch sw1(
  3,                        // 3 pins connected
  SWITCH_POSITIONS(3,4,5),  // pin numbers of connected pins
  SWITCH_VALUES   (2,1,3)); // one value for each pin number

// Another Up/down command switch, but this time
// the default value that the dataref should take
// when none of the pins is connected is 5.
FlightSimUpDownCommandSwitch sw2(
  3,                        // 3 pins connected
  SWITCH_POSITIONS(6,7,8),  // pin numbers of connected pins
  SWITCH_VALUES   (2,1,3),  // one value for each pin number
  5);                       // default value

// Another up/down command switch, but this time
// the default value and tolerance are defined
// in setup()
FlightSimUpDownCommandSwitch sw3(
  4,                        // 4 pins connected
  SWITCH_POSITIONS(9,10,11,12),  // pin numbers of connected pins
  SWITCH_VALUES   (2, 1, 3, 4)); // one value for each pin number


void setup() {
  delay(1000);

  Serial.begin(115200);
  Serial.print  ("CAUTION! This program will not work without defining real and ");
  Serial.println("existing dataref and commands, as sending a command must result");
  Serial.print  ("in a change to the dataref value, which will not happen with the ");
  Serial.println("dummy datarefs used in this example.");
  
  // set switch 1 dataref and commands
  sw1.setDatarefAndCommands(
    XPlaneRef("dataref/for/switch1"), // dataref
    XPlaneRef("up/cmd/switch/1"),     // up command
    XPlaneRef("down/cmd/switch/1")    // down command
    );

  // set switch 2 dataref and commands
  sw2.setDatarefAndCommands(
    XPlaneRef("dataref/for/switch2"), // dataref
    XPlaneRef("up/cmd/switch/2"),     // up command
    XPlaneRef("down/cmd/switch/2")    // down command
    );

  // set switch 3 dataref, commands and options
  sw3.setDatarefAndCommands(
    XPlaneRef("dataref/for/switch3"), // dataref
    XPlaneRef("up/cmd/switch/3"),     // up command
    XPlaneRef("down/cmd/switch/3")    // down command
    );
  sw3.setDefaultValue(5);  // Default value: 5 if no pin is active
  sw3.setTolerance(0.1);   

  switches.setDebug(DEBUG_SWITCHES);
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();
}

