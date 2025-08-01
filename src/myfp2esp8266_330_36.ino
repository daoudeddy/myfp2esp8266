// -------------------------------------------------------
// myFP2ESP8266 FIRMWARE OFFICIAL RELEASE 330-36
// -------------------------------------------------------
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
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


// SEE about.h for DETAILED COMPILE ENVIRONMENT SETTINGS


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable boot messages to be written
// to Serial port
//#define Boot_MsgPrint 1

#ifdef Boot_MsgPrint
#define BootMsgPrint(...) Serial.print(__VA_ARGS__)
#define BootMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define BootMsgPrint(...)
#define BootMsgPrintln(...)
#endif


// -------------------------------------------------------
// FEATURES
// Firmware for ESP8266 myFP2 WiFi Controllers
// -------------------------------------------------------
// Supports Driver boards DRV8825, ULN2003, L298N, L9110S,
//                        L293DMINI, L293D
// Controller modes       ACCESSPOINT, STATION, LOCALSERIAL
// Display                OLED Text or Graphics
// Temperature Probe      DS18B20


// -------------------------------------------------------
// SPECIAL LICENSE
// -------------------------------------------------------
// This code is released under license. If you copy or
// write new code based on the code in these files, you
// MUST
//   include a link to this project on SourceForge
//   include references to the authors of this code.


// -------------------------------------------------------
// CONTRIBUTIONS
// -------------------------------------------------------
// It is costly to continue development and purchase
// boards and components.
//
// This project provides you with all the information
// you need to undertake the project. A high level of
// documentation and support is provided to assist
// your successful implementation of this project.
//
// I only ask that you consider a small donation in
// terms as a thank you, which can be done via PayPal
// and sent to user rbb1brown@gmail.com  (Robert Brown).


// -------------------------------------------------------
// CONFIG SECTION: YOU-TO-DO
// Edit controller_config.h and specify the controller options
// -------------------------------------------------------
// Specify BOARD AND TARGETCPU (NODEMCU 1.0 ESP-12E MODULE)
// Specify DRVBRD
// Specify FIXEDSTEPMODE
// Specify STEPSPERREVOLUTION
// Specify CONTROLLER MODE, ACCESSPOINT, LOCALSERIAL, STATION
// Specify READWIFICONFIG
// Specify IPADDRESSMODE
// Specify ALPACA, TCPIP, WEB SERVERS
// Specify MDNS and MDNS Name
// Specify TEMPERATURE PROBE
// Specify DISPLAYTYPE
// Specify DUCKDNS


// -------------------------------------------------------
// OVERVIEW: TO PROGRAM THE FIRMWARE
// -------------------------------------------------------
// 1. Set your DRVBRD in config.h so that the
//    correct driver board is used
// 2. Set the FIXEDSTEPMODE in config.h
// 3. Set the STEPSPERREVOLUTION in config.h
// 4. Set the controller mode in config.h
// 5. Enable Display type [if fitted] in config.h
// 6. Set your target CPU to match the CPU for your board
// 7. Compile and upload to your controller
// 8. Upload the sketch data files


// -------------------------------------------------------
// SECTION: INCLUDES: START
// -------------------------------------------------------
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"
#include <FS.h>
#include <LittleFS.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#undef DEBUG_ESP_HTTP_SERVER
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ACCESSPOINT DEFINES
// For accesspoint settings see defines/accesspoint_defines.h
#include "defines/accesspoint_defines.h"

// STATION DEFINES
// For station settings see defines/station_defines.h
#include "defines/station_defines.h"

// WEB PAGE COLORS
// For color settings see defines/pagecolors_defines.h
#include "defines/pagecolors_defines.h"

// -------------------------------------------------------
// SECTION: INCLUDES: END
// -------------------------------------------------------


// -------------------------------------------------------
// WARNING: DO NOT DELETE OR MODIFY BELOW THIS LINE !!
// NOTE: memset() and memcpy() are Arduino methods
// -------------------------------------------------------


// -------------------------------------------------------
// SECTION: CLASSES START
// -------------------------------------------------------

// ALPACA SERVER CLASS
// Dependency: WiFi
// Optional
#if defined(ENABLE_ALPACASERVER)
#include "alpaca_server.h"
ALPACA_SERVER *alpacasrvr;
#endif

// CONTROLLER DATA CLASS
// Dependency: Library ArduinoJSON
#include "controller_data.h"
CONTROLLER_DATA *ControllerData;

// DISPLAY
// Dependency: Text:     Library myOLED
// Dependency: Graphics: Library esp8266-oled-ssd1306
// Optional
#if defined(DISPLAYTYPE)
#if (DISPLAYTYPE == TEXT_OLED12864)
int _display_type = TEXT_OLED12864;
#include "display_text.h"
TEXT_DISPLAY *mydisplay;
#elif (DISPLAYTYPE == LILYGO_OLED6432)
int _display_type = LILYGO_OLED6432;
#include "display_lilygo.h"
LILYGO_DISPLAY *mydisplay;
#elif (DISPLAYTYPE == GRAPHIC_OLED12864)
int _display_type = GRAPHIC_OLED12864;
#include "display_graphics.h"
GRAPHIC_DISPLAY *mydisplay;
#else
// no display defined
int _display_type = DISPLAY_NONE;
#endif
#else
// no display defined
int _display_type = DISPLAY_NONE;
#endif

// DRIVER BOARD CLASS [uses Timer0]
// Dependency: Library ESP8266TimerInterrupt
// Dependency: Library myHalfStepperESP32
#include "driver_board.h"
DRIVER_BOARD *driverboard;

// DUCKDNS [STATION ONLY]
// Dependency: Library EasyDDNS
// For additional duckdns settings see
//     defines/duckdns_defines.h
// Optional
extern char duckdnsdomain[BUFFER48LEN];
extern char duckdnstoken[BUFFER48LEN];
#if defined(ENABLE_DUCKDNS)
#include "duckdns.h"
DUCK_DNS *myDuckDNS;
#endif

// MANAGEMENT SERVER
// Dependency: WiFi
// Dependency: Library ArduinoJSON
// Default Configuration: Included
// For additional management server settings see
//     defines/management_defines.h
#include "management_server.h"
MANAGEMENT_SERVER *mngsrvr;

// LOCAL SERIAL CLASS
// Optional
#if (CONTROLLERMODE == LOCALSERIAL)
#include "myQueue.h"  // Steven de Salas
#include "serial_server.h"
LOCAL_SERIAL *serialsrvr;
#endif

// TCP/IP SERVER
// Dependency: WiFi
// Optional
#if defined(ENABLE_TCPIPSERVER)
#include "tcpip_server.h"
TCPIP_SERVER *tcpipsrvr;
#endif

// TEMPERATURE PROBE
// Dependency: Library DallasTemperature
// Optional
#if defined(ENABLE_TEMPERATUREPROBE)
#include "temp_probe.h"
TEMP_PROBE *tempprobe;
#endif

// WEB SERVER
// Dependency: WiFi
// Optional
#if defined(ENABLE_WEBSERVER)
#include "web_server.h"
WEB_SERVER *websrvr;
#endif

// MDNS
// Dependency: WebServer
// Optional
#if ((CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATION))
#if defined(ENABLE_WEBSERVER)
#if defined(ENABLE_MDNS)
#include <ESP8266mDNS.h>
#endif
#endif
#endif

// -------------------------------------------------------
// SECTION: CLASSES END
// -------------------------------------------------------



