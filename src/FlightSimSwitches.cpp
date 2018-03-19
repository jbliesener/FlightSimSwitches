#include "FlightsimSwitches.h"

/*
 * Switch Matrix for Teensy Flightsim projects
 *
 * (c) Jorg Neves Bliesener
 */


/* Switch matrix - handles an arbitrary number of rows, each with up to 32 columns
 *
 * The rows can be multiplexed through a 74HCT/LS154 or 74HCT/LS138 chip. The rows
 * will be the OUTPUT lines, the columns will be the INPUT lines. Exactly one row
 * will be active at any time. Active rows are LOW by default, but can be set to active
 * HIGH.
 */

FlightSimSwitches* FlightSimSwitches::firstMatrix = NULL;
const uint8_t FLIGHTSIM_EMPTY_PINS[]={};

FlightSimSwitches::FlightSimSwitches() {
  if (firstMatrix==NULL) {
    firstMatrix = this;
  }

  this->numberOfRows = 1;
  this->rowPins = FLIGHTSIM_EMPTY_PINS;
  this->numberOfRowPins = 0;
  this->rowsMuxed = false;
  this->numberOfColumns = 0;
  this->columnPins = NULL;
  this->currentRow = 0;
  this->rowData = NULL;
  this->initialized = false;
  this->matrixTimer = 0;
  this->scanRate = DEFAULT_SCAN_RATE;
  this->lastRow = 0xff;
  this->activeLow = true;
  this->hasChangedLoop = false;
  this->hasChangedPoll = false;
  this->changePositionCallback = NULL;
  this->changeMatrixCallback = NULL;
  this->lastEnabled = false;
  this->debugScan = false;
}

FlightSimSwitches::FlightSimSwitches(uint8_t numberOfRows, const uint8_t *rowPins,
                 uint8_t numberOfColumns, const uint8_t *columnPins,
                 uint32_t scanRate, bool activeLow, bool rowsMuxed) {
  if (firstMatrix==NULL) {
    firstMatrix = this;
  }
  this->numberOfRows = numberOfRows;
  this->numberOfRowPins = 0;
  this->rowPins = rowPins;
  this->rowsMuxed = rowsMuxed;
  this->numberOfColumns = numberOfColumns;
  this->columnPins = columnPins;
  this->currentRow = 0;
  this->rowData = NULL;
  this->initialized = false;
  this->matrixTimer = 0;
  this->scanRate = scanRate;
  this->lastRow = 0xff;
  this->activeLow = activeLow;
  this->hasChangedLoop = false;
  this->hasChangedPoll = false;
  this->changePositionCallback = NULL;
  this->changeMatrixCallback = NULL;
  this->lastEnabled = false;
  this->debugScan = false;
}

FlightSimSwitches::FlightSimSwitches(uint8_t numberOfColumns, const uint8_t *columnPins,
                 uint32_t scanRate, bool activeLow)
   : FlightSimSwitches(1,FLIGHTSIM_EMPTY_PINS,numberOfColumns,columnPins,scanRate,activeLow)
   {};

bool FlightSimSwitches::checkInitialized(const char *message, bool mustBeInitialized) {
  if (initialized != mustBeInitialized) {
    Serial.printf("%10lu: FlightSimSwitch ERROR: %s must be called %s begin() %s\n",
      millis(), message, mustBeInitialized ? "after" : "before", mustBeInitialized ? "has been called with a correct setup." : "");
    return false;
  }
  return true;
}

void FlightSimSwitches::begin() {
  if (this->numberOfColumns>32) {
    Serial.printf("%10lu: FlightSimSwitches ERROR: Sorry, 32 columns max\n",millis());
    return;
  }

  if (!this->numberOfColumns) {
    Serial.printf("%10lu: FlightSimSwitches ERROR: Sorry, we need at least one column\n",millis());
    return;
  }

  if (!this->numberOfRows) {
    Serial.printf("%10lu: FlightSimSwitches ERROR: Sorry, we need at least one row\n",millis());
    return;
  }

  if (!this->rowPins) {
    Serial.printf("%10lu: FlightSimSwitches ERROR:  Please set the row pins\n",millis());
    return;
  }

  if (!this->columnPins) {
    Serial.printf("%10lu: FlightSimSwitches ERROR: Please set the column pins\n",millis());
    return;
  }

  if (!this->scanRate) {
    Serial.printf("%10lu: FlightSimSwitch ERROR: scanRate can not be zero!\n",millis());
    return;
  }

  if (!rowsMuxed) {
    this->numberOfRowPins = numberOfRows;
  } else {
    for (int i=0; i<32; i++) {
      if (_BV(i)>=numberOfRows) {
        this->numberOfRowPins = i;
        break;
      }
    }
    if (debugScan) {
      Serial.printf("%10lu: FlightSimSwitch: Number of row pins: %d\n",millis(),numberOfRowPins);
    }
  }

  rowData = (uint32_t*) malloc(numberOfRows * sizeof(uint32_t));
  memset(rowData,0,numberOfRows * sizeof(uint32_t));

  for (int i=0; i<numberOfRowPins; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], activeLow ? HIGH : LOW);
  }

  for (int i=0; i<numberOfColumns; i++) {
    pinMode(columnPins[i],activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
  }

  initialized = true;

  setRowNumber(0);
  this->matrixTimer = 0;
}

