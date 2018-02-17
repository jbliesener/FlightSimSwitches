#ifndef _FLIGHTSIM_SWITCHES_H
#define _FLIGHTSIM_SWITCHES_H

#include <Arduino.h>

#ifndef FLIGHTSIM_INTERFACE
#error "Please use a Teensy board and set USB Type in Arduino to include 'Flight Sim Controls'"
#endif

/*
 * Switch Matrix for Teensy Flightsim projects
 *
 * (c) Jorg Neves Bliesener
 */

// position macros
#define MATRIX(row,column) ((row<<5) | column)
#define MATRIX_ROW(n) (n >> 5)
#define MATRIX_COLUMN(n) (n & 0x1f)

// default values
#define DEFAULT_SCAN_RATE (15) // default scan rate in milliseconds
#define DEFAULT_TOLERANCE (1E-4) // default tolerance for multi-position switches
#define NO_POSITION (0xffffffff)

// helper macros
#define SWITCH_POSITIONS(...) (uint32_t[]){__VA_ARGS__}
#define SWITCH_PINS(...)      (const uint8_t[]){__VA_ARGS__}
#define SWITCH_VALUES(...)    (float[]){__VA_ARGS__}

// alternative names
#define SwitchMatrix FlightSimSwitches
#define MatrixOnOffCommandSwitch FlightSimOnOffCommandSwitch
#define MatrixOnCommandSwitch FlightSimOnCommandSwitch
#define MatrixOffCommandSwitch FlightSimOffCommandSwitch
#define MatrixPushbutton FlightSimPushbutton
#define MatrixUpDownCommandSwitch FlightSimUpDownCommandSwitch
#define MatrixOnOffDatarefSwitch FlightSimOnOffDatarefSwitch
#define MatrixWriteDatarefSwitch FlightSimWriteDatarefSwitch

// debug values
#define DEBUG_SCAN (1)
#define DEBUG_SWITCHES_ONOFF_COMMAND (2)
#define DEBUG_SWITCHES_ON_COMMAND (4)
#define DEBUG_SWITCHES_OFF_COMMAND (8)
#define DEBUG_SWITCHES_PUSHBUTTON (16)
#define DEBUG_SWITCHES_UPDOWN_COMMAND (32)
#define DEBUG_SWITCHES_ONOFF_DATAREF (64)
#define DEBUG_SWITCHES_WRITE_DATAREF (128)
#define DEBUG_SWITCHES (0xFFFFFFFF & ~DEBUG_SCAN)
#define DEBUG_OFF (0)

class FlightSimSwitches {
  public:
    FlightSimSwitches();
    FlightSimSwitches(uint8_t numberOfRows, const uint8_t *rowPins,
                 uint8_t numberOfColumns, const uint8_t *columnPins,
                 uint32_t scanrate=DEFAULT_SCAN_RATE,
                 bool activeLow=true,
                 bool rowsMuxed=false );
    FlightSimSwitches(uint8_t numberOfColumns, const uint8_t *columnPins,
      uint32_t scanrate=DEFAULT_SCAN_RATE,
      bool activeLow=true);
    void setNumberOfOutputs(uint8_t rows) { if (checkInitialized("setNumberOfRows",false)) numberOfRows=rows; }
    void setNumberOfInputs  (uint8_t columns) { if (checkInitialized("setNumberOfColumns",false)) numberOfColumns=columns; }
    void setOutputPins(const uint8_t *rowPins) { if (checkInitialized("setRowPins",false)) this->rowPins = rowPins; }
    void setInputPins(const uint8_t *columnPins) { if (checkInitialized("setColumnPins",false)) this->columnPins = columnPins; }
    void setScanRate(uint32_t scanRate) { this->scanRate = scanRate; }
    void setActiveLow(uint32_t activeLow) { if (checkInitialized("setActiveLow",false)) this->activeLow=activeLow; }
    void setRowsMultiplexed(uint32_t rowsMuxed) { if (checkInitialized("setRowsMultiplexed",false)) this->rowsMuxed=rowsMuxed; }
    void begin();
    void loop();
    uint32_t *getRowData() { return rowData; }
    bool isOn(const uint8_t row, const uint8_t column) { return rowData[row] & _BV(column); }
    void onChangePosition(void (*fptr)(uint8_t, uint8_t, bool)) { changePositionCallback = fptr; }
    void onChangeMatrix(void (*fptr)()) { changeMatrixCallback = fptr; }
    bool hasChanged() { return this->hasChangedPoll; }
    void clearChanged() { this->hasChangedPoll=false; }
    void setDebug(uint32_t debugType);
    void print();
    static FlightSimSwitches *firstMatrix;
  private:
    bool checkInitialized(const char *message, bool mustBeInitialized);
    void setRowNumber(uint32_t currentRow);
    uint32_t getSingleRowData();

