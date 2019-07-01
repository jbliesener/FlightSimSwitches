#include "FlightSimSwitches.h"

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

FlightSimSwitches *FlightSimSwitches::firstMatrix = NULL;
const uint8_t     FLIGHTSIM_EMPTY_PINS[]          = {};

FlightSimSwitches::FlightSimSwitches(uint32_t scanRate, bool activeLow)
{
   if (firstMatrix == NULL)
   {
      firstMatrix = this;
   }

   this->numberOfRows           = 1;
   this->rowPins                = FLIGHTSIM_EMPTY_PINS;
   this->numberOfRowPins        = 0;
   this->rowsMuxed              = false;
   this->numberOfColumns        = 0;
   this->columnPins             = dynamicColumnPins;
   this->columnPinsAreDynamic   = true;
   this->currentRow             = 0;
   this->initialized            = false;
   this->matrixTimer            = 0;
   this->scanRate               = scanRate;
   this->lastRow                = 0xff;
   this->activeLow              = activeLow;
   this->hasChangedLoop         = false;
   this->hasChangedPoll         = false;
   this->changePositionCallback = NULL;
   this->changeMatrixCallback   = NULL;
   this->lastEnabled            = false;
   this->debugScan              = false;
   this->debugConfig            = false;
}


FlightSimSwitches::FlightSimSwitches(uint8_t numberOfRows, const uint8_t *rowPins,
                                     uint8_t numberOfColumns, const uint8_t *columnPins,
                                     uint32_t scanRate, bool activeLow, bool rowsMuxed)
{
   if (firstMatrix == NULL)
   {
      firstMatrix = this;
   }
   this->numberOfRows           = numberOfRows;
   this->numberOfRowPins        = 0;
   this->rowPins                = rowPins;
   this->rowsMuxed              = rowsMuxed;
   this->numberOfColumns        = numberOfColumns;
   this->columnPins             = columnPins;
   this->columnPinsAreDynamic   = false;
   this->currentRow             = 0;
   this->initialized            = false;
   this->matrixTimer            = 0;
   this->scanRate               = scanRate;
   this->lastRow                = 0xff;
   this->activeLow              = activeLow;
   this->hasChangedLoop         = false;
   this->hasChangedPoll         = false;
   this->changePositionCallback = NULL;
   this->changeMatrixCallback   = NULL;
   this->lastEnabled            = false;
   this->debugScan              = false;
   this->debugConfig            = false;
}


FlightSimSwitches::FlightSimSwitches(uint8_t numberOfColumns, const uint8_t *columnPins,
                                     uint32_t scanRate, bool activeLow)
   : FlightSimSwitches(1, FLIGHTSIM_EMPTY_PINS, numberOfColumns, columnPins, scanRate, activeLow)
{
}


void FlightSimSwitches::printTime(Stream *s)
{
   char buf[13];

   sprintf(buf, "%10lu: ", millis());
   s->print(buf);
}


bool FlightSimSwitches::checkInitialized(const __FlashStringHelper *message, bool mustBeInitialized)
{
   if (initialized != mustBeInitialized)
   {
      printTime(&Serial);
      Serial.print(F("FlightSimSwitch ERROR: "));
      Serial.print(message);
      Serial.print(F(" must be called "));
      Serial.print(mustBeInitialized ? F("after") : F("before"));
      Serial.print(F("begin() "));
      if (mustBeInitialized)
      {
         Serial.print(F("has been called with a correct setup."));
      }
      Serial.println();
      return false;
   }
   return true;
}