void FlightSimSwitches::setRowNumber(uint32_t currentRow) {
  if (!checkInitialized("setRowNumber",true))
    return;

  if (debugScan) {
    Serial.printf("%10lu: FlightSimSwitches: Setting row %d\n",millis(),currentRow);
  }
  this->currentRow = currentRow;
  if (!numberOfRows) return;  // do nothing if switches are connected directly to Teensy pins

  if (!rowsMuxed) {
    // turn only selected row output LOW
    if (lastRow != 0xff) {
      digitalWrite(rowPins[lastRow],activeLow ? HIGH : LOW);
    }
    digitalWrite(rowPins[currentRow],activeLow ? LOW : HIGH);
  } else {
    // output row number to row select pins
    for (uint8_t i=0; i<numberOfRowPins; i++) {
      digitalWrite(rowPins[i], (currentRow & _BV(i)) ? HIGH : LOW);
    }
  }
  lastRow = currentRow;
}

uint32_t FlightSimSwitches::getSingleRowData() {
  if (!checkInitialized("getSingleRowData",true))
    return 0;

  uint32_t readVal = 0;
  for (uint8_t i=0; i<numberOfColumns; i++) {
    bool readData = (digitalRead(columnPins[i]) == HIGH) ^ activeLow;
    if (readData) {
      readVal |= _BV(i);
    }
  }
  if (debugScan) {
    Serial.printf("%10lu: FlightSimSwitches: reading row data: %08x\n",millis(),readVal);
  }
  return readVal;
}

void FlightSimSwitches::loop() {
  if (!checkInitialized("loop",true))
    return;

  if (matrixTimer > scanRate) {
    // read current row
    matrixTimer = 0;
    uint32_t readData = getSingleRowData();
    if (rowData[currentRow] != readData) {
      if (changePositionCallback) {
        uint32_t diff = readData ^ rowData[currentRow];
        for (uint8_t i=0; i<numberOfColumns; i++) {
          if (diff & _BV(i)) {
            (*changePositionCallback)(currentRow,i,readData & _BV(i));
          }
        }
      }
      rowData[currentRow] = readData;
      hasChangedPoll = true;
      hasChangedLoop = true;
    }

    currentRow++;
    if (currentRow>=numberOfRows) {
      // end of scan
      currentRow=0;
      if (hasChangedLoop && changeMatrixCallback) {
        (*changeMatrixCallback)();
      }
      hasChangedLoop=false;

      bool enabled = FlightSim.isEnabled();
      bool resync = enabled && (!lastEnabled);
      if (resync) {
        Serial.printf("%10lu: FlightSimSwitches: Flightsim started, resyncing!\n",millis());
      }
      for (MatrixElement *elem = MatrixElement::firstElement; elem; elem=elem->nextElement) {
        if (elem->matrix == this) {
          elem->handleLoop(resync);
        }
      }
      lastEnabled = enabled;
    }

    setRowNumber(currentRow);
  }
}

void FlightSimSwitches::print() {
  if (!checkInitialized("print",true))
    return;

  if (numberOfRows>1) {
    // Matrix
    Serial.printf("%10lu: Switch matrix:\n",millis());
    Serial.printf("%10lu:     ",millis());
    for (uint8_t i=0; i<numberOfColumns; i++) {
      Serial.printf("%3d",i);
    }
    Serial.println();
    for (uint8_t r=0; r<numberOfRows; r++) {
      Serial.printf("%10lu: %2d: ",millis(),r);
      for (uint8_t c=0; c<numberOfColumns; c++) {
        if (rowData[r] & _BV(c)) {
          Serial.print("  X");
        } else {
          Serial.print("   ");
        }
      }
      Serial.println();
    }
    Serial.printf("%10lu: ---------------\n",millis());
  } else {
    // directly connected Switches
    Serial.printf("%10lu: Switches: ",millis());
    for (uint8_t i=0; i<numberOfColumns; i++) {
      Serial.printf("%d: %s",i,(rowData[0] & _BV(i) ? "ON" : "off"));
      if (i<numberOfColumns-1) {
        Serial.print(", ");
      }
    }
    Serial.println();
  }
}

