// -------------------------------------------------------
// myFP2ESP8266 TEMPERATURE PROBE CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger Manz, 2020-2021. All Rights Reserved.
// temp_probe.cpp
// Optional
// Temperature Probe, temperature compensation
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"

#if defined(ENABLE_TEMPERATUREPROBE)

#include <OneWire.h>
#include <DallasTemperature.h>


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Temp Probe messages 
// to be written to Serial port
//#define TEMP_MSGPRINT 1

#ifdef TEMP_MSGPRINT
#define TempMsgPrint(...) Serial.print(__VA_ARGS__)
#define TempMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define TempMsgPrint(...)
#define TempMsgPrintln(...)
#endif


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

#include "temp_probe.h"


// -------------------------------------------------------
// EXTERNALS
// -------------------------------------------------------


// -------------------------------------------------------
// DEFINES
// -------------------------------------------------------
// temperature compensation DO NOT CHANGE
#define TEMP_FALLING 1
#define TEMP_RISING 0
#define DEFAULTEMPPIN 10

OneWire tpWire(DEFAULTEMPPIN);  // temp probe pin
DallasTemperature tpsensor(&tpWire);
DeviceAddress tprobe;


// -------------------------------------------------------
// CLASS
// Pass ControllerData->get_brdtemppin() as the 
// pin number
// -------------------------------------------------------
TEMP_PROBE::TEMP_PROBE(int pin)
  : _pin(pin) {

}


// -------------------------------------------------------
// START TEMP PROBE
// -------------------------------------------------------
bool TEMP_PROBE::start() {
  TempMsgPrint(T_TEMPPROBE);
  TempMsgPrintln(T_START);

  // supported by this board?
  if (ControllerData->get_brdtemppin() == -1) 
  {
    TempMsgPrintln(T_NOTSUPPORTED);
    return false;
  }
  
  TempMsgPrint(T_CHECK);
  TempMsgPrintln(TUC_ENABLE);
  if (ControllerData->get_tempprobe_enable() == STATE_DISABLED) 
  {
    TempMsgPrintln(T_DISABLED);
    _loaded = STATE_NOTLOADED;
    _lasttemp = DEFAULTLASTTEMP;
    tempcomp_state = STATE_DISABLED;
    return false;
  } 
  else 
  {
    TempMsgPrintln(T_ENABLED);
      
    // search for a sensor
    TempMsgPrintln("Find probe");
    tpsensor.begin();

    TempMsgPrint("Probes found ");
    TempMsgPrintln(tpsensor.getDeviceCount(), DEC);

    // report parasite power requirements
    TempMsgPrint("Parasite power is: ");
    if (tpsensor.isParasitePowerMode()) 
    {
      TempMsgPrintln(T_ON);
    } 
    else 
    {
      TempMsgPrintln(T_OFF);
    }

    if (!tpsensor.getAddress(tprobe, 0)) 
    {
      TempMsgPrintln("ERROR: find probe address");
    
      TempMsgPrintln("Search for probe");
      // method 1
      tpWire.reset_search();

      if (!tpWire.search(tprobe)) 
      {
        TempMsgPrintln("ERROR: probe address not found");
        _loaded = STATE_NOTLOADED;
        _lasttemp = DEFAULTLASTTEMP;
        tempcomp_state = STATE_DISABLED;
        return false;
      } 
    } 

    // show the addresses we found on the bus
    TempMsgPrint("Probe Address: ");
    //printAddress(tprobe);
    TempMsgPrintln();

    // request the sensor to begin a temperature reading
    // async request - read
    tpsensor.setWaitForConversion(false);
    tpsensor.requestTemperatures();
    //sensors.setWaitForConversion(true);

    _loaded = STATE_LOADED;
    tempcomp_available = AVAILABLE;
    tempcomp_state = ControllerData->get_tempcomp_onload();
    TempMsgPrintln(T_RUNNING);
    return true;
  }
}