// -------------------------------------------------------
// SECTION: FOCUSER CONTROLLER SETTINGS: START
// -------------------------------------------------------
// FIRST BOOT AFTER UPLOAD FIRMWARE
// At 1st reboot after uploading data files the Management
// and TCP/IP Servers are enabled and started. For all
// other options, the options are not enabled and not
// started.
// If an option is then enabled via the Management server,
// the option state will be saved as enabled.
//
// ON SUBSEQUENT BOOTS
// Any option that is found enabled will be initialised
// and started. If the start is successful then the status
// for that option will be set to STATUS_RUNNING
// -------------------------------------------------------


// -------------------------------------------------------
// FOCUSER PROPERTIES
// -------------------------------------------------------
long ftargetPosition;  // target position
// position               // driverboard->getposition()
// maxincrement           // ControllerData->get_maxstep()
// maxstep                // ControllerData->get_maxstep()
bool isMoving;            // motor moving status
float temp;               // the last temperature read
bool tempcomp_available;  // temp compensation
bool tempcomp_state;      // temp compensation ON or OFF
bool connecting;          // Returns true while device is 
                          // connecting or disconnecting.
                          // Completion variable for the 
                          // asynchronous Connect() and 
                          // Disconnect() methods.


// -------------------------------------------------------
// CONTROLLER PROPERTIES START
// -------------------------------------------------------
// state  = STATE_ENABLED or STATE_ENABLED
// status = STATUS_STOPPED or STATUS_RUNNING
bool alpacasrvr_status;  // status of Alpaca Server Class
bool display_status;     // status of Display Class
bool display_found;      // probe detected?
bool duckdns_status;     // status of DuckDNS Class
bool mngsrvr_status;     // status of Management Server Class
bool serialsrvr_status;  // status of Local Serial Class
bool tcpipsrvr_status;   // status of TCPIP Server Class
bool tempprobe_status;   // status of Temp Probe Class
bool websrvr_status;     // status of Web Server Class
bool tempprobe_found;    // probe found

Display_States _display_screen_status;

volatile bool timerSemaphore = false;  // indicates moving state
volatile uint32_t stepcount;           // number of steps to move
volatile bool halt_alert;              // stop a move immediately

IPAddress ESP8266IPAddress;
IPAddress myIP;
char ipStr[BUFFER16LEN] = "000.000.000.000";

bool filesystemloaded;  // filesystem state
bool bootup;            // indicates a reboot
long rssi;              // network signal strength in Station
char systemuptime[BUFFER12LEN];  // ddd:hh:mm

#if (CONTROLLERMODE == LOCALSERIAL)
Queue<String> queue;
#endif

// Cached: are loaded into runtime char []
char project_author[BUFFER32LEN];  // readonly
char project_name[BUFFER32LEN];    // readonly
char major_version[BUFFER8LEN];    // readonly
char minor_version[BUFFER8LEN];    // readonly
char DeviceName[BUFFER12LEN];      // readonly
char MDNSName[BUFFER12LEN];        // readonly

// COLORS WEB PAGES  /defines/pagecolors_defines.h
// Cached: are loaded into runtime char []
char HeaderColor[BUFFER8LEN];
char TitleColor[BUFFER8LEN];
char SubTitleColor[BUFFER8LEN];
char TextColor[BUFFER8LEN];
char BackColor[BUFFER8LEN];
char FooterColor[BUFFER8LEN];

// CAPTURE compile time settings
// Required to initialize the Controller correctly
int mycontrollermode = CONTROLLERMODE;
int myboardnumber = DRVBRD;
int myfixedstepmode = FIXEDSTEPMODE;

#if (CONTROLLERMODE == ACCESSPOINT)
int mystationipaddressmode = ACCESSPOINTIP;
#elif (CONTROLLERMODE == LOCALSERIAL)
int mystationipaddressmode = LOCALSERIALIP;
#elif (CONTROLLERMODE == STATION)
int mystationipaddressmode = IPADDRESSMODE;
#else
int mystationipaddressmode = IPADDRESSMODE
#endif

int mystepsperrev = STEPSPERREVOLUTION;

// ACCESSPOINT SETTINGS /defines/accesspoint_defines.h
extern char myAPSSID[BUFFER64LEN];
extern char myAPPASSWORD[BUFFER64LEN];
extern IPAddress ap_ip;
extern IPAddress ap_dns;
extern IPAddress ap_gateway;
extern IPAddress ap_subnet;

// STATION SETTINGS /defines/station_defines.h
extern char mySSID[BUFFER64LEN];
extern char myPASSWORD[BUFFER64LEN];
extern char mySSID_1[BUFFER64LEN];
extern char myPASSWORD_1[BUFFER64LEN];

// -------------------------------------------------------
// CONTROLLER PROPERTIES END
// -------------------------------------------------------


// -------------------------------------------------------
// SHARED METHODS: START
// -------------------------------------------------------

// -------------------------------------------------------
// CHECK STRING FOR VALID HEX DIGITS 0..9 AND A..F
// -------------------------------------------------------
bool check_str(String str) {
  bool flag = true;
  int len = str.length();
  for (int i = 0; i < len; i++) {
    char ch = str[i];
    if (check_ishexdigit(ch) == false) {
      flag = false;
    }
  }
  return flag;
}

// -------------------------------------------------------
// CHECK IF CHAR IS A HEXDIGIT 0-9, A-F
// -------------------------------------------------------
bool check_ishexdigit(char c) {
  if ((c >= '0') && (c <= '9')) {
    return true;
  }
  if ((c >= 'a') && (c <= 'f')) {
    return true;
  }
  if ((c >= 'A') && (c <= 'F')) {
    return true;
  }
  return false;
}


//---------------------------------------------------
// RANGE CHECK INTEGER
//---------------------------------------------------
void RangeCheck(int *val, int low, int high) {
  if (*val < low) *val = low;
  if (*val > high) *val = high;
}

//---------------------------------------------------
// RANGE CHECK LONG
//---------------------------------------------------
void RangeCheck(long *val, long low, long high) {
  if (*val < low) *val = low;
  if (*val > high) *val = high;
}

//---------------------------------------------------
// RANGE CHECK UNSIGNED LONG
//---------------------------------------------------
void RangeCheck(unsigned long *val, unsigned long low, unsigned long high) {
  if (*val < low) *val = low;
  if (*val > high) *val = high;
}

//---------------------------------------------------
// RANGE CHECK FLOAT
//---------------------------------------------------
void RangeCheck(float *val, float low, float high) {
  if (*val < low) *val = low;
  if (*val > high) *val = high;
}


// -------------------------------------------------------
// START SERIAL SERVER
// Optional
// -------------------------------------------------------
void start_serialserver() {
#if (CONTROLLERMODE == LOCALSERIAL)
  if (serialsrvr_status == STATUS_RUNNING) {
    // already running
    return;
  }
  serialsrvr->start(SERIALPORTSPEED);
#endif
}

// -------------------------------------------------------
// CHECK SERIAL SERVER
// Optional
// -------------------------------------------------------
void check_serialserver(bool pdstatus) {
#if (CONTROLLERMODE == LOCALSERIAL)
  if (serialsrvr_status == STATUS_RUNNING) {
    serialsrvr->serialEvent();
    // check for client requests
    if (queue.count() >= 1) {
      serialsrvr->process_cmd(pdstatus);
    }
  }
#endif
}