void FlightSimSwitches::begin()
{
   if (this->columnPinsAreDynamic)
   {
      // find column pins
      size_t  colIdxCtr    = 0;
      uint8_t *colIdxPtr   = dynamicColumnPins;
      bool    allPinsFound = true;
      for (MatrixElement *elem = MatrixElement::firstElement; elem; elem = elem->nextElement)
      {
         if (elem->matrix == this)
         {
            size_t elementsStored = elem->setPinData(colIdxPtr, colIdxCtr);
            if (elementsStored)
            {
               colIdxCtr += elementsStored;
               colIdxPtr += elementsStored;
            }
            else
            {
               allPinsFound = false;
            }
         }
      }
      numberOfColumns = colIdxCtr;


      if (!allPinsFound)
      {
         printTime(&Serial);
         Serial.print(F("FlightSimSwitches WARNING: Not all pins could be assigned, only the first "));
         Serial.print(MAX_COLUMNS);
         Serial.println(F(" pins are active"));
      }

      if (debugConfig)
      {
         printTime(&Serial);
         Serial.print(F("Pins: "));
         for (size_t i = 0; i < numberOfColumns; i++)
         {
            Serial.print(columnPins[i]);
            Serial.print(F(" "));
         }
         Serial.println();
      }
   }

   if (this->numberOfColumns > MAX_COLUMNS)
   {
      printTime(&Serial);
      Serial.print(F("FlightSimSwitches ERROR: Sorry, "));
      Serial.print(MAX_COLUMNS);
      Serial.println(F(" columns max"));
      return;
   }

   if (!this->numberOfColumns)
   {
      printTime(&Serial);
      Serial.println(F("FlightSimSwitches ERROR: Sorry, we need at least one column"));
      return;
   }

   if (this->numberOfRows > MAX_ROWS)
   {
      printTime(&Serial);
      Serial.print(F("FlightSimSwitches ERROR: Sorry, "));
      Serial.print(MAX_ROWS);
      Serial.println(F(" rows max"));
      return;
   }

   if (!this->numberOfRows)
   {
      printTime(&Serial);
      Serial.println(F("FlightSimSwitches ERROR: Sorry, we need at least one row"));
      return;
   }

   if (!this->rowPins)
   {
      Serial.println(F("FlightSimSwitches ERROR:  Please set the row pins"));
      return;
   }

   if (!this->columnPins)
   {
      printTime(&Serial);
      Serial.println(F("FlightSimSwitches ERROR: Please set the column pins"));
      return;
   }

   if (!this->scanRate)
   {
      printTime(&Serial);
      Serial.println(F("FlightSimSwitch ERROR: scanRate can not be zero"));
      return;
   }

   if (!rowsMuxed)
   {
      this->numberOfRowPins = numberOfRows;
   }
   else
   {
      for (int i = 0; i < 32; i++)
      {
         if (_BV32(i) >= numberOfRows)
         {
            this->numberOfRowPins = i;
            break;
         }
      }
      if (debugScan)
      {
         printTime(&Serial);
         Serial.print(F("FlightSimSwitch: Number of row pins: "));
         Serial.println(numberOfRowPins);
      }
   }

   memset(rowData, 0, MAX_ROWS * sizeof(uint32_t));

   if (rowPins != FLIGHTSIM_EMPTY_PINS) {
     for (int i = 0; i < numberOfRowPins; i++)
     {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], activeLow ? HIGH : LOW);
     }
   }

   for (int i = 0; i < numberOfColumns; i++)
   {
#ifdef INPUT_PULLDOWN
      pinMode(columnPins[i], activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
#else
      pinMode(columnPins[i], activeLow ? INPUT_PULLUP : INPUT);
#endif
   }

   initialized = true;

   setRowNumber(0);
   this->matrixTimer = 0;
}


void FlightSimSwitches::setRowNumber(uint32_t currentRow)
{
   if (!checkInitialized(F("setRowNumber"), true))
   {
      return;
   }

   this->currentRow = currentRow;
   if (rowPins == FLIGHTSIM_EMPTY_PINS)
   {
      return;                      // do nothing if switches are connected directly to Teensy pins
   }

   if (debugScan)
   {
      printTime(&Serial);
      Serial.print(F("FlightSimSwitches: Setting row "));
      Serial.println(currentRow);
   }

   if (!rowsMuxed)
   {
      // turn only selected row output LOW
      if (lastRow != 0xff)
      {
         digitalWrite(rowPins[lastRow], activeLow ? HIGH : LOW);
      }
      digitalWrite(rowPins[currentRow], activeLow ? LOW : HIGH);
   }
   else
   {
      // output row number to row select pins
      for (uint8_t i = 0; i < numberOfRowPins; i++)
      {
         digitalWrite(rowPins[i], (currentRow & _BV32(i)) ? HIGH : LOW);
      }
   }
   lastRow = currentRow;
}


