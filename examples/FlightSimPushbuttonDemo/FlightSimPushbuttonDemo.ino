#include <FlightSimSwitches.h>

// always declare FlightSimSwitches first
FlightSimSwitches switches;

// Pin number specified in object declaration
FlightSimPushbutton pb1(2); // Pushbutton between pin 2 and GND

// Pin Number specified in setup()
FlightSimPushbutton pb2;

// Inverted logic: Send END when button closes and BEGIN when
// button opens
FlightSimPushbutton pb3(4, true); // normally closed (N/C) pushbutton 
                                  // between pin 4 and GND

// Set parameters in setup()
FlightSimPushbutton pb4; 

void setup() {
  delay(1000);
  // set pushbutton 1 command
  pb1 = XPlaneRef("command/1");

  // set pushbutton 2 pin and commands
  pb2.setPosition(3);
  pb2 = XPlaneRef("command/2");

  // set pushbutton 3 command (inverted)
  pb3 = XPlaneRef("command/3");

  // set pushbutton 4 pin and commands
  pb4.setPosition(5);
  pb4.setInverted(true);
  pb4 = XPlaneRef("command/4");

  switches.setDebug(DEBUG_SWITCHES);
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();
}