// -------------------------------------------------------
// DISPLAY START
// Optional
// NOTE
// display_found is passed to the method display::start
// so that its state can be updated by the method.
// The method returns bool, but there are multiple
// things that can cause a return of false. By
// passing display_found, we now have a better idea of
// what caused the issue of the method returning false
// when an error occurs with display start.
// -------------------------------------------------------
bool display_start() {
  BootMsgPrint(T_DISPLAY);
  BootMsgPrintln(TUC_START);

  if (display_status == STATUS_RUNNING) {
    BootMsgPrintln(T_RUNNING);
    return true;
  }
#if defined(DISPLAYTYPE)
  // start the display if enabled in ControllerData
  switch (_display_type) {
    case DISPLAY_NONE:
      BootMsgPrintln(T_DISPLAYNONE);
      display_found = NOT_FOUND;
      _display_screen_status = DisplayOff;
      return false;
      break;

    case TEXT_OLED12864:
      BootMsgPrintln(T_DISPLAYTEXT);
      break;

    case LILYGO_OLED6432:
      BootMsgPrintln(T_DISPLAYLILYGO);
      break;

    case GRAPHIC_OLED12864:
      BootMsgPrintln(T_DISPLAYGRAPHIC);
      break;

    default:
      BootMsgPrintln(T_DISPLAYNONE);
      display_found = NOT_FOUND;
      _display_screen_status = DisplayOff;
      return false;
      break;
  }

  if (ControllerData->get_display_enable() == STATE_ENABLED) {
    BootMsgPrintln(T_ENABLED);
    display_status = mydisplay->start();
    if (display_status == STATUS_RUNNING) {
      BootMsgPrintln(T_RUNNING);
      BootMsgPrintln(T_ON);
      _display_screen_status = DisplayOn;
      return true;
    } else {
      BootMsgPrintln(T_STOPPED);
      BootMsgPrintln(T_OFF);
      _display_screen_status = DisplayOff;
      return false;
    }
  } else {
    BootMsgPrintln(T_DISABLED);
    _display_screen_status = DisplayOff;
  }
#else
  // DisplayType not defined
  BootMsgPrintln(T_NOTFOUND);
  display_found = NOT_FOUND;
  _display_screen_status = DisplayOff;
#endif
  return false;
}

// -------------------------------------------------------
// DISPLAY STOP
// -------------------------------------------------------
void display_stop(void) {
#if defined(DISPLAYTYPE)
  if (display_status == STATUS_RUNNING) {
    mydisplay->stop();
  }
#endif
  _display_screen_status = DisplayOff;
  display_status = STATUS_STOPPED;
}

// -------------------------------------------------------
// DISPLAY UPDATE PAGE
// -------------------------------------------------------
void display_update_page(long position) {
#if defined(DISPLAYTYPE)
  if (display_status == STATUS_RUNNING) {
    if (_display_screen_status == DisplayOn) {
      mydisplay->update_page(position);
    }
  }
#endif
}


// -------------------------------------------------------
// DISPLAY UPDATE POSITION
// -------------------------------------------------------
void display_update_position(long position) {
#if defined(DISPLAYTYPE)
  if (display_status == STATUS_RUNNING) {
    if (_display_screen_status == DisplayOn) {
      mydisplay->update_position(position);
    }
  }
#endif
}

// -------------------------------------------------------
// DISPLAY OFF
// -------------------------------------------------------
void display_off() {
#if defined(DISPLAYTYPE)
  if (display_status == STATUS_RUNNING) {
    if (_display_screen_status == DisplayOn) {
      mydisplay->clear();
      mydisplay->off();
    }
  }
#endif
  _display_screen_status = DisplayOff;
  display_status = STATUS_STOPPED;
}

// -------------------------------------------------------
// DISPLAY ON
// -------------------------------------------------------
void display_on() {
#if defined(DISPLAYTYPE)
  if (display_status == STATUS_RUNNING) {
    if (_display_screen_status == DisplayOff) {
      mydisplay->on();
      _display_screen_status = DisplayOn;
    }
  }
#endif
}


// -------------------------------------------------------
// DISPLAY CLEAR
// -------------------------------------------------------
void display_clear() {
#if defined(DISPLAYTYPE)
  if (ControllerData->get_display_enable() == STATE_ENABLED) {
    if (display_status == STATUS_RUNNING) {
      mydisplay->clear();
    }
  }
#endif
}


// -------------------------------------------------------
// DUCKDNS SERVICE START
// -------------------------------------------------------
bool duckdns_start() {
#if defined(ENABLE_DUCKDNS)
  BootMsgPrint(T_DUCKDNS);
  BootMsgPrint(TUC_START);
  if (duckdns_status == STATUS_RUNNING) {
    return duckdns_status;
  }
  if (ControllerData->get_duckdns_enable() == STATE_ENABLED) {
    duckdns_status = myDuckDNS->start();
    if (duckdns_status = STATUS_RUNNING) {
      BootMsgPrintln(T_OK);
    } else {
      BootMsgPrintln(T_NOTOK);
    }
    return duckdns_status;
  }
#endif
  return duckdns_status;
}

// -------------------------------------------------------
// DUCKDNS STOP
// -------------------------------------------------------
void duckdns_stop() {
#if defined(ENABLE_DUCKDNS)
  if (duckdns_status == STATUS_RUNNING) {
    myDuckDNS->stop();
  }
#endif
  duckdns_status = STATUS_STOPPED;
  BootMsgPrint(T_DUCKDNS);
  BootMsgPrint(TUC_STOP);
}

// -------------------------------------------------------
// DUCKDNS GET IP
// -------------------------------------------------------
String duckdns_getip() {
#if defined(ENABLE_DUCKDNS)
  if (duckdns_status) {
    return myDuckDNS->get_ddns_ip();
  }
#endif
  return String("000.000.000.000");
}

// -------------------------------------------------------
// DUCKDNS UPDATE
// -------------------------------------------------------
void duckdns_update() {
#if defined(ENABLE_DUCKDNS)
  if (duckdns_status) {
    myDuckDNS->updatenow();
  }
#endif
}


// -------------------------------------------------------
// TEMPERATURE PROBE START
// -------------------------------------------------------
bool start_temperature_probe(void) {
#if defined(ENABLE_TEMPERATUREPROBE)
  BootMsgPrint(T_TEMPPROBE);
  BootMsgPrint(TUC_START);
  if (ControllerData->get_brdtemppin() == -1) {
    BootMsgPrintln(T_NOTSUPPORTED);
    return false;
  }
  // check if already started
  if (tempprobe_status == STATUS_RUNNING) {
    BootMsgPrintln(T_RUNNING);
    return true;
  } else {
    // start probe if enabled
    if (ControllerData->get_tempprobe_enable()) {
      tempprobe_status = tempprobe->start();
      if (tempprobe_status == STATUS_RUNNING) {
        BootMsgPrintln(T_OK);
        return true;
      } else {
        BootMsgPrintln(T_ERRSTART);
        return false;
      }
    } else {
      BootMsgPrintln(T_DISABLED);
    }
  }
#endif
  return false;
}

// -------------------------------------------------------
// TEMPERATURE PROBE STOP
// -------------------------------------------------------
void stop_temperature_probe(void) {
#if defined(ENABLE_TEMPERATUREPROBE)
  if (tempprobe_status == STATUS_RUNNING) {
    tempprobe->stop();
  }
#endif
  tempprobe_status = STATUS_STOPPED;
  BootMsgPrint(T_TEMPPROBE);
  BootMsgPrint(T_STOP);
}