uint32_t FlightSimSwitches::getSingleRowData()
{
   if (!checkInitialized(F("getSingleRowData"), true))
   {
      return 0;
   }

   uint32_t readVal = 0;
   for (uint8_t i = 0; i < numberOfColumns; i++)
   {
      bool readData = (digitalRead(columnPins[i]) == HIGH) ^ activeLow;
      if (readData)
      {
         readVal |= _BV32(i);
      }
   }
   if (debugScan)
   {
      printTime(&Serial);
      Serial.print(F("FlightSimSwitches: reading row data: "));
      Serial.println(readVal);
   }
   return readVal;
}


void FlightSimSwitches::loop()
{
   if (!checkInitialized(F("loop"), true))
   {
      return;
   }

   if (matrixTimer > scanRate)
   {
      // read current row
      matrixTimer = 0;
      uint32_t readData = getSingleRowData();
      if (rowData[currentRow] != readData)
      {
         if (changePositionCallback)
         {
            uint32_t diff = readData ^ rowData[currentRow];
            for (uint8_t i = 0; i < numberOfColumns; i++)
            {
               if (diff & _BV32(i))
               {
                  (*changePositionCallback)(currentRow, i, readData & _BV32(i));
               }
            }
         }
         rowData[currentRow] = readData;
         hasChangedPoll      = true;
         hasChangedLoop      = true;
      }

      currentRow++;
      if (currentRow >= numberOfRows)
      {
         // end of scan
         currentRow = 0;
         if (hasChangedLoop && changeMatrixCallback)
         {
            (*changeMatrixCallback)();
         }
         hasChangedLoop = false;

         bool enabled = FlightSim.isEnabled();
         bool resync  = enabled && (!lastEnabled);
         if (resync)
         {
            printTime(&Serial);
            Serial.println(F("FlightSimSwitches: Flightsim started, resyncing!"));
         }
         for (MatrixElement *elem = MatrixElement::firstElement; elem; elem = elem->nextElement)
         {
            if (elem->matrix == this)
            {
               elem->handleLoop(resync);
            }
         }
         lastEnabled = enabled;
      }

      setRowNumber(currentRow);
   }
}


void FlightSimSwitches::print()
{
   if (!checkInitialized(F("print"), true))
   {
      return;
   }

   if (numberOfRows > 1)
   {
      // Matrix
      printTime(&Serial);
      Serial.println(F("Switch matrix"));
      printTime(&Serial);
      Serial.print(F("    "));
      for (uint8_t i = 0; i < numberOfColumns; i++)
      {
         Serial.print(F(" "));
         if (i < 10)
         {
            Serial.print(F(" "));
         }
         Serial.print(i);
      }
      Serial.println();
      for (uint8_t r = 0; r < numberOfRows; r++)
      {
         printTime(&Serial);
         if (r < 10)
         {
            Serial.print(F(" "));
         }
         Serial.print(r);
         Serial.print(F(": "));
         for (uint8_t c = 0; c < numberOfColumns; c++)
         {
            if (rowData[r] & _BV32(c))
            {
               Serial.print(F("  X"));
            }
            else
            {
               Serial.print(F("   "));
            }
         }
         Serial.println();
      }
      printTime(&Serial);
      Serial.println(F("---------------"));
   }
   else
   {
      // directly connected Switches
      printTime(&Serial);
      Serial.print(F("Switches: "));
      for (uint8_t i = 0; i < numberOfColumns; i++)
      {
         Serial.print(i);
         Serial.print(F(": "));
         Serial.print(rowData[0] & _BV32(i) ? F("ON") : F("off"));
         if (i < numberOfColumns - 1)
         {
            Serial.print(F(", "));
         }
      }
      Serial.println();
   }
}


