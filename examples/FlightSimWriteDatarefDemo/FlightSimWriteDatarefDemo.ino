#include <FlightSimSwitches.h>

// always declare FlightSimSwitches first
FlightSimSwitches switches;

// Multi-position switch connected to pins 3, 4 and 5. 
// When pin 3 is connected to GND, the value 2 will be
// written to the dataref. Connecting pin 4 to GND writes
// 1 to the dataref. Same for pin 5 and value 3. 
// When no pin is connected to GND, the value 0 is written
// to the dataref (default value).
// The specific dataref to write to will be defined in setup().
FlightSimWriteDatarefSwitch sw1(
  3,                        // 3 pins connected
  SWITCH_POSITIONS(3,4,5),  // pin numbers of connected pins
  SWITCH_VALUES   (2,1,3)); // one value for each pin number

// Another multi-position switch, but this time
// the default value that the dataref should take
// when none of the pins is connected is 5.
FlightSimWriteDatarefSwitch sw2(
  3,                        // 3 pins connected
  SWITCH_POSITIONS(6,7,8),  // pin numbers of connected pins
  SWITCH_VALUES   (2,1,3),  // one value for each pin number
  5);                       // default value

// Another multi-position switch, but this time
// the default value and tolerance are defined
// in setup()
FlightSimWriteDatarefSwitch sw3(
  4,                        // 4 pins connected
  SWITCH_POSITIONS(9,10,11,12),  // pin numbers of connected pins
  SWITCH_VALUES   (2, 1, 3, 4)); // one value for each pin number


void setup() {
  delay(1000);

  Serial.begin(115200);
  
  // set switch 1 dataref
  sw1 = XPlaneRef("dataref/for/switch1"); 

  // set switch 2 dataref
  sw2 = XPlaneRef("dataref/for/switch2");

  // set switch 3 dataref, commands and options
  sw3 = XPlaneRef("dataref/for/switch3");
  sw3.setDefaultValue(5);  // Default value: 5 if no pin is active
  sw3.setTolerance(0.1);   

  switches.setDebug(DEBUG_SWITCHES);
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();
}