// -------------------------------------------------------
// TEMPERATURE PROBE UPDATE
// -------------------------------------------------------
float update_temperature() {
  static float lasttemp = 20.0f;
#if defined(ENABLE_TEMPERATUREPROBE)
  if (tempprobe_status == STATUS_RUNNING) {
    temp = tempprobe->update();
    lasttemp = temp;
    return temp;
  }
#endif
  return lasttemp;
}


// -------------------------------------------------------
// ALPACA SERVER START
// -------------------------------------------------------
bool start_alpacaserver(void) {
#if defined(ENABLE_ALPACASERVER)
  BootMsgPrint(T_ALPACASERVER);
  BootMsgPrint(TUC_START);
  BootMsgPrint(T_PORT);
  BootMsgPrintln(String(ALPACASERVERPORT));

  alpacasrvr_status = alpacasrvr->start();
  if (alpacasrvr_status == STATUS_RUNNING) {
    BootMsgPrintln(T_OK);
  } else {
    BootMsgPrintln(T_NOTOK);
  }
#endif
  return alpacasrvr_status;
}

// -------------------------------------------------------
// ALPACA SERVER STOP
// -------------------------------------------------------
void stop_alpacaserver(void) {
#if defined(ENABLE_ALPACASERVER)
  alpacasrvr->stop();
#endif
  alpacasrvr_status = STATUS_STOPPED;
  BootMsgPrint(T_ALPACASERVER);
  BootMsgPrint(TUC_STOP);
}

// -------------------------------------------------------
// CHECK ALPACA SERVER
// -------------------------------------------------------
void check_alpaca_server() {
#if defined(ENABLE_ALPACASERVER)
  if (alpacasrvr_status == STATUS_RUNNING) {
    alpacasrvr->loop();
  }
#endif
}


// -------------------------------------------------------
// MANAGEMENT SERVER START
// -------------------------------------------------------
bool start_managementserver(void) {
  BootMsgPrint(T_MNGMNTSERVER);
  BootMsgPrint(TUC_START);
  BootMsgPrint(T_PORT);
  BootMsgPrintln(String(MNGSERVERPORT));  
  if (ControllerData->get_mngsrvr_enable()) {
    mngsrvr_status = mngsrvr->start();
    if (mngsrvr_status = STATUS_RUNNING) {
      BootMsgPrintln(T_OK);
    } else {
      BootMsgPrintln(T_NOTOK);
    }
  }
  return mngsrvr_status;
}

// -------------------------------------------------------
// MANAGEMENT SERVER STOP
// -------------------------------------------------------
void stop_mangementserver(void) {
  mngsrvr->stop();
  mngsrvr_status = STATUS_STOPPED;
  BootMsgPrint(T_MNGMNTSERVER);
  BootMsgPrintln(TUC_STOP);
}

// -------------------------------------------------------
// CHECK MANAGEMENT SERVER
// -------------------------------------------------------
void check_management_server() {
  if (mngsrvr_status == STATUS_RUNNING) {
    mngsrvr->loop();
  }
}


// -------------------------------------------------------
// TCP/IP SERVER START
// -------------------------------------------------------
bool start_tcpipserver() {
#if defined(ENABLE_TCPIPSERVER)
  BootMsgPrint(T_TCPIPSERVER);
  BootMsgPrint(TUC_START);
  BootMsgPrint(T_PORT);
  BootMsgPrintln(String(TCPIPSERVERPORT));   
  tcpipsrvr_status = tcpipsrvr->start();
  if (tcpipsrvr_status = STATUS_RUNNING) {
    BootMsgPrintln(T_OK);
  } else {
    BootMsgPrintln(T_NOTOK);
  }
  return tcpipsrvr_status;
#endif
  return false;
}

// -------------------------------------------------------
// TCP/IP SERVER STOP
// -------------------------------------------------------
void stop_tcpipserver(void) {
#if defined(ENABLE_TCPIPSERVER)
  tcpipsrvr->stop();
#endif
  tcpipsrvr_status = STATUS_STOPPED;
  BootMsgPrint(T_TCPIPSERVER);
  BootMsgPrintln(TUC_STOP);
}

// -------------------------------------------------------
// CHECK TCP/IP SERVER FOR CLIENT REQUESTS
// -------------------------------------------------------
bool check_tcpipsrvr(bool pwrdns) {
#if defined(ENABLE_TCPIPSERVER)
  if (tcpipsrvr_status == STATUS_RUNNING) {
    bool result = tcpipsrvr->loop(pwrdns);
    return result;
  }
#endif
  return false;
}

// -------------------------------------------------------
// WEB SERVER START
// -------------------------------------------------------
bool start_webserver() {
#if defined(ENABLE_WEBSERVER)
  BootMsgPrint(T_WEBSERVER);
  BootMsgPrintln(TUC_START);
  BootMsgPrint(T_PORT);
  BootMsgPrintln(String(WEBSERVERPORT));   
  websrvr_status = websrvr->start();
  if (websrvr_status = STATUS_RUNNING) {
    BootMsgPrintln(T_OK);
  } else {
    BootMsgPrintln(T_NOTOK);
  }
  return websrvr_status;
#endif
  return false;
}

// -------------------------------------------------------
// WEB SERVER STOP
// -------------------------------------------------------
void stop_webserver(void) {
#if defined(ENABLE_WEBSERVER)
  websrvr->stop();
#endif
  websrvr_status = STATUS_STOPPED;
  BootMsgPrint(T_WEBSERVER);
  BootMsgPrintln(TUC_STOP);
}

// -------------------------------------------------------
// CHECK WEB SERVER FOR CLIENT REQUESTS
// -------------------------------------------------------
void check_webserver() {
#if defined(ENABLE_WEBSERVER)
  if (websrvr_status == STATUS_RUNNING) {
    websrvr->loop();
  }
#endif
}


// -------------------------------------------------------
// GET WIFI SIGNAL STRENGTH (IN STATIONMODE)
// -------------------------------------------------------
long getrssi() {
  long strength = WiFi.RSSI();
  return strength;
}

// -------------------------------------------------------
// CALCULATE SYSTEM UPTIME
// Outputs:  String systemuptime as days:hours:minutes
// -------------------------------------------------------
void get_systemuptime() {
  unsigned long elapsedtime = millis();
  int systemuptime_m = int((elapsedtime / (1000 * 60)) % 60);
  int systemuptime_h = int((elapsedtime / (1000 * 60 * 60)) % 24);
  int systemuptime_d = int((elapsedtime / (1000 * 60 * 60 * 24)) % 365);
  snprintf(systemuptime, 10, "%03d:%02d:%02d", systemuptime_d, systemuptime_h, systemuptime_m);
}

// -------------------------------------------------------
// TIMECHECK
// -------------------------------------------------------
byte TimeCheck(unsigned long x, unsigned long Delay) {
  unsigned long y = x + Delay;
  unsigned long z = millis();  // get current time

  if ((x > y) && (x < z))
    return 0;  // overflow y
  if ((x < y) && (x > z))
    return 1;  // overflow z

  return (y < z);  // no or (z and y) overflow
}

// -------------------------------------------------------
// REBOOT CONTROLLER
// -------------------------------------------------------
void software_Reboot(int Reboot_delay) {
  if (isMoving == true) {
    driverboard->end_move();
  }
  // save the focuser settings immediately
  ControllerData->SaveNow(driverboard->getposition(), driverboard->getdirection());
  delay(Reboot_delay);
  ESP.restart();
}

// -------------------------------------------------------
// SECTION: SHARED METHODS: END
// -------------------------------------------------------


