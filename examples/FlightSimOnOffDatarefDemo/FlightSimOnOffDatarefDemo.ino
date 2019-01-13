#include <FlightSimSwitches.h>

// always declare FlightSimSwitches first
FlightSimSwitches switches;

// Pin number specified in object declaration
FlightSimOnOffDatarefSwitch sw1(2); // Switch between pin 2 and GND

// Pin Number specified in setup()
FlightSimOnOffDatarefSwitch sw2;

// Inverted logic: Write 0 when switch closes and 1 when
// switch opens
FlightSimOnOffDatarefSwitch sw3(4, true); // inverted switch 
                                          // between pin 4 and GND

// Set parameters in setup()
FlightSimOnOffDatarefSwitch sw4; 

void setup() {
  delay(1000);
  // set pushbutton 1 command
  sw1 = XPlaneRef("dataref/1");

  // set pushbutton 2 pin and commands
  sw2.setPosition(3);
  sw2 = XPlaneRef("dataref/2");

  // set pushbutton 3 command (inverted)
  sw3 = XPlaneRef("dataref/3");

  // set pushbutton 4 pin and commands
  sw4.setPosition(5);
  sw4.setInverted(true);
  sw4 = XPlaneRef("dataref/4");

  switches.setDebug(DEBUG_SWITCHES);
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();
}