    uint8_t numberOfRows;
    uint8_t numberOfRowPins;
    const uint8_t *rowPins;
    bool rowsMuxed;

    uint8_t numberOfColumns;
    const uint8_t *columnPins;

    bool activeLow;
    uint32_t scanRate;
    elapsedMillis matrixTimer;

    uint8_t currentRow;
    uint8_t lastRow;
    uint32_t *rowData;
    bool initialized;
    bool hasChangedLoop;
    bool hasChangedPoll;
    bool lastEnabled;

    bool debugScan;

    void (*changePositionCallback)(uint8_t,uint8_t,bool);
    void (*changeMatrixCallback)();
};


class MatrixElement {
  friend class FlightSimSwitches;

  public:
    MatrixElement(FlightSimSwitches *matrix);
    MatrixElement(FlightSimSwitches &matrix) : MatrixElement(&matrix) {};
    void setDebug(bool debug) { this->debug = debug; }
  protected:
    FlightSimSwitches *matrix;
    MatrixElement* nextElement;
    static MatrixElement *firstElement;
    static MatrixElement *lastElement;
    static bool lastEnabled;
    bool debug;
    bool getPositionData(uint32_t position);
    virtual void handleLoop(bool resync) =0;
    virtual uint32_t getDebugMask() = 0;
};

class FlightSimOnOffCommandSwitch : MatrixElement {
  public:
    FlightSimOnOffCommandSwitch(FlightSimSwitches *matrix, uint32_t matrixPosition);

    FlightSimOnOffCommandSwitch(uint32_t matrixPosition) : FlightSimOnOffCommandSwitch(FlightSimSwitches::firstMatrix, matrixPosition) {};
    FlightSimOnOffCommandSwitch(FlightSimSwitches &matrix, uint32_t matrixPosition) : FlightSimOnOffCommandSwitch(&matrix, matrixPosition) {};
    FlightSimOnOffCommandSwitch(FlightSimSwitches &matrix) : FlightSimOnOffCommandSwitch(&matrix, NO_POSITION) {};
    FlightSimOnOffCommandSwitch() : FlightSimOnOffCommandSwitch(FlightSimSwitches::firstMatrix, NO_POSITION) {};

    void setPosition(uint32_t matrixPosition) { this->matrixPosition = matrixPosition; }
    void setOnOffCommands(const _XpRefStr_ *onCommand, const _XpRefStr_ *offCommand);
  protected:
    void setOnCommandOnly(const _XpRefStr_ *onCommand);
    void setOffCommandOnly(const _XpRefStr_ *offCommand);
    virtual void handleLoop(bool resync);
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_ONOFF_COMMAND; }
  private:
    uint32_t matrixPosition;
    bool oldValue;
    bool hasOnCommand;
    bool hasOffCommand;
    const _XpRefStr_ *onName;
    const _XpRefStr_ *offName;
    FlightSimCommand onCommand;
    FlightSimCommand offCommand;
};

class FlightSimOnCommandSwitch : FlightSimOnOffCommandSwitch {
  public:
    FlightSimOnCommandSwitch(FlightSimSwitches *matrix, uint32_t matrixPosition) : FlightSimOnOffCommandSwitch(matrix, matrixPosition) {};

    FlightSimOnCommandSwitch(uint32_t matrixPosition) : FlightSimOnCommandSwitch(FlightSimSwitches::firstMatrix, matrixPosition) {};
    FlightSimOnCommandSwitch(FlightSimSwitches &matrix, uint32_t matrixPosition) : FlightSimOnCommandSwitch(&matrix, matrixPosition) {};
    FlightSimOnCommandSwitch(FlightSimSwitches &matrix) : FlightSimOnCommandSwitch(&matrix, NO_POSITION) {};
    FlightSimOnCommandSwitch() : FlightSimOnCommandSwitch(FlightSimSwitches::firstMatrix, NO_POSITION) {};