// -------------------------------------------------------
// READ WIFICONFIG SSID/PASSWORD FROM FILE
// Inputs: wificonfig.jsn
// Outputs: mstatus
// -------------------------------------------------------
bool readwificonfig(char *xSSID, char *xPASSWORD, char *ySSID, char *yPASSWORD) {
#if defined(READWIFICONFIG)
  const String filename = "/wificonfig.jsn";
  String SSID_1, SSID_2;
  String PASSWORD_1, PASSWORD_2;

  // LittleFS may have failed to start
  if (!filesystemloaded) {
    if (LittleFS.begin()) {
      filesystemloaded = true;
    } else {
      software_Reboot(REBOOTDELAY);
    }
  }

  File f = LittleFS.open(filename, "r");
  if (f) {
    String fdata = f.readString();
    f.close();

    JsonDocument doc;

    DeserializationError jerror = deserializeJson(doc, fdata);
    if (!jerror) {
      // Decode JSON/Extract values
      SSID_1 = doc["mySSID"].as<const char *>();
      PASSWORD_1 = doc["myPASSWORD"].as<const char *>();
      SSID_2 = doc["mySSID_1"].as<const char *>();
      PASSWORD_2 = doc["myPASSWORD_1"].as<const char *>();

      // get first pair
      SSID_1.toCharArray(xSSID, SSID_1.length() + 1);
      PASSWORD_1.toCharArray(xPASSWORD, PASSWORD_1.length() + 1);

      // get second pair
      SSID_2.toCharArray(ySSID, SSID_2.length() + 1);
      PASSWORD_2.toCharArray(yPASSWORD, PASSWORD_2.length() + 1);
      return true;
    }
  }
#endif
  return false;
}

//-------------------------------------------------
// LOAD CACHED VARS, gives quicker access for web
// pages etc
// when any cached variable changes, it is reloaded
//-------------------------------------------------
void load_vars() {
  // focuser settings
  isMoving = false;
  connecting = false;
  temp = DEFAULTLASTTEMP;
  tempcomp_state = STATE_DISABLED;
  tempcomp_available = NOT_AVAILABLE;

  // controller settings
  filesystemloaded = false;
  halt_alert = false;
  timerSemaphore = false;

  // alpaca
  alpacasrvr_status = STATUS_STOPPED;

  // display
  display_found = false;
  display_status = STATUS_STOPPED;
  _display_screen_status = DisplayOff;

  // duckdns
  duckdns_status = STATUS_STOPPED;

  // management server
  mngsrvr_status = STATUS_STOPPED;

  // tcpip server
  tcpipsrvr_status = STATUS_STOPPED;

  // temperature probe
  tempprobe_found = NOT_FOUND;
  tempprobe_status = STATUS_STOPPED;

  // webserver
  websrvr_status = STATUS_STOPPED;

  // CACHE SETTINGS
  // DeviceName and MDNSName handled by ControllerData

  // load web page colors
  String tmp = String(DEFAULT_HEADERCOLOR);
  tmp.toCharArray(HeaderColor, tmp.length() + 1);

  tmp = String(DEFAULT_TITLECOLOR);
  tmp.toCharArray(TitleColor, tmp.length() + 1);

  tmp = String(DEFAULT_SUBTITLECOLOR);
  tmp.toCharArray(SubTitleColor, tmp.length() + 1);

  tmp = String(DEFAULT_TEXTCOLOR);
  tmp.toCharArray(TextColor, tmp.length() + 1);

  tmp = String(DEFAULT_BACKCOLOR);
  tmp.toCharArray(BackColor, tmp.length() + 1);

  tmp = String(DEFAULT_FOOTERCOLOR);
  tmp.toCharArray(FooterColor, tmp.length() + 1);
}