void FlightSimSwitches::setDebug(uint32_t debug_type) {
  for (MatrixElement *elem = MatrixElement::firstElement; elem; elem=elem->nextElement) {
    if (elem->matrix == this) {
      elem->setDebug(debug_type & elem->getDebugMask());
    }
  }
  debugScan = debug_type & DEBUG_SCAN;
}



/*
 * Generic matrix element. Not to be instantiated directly. Only keeps reference
 * to matrix and chain of elements.
 */

MatrixElement *MatrixElement::firstElement = NULL;
MatrixElement *MatrixElement::lastElement = NULL;
bool MatrixElement::lastEnabled = false;

MatrixElement::MatrixElement(FlightSimSwitches *matrix) {
  this->matrix = matrix;
  if (firstElement == NULL) {
    firstElement = this;
  } else {
    lastElement->nextElement = this;
  }
  lastElement = this;
  this->debug = false;
  this->change_callback = NULL;
  this->callbackContext = NULL;
  this->hasCallbackContext = false;
}

bool MatrixElement::getPositionData(uint32_t position) {
  if (!matrix) {
    Serial.printf("%10lu: FlightSimSwitch ERROR: Please define a FlightSimSwitches or MatrixSwitches object before defining individual switches!\n", millis());
    return 0;
  }
  if (position == NO_POSITION) {
    Serial.printf("%10lu: FlightSimSwitch ERROR: Switch position not set!\n",millis());
  }
  uint32_t *rowData = matrix->getRowData();
  uint32_t rowValue = rowData[MATRIX_ROW(position)];
  return rowValue & _BV(MATRIX_COLUMN(position));
}

void MatrixElement::callback(float newValue) {
  if (!change_callback)
    return;

  if (hasCallbackContext) {
    (*(void(*)(float,void*))change_callback)(newValue,callbackContext);
  } else {
    (*change_callback)(newValue);
  }
}


/*
 * Matrix On-Off switch. Sends one of two commands to X-Plane.
 *
 * This switch corresponds to a single position in the matrix
 */

FlightSimOnOffCommandSwitch::FlightSimOnOffCommandSwitch(FlightSimSwitches *matrix, uint32_t matrixPosition)
  : MatrixElement(matrix) {
  this->matrixPosition = matrixPosition;
  this->oldValue = false;
  this->onName = XPlaneRef("(null)");
  this->offName = XPlaneRef("(null)");
  this->hasOnCommand = false;
  this->hasOffCommand = false;
}

void FlightSimOnOffCommandSwitch::setOnOffCommands(const _XpRefStr_ *onCommand, const _XpRefStr_ *offCommand) {
  this->onName = onCommand;
  this->offName = offCommand;
  this->onCommand.assign(onCommand);
  this->offCommand.assign(offCommand);
  this->hasOnCommand = true;
  this->hasOffCommand = true;
}

void FlightSimOnOffCommandSwitch::setOnCommandOnly(const _XpRefStr_ *onCommand) {
  this->onName = onCommand;
  this->onCommand.assign(onCommand);
  this->hasOnCommand = true;
}

void FlightSimOnOffCommandSwitch::setOffCommandOnly(const _XpRefStr_ *offCommand) {
  this->offName = offCommand;
  this->offCommand.assign(offCommand);
  this->hasOffCommand = true;
}

void FlightSimOnOffCommandSwitch::handleLoop(bool resync) {
  bool value = getPositionData(matrixPosition);
  if ((value != oldValue) || resync) {
    oldValue = value;

    if (!hasOnCommand && !hasOffCommand) {
      Serial.printf("%10lu: FlightSimOnOffSwitch ERROR: OnOffCommandSwitch: Commands not set\n");
      return;
    }

    if (value) {
      if (hasOnCommand) {
        if (debug) {
          Serial.printf("%10lu: FlightSimOnOffCommandSwitch: Sending ON command %s\n", millis(), onName);
        }
        onCommand.once();
        callback(1.0);
      }
    } else {
      if (hasOffCommand) {
        if (debug) {
          Serial.printf("%10lu: FlightSimOnOffCommandSwitch: Sending OFF command %s\n",millis(), offName);
        }
        offCommand.once();
        callback(0.0);
      }
    }
  }
}

float FlightSimOnOffCommandSwitch::getValue() {
  return (oldValue ? 1.0 : 0.0);
}