    void setPosition(uint32_t matrixPosition)  { FlightSimOnOffCommandSwitch::setPosition(matrixPosition); };
    void setCommand(const _XpRefStr_ *onCommand)  { FlightSimOnOffCommandSwitch::setOnCommandOnly(onCommand); };
    FlightSimOnCommandSwitch & operator = (const _XpRefStr_ *s) { setCommand(s); return *this; }
  protected:
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_ON_COMMAND; }
};

class FlightSimOffCommandSwitch : FlightSimOnOffCommandSwitch {
  public:
    FlightSimOffCommandSwitch(FlightSimSwitches *matrix, uint32_t matrixPosition) : FlightSimOnOffCommandSwitch(matrix, matrixPosition) {};

    FlightSimOffCommandSwitch(uint32_t matrixPosition) : FlightSimOffCommandSwitch(FlightSimSwitches::firstMatrix, matrixPosition) {};
    FlightSimOffCommandSwitch(FlightSimSwitches &matrix, uint32_t matrixPosition) : FlightSimOffCommandSwitch(&matrix, matrixPosition) {};
    FlightSimOffCommandSwitch(FlightSimSwitches &matrix) : FlightSimOffCommandSwitch(&matrix, NO_POSITION) {};
    FlightSimOffCommandSwitch() : FlightSimOffCommandSwitch(FlightSimSwitches::firstMatrix, NO_POSITION) {};

    void setPosition(uint32_t matrixPosition) { FlightSimOnOffCommandSwitch::setPosition(matrixPosition); }
    void setCommand(const _XpRefStr_ *offCommand)  { FlightSimOnOffCommandSwitch::setOffCommandOnly(offCommand); };
    FlightSimOffCommandSwitch & operator = (const _XpRefStr_ *s) { setCommand(s); return *this; }
  protected:
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_OFF_COMMAND; }
};

class FlightSimPushbutton : MatrixElement {
  public:
    FlightSimPushbutton(FlightSimSwitches *matrix, uint32_t matrixPosition, bool inverted=false);

    FlightSimPushbutton(uint32_t matrixPosition, bool inverted=false) : FlightSimPushbutton(FlightSimSwitches::firstMatrix, matrixPosition, inverted) {};
    FlightSimPushbutton(FlightSimSwitches &matrix, uint32_t matrixPosition, bool inverted=false) : FlightSimPushbutton(&matrix, matrixPosition, inverted) {};
    FlightSimPushbutton() : FlightSimPushbutton(FlightSimSwitches::firstMatrix, NO_POSITION) {};

    void setPosition(uint32_t matrixPosition) { this->matrixPosition = matrixPosition; }
    void setInverted(bool inverted) { this->inverted = inverted; }
    void setCommand(const _XpRefStr_ *command)  { this->command = command; this->commandName = command; };
    FlightSimPushbutton & operator = (const _XpRefStr_ *s) { setCommand(s); return *this; }
  protected:
    virtual void handleLoop(bool resync);
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_PUSHBUTTON; }
    uint32_t matrixPosition;
    bool oldValue;
    bool inverted;
    const _XpRefStr_ *commandName;
    FlightSimCommand command;
};



class FlightSimUpDownCommandSwitch : MatrixElement {
  public:
    FlightSimUpDownCommandSwitch(FlightSimSwitches *matrix,
      uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue=0, float tolerance=DEFAULT_TOLERANCE);

    FlightSimUpDownCommandSwitch(FlightSimSwitches &matrix,
      uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue=0, float tolerance=DEFAULT_TOLERANCE) :
        FlightSimUpDownCommandSwitch(&matrix, numberOfPositions, positions, values, defaultValue, tolerance) {}

    FlightSimUpDownCommandSwitch(
      uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue=0, float tolerance=DEFAULT_TOLERANCE) :
        FlightSimUpDownCommandSwitch(FlightSimSwitches::firstMatrix, numberOfPositions, positions, values, defaultValue, tolerance) {}