void FlightSimSwitches::setDebug(uint32_t debug_type)
{
   for (MatrixElement *elem = MatrixElement::firstElement; elem; elem = elem->nextElement)
   {
      if (elem->matrix == this)
      {
         elem->setDebug(debug_type & elem->getDebugMask());
      }
   }
   debugScan   = debug_type & DEBUG_SCAN;
   debugConfig = debug_type & DEBUG_SWITCHES_CONFIG;
}


/*
 * Generic matrix element. Not to be instantiated directly. Only keeps reference
 * to matrix and chain of elements.
 */

MatrixElement *MatrixElement::firstElement = NULL;
MatrixElement *MatrixElement::lastElement  = NULL;
bool          MatrixElement::lastEnabled   = false;

MatrixElement::MatrixElement(FlightSimSwitches *matrix)
{
   this->matrix = matrix;
   if (firstElement == NULL)
   {
      firstElement = this;
   }
   else
   {
      lastElement->nextElement = this;
   }
   lastElement              = this;
   this->debug              = false;
   this->change_callback    = NULL;
   this->callbackContext    = NULL;
   this->hasCallbackContext = false;
}


size_t MatrixElement::setGenericPinData(uint8_t *destination, uint32_t startPinIndex, uint32_t *matrixPositions, size_t count)
{
   if (startPinIndex < MAX_COLUMNS + count)
   {
      for (size_t i = 0; i < count; i++)
      {
         *destination     = (uint8_t)(*matrixPositions);
         *matrixPositions = startPinIndex;
         destination++;
         matrixPositions++;
         startPinIndex++;
      }
      return count;
   }
   else
   {
      return 0;
   }
}


bool MatrixElement::getPositionData(uint32_t position)
{
   if (!matrix)
   {
      matrix->printTime(&Serial);
      Serial.println(F("FlightSimSwitch ERROR: Please define a FlightSimSwitches or MatrixSwitches object before defining individual switches!"));
      return 0;
   }
   if (position == NO_POSITION)
   {
      matrix->printTime(&Serial);
      Serial.println(F("FlightSimSwitch ERROR: Switch position not set!"));
   }
   uint32_t *rowData = matrix->getRowData();
   uint32_t rowValue = rowData[MATRIX_ROW(position)];
   return rowValue & _BV32(MATRIX_COLUMN(position));
}


void MatrixElement::callback(float newValue)
{
   if (!change_callback)
   {
      return;
   }

   if (hasCallbackContext)
   {
      (*(void (*)(float, void *))change_callback)(newValue, callbackContext);
   }
   else
   {
      (*change_callback)(newValue);
   }
}


/*
 * Matrix On-Off switch. Sends one of two commands to X-Plane.
 *
 * This switch corresponds to a single position in the matrix
 */

FlightSimOnOffCommandSwitch::FlightSimOnOffCommandSwitch(FlightSimSwitches *matrix, uint32_t matrixPosition)
   : MatrixElement(matrix)
{
   this->matrixPosition = matrixPosition;
   this->oldValue       = false;
   this->onName         = XPlaneRef("(null)");
   this->offName        = XPlaneRef("(null)");
   this->hasOnCommand   = false;
   this->hasOffCommand  = false;
}


void FlightSimOnOffCommandSwitch::setOnOffCommands(const _XpRefStr_ *onCommand, const _XpRefStr_ *offCommand)
{
   this->onName  = onCommand;
   this->offName = offCommand;
   this->onCommand.assign(onCommand);
   this->offCommand.assign(offCommand);
   this->hasOnCommand  = true;
   this->hasOffCommand = true;
}


void FlightSimOnOffCommandSwitch::setOnCommandOnly(const _XpRefStr_ *onCommand)
{
   this->onName = onCommand;
   this->onCommand.assign(onCommand);
   this->hasOnCommand = true;
}