/*
 * Matrix pushbutton. Sends a single command with begin/end to X-Plane.
 *
 * This pushbutton corresponds to a single position in the matrix
 */

FlightSimPushbutton::FlightSimPushbutton(FlightSimSwitches *matrix, uint32_t matrixPosition, bool inverted)
  : MatrixElement(matrix) {
  this->matrixPosition = matrixPosition;
  this->oldValue = false;
  this->commandName = XPlaneRef("(null)");
  this->inverted = inverted;
}

void FlightSimPushbutton::handleLoop(bool resync) {
  bool value = getPositionData(matrixPosition);
  if ((value != oldValue) || resync) {
    oldValue = value;

    if (value ^ inverted) {
      if (debug) {
        Serial.printf("%10lu: FlightSimPushbutton: Sending command %s BEGIN\n", millis(), commandName);
      }
      command.begin();
      callback(1.0);
    } else {
      if (debug) {
        Serial.printf("%10lu: FlightSimPushbutton: Sending command %s END\n",millis(), commandName);
      }
      command.end();
      callback(0.0);
    }
  }
}

float FlightSimPushbutton::getValue() {
  return (oldValue ? 1.0 : 0.0);
}


/*
 * Matrix Multi-position switch. Sends either up or down commands to X-Plane,
 * in order to match a switch position to a defined value
 *
 * Each position in the matrix corresponds to one value the switch can take
 */


FlightSimUpDownCommandSwitch::FlightSimUpDownCommandSwitch(FlightSimSwitches *matrix, uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue, float tolerance)
  : MatrixElement(matrix) {
  this->numberOfPositions = numberOfPositions;
  this->matrixPositions = positions;
  this->values = values;
  this->defaultValue = defaultValue;
  this->oldDatarefValue = 0.0;
  this->oldSwitchValue = 0.0;
  this->switchChanged = false;
  this->tolerance = tolerance;
  this->commandSent = false;
  this->name = XPlaneRef("(null)");
  this->pushbuttonPositions = 0;
  this->pushbuttonCommand = NULL;
}

void FlightSimUpDownCommandSwitch::setDatarefAndCommands(const _XpRefStr_ *positionDataref, const _XpRefStr_ *upCommand, const _XpRefStr_ *downCommand) {
  this->name = positionDataref;
  this->positionDataref.assign(positionDataref);
  this->upCommand.assign(upCommand);
  this->downCommand.assign(downCommand);
}

float FlightSimUpDownCommandSwitch::findValue(int8_t *valueIndex) {
  for (uint8_t i=0; i<numberOfPositions; i++) {
    if (getPositionData(matrixPositions[i])) {
      if (valueIndex) {
        *valueIndex = i;
      }
      return values[i];
    }
  }
  return defaultValue;
}

void FlightSimUpDownCommandSwitch::handleLoop(bool resync) {
  int8_t valueIndex = -1;
  float switchValue = findValue(&valueIndex);  // get current value and value index on matrix
  float datarefValue = positionDataref.read(); // get current value in X-Plane

  if ((switchValue != oldSwitchValue) || resync) {
    if (debug) {
      Serial.printf("%10lu: FlightSimUpDownCommandSwitch: dataref name: %s, switch value=", millis(), name);
      Serial.print(switchValue);
      Serial.print(", old switch value=");
      Serial.print(oldSwitchValue);
      Serial.printf(", resync=%d, valueIndex=%d, switch changed!\n", resync, valueIndex);
    }
    switchChanged = true;
    oldSwitchValue = switchValue;
    callback(switchValue);
  }

  if (abs(switchValue-datarefValue)<tolerance) {
    if (switchChanged) {
      // below tolerance is not considered a change
      if (debug) {
        Serial.printf("%10lu: FlightSimUpDownCommandSwitch: dataref name: %s, switch value=", millis(), name);
        Serial.print(switchValue);
        Serial.print(", dataref value=");
        Serial.print(datarefValue);
        Serial.println(", final position reached!");
      }
      switchChanged = false; // reached final value
    }
    commandSent = false;
    return;
  }

  if (commandSent) {
    // check if previous command has finished
    if (abs(datarefValue-oldDatarefValue)<tolerance) {
      // no, dataref still has previous value. Wait for dataref
      // to change
      return;
    } else {
      // dataref has changed, we can send the next command
      if (debug) {
        Serial.printf("%10lu: FlightSimUpDownCommandSwitch: dataref name: %s, dataref value=" , millis(), name);
        Serial.print(datarefValue);
        Serial.print(", old dataref value=");
        Serial.print(oldDatarefValue);
        Serial.println(", previous command handled!");
      }
      commandSent = false;
    }
  }

  if (switchChanged) {
    commandSent = true;
    oldDatarefValue = datarefValue;
    if (switchValue > datarefValue) {
      if (debug) {
        Serial.printf("%10lu: FlightSimUpDownCommandSwitch: dataref name: %s, switch value=" , millis(), name);
        Serial.print(switchValue);
        Serial.print(", dataref value=");
        Serial.print(datarefValue);
        Serial.printf(", sending UP command for %s\n", name);
      }
      if (pushbuttonCommand) {
        pushbuttonCommand->end();
        pushbuttonCommand = NULL;
      }
      if (valueIndex != -1 && (pushbuttonPositions & _BV(valueIndex))) {
        upCommand.begin();
        pushbuttonCommand = &upCommand;
      } else {
        upCommand.once();
      }
    } else {
      if (debug) {
        Serial.printf("%10lu: FlightSimUpDownCommandSwitch: dataref name: %s, switch value=" , millis(), name);
        Serial.print(switchValue);
        Serial.print(", dataref value=");
        Serial.print(datarefValue);
        Serial.printf(", sending DOWN command for %s\n", name);
      }
      if (pushbuttonCommand) {
        pushbuttonCommand->end();
        pushbuttonCommand = NULL;
      }
      if (valueIndex != -1 && (pushbuttonPositions & _BV(valueIndex))) {
        pushbuttonCommand = &downCommand;
        downCommand.begin();
      } else {
        downCommand.once();
      }
    }
  }
}