//-------------------------------------------------
// SETUP
//-------------------------------------------------
void setup() {
  bootup = true;
  serialsrvr_status = STATUS_STOPPED;


  //-------------------------------------------------
  // CONTROLLER MODE: LOCALSERIAL START
  //-------------------------------------------------
#if (CONTROLLERMODE == LOCALSERIAL)
  BootMsgPrintln("MODE LOCALSERIAL");
  serialsrvr = new LOCAL_SERIAL(Serial);
  serialsrvr->start(SERIALPORTSPEED);
  delay(1000);
  serialsrvr_status = STATUS_RUNNING;
#else
  Serial.begin(SERIALPORTSPEED);
  while (!Serial) {
    ;  // wait for serial port to start
  }
#endif // #if (CONTROLLERMODE == LOCALSERIAL)
  delay(500);

  BootMsgPrintln("FOCUSER START");


  //-------------------------------------------------
  // READ FOCUSER SETTINGS FROM CONFIG FILES
  //-------------------------------------------------
  BootMsgPrintln("LOAD FOCUSER SETTINGS");
  ControllerData = new CONTROLLER_DATA();


  //-------------------------------------------------
  // INITIALISE VARS
  //-------------------------------------------------
  BootMsgPrintln("INIT VARS");
  load_vars();


  //-------------------------------------------------
  // CONTROLLER MODE: ACCESSPOINT START
  //-------------------------------------------------
#if (CONTROLLERMODE == ACCESSPOINT)
  if (mycontrollermode == ACCESSPOINT) {
    BootMsgPrint(T_ACCESSPOINT);
    BootMsgPrint(TUC_START);
    WiFi.mode(WIFI_AP);
    delay(500);
    WiFi.config(ap_ip, ap_dns, ap_gateway, ap_subnet);
    delay(500);
    WiFi.softAP(myAPSSID, myAPPASSWORD);

#ifdef ENABLE_MDNS
    if (websrvr_status == STATUS_RUNNING) {
      // myfp28266.local
      BootMsgPrint(T_MDNS);
      BootMsgPrintln(TUC_START);
      if (!MDNS.begin(ControllerData->get_mdnsname())) {
        BootMsgPrintln(T_ERROR);
        while (1) {
          delay(1000);
        }
      }
      BootMsgPrint(T_MDNS);
      BootMsgPrintln(ControllerData->get_mdnsname());
      MDNS.addService("http", "tcp", 80);
    }
#endif  //#ifdef ENABLE_MDNS
  }
#endif  //#if (CONTROLLERMODE == ACCESSPOINT)


  //-------------------------------------------------
  // STATION START
  // A station connecting to an existing wifi network
  //-------------------------------------------------
#if (CONTROLLERMODE == STATION)
  if (mycontrollermode == STATION) {
    BootMsgPrint(T_STATION);
    BootMsgPrintln(TUC_START);
#if defined(READWIFICONFIG)
    // READWIFICONFIG, Station Mode only
    // read mySSID, myPASSWORD from file
    // if file exists,
    // otherwise use defaults
    BootMsgPrint("READWIFICONFIG ");
    BootMsgPrintln(TUC_START);
    readwificonfig(mySSID, myPASSWORD, mySSID_1, myPASSWORD_1);
#endif  //#if defined(READWIFICONFIG)

    WiFi.mode(WIFI_STA);
    // if static ip then set this up before starting
    if (mystationipaddressmode == STATICIP) {
      if (!WiFi.config(station_ip, station_gateway, station_subnet, station_dns1, station_dns2)) {
        BootMsgPrintln("ERROR STATION STATIC IP");
      }
      delay(50);
    }
    // attempt to connect to user's wifi
    WiFi.begin(mySSID, myPASSWORD);
    for (int attempts = 0; WiFi.status() != WL_CONNECTED; attempts++) {
      delay(500);
      if (attempts > 9) {
        delay(500);
        break;
      }
    }

    // check if connected after using first set of credentials
    // if not connected try second pair of credentials
    if (WiFi.status() != WL_CONNECTED) {
      // try again with 2nd set of credentials then reboot
      // after 10 failed attempts to log on
      // copy 2nd credentials into mySSID and myPASSWORD
      memset(mySSID, 0, 64);
      memset(myPASSWORD, 0, 64);
      memcpy(mySSID, mySSID_1, (sizeof(mySSID_1) / sizeof(mySSID_1[0])));
      memcpy(myPASSWORD, myPASSWORD_1, (sizeof(myPASSWORD_1) / sizeof(myPASSWORD_1[0])));
      // attempt to start the WiFi with 2nd set alternative credentials
      WiFi.begin(mySSID, myPASSWORD);
      delay(1000);
      for (int attempts = 0; WiFi.status() != WL_CONNECTED; attempts++) {
        delay(500);
        if (attempts > 9) {
          BootMsgPrintln("ERROR CONNECT WiFi. REBOOT");
          software_Reboot(REBOOTDELAY);
        }
      }
    }
  }
#endif  // #if (CONTROLLERMODE == STATION)


#if ((CONTROLLERMODE == STATION) || (CONTROLLERMODE == ACCESSPOINT))
  //-------------------------------------------------
  // CONNECTION DETAILS
  //-------------------------------------------------
  ESP8266IPAddress = WiFi.localIP();
  snprintf(ipStr, sizeof(ipStr), "%i.%i.%i.%i", ESP8266IPAddress[0], ESP8266IPAddress[1], ESP8266IPAddress[2], ESP8266IPAddress[3]);
  BootMsgPrint("IP: ");
  BootMsgPrintln(ipStr);
#endif  // #if ((CONTROLLERMODE == STATION) || (CONTROLLERMODE == ACCESSPOINT))

  BootMsgPrint(T_DEVICENAME);
  BootMsgPrintln(ControllerData->get_devicename());

#if (CONTROLLERMODE == LOCALSERIAL)
  snprintf(ipStr, sizeof(ipStr), "000.000.000.000");
#endif


  //-------------------------------------------------
  // SYSTEM UP TIME START
  // days:hours:minutes
  //-------------------------------------------------
  BootMsgPrintln("SYSTEM UPTIME START");
  get_systemuptime();

  //-------------------------------------------------
  // SETUP DRIVER BOARD
  //-------------------------------------------------
  BootMsgPrint(T_DRIVERBOARD);
  BootMsgPrintln(TUC_START);
  // ensure targetposition will be same as focuser position
  // else after loading driverboard focuser will start
  // moving immediately
  ftargetPosition = ControllerData->get_fposition();
  driverboard = new DRIVER_BOARD();
  driverboard->start(ControllerData->get_fposition());

  // SET COILPOWER
  if (ControllerData->get_coilpower_enable() == STATE_DISABLED) {
    driverboard->releasemotor();
  } else {
    driverboard->enablemotor();
  }

  // ensure driverboard position is same as setupData
  // set focuser position in DriverBoard
  driverboard->setposition(ControllerData->get_fposition());

  // The following options require a network to run
  
#if ((CONTROLLERMODE == STATION) || (CONTROLLERMODE == ACCESSPOINT))
  //-------------------------------------------------
  // ALPACA SERVER
  // Dependancy: WiFi must be running
  // Optional
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
#if defined(ENABLE_ALPACASERVER)
  alpacasrvr = new ALPACA_SERVER();
  if (ControllerData->get_alpacasrvr_enable()) {
    alpacasrvr_status = start_alpacaserver();
  }
#endif  // #if defined(ENABLE_ALPACASERVER)

  //-------------------------------------------------
  // TCP/IP SERVER
  // Dependancy: WiFi must be running
  // Optional
  // Manage via Management Server
  // Default state:  Not Enabled: Stopped
  //-------------------------------------------------
#if defined(ENABLE_TCPIPSERVER)
  tcpipsrvr = new TCPIP_SERVER();
  if (ControllerData->get_tcpipsrvr_enable()) {
    tcpipsrvr_status = start_tcpipserver();
  }
#endif  // #if defined(ENABLE_TCPIPSERVER)

  //-------------------------------------------------
  // WEB SERVER
  // Dependancy: WiFi must be running
  // Optional
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
#ifdef ENABLE_WEBSERVER
  websrvr = new WEB_SERVER();
  if (ControllerData->get_websrvr_enable()) {
    websrvr_status = start_webserver();
#ifdef ENABLE_MDNS
    if (websrvr_status == STATUS_RUNNING) {
      // myfp28266.local
      BootMsgPrint(T_MDNS);
      BootMsgPrint(TUC_START);
      BootMsgPrintln(ControllerData->get_mdnsname());
      if (!MDNS.begin(ControllerData->get_mdnsname())) {
        BootMsgPrintln(T_ERROR);
        while (1) {
          delay(1000);
        }
      }
      MDNS.addService("http", "tcp", 80);
    }
#endif  // #ifdef ENABLE_MDNS
#endif  // #ifdef ENABLE_WEBSERVER
  }

  //-------------------------------------------------
  // MANAGEMENT SERVER
  // Dependancy: WiFi must be running
  // Default state:  Enabled: Started
  //-------------------------------------------------
  mngsrvr = new MANAGEMENT_SERVER();
  // check if management server is to be started
  if (ControllerData->get_mngsrvr_enable() == STATE_ENABLED) {
    mngsrvr_status = start_managementserver();
  }

#if (CONTROLLERMODE == STATION)
  //-------------------------------------------------
  // DUCKDNS
  // Dependancy: WiFi, STATION
  // Optional
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
#if defined(ENABLE_DUCKDNS)
  if (ControllerData->get_duckdns_enable() == STATE_ENABLED) {
    myDuckDNS = new DUCK_DNS;
    duckdns_status = duckdns_start();
  }
#endif  // #if defined(ENABLE_DUCKDNS)
#endif  // #if (CONTROLLERMODE == STATION)
#endif  // #if ((CONTROLLERMODE == STATION) || (CONTROLLERMODE == ACCESSPOINT))


  //-------------------------------------------------
  // I2C
  // Standard ESP8266 Node MCU 12E Module
  // Data = GPIO4, Clock = GPIO5
  // Set Clock to 100kHz
  //-------------------------------------------------
  Wire.begin(I2CDATAPIN, I2CCLKPIN);
  Wire.setClock(100000L);


  //-------------------------------------------------
  // DISPLAY : Managed via Management Server
  // Dependancy: Wire
  // Optional
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
#if defined(DISPLAYTYPE)
#if (DISPLAYTYPE == TEXT_OLED12864)
  BootMsgPrint(T_DISPLAYTEXT);
  BootMsgPrint(TUC_START);
  BootMsgPrint(T_ADDRESS);
  BootMsgPrintln(OLED_ADDR, HEX);
  mydisplay = new TEXT_DISPLAY(OLED_ADDR);
#endif  // #if (DISPLAYTYPE == TEXT_OLED12864)
#if (DISPLAYTYPE == GRAPHIC_OLED12864)
  BootMsgPrint(T_DISPLAYGRAPHIC);
  BootMsgPrint(TUC_START);
  BootMsgPrint(T_ADDRESS);
  BootMsgPrintln(OLED_ADDR, HEX);
  mydisplay = new GRAPHIC_DISPLAY(OLED_ADDR);
#endif  // #if (DISPLAYTYPE == GRAPHIC_OLED12864)
#if (DISPLAYTYPE == DISPLAY_NONE)
  BootMsgPrintln(_DISPLAYNONE);
#endif
  if (mydisplay) {
    if (ControllerData->get_display_enable()) {
      display_status = display_start();
      if (display_status == STATUS_STOPPED) {
        BootMsgPrintln(T_ERROR);
      }
    }
  } else {
    BootMsgPrintln("DISPLAY NOT CREATED");
  }
#endif  // #if defined(DISPLAYTYPE)


  //-------------------------------------------------
  // TEMPERATURE PROBE
  // Optional
  // Manage via Management Server
  // Default state:  NotEnabled: Stopped
  //-------------------------------------------------
#if defined(ENABLE_TEMPERATUREPROBE)
  if (ControllerData->get_tempprobe_enable()) {
    tempprobe = new TEMP_PROBE(ControllerData->get_brdtemppin());
    tempprobe_status = start_temperature_probe();
    if (tempprobe_status) {
      tempprobe_found = FOUND;
      BootMsgPrintln("Read temp");
      temp = tempprobe->read();
      BootMsgPrint("Temp = ");
      BootMsgPrintln(temp);
      if (ControllerData->get_tempcomp_onload() == STATE_ENABLED) {
        // enable temp comp
        tempcomp_state = STATE_ENABLED;
      }
    }
  }
#endif  // #if defined(ENABLE_TEMPERATUREPROBE)

#if ((CONTROLLERMODE == STATION) || (CONTROLLERMODE == ACCESSPOINT))
  // Turn off WiFi power save mode
  WiFi.setSleep(false);

  // WiFi reconnect if connection is lost
#if (CONTROLLERMODE == STATION)
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
#endif  // #if (CONTROLLERMODE == STATION)
#endif  // #if ((CONTROLLERMODE == STATION) || (CONTROLLERMODE == ACCESSPOINT))

  bootup = false;
  BootMsgPrintln("READY");
}