void FlightSimOnOffCommandSwitch::setOffCommandOnly(const _XpRefStr_ *offCommand)
{
   this->offName = offCommand;
   this->offCommand.assign(offCommand);
   this->hasOffCommand = true;
}


void FlightSimOnOffCommandSwitch::handleLoop(bool resync)
{
   bool value = getPositionData(matrixPosition);

   if ((value != oldValue) || resync)
   {
      oldValue = value;

      if (!hasOnCommand && !hasOffCommand)
      {
         matrix->printTime(&Serial);
         Serial.println(F("FlightSimOnOffSwitch ERROR: OnOffCommandSwitch: Commands not set"));
         return;
      }

      if (value)
      {
         if (hasOnCommand)
         {
            if (debug)
            {
               matrix->printTime(&Serial);
               Serial.print(F("FlightSimOnOffCommandSwitch: Sending ON command "));
               Serial.println(PRINT_DATAREF(onName));
            }
            onCommand.once();
            callback(1.0);
         }
      }
      else
      {
         if (hasOffCommand)
         {
            if (debug)
            {
               matrix->printTime(&Serial);
               Serial.print(F("FlightSimOnOffCommandSwitch: Sending OFF command "));
               Serial.println(PRINT_DATAREF(offName));
            }
            offCommand.once();
            callback(0.0);
         }
      }
   }
}


float FlightSimOnOffCommandSwitch::getValue()
{
   return oldValue ? 1.0 : 0.0;
}


/*
 * Matrix pushbutton. Sends a single command with begin/end to X-Plane.
 *
 * This pushbutton corresponds to a single position in the matrix
 */

FlightSimPushbutton::FlightSimPushbutton(FlightSimSwitches *matrix, uint32_t matrixPosition, bool inverted)
   : MatrixElement(matrix)
{
   this->matrixPosition = matrixPosition;
   this->oldValue       = false;
   this->commandName    = XPlaneRef("(null)");
   this->inverted       = inverted;
}


void FlightSimPushbutton::handleLoop(bool resync)
{
   bool value = getPositionData(matrixPosition);

   if ((value != oldValue) || resync)
   {
      oldValue = value;

      if (value ^ inverted)
      {
         if (debug)
         {
            matrix->printTime(&Serial);
            Serial.print(F("FlightSimOnOffCommandSwitch: Sending command "));
            Serial.print(PRINT_DATAREF(commandName));
            Serial.println(F(" BEGIN"));
         }
         command.begin();
         callback(1.0);
      }
      else
      {
         if (debug)
         {
            matrix->printTime(&Serial);
            Serial.print(F("FlightSimOnOffCommandSwitch: Sending command "));
            Serial.print(PRINT_DATAREF(commandName));
            Serial.println(F(" END"));
         }
         command.end();
         callback(0.0);
      }
   }
}


float FlightSimPushbutton::getValue()
{
   return oldValue ? 1.0 : 0.0;
}


/*
 * Matrix Multi-position switch. Sends either up or down commands to X-Plane,
 * in order to match a switch position to a defined value
 *
 * Each position in the matrix corresponds to one value the switch can take
 */


FlightSimUpDownCommandSwitch::FlightSimUpDownCommandSwitch(FlightSimSwitches *matrix, uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue, float tolerance)
   : MatrixElement(matrix)
{
   this->numberOfPositions = numberOfPositions;
   this->matrixPositions   = positions;
   this->values            = values;
   this->defaultValue      = defaultValue;
   this->oldDatarefValue   = 0.0;
   this->oldSwitchValue    = 0.0;
   this->switchChanged     = false;
   this->tolerance         = tolerance;
   this->commandSent       = false;
   this->name = XPlaneRef("(null)");
   this->pushbuttonPositions = 0;
   this->pushbuttonCommand   = NULL;
}


void FlightSimUpDownCommandSwitch::setDatarefAndCommands(const _XpRefStr_ *positionDataref, const _XpRefStr_ *upCommand, const _XpRefStr_ *downCommand)
{
   this->name = positionDataref;
   this->positionDataref.assign(positionDataref);
   this->upCommand.assign(upCommand);
   this->downCommand.assign(downCommand);
}


