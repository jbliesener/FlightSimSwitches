#include <FlightSimSwitches.h>

// always declare FlightSimSwitches first
FlightSimSwitches switches;

// Pin number specified in object declaration
FlightSimOnOffCommandSwitch switch1(2); // Switch connected between pin 2 and GND

// Pin Number specified in setup()
FlightSimOnOffCommandSwitch switch2;

void setup() {
  delay(1000);
  // ...
  // set switch1 commands
  switch1.setOnOffCommands(XPlaneRef("on/command/1"),XPlaneRef("off/command/1"));

  // set switch2 pin and Commands
  switch2.setPosition(3);
  switch2.setOnOffCommands(XPlaneRef("on/command/2"),XPlaneRef("off/command/2"));

  switches.setDebug(DEBUG_SWITCHES);
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();
}

