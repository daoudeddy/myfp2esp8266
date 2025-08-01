// -------------------------------------------------------
// myFP2ESP8266 LOCALSERIAL CLASS DEFINTIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// serial_server.h
// Optional Configuration
// For ASCOM client via myFP2ASCOM driver
// For Windows and Linux applications (myFP2 Serial only)
// For INDI clients via myFP2 INDI driver using Serial connection
// -------------------------------------------------------


// -------------------------------------------------------
// Arduino IDE 2.3.4
// Arduino ESP8266 Core 3.1.2
// BOARD NODEMCU 1.0 (ESP-12E) ESP8266
// Flash Size 4MB (FS:2MB OTA:~1019KB)
// MMU 16KB Cache + 48KB IRAM and 2nd Heap (shared)
// Debug Level NONE
// Compiler Warnings DEFAULT
// -------------------------------------------------------


#ifndef _serial_server_h
#define _serial_server_h

#include <Arduino.h>
#include "config.h"


#if (CONTROLLERMODE == LOCALSERIAL)
// -------------------------------------------------------
// CLASS
// -------------------------------------------------------
class LOCAL_SERIAL {
public:
  explicit LOCAL_SERIAL(HardwareSerial &serial);

  void start(uint32_t portspeed);
  void process_cmd(bool);
  void serialEvent(void);
  void clearSerialPort(void);

private:
  char *ftoa(char *, double, int);
  void send_reply(const char *);

  void build_reply(const char, const bool);
  void build_reply(const char, const char *);
  void build_reply(const char, unsigned char);
  void build_reply(const char, float, int);
  void build_reply(const char, int);
  void build_reply(const char, long);
  void build_reply(const char, unsigned long);
  void build_reply(const char, String);

  HardwareSerial &_dev;
  String line;
  const char _SOC = ':';  // start of command
  const char _EOC = '#';  // end of command
  const char _RTOKEN = '$';
};

#endif
#endif
