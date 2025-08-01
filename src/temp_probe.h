// -------------------------------------------------------
// myFP2ESP8266 support for DS18B20 Temperature Sensor
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger Manz, 2020-2021. All Rights Reserved.
// temp_probe.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#if !defined(_temp_probe_h_)
#define _temp_probe_h_

#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_TEMPERATUREPROBE)

#include <OneWire.h>
#include <DallasTemperature.h>



// -------------------------------------------------------
// TEMPERATURE CLASS
// -------------------------------------------------------
class TEMP_PROBE {
public:
  TEMP_PROBE(int);
  bool start(void);
  void stop(void);
  float update(void);

  void request(void);
  float read(void);

private:
  void printAddress(DeviceAddress);
  uint8_t _pin; 
  bool _loaded = STATE_NOTLOADED;
  float _lasttemp = DEFAULTLASTTEMP;
};

#endif
#endif