void loop() {
  static Focuser_States FocuserState = State_Idle;
  static uint32_t TimeStampDelayAfterMove = 0;
  static uint32_t TimeStampDisplay = millis();
  static uint32_t TimeStampEndMove = millis();
  static uint32_t TimeStampPowerDown = millis();
  static uint32_t TimeStampTemperature = millis();
  static uint32_t TimeStampDuckDNS = millis();
  static unsigned long updatetimestamp = 0;
  static bool PowerDown_Status = true;

  static uint32_t backlash_count = 0;
  static bool DirOfTravel = (bool)ControllerData->get_focuserdirection();
  static uint32_t steps = 0;
  static int damcounter = 0;
  static uint8_t updatecount = 0;

  // handle all Server loop() checks, for new client or client requests

  if (serialsrvr_status == STATUS_RUNNING) {
    check_serialserver(PowerDown_Status);
  }

#if ((CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATION))

  // check ALPACA server (4040) for web client requests
  if (alpacasrvr_status == STATUS_RUNNING) {
    check_alpaca_server();
  }

  // check Management Server (6060) for web client requests
  if (mngsrvr_status == STATUS_RUNNING) {
    check_management_server();
  }

  // check TCP/IP Server (2020) for client requests
  if (tcpipsrvr_status == STATUS_RUNNING) {
    if (check_tcpipsrvr(PowerDown_Status) == false) {
      // is a range of possible causes
      // but mainly no client connected;
      // which is NOT an error
    }
  }

  // check Web Server (80) for client requests
  if (websrvr_status == STATUS_RUNNING) {
    check_webserver();
  }

  // check DuckDNS
  if (duckdns_status == STATUS_RUNNING) {
    updatetimestamp = DUCKDNS_REFRESHRATE * 1000;
    if (TimeCheck(TimeStampDuckDNS, updatetimestamp)) {
      TimeStampDuckDNS = millis();
      duckdns_update();
    }
  }

#ifdef ENABLE_MDNS
  MDNS.update();
#endif

#endif  // #if ((CONTROLLERMODE == ACCESSPOINT) || (CONTROLLERMODE == STATION))


  //-------------------------------------------------
  // FOCUSER STATE ENGINE
  //-------------------------------------------------
  switch (FocuserState) {
      //-------------------------------------------------
      // State_Idle
      // If current position NOT EQUAL to Target position
      //     Ismoving true
      //     goto next state
      // Position = Target, idle
      //     Save config files
      //     Display refresh
      //     Temperature refresh
      //-------------------------------------------------
    case State_Idle:
      if (driverboard->getposition() != ftargetPosition) {
        BootMsgPrint("State_Idle:positon != target: ");
        BootMsgPrintln(ftargetPosition);
        // prepare to move focuser
        BootMsgPrintln("State_Idle:powerdown=false:displayon");
        PowerDown_Status = false;
        display_on();
        isMoving = true;
        FocuserState = State_InitMove;
        BootMsgPrint("State_Idle:initmove to ");
        BootMsgPrintln(ftargetPosition);
      } else {
        // focuser is stationary
        // save config
        // timestamp is in SaveConfiguration()
        if (ControllerData->SaveConfiguration(driverboard->getposition(), DirOfTravel)) {
          BootMsgPrintln("State_Idle:config saved");
        }

        updatetimestamp = ControllerData->get_powerdown_time() * 1000;
        if (TimeCheck(TimeStampPowerDown, updatetimestamp)) {
          TimeStampPowerDown = millis();
          // check powerdown enable state
          if (ControllerData->get_powerdown_enable()) {
            BootMsgPrintln("State_Idle:powerdown=true:displayoff");
            PowerDown_Status = true;
            display_off();
          }
        };

        // update display
        if (_display_screen_status == DisplayOn) {
          updatetimestamp = DISPLAYPAGETIME * 1000;
          if (TimeCheck(TimeStampDisplay, updatetimestamp)) {
            TimeStampDisplay = millis();
            //BootMsgPrint("State_Idle:updatedisplay:displaypagetime: ");
            //BootMsgPrintln(updatetimestamp);
            //BootMsgPrintln("State_Idle:updatedisplay:times up: call display_update_page");
            display_update_page(driverboard->getposition());
          }
        }

        // update temperature
        if (tempprobe_status == STATUS_RUNNING) {
          if (ControllerData->get_tempprobe_enable() == STATE_ENABLED) {
            if (TimeCheck(TimeStampTemperature, DEFAULTTEMPREFRESHTIME)) {
              TimeStampTemperature = millis();
              // read temp AND check Temperature Compensation
              temp = update_temperature();
            }
          }
        }
      }
      break;

      //-------------------------------------------------
      // STATE_INITMOVE
      // Determine move direction
      // Enable motor
      // Determine Backlash direction and steps
      // Calculate number of motor steps to move from
      //    current position to target position
      // Start motor timer to control stepping of motor
      //-------------------------------------------------
    case State_InitMove:
      isMoving = true;
      backlash_count = 0;

      DirOfTravel = (ftargetPosition > driverboard->getposition()) ? moving_out : moving_in;
      BootMsgPrintln("State_InitMove");
      BootMsgPrintln("Enable Motor");
      driverboard->enablemotor();

      // check if move direction has changed
      if (ControllerData->get_focuserdirection() != DirOfTravel) {
        // yes, move is in opposite direction
        ControllerData->set_focuserdirection(DirOfTravel);
      }

      // check for backlash settings
      if (DirOfTravel == moving_in) {
        BootMsgPrintln("State_InitMove:Move In");
        // get backlash in count
        backlash_count = (ControllerData->get_reverse_enable() == 0) ? ControllerData->get_backlashsteps_in() : ControllerData->get_backlashsteps_out();
      } else {
        BootMsgPrintln("State_InitMove:Move Out");
        // get backlash out count
        backlash_count = (ControllerData->get_reverse_enable() == 0) ? ControllerData->get_backlashsteps_out() : ControllerData->get_backlashsteps_in();
      }

      // handle bl counter on the graphics display
      if (_display_type == GRAPHIC_OLED12864) {
        uint32_t sm = ControllerData->get_brdstepmode();
        uint32_t bl = backlash_count * sm;
        if (DirOfTravel == moving_out) {
          // Trip to tuning point should be a fullstep position
          backlash_count = bl + sm - ((ftargetPosition + bl) % sm);
        } else {
          // Trip to tuning point should be a fullstep position
          backlash_count = bl + sm + ((ftargetPosition - bl) % sm);
        }
        BootMsgPrint("State_InitMove:bl steps ");
        BootMsgPrintln(backlash_count);
      }

      // calculate number of steps to move motor
      // if target pos > current pos then
      //    steps = target pos - current pos
      // if target pos < current pos then
      //    steps = current pos - target pos
      steps = (ftargetPosition > driverboard->getposition()) ? ftargetPosition - driverboard->getposition() : driverboard->getposition() - ftargetPosition;
      BootMsgPrint("State_InitMove:Steps to move: ");
      BootMsgPrintln(steps);

      // NOTE
      // Cannot combine backlash steps to steps because that
      // alters position.

      // Backlash move SHOULD NOT alter focuser position as
      // focuser is not actually moving.

      // Backlash is taking up the slack in the stepper
      // motor/focuser mechanism, so position is not
      // actually changing

      if (backlash_count) {
        BootMsgPrintln("State_InitMove:backlash:yes");
        BootMsgPrintln("State_InitMove > StateBacklash");
        FocuserState = State_Backlash;
      } else {
        BootMsgPrintln("backlash:no");
        // if target pos > current pos then
        //    steps = target pos - current pos
        // if target pos < current pos then
        //    steps = current pos - target pos
        // initmove enables coil power and starts the motor timer
        BootMsgPrintln("State_InitMove:Enable move timer");
        driverboard->initmove(DirOfTravel, steps);
        BootMsgPrintln("State_InitMove > StateMoving");
        FocuserState = State_Moving;
      }
      break;


      //-------------------------------------------------
      // STATE_BACKLASH
      // Move motor to take up Backlash
      // Do NOT adjust focuser position
      // When Backlash is complete, start Motor Timer to
      // move the motor.
      //-------------------------------------------------
    case State_Backlash:
      BootMsgPrintln("State_Backlash");

      // apply backlash
      while (backlash_count != 0) {
        // take 1 step and do not adjust position
        driverboard->movemotor(DirOfTravel, false);
        // ensure delay between steps
        delayMicroseconds(ControllerData->get_brdmsdelay());
        backlash_count--;
      }

      // backlash count is 0, backlash move done, goto moving now
      BootMsgPrintln("State_Backlash:BL DONE");
      BootMsgPrintln("State_Backlash:driverboard->initmove");
      driverboard->initmove(DirOfTravel, steps);
      BootMsgPrintln("State_Backlash > StateMoving");
      FocuserState = State_Moving;
      break;


      //-------------------------------------------------
      // STATE_MOVING
      // Motor is stepped by Motor Timer
      // timerSemaphore TRUE indicated move has ended
      //    disable motor timer
      // timerSemaphore FALSE means motor is still moving
      //    Check if a HALT command was received
      //        Stop the move (disable Motor timer)
      //        Set Target position to current position
      //    If still moving
      //        Update position on display
      //-------------------------------------------------
    case State_Moving:
      if (timerSemaphore == true) {
        BootMsgPrintln("State_Moving:MOVE DONE");
        // disable interrupt timer that moves motor
        driverboard->end_move();
        TimeStampDelayAfterMove = millis();
        BootMsgPrintln("State_Moving > StateDelayAfterMove");
        FocuserState = State_DelayAfterMove;
      } else {
        // timerSemaphore false
        // still moving, check for halt
        // halt_alert set by tcpip_server.cpp
        // web_server.cpp and serial_comms.cpp
        //Serial.print(".");
        if (halt_alert) {
          BootMsgPrintln("State_Moving:halt_alert:true");
          BootMsgPrintln("State_Moving:halt_alert::resetting to false");
          halt_alert = false;
          BootMsgPrint("halt_alert = ");
          BootMsgPrintln(halt_alert);
          BootMsgPrintln("State_Moving:driverboard->end_move()");
          driverboard->end_move();
          // check for < 0
          if (driverboard->getposition() < 0) {
            driverboard->setposition(0);
          }
          ftargetPosition = driverboard->getposition();
          ControllerData->set_fposition(driverboard->getposition());

          //BootMsgPrint("State_Moving:Position: ");
          //BootMsgPrintln(driverboard->getposition());

          // no longer need to keep track of steps here
          // or halt because driverboard updates position
          // on every move
          TimeStampDelayAfterMove = millis();
          FocuserState = State_DelayAfterMove;
        }  // if ( halt_alert )

        // if the update position on display when moving
        // is enabled, then update the display
        if (ControllerData->get_display_updateonmove() == 1) {
          // if display running then update displayed position
          if (display_status == STATUS_RUNNING) {
            updatecount++;
            //  update every 15th move to avoid overhead
            if (updatecount > DISPLAYUPDATEONMOVE) {
              updatecount = 0;
              BootMsgPrintln("State_Moving:update display");
              display_update_position(driverboard->getposition());
            }
          }
        }  // if ( get_display_updateonmove() == 1)
      }    // if (timerSemaphore == true)
      break;


      //-------------------------------------------------
      // STATE_DELAYAFTERMOVE
      // Settling time to allow vibrations of move to stop
      // Helps prevent blurring of short camera exposures
      //-------------------------------------------------
    case State_DelayAfterMove:
      BootMsgPrintln("State_DelayAfterMove");
      if (ControllerData->get_delayaftermove_time() > 0) {
        updatetimestamp = ControllerData->get_delayaftermove_time() * 1000;
        if (TimeCheck(TimeStampDelayAfterMove, updatetimestamp)) {
          damcounter = 0;
          FocuserState = State_EndMove;
        }
        // keep looping around till timecheck for delayaftermove succeeds
        // BUT ensure there is a way to exit state
        // if delayaftermove fails to timeout
        // a loop cycle is ~1-4ms
        damcounter++;
        if (damcounter > 255) {
          damcounter = 0;
          FocuserState = State_EndMove;
        }
      } else {
        // delay after move is disabled, so go to
        // next focuser state
        BootMsgPrintln("State_DelayAfterMove:DONE");
        BootMsgPrintln("State_DelayAfterMove > StateEndMove");
        FocuserState = State_EndMove;
      }
      break;


      //-------------------------------------------------
      // STATE_ENDMOVE
      //-------------------------------------------------
    case State_EndMove:
      isMoving = false;
      TimeStampEndMove = millis();
      TimeStampDisplay = millis();
      TimeStampPowerDown = millis();
      // end of move, check and disable coil power if required
      if (ControllerData->get_coilpower_enable() == STATE_DISABLED) {
        BootMsgPrintln("lp:CoilPower off");
        driverboard->releasemotor();
      }
      BootMsgPrintln("State_EndMove > State_Idle");
      FocuserState = State_Idle;
      break;

    default:  // DONE
      BootMsgPrintln("lp:ERR:wrong State");
      BootMsgPrintln("lp:go State_Idle");
      FocuserState = State_Idle;
      break;
  }
}