float FlightSimUpDownCommandSwitch::findValue(int8_t *valueIndex)
{
   for (uint8_t i = 0; i < numberOfPositions; i++)
   {
      if (getPositionData(matrixPositions[i]))
      {
         if (valueIndex)
         {
            *valueIndex = i;
         }
         return values[i];
      }
   }
   return defaultValue;
}


void FlightSimUpDownCommandSwitch::handleLoop(bool resync)
{
   int8_t valueIndex   = -1;
   float  switchValue  = findValue(&valueIndex);        // get current value and value index on matrix
   float  datarefValue = positionDataref.read();        // get current value in X-Plane

   if ((switchValue != oldSwitchValue) || resync)
   {
      if (debug)
      {
         matrix->printTime(&Serial);
         Serial.print(F("FlightSimUpDownCommandSwitch: dataref name: "));
         Serial.print(PRINT_DATAREF(name));
         Serial.print(F(", switch value="));
         Serial.print(switchValue);
         Serial.print(F(", old switch value="));
         Serial.print(oldSwitchValue);
         Serial.print(F(", resync="));
         Serial.print(resync ? F("TRUE") : F("FALSE"));
         Serial.print(F(", valueIndex="));
         Serial.print(valueIndex);
         Serial.println(F(", switch changed!"));
      }
      switchChanged  = true;
      oldSwitchValue = switchValue;
      callback(switchValue);
   }

   if (abs(switchValue - datarefValue) < tolerance)
   {
      if (switchChanged)
      {
         // below tolerance is not considered a change
         if (debug)
         {
            matrix->printTime(&Serial);
            Serial.print(F("FlightSimUpDownCommandSwitch: dataref name: "));
            Serial.print(PRINT_DATAREF(name));
            Serial.print(F(", switch value="));
            Serial.print(switchValue);
            Serial.print(F(", dataref value="));
            Serial.print(datarefValue);
            Serial.println(F(", final position reached!"));
         }
         switchChanged = false;                // reached final value
      }
      commandSent = false;
      return;
   }

   if (commandSent)
   {
      // check if previous command has finished
      if (abs(datarefValue - oldDatarefValue) < tolerance)
      {
         // no, dataref still has previous value. Wait for dataref
         // to change
         return;
      }
      else
      {
         // dataref has changed, we can send the next command
         if (debug)
         {
            matrix->printTime(&Serial);
            Serial.print(F("FlightSimUpDownCommandSwitch: dataref name: "));
            Serial.print(PRINT_DATAREF(name));
            Serial.print(F(", switch value="));
            Serial.print(switchValue);
            Serial.print(F(", dataref value="));
            Serial.print(datarefValue);
            Serial.print(F(", old dataref value="));
            Serial.print(oldDatarefValue);
            Serial.println(F(", previous command handled!"));
         }
         commandSent = false;
      }
   }

   if (switchChanged)
   {
      commandSent     = true;
      oldDatarefValue = datarefValue;
      if (switchValue > datarefValue)
      {
         if (debug)
         {
            matrix->printTime(&Serial);
            Serial.print(F("FlightSimUpDownCommandSwitch: dataref name: "));
            Serial.print(PRINT_DATAREF(name));
            Serial.print(F(", switch value="));
            Serial.print(switchValue);
            Serial.print(F(", dataref value="));
            Serial.print(datarefValue);
            Serial.println(F(", sending UP command"));
         }
         if (pushbuttonCommand)
         {
            pushbuttonCommand->end();
            pushbuttonCommand = NULL;
         }
         if ((valueIndex != -1) && (pushbuttonPositions & _BV32(valueIndex)))
         {
            upCommand.begin();
            pushbuttonCommand = &upCommand;
         }
         else
         {
            upCommand.once();
         }
      }
      else
      {
         if (debug)
         {
            matrix->printTime(&Serial);
            Serial.print(F("FlightSimUpDownCommandSwitch: dataref name: "));
            Serial.print(PRINT_DATAREF(name));
            Serial.print(F(", switch value="));
            Serial.print(switchValue);
            Serial.print(F(", dataref value="));
            Serial.print(datarefValue);
            Serial.println(F(", sending DOWN command"));
         }
         if (pushbuttonCommand)
         {
            pushbuttonCommand->end();
            pushbuttonCommand = NULL;
         }
         if ((valueIndex != -1) && (pushbuttonPositions & _BV32(valueIndex)))
         {
            pushbuttonCommand = &downCommand;
            downCommand.begin();
         }
         else
         {
            downCommand.once();
         }
      }
   }
}