float FlightSimUpDownCommandSwitch::getValue() {
  return (oldSwitchValue);
}


/*
 * Matrix On-Off switch that writes values to a dataref
 *
 */
FlightSimOnOffDatarefSwitch::FlightSimOnOffDatarefSwitch(FlightSimSwitches *matrix, uint32_t position, bool inverted)
  : MatrixElement(matrix) {
  this->matrixPosition = position;
  this->inverted = inverted;
  this->oldValue = false;
  this->name=XPlaneRef("(null)");
}

void FlightSimOnOffDatarefSwitch::setDataref(const _XpRefStr_ *positionDataref) {
  this->name = positionDataref;
  this->dataref.assign(positionDataref);
}

void FlightSimOnOffDatarefSwitch::handleLoop(bool resync) {
  bool switchOn = getPositionData(matrixPosition);
  if (switchOn != oldValue || resync) {
    int32_t value = switchOn ^ inverted ? 1 : 0;
    if (debug) {
      Serial.printf("%10lu: FlightSimOnOffDatarefSwitch: Writing value %d to dataref %s\n",millis(),value,name);
    }
    dataref.write(value);
    oldValue = switchOn;
    callback(switchOn ? 1.0 : 0.0);
  }
}

float FlightSimOnOffDatarefSwitch::getValue() {
  return (oldValue ? 1.0 : 0.0);
}

/*
 * Matrix Multi-position switch that writes values to a dataref
 *
 * Each position in the matrix corresponds to one value the switch can take
 */


FlightSimWriteDatarefSwitch::FlightSimWriteDatarefSwitch(FlightSimSwitches *matrix, uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue, float tolerance)
  : MatrixElement(matrix) {
  this->numberOfPositions = numberOfPositions;
  this->matrixPositions = positions;
  this->values = values;
  this->defaultValue = defaultValue;
  this->tolerance = tolerance;
  this->oldSwitchValue = 0.0;
  this->name = XPlaneRef("(null)");
}

void FlightSimWriteDatarefSwitch::setDataref(const _XpRefStr_ *positionDataref) {
  this->name = positionDataref;
  this->positionDataref.assign(positionDataref);
}

float FlightSimWriteDatarefSwitch::findValue() {
  for (uint8_t i=0; i<numberOfPositions; i++) {
    if (getPositionData(matrixPositions[i])) {
      return values[i];
    }
  }
  return defaultValue;
}

void FlightSimWriteDatarefSwitch::handleLoop(bool resync) {
  float switchValue = findValue();  // get current value on matrix
  if ((switchValue != oldSwitchValue) || resync) {
    oldSwitchValue = switchValue;
    if (debug) {
      Serial.printf("%10lu: FlightSimWriteDatarefSwitch: Writing value ", millis());
      Serial.print(switchValue);
      Serial.printf(" to dataref %s\n",name);
    }
    positionDataref.write(switchValue);
    callback(switchValue);
  }
}

float FlightSimWriteDatarefSwitch::getValue() {
  return (oldSwitchValue);
}