    void setDefaultValue(float defaultValue) { this->defaultValue = defaultValue; }
    void setTolerance(float tolerance) { this->tolerance = tolerance; }
    void setDatarefAndCommands(const _XpRefStr_ *positionDataref, const _XpRefStr_ *upCommand, const _XpRefStr_ *downCommand);
    void setPushbuttonPosition(const uint8_t pushbuttonPosition) { this->pushbuttonPositions |= _BV(pushbuttonPosition); }
  protected:
    virtual float getValue(uint8_t *valueIndex);
    virtual void handleLoop(bool resync);
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_UPDOWN_COMMAND; }
  private:
    uint8_t  numberOfPositions;
    uint32_t *matrixPositions;
    uint32_t pushbuttonPositions;
    FlightSimCommand *pushbuttonCommand;
    float    *values;
    float    defaultValue;
    float    tolerance;

    bool     switchChanged;
    const _XpRefStr_ *name;
    FlightSimFloat positionDataref;
    FlightSimCommand upCommand;
    FlightSimCommand downCommand;
    bool commandSent;
    float oldDatarefValue;
    float oldSwitchValue;
};

class FlightSimWriteDatarefSwitch : MatrixElement {
  public:
    FlightSimWriteDatarefSwitch(FlightSimSwitches *matrix,
      uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue=0, float tolerance=DEFAULT_TOLERANCE);

    FlightSimWriteDatarefSwitch(
      uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue=0, float tolerance=DEFAULT_TOLERANCE)
        : FlightSimWriteDatarefSwitchFlightSimSwitches::firstMatrix, numberOfPositions, positions, values, defaultValue, tolerance) {}
    FlightSimWriteDatarefSwitch(FlightSimSwitches &matrix,
      uint8_t numberOfPositions, uint32_t *positions, float *values, float defaultValue=0, float tolerance=DEFAULT_TOLERANCE)
        : FlightSimWriteDatarefSwitch(&matrix, numberOfPositions, positions, values, defaultValue, tolerance) {}
    void setDefaultValue(float defaultValue) { this->defaultValue = defaultValue; }
    void setTolerance(float tolerance) { this->tolerance = tolerance; }
    void setDataref(const _XpRefStr_ *positionDataref);
    FlightSimWriteDatarefSwitch & operator = (const _XpRefStr_ *s) { setDataref(s); return *this; }
  protected:
    virtual float getValue();
    virtual void handleLoop(bool resync);
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_WRITE_DATAREF; }
  private:
    uint8_t  numberOfPositions;
    uint32_t *matrixPositions;
    float    oldSwitchValue;
    float    *values;
    float    tolerance;
    float    defaultValue;
    const _XpRefStr_ *name;
    FlightSimFloat positionDataref;
};

class FlightSimOnOffDatarefSwitch : MatrixElement {
  public:
    FlightSimOnOffDatarefSwitch(FlightSimSwitches *matrix, uint32_t position, bool inverted=false);

    FlightSimOnOffDatarefSwitch(FlightSimSwitches &matrix, uint32_t position, bool inverted=false)
      : FlightSimOnOffDatarefSwitch(&matrix,position,inverted) {};
    FlightSimOnOffDatarefSwitch(uint32_t matrixPosition, bool inverted=false)
      : FlightSimOnOffDatarefSwitch(FlightSimSwitches::firstMatrix, matrixPosition, inverted) {};
    FlightSimOnOffDatarefSwitch(FlightSimSwitches &matrix)
      : FlightSimOnOffDatarefSwitch(&matrix, NO_POSITION, false) {};
    FlightSimOnOffDatarefSwitch()
      : FlightSimOnOffDatarefSwitch(FlightSimSwitches::firstMatrix, NO_POSITION, false) {};


    FlightSimOnOffDatarefSwitch & operator = (const _XpRefStr_ *s) { setDataref(s); return *this; }
    void setDataref(const _XpRefStr_ *positionDataref);
    void setPosition(uint32_t matrixPosition) { this->matrixPosition = matrixPosition; }
    void setInverted(bool inverted) { this->inverted = inverted; }
  protected:
    virtual void handleLoop(bool resync);
    virtual uint32_t getDebugMask() { return DEBUG_SWITCHES_ONOFF_DATAREF; }
  private:
    uint32_t matrixPosition;
    bool inverted;
    bool oldValue;
    const _XpRefStr_ *name;
    FlightSimInteger dataref;
};

#endif // _FLIGHTSIM_SWITCHES_H