// -------------------------------------------------------
// STOP PROBE
// -------------------------------------------------------
void TEMP_PROBE::stop() {
  _loaded = STATE_NOTLOADED;
  tempcomp_available = NOT_AVAILABLE;
  tempcomp_state = STATE_OFF;
}

// -------------------------------------------------------
// START A TEMPERATURE READING
// Can take up to 700mS, but we do an async read rather
// than a blocking call
// -------------------------------------------------------
void TEMP_PROBE::request() {
  if (_loaded == STATE_NOTLOADED) {
    return;
  }
  tpsensor.setWaitForConversion(false);
  tpsensor.requestTemperatures();
}


// -------------------------------------------------------
// READ TEMP PROBE VALUE
// -------------------------------------------------------
float TEMP_PROBE::read(void) {
  if (_loaded == STATE_NOTLOADED) {
    return _lasttemp;
  }

  // get temperature, always in celsius
  float result = tpsensor.getTempCByIndex(0);
  // avoid erronous readings
  if (result > -40.0 && result < 80.0) {
    // valid, save in _lasttemp
    _lasttemp = result;
  } else {
    // invalid, use the last valid temp reading
    result = _lasttemp;
  }
  return result;
}

// -------------------------------------------------------
// UPDATE_TEMP PROBE
// read temperature
// check for temperature compensation, apply tc rules
// -------------------------------------------------------
float TEMP_PROBE::update(void) {
  if (_loaded == STATE_NOTLOADED) {
    return _lasttemp;
  }

  // track tempcompenabled changes
  static bool tcchanged = tempcomp_state;
  // start with a temp request
  static byte requesttempflag = 0;
  static float tempval;
  // start temperature to use when temperature compensation is enabled
  static float starttemp;

  if (tcchanged != tempcomp_state) {
    tcchanged = tempcomp_state;
    if (tcchanged == 1) {
      starttemp = read();
    }
  }

  if (requesttempflag) {
    TempMsgPrintln("TP READ");
    tempval = read();
  } else {
    TempMsgPrintln("TP REQUEST");
    tpsensor.requestTemperatures();
  }

  // toggle flag
  requesttempflag ^= 1;

  // check for temperature compensation
  if (tempcomp_state) {
    // calculate if temp has moved by more than 1 degree
    if ((abs)(starttemp - tempval) >= 1) {
      long newPos;
      // temperature falling (1) or rising (0)?
      byte temperaturedirection;
      temperaturedirection = (tempval < starttemp) ? TEMP_FALLING : TEMP_RISING;

      // check if tc direction for compensation is inwards
      if (ControllerData->get_tcdirection() == TC_DIRECTION_IN) {
        // temperature compensation direction is inwards,
        // if temperature falling then move in else move out
        if (temperaturedirection == TEMP_FALLING) {
          // then move inwards
          newPos = ftargetPosition - ControllerData->get_tempcoefficient();
        } else {
          // else move outwards
          newPos = ftargetPosition + ControllerData->get_tempcoefficient();
        }
      } else {
        // temperature compensation direction is out,
        // if a fall then move out else move in
        if (temperaturedirection == TEMP_FALLING) {
          newPos = ftargetPosition + ControllerData->get_tempcoefficient();
        } else {
          newPos = ftargetPosition - ControllerData->get_tempcoefficient();
        }  // if ( temperaturedirection == 1 )
      }    // if (ControllerData->get_tcdirection() == 0)

      RangeCheck(&newPos, 0, ControllerData->get_maxstep());
      ftargetPosition = newPos;

      // save this current temp point for future reference
      starttemp = tempval;
    }  // end of check for tempchange >=1
  }    // end of check for tempcomp enabled
  return tempval;
}

// function to print a device address
void TEMP_PROBE::printAddress(DeviceAddress deviceAddress)
{
  String addr;
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) {
      addr += "0";      
    }
    addr += String(deviceAddress[i], HEX);
  }
}

#endif
