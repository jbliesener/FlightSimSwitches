#include <FlightSimSwitches.h>

FlightSimSwitches switches;
FlightSimWriteDatarefSwitch switch1(3,SWITCH_POSITIONS(0,1,2),SWITCH_VALUES(1,2,3));
FlightSimOnOffCommandSwitch switch2(3);
FlightSimOnOffDatarefSwitch switch3(4, true); // inverted values!

// This callback function will be called whenever switch 1 changes
void sw1ChangeFunction(float value) {
  Serial.print("Switch 1 changed to ");
  Serial.println(switch1.getValue());
}

// This callback function will be called whenever switch 2 or 3 changes.
// The "context" parameter is the pin number
void sw23ChangeFunction(float value, void* context) {
  uint32_t pinNumber = (uint32_t) context;
  Serial.print("Pin ");
  Serial.print(pinNumber);
  Serial.print(" changed to ");
  Serial.println(value);
}

void setup() {
  delay(1000);
  Serial.println("Callback demo");

  switch1 = XPlaneRef("put/dataref/here");
  switch2.setOnOffCommands(
    XPlaneRef("put/the/on/command/here"), 
    XPlaneRef("put/the/off/command/here"));
  switch3 = XPlaneRef("put/another/dataref/here");
  switch1.onChange(sw1ChangeFunction);
  switch2.onChange(sw23ChangeFunction,(void*) 3); // Switch 2 is on pin 3
  switch3.onChange(sw23ChangeFunction,(void*) 4); // Switch 3 is on pin 4

  switches.setDebug(DEBUG_SWITCHES);
  switches.begin();
}

void loop() {
  FlightSim.update();
  switches.loop();
}