float FlightSimUpDownCommandSwitch::getValue()
{
   return oldSwitchValue;
}


/*
 * Matrix On-Off switch that writes values to a dataref
 *
 */
FlightSimOnOffDatarefSwitch::FlightSimOnOffDatarefSwitch(FlightSimSwitches *matrix, uint32_t position, bool inverted)
   : MatrixElement(matrix)
{
   this->matrixPosition = position;
   this->inverted       = inverted;
   this->oldValue       = false;
   this->name           = XPlaneRef("(null)");
}


void FlightSimOnOffDatarefSwitch::setDataref(const _XpRefStr_ *positionDataref)
{
   this->name = positionDataref;
   this->dataref.assign(positionDataref);
}


void FlightSimOnOffDatarefSwitch::handleLoop(bool resync)
{
   bool switchOn = getPositionData(matrixPosition);

   if ((switchOn != oldValue) || resync)
   {
      int32_t value = switchOn ^ inverted ? 1 : 0;
      if (debug)
      {
         matrix->printTime(&Serial);
         Serial.print(F("FlightSimOnOffDatarefSwitch: Writing value "));
         Serial.print(value);
         Serial.print(F(" to dataref "));
         Serial.println(PRINT_DATAREF(name));
      }
      dataref.write(value);
      oldValue = switchOn;
      callback(switchOn ? 1.0 : 0.0);
   }
}


float FlightSimOnOffDatarefSwitch::getValue()
{
   return oldValue ? 1.0 : 0.0;
}


/*
 * Matrix Multi-position switch that writes values to a dataref
 *
 * Each position in the matrix corresponds to one value the switch can take
 */


FlightSimWriteDatarefSwitch::FlightSimWriteDatarefSwitch(FlightSimSwitches *matrix, uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue, float tolerance)
   : MatrixElement(matrix)
{
   this->numberOfPositions = numberOfPositions;
   this->matrixPositions   = positions;
   this->values            = values;
   this->defaultValue      = defaultValue;
   this->tolerance         = tolerance;
   this->oldSwitchValue    = 0.0;
   this->name = XPlaneRef("(null)");
}


void FlightSimWriteDatarefSwitch::setDataref(const _XpRefStr_ *positionDataref)
{
   this->name = positionDataref;
   this->positionDataref.assign(positionDataref);
}


float FlightSimWriteDatarefSwitch::findValue()
{
   for (uint8_t i = 0; i < numberOfPositions; i++)
   {
      if (getPositionData(matrixPositions[i]))
      {
         return values[i];
      }
   }
   return defaultValue;
}


void FlightSimWriteDatarefSwitch::handleLoop(bool resync)
{
   float switchValue = findValue();      // get current value on matrix

   if ((switchValue != oldSwitchValue) || resync)
   {
      oldSwitchValue = switchValue;
      if (debug)
      {
         matrix->printTime(&Serial);
         Serial.print(F("FlightSimWriteDatarefSwitch: Writing value "));
         Serial.print(switchValue);
         Serial.print(F(" to dataref "));
         Serial.println(PRINT_DATAREF(name));
      }
      positionDataref.write(switchValue);
      callback(switchValue);
   }
}


float FlightSimWriteDatarefSwitch::getValue()
{
   return oldSwitchValue;
}
