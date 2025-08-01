//---------------------------------------------------
// myFP2ESP8266 ALPACA SERVER CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// alpaca_server.cpp
// Optional
// NODEMCU 1.0 (ESP-12E) ESP8266
//---------------------------------------------------


//---------------------------------------------------
// INCLUDES:
//---------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_ALPACASERVER)

#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// Alpaca discovery protocol uses UDP
#include <WiFiUdp.h>


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Alpaca Server messages to 
// be written to Serial port
//#define ALPACASRVR_MsgPrint 1

#ifdef ALPACASRVR_MsgPrint
#define AlpacaMsgPrint(...) Serial.print(__VA_ARGS__)
#define AlpacaMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define AlpacaMsgPrint(...)
#define AlpacaMsgPrintln(...)
#endif


//---------------------------------------------------
// CLASSES
//---------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

#include "alpaca_server.h"
extern ALPACA_SERVER *alpacasrvr;


//---------------------------------------------------
// EXTERNS
//---------------------------------------------------
// all declared in defines.h


//---------------------------------------------------
// ABOUT 
//---------------------------------------------------
// The ALPACA Server has direct access to all 
// Controller Settings, and does NOT require 
// any form of "establish connection" like 
// what exists in an ASCOM Driver opening 
// a Serial connection to a Controller.

// This means that the Connect and Disconnect 
// methods are different and less complicated.
// It is also easier to get Focuser values 
// because there is no need to send a request, 
// wait for a response, then interpret the 
// response to extract the specified setting.


//---------------------------------------------------
// DATA AND DEFINITIONS
//---------------------------------------------------
// Stepmodes are declares in /defines.h

// ASCOM ERROR CODES
#define ALPACA_SUCCESS 0
#define ALPACA_NOTIMPLEMENTED 0x400
#define ALPACA_INVALIDVALUE 0x401
#define ALPACA_VALUENOTSET 0x402
#define ALPACA_NOTCONNECTED 0x407
#define ALPACA_INVALIDOPERATION 0x40B
#define ALPACA_ACTIONNOTIMPLEMENTED 0x40C

// Maximum Args URI request
#define ALPACA_MAXIMUMARGS 10

// ALPACA HTML REQUEST TYPE (GET PUT)
bool alpaca_setup_type;
bool alpaca_test_type;

// instance of ASCOM Discovery via UDP.
WiFiUDP _ALPACADISCOVERYUdp;


//---------------------------------------------------
// ALPACA SERVER 
//---------------------------------------------------
void alpacaget_notfound() {
  alpacasrvr->get_notfound();
}

//---------------------------------------------------
// XHTML SUPPORT
//---------------------------------------------------
void alpacaget_heap(void) {
  alpacasrvr->get_heap();
}

void alpacaget_sut(void) {
  alpacasrvr->get_sut();
}


//---------------------------------------------------
// ASCOM METHODS COMMON TO ALL DEVICES
//---------------------------------------------------
void alpacaget_home() {
  alpacasrvr->get_home();
}

// /setup/v1/focuser/0/setup
// get
void alpacaget_focusersetup() {
  alpaca_setup_type = GeT;
  alpacasrvr->get_focusersetup();
}

// /setup/v1/focuser/0/setup
// post
void alpacaset_focusersetup() {
  alpaca_setup_type = PosT;
  alpacasrvr->get_focusersetup();
}

void alpacaput_action() {
  alpacasrvr->put_action();
}

void alpacaput_commandblind() {
  alpacasrvr->put_commandblind();
}

void alpacaput_commandbool() {
  alpacasrvr->put_commandbool();
}

void alpacaput_commandstring() {
  alpacasrvr->put_commandstring();
}

void alpacaput_connect(void) {
  alpacasrvr->put_connect();
}

void alpacaget_connected() {
  alpacasrvr->get_connected();
}

void alpacaput_connected() {
  alpacasrvr->put_connected();
}

void alpacaget_connecting(void) {
  alpacasrvr->get_connecting();
}

void alpacaget_description() {
  alpacasrvr->get_description();
}

void alpacaget_devicestate() {
  alpacasrvr->get_devicestate();
}

void alpacaput_disconnect(void) {
  alpacasrvr->put_disconnect();
}

void alpacaget_driverinfo() {
  alpacasrvr->get_driverinfo();
}

void alpacaget_driverversion() {
  alpacasrvr->get_driverversion();
}

void alpacaget_interfaceversion() {
  alpacasrvr->get_interfaceversion();
}

void alpacaget_name() {
  alpacasrvr->get_name();
}

void alpacaget_supportedactions() {
  alpacasrvr->get_supportedactions();
}

// MANAGEMENT SPECIFIC
void alpacaget_man_apiversions() {
  alpacasrvr->get_man_apiversions();
}

void alpacaget_man_description() {
  alpacasrvr->get_man_description();
}

void alpacaget_man_configureddevices() {
  alpacasrvr->get_man_configureddevices();
}

// FOCUSER SPECIFIC
void alpacaget_absolute() {
  alpacasrvr->get_absolute();
}

void alpacaget_ismoving() {
  alpacasrvr->get_ismoving();
}

void alpacaget_maxincrement() {
  alpacasrvr->get_maxincrement();
}

void alpacaget_maxstep() {
  alpacasrvr->get_maxstep();
}

void alpacaget_position() {
  alpacasrvr->get_position();
}

void alpacaget_stepsize() {
  alpacasrvr->get_stepsize();
}

void alpacaget_tempcomp() {
  alpacasrvr->get_tempcomp();
}

void alpacaput_tempcomp() {
  alpacasrvr->put_tempcomp();
}

void alpacaget_tempcompavailable() {
  alpacasrvr->get_tempcompavailable();
}

void alpacaget_temperature() {
  alpacasrvr->get_temperature();
}

void alpacaput_halt() {
  alpacasrvr->put_halt();
}

void alpacaput_move() {
  alpacasrvr->put_move();
}


//---------------------------------------------------
// ALPACA REMOTE SERVER CLASS CONSTRUCTOR
//---------------------------------------------------
ALPACA_SERVER::ALPACA_SERVER() {
  _ConnectedState = STATE_NOTCONNECTED;
  _discoverystatus = STATE_DISABLED;
  _TempCompState = STATE_DISABLED;
}


//---------------------------------------------------
// ALPACA SERVER START
// Create and start instance of _alpacaserver
//---------------------------------------------------
bool ALPACA_SERVER::start() {
  AlpacaMsgPrint(T_ALPACASERVER);
  AlpacaMsgPrintln(TUC_START);

  // prevent any attempt to start if already started
  if (_loaded == STATE_LOADED) {
    AlpacaMsgPrintln(TUC_RUNNING);
    return true;
  }

  // check for filesystem loaded
  if (filesystemloaded == STATE_NOTLOADED) {
    AlpacaMsgPrintln(T_FILESYSTEMERROR);
    return false;
  }

  // if Alpaca Server is disabled then return
  if (ControllerData->get_alpacasrvr_enable() == STATE_DISABLED) {
    AlpacaMsgPrintln(T_DISABLED);
    return false;
  }

  // create instance of an ALPACA server
  _alpacaserver = new ESP8266WebServer(ALPACASERVERPORT);

  // check alpaca discovery state: ensure it is running
  if (_discoverystatus == STATUS_STOPPED) {
    AlpacaMsgPrint(TALPACA_DISCOVERY);
    AlpacaMsgPrint(TUC_START);
    _ALPACADISCOVERYUdp.begin(ALPACADISCOVERYPORT);
    _discoverystatus = STATUS_RUNNING;
    AlpacaMsgPrintln(TUC_RUNNING);
  }

  // HTML BROWSER USER INTERFACE
  _alpacaserver->on("/", alpacaget_home);
  _alpacaserver->on("/setup", alpacaget_home);

  // HTML BROWSER USER INTERFACE
  _alpacaserver->on("/setup/v1/focuser/0/setup", HTTP_GET, alpacaget_focusersetup);
  _alpacaserver->on("/setup/v1/focuser/0/setup", HTTP_POST, alpacaset_focusersetup);

  // HANDLE API REQUESTS
  // ASCOM METHODS COMMON TO ALL DEVICES
  // PUT action
  // PUT commandblind
  // PUT commandbool
  // PUT commandstring
  // PUT connect
  // GET connected
  // PUT connected
  // GET connecting
  // GET description
  // GET devicestate
  // PUT disconnect
  // GET driverinfo
  // GET driverversion
  // GET interfaceversion
  // GET name
  // GET supportedactions

  // ASCOM METHODS COMMON TO ALL DEVICES
  _alpacaserver->on("/api/v1/focuser/0/action", HTTP_PUT, alpacaput_action);
  _alpacaserver->on("/api/v1/focuser/0/commandblind", HTTP_PUT, alpacaput_commandblind);
  _alpacaserver->on("/api/v1/focuser/0/commandbool", HTTP_PUT, alpacaput_commandbool);
  _alpacaserver->on("/api/v1/focuser/0/commandstring", HTTP_PUT, alpacaput_commandstring);
  _alpacaserver->on("/api/v1/focuser/0/connect", HTTP_PUT, alpacaput_connect);
  _alpacaserver->on("/api/v1/focuser/0/connected", HTTP_GET, alpacaget_connected);
  _alpacaserver->on("/api/v1/focuser/0/connected", HTTP_PUT, alpacaput_connected);
  _alpacaserver->on("/api/v1/focuser/0/connecting", HTTP_GET, alpacaget_connecting);
  _alpacaserver->on("/api/v1/focuser/0/description", HTTP_GET, alpacaget_description);
  _alpacaserver->on("/api/v1/focuser/0/devicestate", HTTP_GET, alpacaget_devicestate);
  _alpacaserver->on("/api/v1/focuser/0/disconnect", HTTP_PUT, alpacaput_disconnect);
  _alpacaserver->on("/api/v1/focuser/0/driverinfo", HTTP_GET, alpacaget_driverinfo);
  _alpacaserver->on("/api/v1/focuser/0/driverversion", HTTP_GET, alpacaget_driverversion);
  _alpacaserver->on("/api/v1/focuser/0/interfaceversion", HTTP_GET, alpacaget_interfaceversion);
  _alpacaserver->on("/api/v1/focuser/0/name", HTTP_GET, alpacaget_name);
  _alpacaserver->on("/api/v1/focuser/0/supportedactions", HTTP_GET, alpacaget_supportedactions);

  // MANAGEMENT API METHODS
  // GET Apiversions
  // GET Description
  // GET Configureddevices
  _alpacaserver->on("/ascom/management/apiversions", alpacaget_man_apiversions);
  _alpacaserver->on("/ascom/management/v1/description", alpacaget_man_description);
  _alpacaserver->on("/ascom/management/v1/configureddevices", alpacaget_man_configureddevices);
  
  // FOCUSER API METHODS
  // GET Absolute
  // GET IsMoving
  // GET MaxIncrement
  // GET MaxStep
  // GET Position
  // GET Stepsize
  // GET tempcomp
  // PUT tempcomp
  // GET tempcompavailable
  // GET temperature
  // PUT halt
  // PUT move
  _alpacaserver->on("/api/v1/focuser/0/absolute", HTTP_GET, alpacaget_absolute);
  _alpacaserver->on("/api/v1/focuser/0/ismoving", HTTP_GET, alpacaget_ismoving);
  _alpacaserver->on("/api/v1/focuser/0/maxincrement", HTTP_GET, alpacaget_maxincrement);
  _alpacaserver->on("/api/v1/focuser/0/maxstep", HTTP_GET, alpacaget_maxstep);
  _alpacaserver->on("/api/v1/focuser/0/position", HTTP_GET, alpacaget_position);
  _alpacaserver->on("/api/v1/focuser/0/stepsize", HTTP_GET, alpacaget_stepsize);
  _alpacaserver->on("/api/v1/focuser/0/tempcomp", HTTP_GET, alpacaget_tempcomp);
  _alpacaserver->on("/api/v1/focuser/0/tempcomp", HTTP_PUT, alpacaput_tempcomp);
  _alpacaserver->on("/api/v1/focuser/0/tempcompavailable", HTTP_GET, alpacaget_tempcompavailable);
  _alpacaserver->on("/api/v1/focuser/0/temperature", HTTP_GET, alpacaget_temperature);
  _alpacaserver->on("/api/v1/focuser/0/halt", HTTP_PUT, alpacaput_halt);
  _alpacaserver->on("/api/v1/focuser/0/move", HTTP_PUT, alpacaput_move);

  // XHTML FOR HOME AND SETUP WEB PAGES
  _alpacaserver->on("/he", alpacaget_heap);
  _alpacaserver->on("/su", alpacaget_sut);

  // HANDLE URL NOT FOUND 404
  _alpacaserver->onNotFound(alpacaget_notfound);

  // START ALPACA SERVER
  _alpacaserver->begin();
  _loaded = STATE_LOADED;
  alpacasrvr_status = STATUS_RUNNING;
  AlpacaMsgPrintln(TUC_RUNNING);
  return _loaded;
}

//---------------------------------------------------
// STOP ALPACA SERVER
// This stops and deletes the instance of _alpacaserver
//---------------------------------------------------
void ALPACA_SERVER::stop(void) {
  AlpacaMsgPrint(T_ALPACASERVER);
  AlpacaMsgPrintln(TUC_STOP);

  alpacasrvr_status = STATUS_STOPPED; 

  if (_loaded == STATE_LOADED) {
    if (_discoverystatus == STATUS_RUNNING) {
      _ALPACADISCOVERYUdp.stop();
      _discoverystatus = STATUS_STOPPED;
    }
  }
  _alpacaserver->stop();
  _loaded = STATUS_STOPPED;
  _ConnectedState = STATE_NOTCONNECTED;
  _connecting = false;
  _discoverystatus = STATE_DISABLED;
  _TempCompState = STATE_DISABLED;

  if (_alpacaserver) {
    delete _alpacaserver;
  }  
}


//---------------------------------------------------
// CHECK ALPACA SERVER 
//   For New Clients
//   Handle any existing client requests
//---------------------------------------------------
void ALPACA_SERVER::loop() {
  // avoid a crash
  if (_loaded == STATE_NOTLOADED) {
    return;
  }

  // check for any client requests
  if (alpacasrvr_status == STATUS_RUNNING) {
    _alpacaserver->handleClient();
    // check for ASCOM discovery received packets
    if (_discoverystatus == STATUS_RUNNING) {
      check_Alpaca_Discovery();
    }
  }
}


//---------------------------------------------------
// FILE SYSTEM NOT LOADED
// Server internal error
// Check the error message for detailed information
//---------------------------------------------------
void ALPACA_SERVER::file_sys_error(void) {
  // file does not exist
  AlpacaMsgPrintln(T_FILESYSTEMERROR);
  String msg = T_FILESYSTEMERROR;
  _alpacaserver->send(HTML_SERVERERROR, PLAINTEXTPAGETYPE, msg);
}

//---------------------------------------------------
// SEND REPONSE HEADER TO CLIENT
//---------------------------------------------------
void ALPACA_SERVER::sendmyheader() {
  _alpacaserver->client().println("HTTP/1.1 200 OK");
  _alpacaserver->client().println("Content-type:text/html");
  _alpacaserver->client().println();
}

//---------------------------------------------------
// SEND RESPONSE BODY TO CLIENT
//---------------------------------------------------
void ALPACA_SERVER::sendmycontent(String pg) {
  _alpacaserver->client().print(pg);
}

//---------------------------------------------------
// SEND A JSON REPLY TO CLIENT
//---------------------------------------------------
void ALPACA_SERVER::send_reply(int replycode, String contenttype, String jsonstr) {
  _alpacaserver->send(replycode, contenttype, jsonstr);
}


//---------------------------------------------------
// ALPCACA DISCOVERY
//---------------------------------------------------
void ALPACA_SERVER::check_Alpaca_Discovery(void) {
  // (c) Daniel VanNoord
  // https://github.com/DanielVanNoord/AlpacaDiscoveryTests/blob/master/Alpaca8266/Alpaca8266.ino

  // if there's data available, read a packet
  int packetSize = _ALPACADISCOVERYUdp.parsePacket();
  if (packetSize) {
    char ipaddr[16];
    IPAddress remoteIp = _ALPACADISCOVERYUdp.remoteIP();
    snprintf(ipaddr, sizeof(ipaddr), "%i.%i.%i.%i", remoteIp[0], remoteIp[1], remoteIp[2], remoteIp[3]);

    // read the packet into packetBufffer
    int len = _ALPACADISCOVERYUdp.read(_packetBuffer, 255);
    if (len > 0) {
      // Ensure that it is null terminated
      _packetBuffer[len] = 0;
    }
    // No undersized packets allowed
    if (len < 16) {
      AlpacaMsgPrint(TALPACA_DISCOVERY);
      AlpacaMsgPrintln(TALPACA_PKTSMALL);
      return;
    }

    // 0-14 "alpacadiscovery", 15 ASCII Version number of 1
    if (strncmp("alpacadiscovery1", _packetBuffer, 16) != 0) {
      AlpacaMsgPrint(TALPACA_DISCOVERY);
      AlpacaMsgPrintln(TALPACA_PKTINVALID);
      return;
    }

    String strresponse = "{\"AlpacaPort\":" + String(ALPACASERVERPORT) + "}";
    uint8_t aresponse[36] = { 0 };
    len = strresponse.length();
    // copy to response
    for (int i = 0; i < len; i++) {
      aresponse[i] = (uint8_t)strresponse[i];
    }
    _ALPACADISCOVERYUdp.beginPacket(_ALPACADISCOVERYUdp.remoteIP(), _ALPACADISCOVERYUdp.remotePort());
    _ALPACADISCOVERYUdp.write(aresponse, len);
    _ALPACADISCOVERYUdp.endPacket();
    AlpacaMsgPrint(TALPACA_DISCOVERY);
    AlpacaMsgPrint(TALPACA_PKTRESPONSE);
    AlpacaMsgPrintln(strresponse);
  }
}

//---------------------------------------------------
// GETURLPARAMETERS
// get all args client sent as part of request
//---------------------------------------------------
void ALPACA_SERVER::getURLParameters() {
  String str;
  // get server args, translate server args
  // to lowercase, they can be mixed case
  AlpacaMsgPrintln("AS:getURLParameters()");
  for (int i = 0; i < _alpacaserver->args(); i++) {
    if (i >= ALPACA_MAXIMUMARGS) {
      break;
    }
    str = _alpacaserver->argName(i);
    str.toLowerCase();

    // take action based on server args
    AlpacaMsgPrint("Arg[");
    AlpacaMsgPrint(String(i));
    AlpacaMsgPrint("] name= ");
    AlpacaMsgPrintln(str);

    // EXTRACT CLIENTID
    if (str.equals("clientid")) {
      _ALPACA_ClientID = (unsigned int)_alpacaserver->arg(i).toInt();
    }

    if (str.equals("clienttransactionid")) {
      _ALPACA_ClientTransactionID = (unsigned int)_alpacaserver->arg(i).toInt();
    }

    // PUT COMMANDS
    // Check for any PUT requests and extract arg's
    // action
    if (str.equals("action")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      AlpacaMsgPrint(TALPACA_ACTION);
      AlpacaMsgPrintln(strtmp);

      // TODO

    }

    // command blind
    if (str.equals("commandblind")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      AlpacaMsgPrint("commandblind: ");
      AlpacaMsgPrintln(strtmp);

      // { "ClientTransactionID": 1, "ServerTransactionID": 1,
      //   "ErrorNumber": 0, "ErrorMessage": "" }

      // handled by put_commandblind();
    }

    // command bool
    if (str.equals("commandbool")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      AlpacaMsgPrint("commandbool: ");
      AlpacaMsgPrintln(strtmp);

      // { "ClientTransactionID": 1, "ServerTransactionID": 1,
      //   "ErrorNumber": 0, "ErrorMessage": "" }

      // handled by put_commandbool();

    }

    // command string
    if (str.equals("commandstring")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      AlpacaMsgPrint("commandstring: ");
      AlpacaMsgPrintln(strtmp);

      // { "ClientTransactionID": 1, "ServerTransactionID": 1,
      //   "ErrorNumber": 0, "ErrorMessage": "" }

      // handled by put_commandstring();

    }

    // PUT connect
    if (str.equals("connect")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      //AlpacaMsgPrint(TALPACA_CONNECT);
      AlpacaMsgPrint("connect arg=");
      if (strtmp.equals("true")) {
        AlpacaMsgPrintln("true");
        _ConnectedState = true;
      } else {
        AlpacaMsgPrintln("false");
        _ConnectedState = false;
      }
      AlpacaMsgPrint("put connect: ");
      AlpacaMsgPrintln(strtmp);
    }

    // PUT connected
    if (str.equals("connected")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      //AlpacaMsgPrint("put connected: ");
      AlpacaMsgPrint("connected arg=");
      AlpacaMsgPrintln(strtmp);
      if (strtmp.equals("true")) {
        AlpacaMsgPrintln("true");
        _ConnectedState = true;
      } else {
        AlpacaMsgPrintln("false");
        _ConnectedState = false;
      }
    }

    // PUT disconnect
    if (str.equals("disconnect")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      AlpacaMsgPrint(TALPACA_PUTCONNECTED);
      if (strtmp.equals("true") || strtmp.equals("1")) {
        AlpacaMsgPrintln("true");
        _ConnectedState = true;
      } else {
        AlpacaMsgPrintln("false");
        _ConnectedState = false;
      }
      AlpacaMsgPrint("put disconnect: ");
      AlpacaMsgPrintln(strtmp);

      //TODO

    }

    // check arg tempcomp and set flag to requested state
    if (str.equals("tempcomp")) {
      String strtmp = _alpacaserver->arg(i);
      strtmp.toLowerCase();
      // set tempcomp state
      if (tempcomp_available) {
        AlpacaMsgPrint("put tempcomp: ");
        if (strtmp.equals("true")) {
          AlpacaMsgPrintln("true");          
          _TempCompState = STATE_ENABLED;
        } else {
          // "false"
          AlpacaMsgPrintln("false");
          _TempCompState = STATE_DISABLED;
        }
      }
    }

    // halt
    if (str.equals("halt")) {
      // no arg necessary
      AlpacaMsgPrintln("put halt");
      halt_alert = true;
    }

    // move - check arg position
    // The focuser can step between 0 and MaxStep. If an attempt 
    // is made to move the focuser beyond these limits, it will 
    // automatically stop at the limit. 
    if (str.equals("position")) {
      String str1 = _alpacaserver->arg(i);
      AlpacaMsgPrint("position: ");
      AlpacaMsgPrintln(str1);
      _pos = _alpacaserver->arg(i).toInt();
    }
  } // end... for (int i = 0; i < _alpacaserver->args(); i++) {
}

//---------------------------------------------------
// ADDCLIENTINFO
// Adds
// ClientTransactionID, ServerTransactionID
// ErrorNumber, ErrorMessage
//---------------------------------------------------
String ALPACA_SERVER::addclientinfo(String str) {
  String str1 = str;

  // add clienttransactionid
  str1 = str1 + "\"ClientTransactionID\": " + String(_ALPACA_ClientTransactionID) + ", ";
  // add ServerTransactionID
  str1 = str1 + "\"ServerTransactionID\": " + String(_ALPACA_ServerTransactionID) + ", ";
  // add ErrorNumber
  str1 = str1 + "\"ErrorNumber\": " + String(_ALPACA_ErrorNumber) + ", ";
  // add ErrorMessage
  str1 = str1 + "\"ErrorMessage\": \"" + _ALPACA_ErrorMessage + "\" ";
  AlpacaMsgPrint(TALPACA_ADDCLIENTINFO);
  AlpacaMsgPrint("2: ");
  AlpacaMsgPrintln(str1);
  return str1;
}

//---------------------------------------------------
// HTML BROWSER USER INTERFACE
// ALPACA HOME PAGE
// url / or /setup
// The web page describes the overall device
// including name, manufacturer and version number.
// content-type: text/html
//---------------------------------------------------
void ALPACA_SERVER::get_home() {
  String AlpacaPg;
  AlpacaPg.reserve(2624);  // 2568

  AlpacaMsgPrint(T_ALPACASERVER);
  AlpacaMsgPrint(T_GET);
  AlpacaMsgPrintln("/");

  // avoid crash
  if (_loaded == STATE_NOTLOADED) {
    AlpacaMsgPrintln(T_NOTLOADED);
    return;
  }

  // file exists
  File file = LittleFS.open("/alpacahome.html", "r");
  if (!file) {
    AlpacaPg = ALPACA_NOTFOUNDSTR;
    AlpacaMsgPrintln(T_NOTFOUND);
  } else {
    AlpacaPg = file.readString();
    file.close();

    AlpacaPg.replace("%TXC%", TextColor);
    AlpacaPg.replace("%BKC%", BackColor);
    AlpacaPg.replace("%HEC%", HeaderColor);
    AlpacaPg.replace("%DVN%", DeviceName);
    AlpacaPg.replace("%TIC%", TitleColor);
    AlpacaPg.replace("%STC%", SubTitleColor);

    // DeviceID-Name
    AlpacaPg.replace("%ASDN%", DeviceName);
    // GUID CAA62EC5-7A54-490C-9E9A-24B747DC8C9C
    AlpacaPg.replace("%ASGUID%", String(_ALPACA_GUID));
    // Interface-Version 3
    AlpacaPg.replace("%ASIV%", String(_interfaceversion));
    // IP Address
    AlpacaPg.replace("%IPS%", ipStr);
    // Alpaca port number
    AlpacaPg.replace("%ALP%", String(ALPACASERVERPORT));
    // Discovery state
    if (_discoverystatus == STATUS_STOPPED) {
      AlpacaPg.replace("%DIS%", T_STOPPED);
    } else {
      AlpacaPg.replace("%DIS%", T_RUNNING);
    }
    // Discovery Port
    AlpacaPg.replace("%DIP%", String(ALPACADISCOVERYPORT));

    // Manufacturer inline

    // Alpaca Server Version Number
    AlpacaPg.replace("%ASVN%", String(_ALPACA_SERVER_VERSION));

    // Project URL - this is inline in .html file

    // footer
    AlpacaPg.replace("%FTR%", FooterColor);
    AlpacaPg.replace("%NAM%", ControllerData->get_brdname());
    AlpacaPg.replace("%VER%", String(major_version));
    AlpacaPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AlpacaPg.replace("%SUT%", systemuptime);
  }
  _ALPACA_ServerTransactionID++;

  AlpacaMsgPrint("/alpacahome.html ");
  AlpacaMsgPrintln(String(AlpacaPg.length()));

  //_alpacaserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AlpacaPg);
  sendmyheader();
  sendmycontent(AlpacaPg);
}

//---------------------------------------------------
// HTML BROWSER USER INTERFACE
// ALPACA FOCUSER SETUP PAGE
// get_focusersetup()
// url: /setup/v1/focuser/0/setup
//---------------------------------------------------
void ALPACA_SERVER::get_focusersetup() {
  String AlpacaPg;
  String tmp;

  AlpacaPg.reserve(3264);  // 3208

  AlpacaMsgPrintln(TALPACA_GETSETUP);

  if (_loaded == STATE_NOTLOADED) {
    AlpacaMsgPrintln(T_NOTLOADED);
    return;
  }

  if (alpaca_setup_type == PosT) {
    String msg;

    AlpacaMsgPrintln(T_POST);

    // position set NOT A MOVE
    msg = _alpacaserver->arg("setpos");
    if (msg != "") {
      // get new position Value from text field pos
      // set position only: this is NOT a move
      String fp = _alpacaserver->arg("pos");
      if (fp != "") {
        long tp = fp.toInt();
        RangeCheck(&tp, 0L, ControllerData->get_maxstep());
        driverboard->setposition(tp);
        ControllerData->set_fposition(tp);
        ftargetPosition = tp;
        AlpacaMsgPrint("AS:setpos ");
        AlpacaMsgPrintln(String(ftargetPosition));
        goto Get_Handler;
      }
    }

    // maxsteps
    msg = _alpacaserver->arg("setmax");
    if (msg != "") {
      String newmaxpos = _alpacaserver->arg("max");
      if (newmaxpos != "") {
        long tp = (long)newmaxpos.toInt();
        long cpos = driverboard->getposition();
        RangeCheck(&tp, cpos, FOCUSERUPPERLIMIT);
        ControllerData->set_maxstep(tp);
        AlpacaMsgPrint("AS:setmax ");
        AlpacaMsgPrintln(String(tp));
      }
      goto Get_Handler;
    }

    // coilpower
    msg = _alpacaserver->arg("cpst");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_coilpower_enable(STATE_ENABLED);
        driverboard->enablemotor();
      } else if (msg = TLC_OFF) {
        ControllerData->set_coilpower_enable(STATE_DISABLED);
        driverboard->releasemotor();
      }
      AlpacaMsgPrint("AS:setcp ");
      AlpacaMsgPrintln(msg);
      goto Get_Handler;
    }

    // motorspeed
    msg = _alpacaserver->arg("ms");
    if (msg != "") {
      int mspd = 0;
      mspd = msg.toInt();
      RangeCheck(&mspd, SLOW, FAST);
      ControllerData->set_motorspeed(mspd);
      AlpacaMsgPrint("AS:setmotorspd ");
      AlpacaMsgPrintln(msg);
      goto Get_Handler;
    }

    // reverse direction
    msg = _alpacaserver->arg("rdst");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_reverse_enable(STATE_ENABLED);
      } else if (msg = TLC_OFF) {
        ControllerData->set_reverse_enable(STATE_DISABLED);
      }
      AlpacaMsgPrint("AS:setreverse:");
      AlpacaMsgPrintln(msg);
      goto Get_Handler;
    }


    //---------------------------------------------------
    // BASIC RULE FOR SETTING STEPMODE
    // Set driverboard->setstepmode(xx);
    // this sets the physical pins and saves new step mode
    // AND
    // drvbrd also sets ControllerData->set_brdstepmode(xx)
    //---------------------------------------------------
    // if update stepmode
    // 1=Full, 2=Half or fixed

    // Fixed Step Mode Boards
    // WEMOSDRV8825H, WEMOSDRV8825, PRO2EDRV8825, PRO2EL293D28BYJ48, PRO2EL293DNEMA

    // SET IN FIRMWARE Full-Half
    // PRO2EL9110S, PRO2EL293DMINI, PRO2EL298N

    // stepmode
    msg = _alpacaserver->arg("setsm");
    if (msg != "") {
      AlpacaMsgPrint("post:setsm = ");
      AlpacaMsgPrintln(msg);
      if (msg == "half") {
        driverboard->setstepmode(STEP2);
      } else if (msg == "full") {
        driverboard->setstepmode(STEP1);
      }
      // ignore all others because sm is FIXED
      goto Get_Handler;
    }

    // stepsize Value
    msg = _alpacaserver->arg("setss");
    if (msg != "") {
      String st = _alpacaserver->arg("ssv");
      float steps = st.toFloat();
      RangeCheck(&steps, MINIMUMSTEPSIZE, MAXIMUMSTEPSIZE);
      ControllerData->set_stepsize(steps);
      AlpacaMsgPrint("AS:setstepsize:");
      AlpacaMsgPrintln(msg);
      goto Get_Handler;
    }
  }

Get_Handler:
  // Build focuser setup page
  AlpacaMsgPrintln("Alpaca-request /setup/v1/focuser/0/setup ");
  AlpacaMsgPrintln(">> get /alpacasetup.html");

  // Build /setup/v1/focuser/0/setup
  File file = LittleFS.open("/alpacasetup.html", "r");
  if (!file) {
    AlpacaPg = ALPACA_NOTFOUNDSTR;
  } else {
    AlpacaPg = file.readString();
    file.close();

    AlpacaPg.replace("%TXC%", TextColor);
    AlpacaPg.replace("%BKC%", BackColor);
    AlpacaPg.replace("%HEC%", HeaderColor);
    AlpacaPg.replace("%DVN%", DeviceName);
    AlpacaPg.replace("%TIC%", TitleColor);
    AlpacaPg.replace("%STC%", SubTitleColor);

    // position
    AlpacaPg.replace("%posval%", String(driverboard->getposition()));

    // maxsteps
    AlpacaPg.replace("%maxval%", String(ControllerData->get_maxstep()));

    // Coil Power
    if (ControllerData->get_coilpower_enable() == STATE_ENABLED) {
      AlpacaPg.replace("%CPS%", T_ENABLED);
      AlpacaPg.replace("%CPV%", TLC_OFF);
      AlpacaPg.replace("%CPSB%", TUC_DISABLE);
    } else {
      AlpacaPg.replace("%CPS%", T_DISABLED);
      AlpacaPg.replace("%CPV%", TLC_ON);
      AlpacaPg.replace("%CPSB%", TUC_ENABLE);
    }

    // motorspeed
    switch (ControllerData->get_motorspeed()) {
      case 0:
        AlpacaPg.replace("%MSS%", T_CHECKED);
        AlpacaPg.replace("%MSM%", T_SPACE);
        AlpacaPg.replace("%MSF%", T_SPACE);
        break;
      case 1:
        AlpacaPg.replace("%MSS%", T_SPACE);
        AlpacaPg.replace("%MSM%", T_CHECKED);
        AlpacaPg.replace("%MSF%", T_SPACE);
        break;
      case 2:
        AlpacaPg.replace("%MSS%", T_SPACE);
        AlpacaPg.replace("%MSM%", T_SPACE);
        AlpacaPg.replace("%MSF%", T_CHECKED);
        break;
      default:
        AlpacaPg.replace("%MSS%", T_SPACE);
        AlpacaPg.replace("%MSM%", T_SPACE);
        AlpacaPg.replace("%MSF%", T_CHECKED);
        break;
    }

    // Reverse Direction
    if (ControllerData->get_reverse_enable() == STATE_ENABLED) {
      AlpacaPg.replace("%RDS%", T_ENABLED);
      AlpacaPg.replace("%RV%", TLC_OFF);
      AlpacaPg.replace("%RVB%", TUC_DISABLE);
    } else {
      AlpacaPg.replace("%RDS%", T_DISABLED);
      AlpacaPg.replace("%RV%", TLC_ON);
      AlpacaPg.replace("%RVB%", TUC_ENABLE);
    }


    int sm = ControllerData->get_brdstepmode();
    // %SMV% current step mode setting
    // %SMN% hidden  desired setting "full" or "half"
    // %SMB% button text H or F or ---

    // halfstepper, Stepmode1 and Stepmode2
    if ((myboardnumber == PRO2EULN2003) || (myboardnumber == PRO2EL293DNEMA) || (myboardnumber == PRO2EL298N) || (myboardnumber == PRO2EL293DMINI) || (myboardnumber == PRO2EL9110S)) {
      // half stepper boards, Button switches bewteen Step1, Step2
      if (sm == 1) {
        // Current value [%SMV%]
        AlpacaPg.replace("%SMV%", String(sm));
        // SMN
        AlpacaPg.replace("%SMN%", "half");
        // button SMB
        AlpacaPg.replace("%SMB%", "1/2");
      } else {
        // Current value [%SMV%]
        AlpacaPg.replace("%SMV%", String(sm));
        // SMN
        AlpacaPg.replace("%SMN%", "full");
        // button SMB
        AlpacaPg.replace("%SMB%", "FULL");
      }
    } else {
      // all other boards have a fixed step mode
      // hardware jumpers set step mode on the driver board
      // step mode is set to the config.h value of
      //    FIXEDSTEPMODE
      // so to change it, user must edit config.h and
      // upload the new firmware to the Controller

      // Current value [%SMV%]
      AlpacaPg.replace("%SMV%", String(sm));
      // SMN
      AlpacaPg.replace("%SMN%", "fixed");
      // button SMB
      AlpacaPg.replace("%SMB%", "---");
    }

    // step size Value
    AlpacaPg.replace("%ssnum%", String(ControllerData->get_stepsize()));

    // footer
    AlpacaPg.replace("%FTR%", FooterColor);
    AlpacaPg.replace("%NAM%", ControllerData->get_brdname());
    AlpacaPg.replace("%VER%", String(major_version));
    AlpacaPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AlpacaPg.replace("%SUT%", systemuptime);
  }

  // now send the ASCOM setup page
  _ALPACA_ServerTransactionID++;
  AlpacaMsgPrint("/setup/v1/focuser/0/setup ");
  AlpacaMsgPrintln(String(AlpacaPg.length()));
  //_alpacaserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AlpacaPg);
  sendmyheader();
  sendmycontent(AlpacaPg);
}


//---------------------------------------------------
// MANAGEMENT API FUNCTIONS
//---------------------------------------------------

//---------------------------------------------------
// GET_MAN_APIVERSIONS()
// Returns an integer array of supported Alpaca API 
// version numbers.
// http://192.168.2.253:4040/management/v1/apiversions?ClientID=123&ClientTransactionID=1234" -H "accept: application/json"
//---------------------------------------------------
void ALPACA_SERVER::get_man_apiversions() {
  // curl   http://192.168.2.253:4040/management/v1/apiversions?ClientID=123&ClientTransactionID=1234" -H "accept: application/json"

  String jsonretstr = "";

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ \"Value\": [1], ";
  jsonretstr = jsonretstr + "\"ClientTransactionID\": " + String(_ALPACA_ClientTransactionID) + ", ";
  jsonretstr = jsonretstr + "\"ServerTransactionID\": " + String(_ALPACA_ServerTransactionID) + " }";

  AlpacaMsgPrint("get_man_apiversions: ");
  AlpacaMsgPrintln(jsonretstr);

  // send_reply builds http header, sets content type, 
  // and then sends jsonretstr
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// GET_MAN_DESCRIPTION()
// 192.168.2.253:4040/management/v1/description?ClientID=123&ClientTransactionID=1234"
//---------------------------------------------------
void ALPACA_SERVER::get_man_description() {
  // curl   http://192.168.2.253:4040/management/v1/description?ClientID=123&ClientTransactionID=1234" -H "accept: application/json"
  // url   192.168.2.165:4040/ascom/management/v1/description?ClientID=1&ClientTransactionID=2

  // response { "Value": { "ServerName": "string", 
  //                       "Manufacturer": "string",
  //                       "ManufacturerVersion": "string",
  //                       "Location": "string"
  //            },
  //            "ClientTransactionID": 4294967295,
  //            "ServerTransactionID": 4294967295
  //          }

  String jsonretstr = "";

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ \"Value\": " + String(ALPACA_MANAGEMENTINFO) + ", ";
  jsonretstr = jsonretstr + "\"ClientTransactionID\": " + String(_ALPACA_ClientTransactionID) + ", ";
  jsonretstr = jsonretstr + "\"ServerTransactionID\": " + String(_ALPACA_ServerTransactionID) + " }";

  jsonretstr.replace("%VNUM%", _ALPACA_SERVER_VERSION);
  AlpacaMsgPrint("get_man_description ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// GET_MAN_CONFIGUREDDEVICES()
// /management/v1/configureddevices
//
// Returns an array of device description objects, 
// providing unique information for each served 
// device, enabling them to be accessed through 
// the Alpaca Device API.
// content-type: application/json
// { "Value": [{"DeviceName": "Super focuser 1",
// "DeviceType": "Focuser", "DeviceNumber": 0,"UniqueID": 
// "277C652F-2AA9-4E86-A6A6-9230C42876FA"} ], 
//  "ClientTransactionID": 9876,"ServerTransactionID": 54321 }
//---------------------------------------------------
void ALPACA_SERVER::get_man_configureddevices() {
  // curl -X -GET "192.168.2.165:4040/ascom/management/v1/configureddevices?ClientID=1&ClientTransactionID=2" -H "accept: application/json"
  // url 192.168.2.165:4040/ascom/management/v1/configureddevices?ClientID=1&ClientTransactionID=2
  String jsonretstr = "";

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ \"Value\":[ { \"DeviceName\": \"" + String(DeviceName) + "\", \"DeviceType\": \"focuser\", \"DeviceNumber\": 0,\"UniqueID\": \"" + String(_ALPACA_GUID) + "\" } ],";
  jsonretstr = jsonretstr + "\"ClientTransactionID\": " + String(_ALPACA_ClientTransactionID) + ", ";
  jsonretstr = jsonretstr + "\"ServerTransactionID\": " + String(_ALPACA_ServerTransactionID) + " }";

  AlpacaMsgPrint("get_man_configureddevices ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}


//---------------------------------------------------
// ALPACA API
//---------------------------------------------------

//---------------------------------------------------
// SEND RESPONSE TO /API REQUEST
//---------------------------------------------------

//---------------------------------------------------
// SEND RESPONSE TO /API REQUEST: BOOL
//---------------------------------------------------
void ALPACA_SERVER::send_apianswer(String smn, bool smv) {
  String jsonretstr;

  AlpacaMsgPrintln("AS::send_apianswer(String, bool)");
  AlpacaMsgPrint("smn=");
  AlpacaMsgPrintln(smn);
  AlpacaMsgPrint("smv=");
  if (smv) {
    AlpacaMsgPrintln(TALPACATRUE);
  } else {
    AlpacaMsgPrintln(TALPACAFALSE);
  }

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  if (smv) {
    jsonretstr = jsonretstr + ", \"Value\": true }";
  } else {
    jsonretstr = jsonretstr + ", \"Value\": false }";
  }
  AlpacaMsgPrint("send_apianswer (bool): ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// SEND RESPONSE TO /API REQUEST: INT
//---------------------------------------------------
void ALPACA_SERVER::send_apianswer(String smn, int smv) {
  String jsonretstr;

  AlpacaMsgPrintln("AS::send_apianswer(String, int)");
  AlpacaMsgPrint("smv=");
  AlpacaMsgPrintln(String(smv));

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr = jsonretstr + ", \"Value\": " + String(smv) + " }";

  AlpacaMsgPrint("send_apianswer (int): ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// SEND RESPONSE TO /API REQUEST: LONG
//---------------------------------------------------
void ALPACA_SERVER::send_apianswer(String smn, long smv) {
  String jsonretstr;

  AlpacaMsgPrintln("AS::send_apianswer(String, long)");
  AlpacaMsgPrint("smn=");
  AlpacaMsgPrintln(smn);
  AlpacaMsgPrint("smv=");
  AlpacaMsgPrintln(String(smv));

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr = jsonretstr + ", \"Value\": " + String(smv) + " }";

  AlpacaMsgPrint("send_apianswer (long): ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// SEND RESPONSE TO /API REQUEST: FLOAT
//---------------------------------------------------
void ALPACA_SERVER::send_apianswer(String smn, float smv) {
  String jsonretstr;

  AlpacaMsgPrintln("AS::send_apianswer(String, float)");
  AlpacaMsgPrint("smn=");
  AlpacaMsgPrintln(smn);
  AlpacaMsgPrint("smv=");
  AlpacaMsgPrintln(String(smv, 2));

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr = jsonretstr + ", \"Value\": " + String(smv, 2) + " }";

  AlpacaMsgPrint("send_apianswer (float): ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// SEND RESPONSE TO /API REQUEST: STRING
//---------------------------------------------------
void ALPACA_SERVER::send_apianswer(String smn, String smv) {
  String jsonretstr;

  AlpacaMsgPrintln("AS::send_apianswer(String, String)");
  AlpacaMsgPrint("smn=");
  AlpacaMsgPrintln(smn);
  AlpacaMsgPrint("smv=");
  AlpacaMsgPrintln(smv);
  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr = jsonretstr + ", \"Value\": \"" + smv + "\" }";

  AlpacaMsgPrint("send_apianswer (String): ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// void ALPACA_SERVER::put_action() { }
// Invokes the named device-specific action.
// Actions and SupportedActions are a standardised
// means for drivers to extend functionality beyond
// the built-in capabilities of the ASCOM device
// interfaces.
//
// This method should return an error message and
// NotImplementedException error number (0x400)
// if the driver just implements the standard ASCOM
// device methods and has no bespoke, unique, functionality.
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": "string" }
//---------------------------------------------------
void ALPACA_SERVER::put_action() {
  // curl -X PUT "http:192.168.2.253:4040/api/v1/focuser/0/action" -H "accept: application/json" -H "Content-Type: application/x-www-form-urlencoded" -d "ClientID=1&ClientTransactionID=1&Action=string&Parameters=string"

  String jsonretstr;

  AlpacaMsgPrintln("AS::put_action");

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 1036;
  _ALPACA_ErrorMessage = "Action not implemented";
  getURLParameters();

  send_reply(BADREQUESTWEBPAGE, "text/plain", _ALPACA_ErrorMessage);
}

//---------------------------------------------------
// void ALPACA_SERVER::put_commandblind() { }
// Transmits an arbitrary string to the device and
// does not wait for a response
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "" }
//---------------------------------------------------
void ALPACA_SERVER::put_commandblind() {
  String jsonretstr;

  AlpacaMsgPrintln("AS:put_commandblind");

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr) + " }";
  AlpacaMsgPrint("put_commandblind: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// void ALPACA_SERVER::put_commandbool() { }
// Transmits an arbitrary string to the device and
// waits for a bool response
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "",
//   "Value": true }
//---------------------------------------------------
void ALPACA_SERVER::put_commandbool() {
  String jsonretstr;

  AlpacaMsgPrintln("AS:put_commandbool");
  send_apianswer(TALPACA_GETCONNECTED, true);
}

//---------------------------------------------------
// void ALPACA_SERVER::put_commandstring() { }
// Transmits an arbitrary string to the device and
// waits for a string response.
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "",
//   "Value": "string" }
//---------------------------------------------------
void ALPACA_SERVER::put_commandstring() {
  // curl -X 'PUT' 
  // 'https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/focuser/0/commandstring' 
  // -H 'accept: application/json' 
  // -H 'Content-Type: application/x-www-form-urlencoded' 
  // -d 'ClientID=1&ClientTransactionID=1&Command=hello&Raw=true'
  //
  // response
  // { "ClientTransactionID": 1, "ServerTransactionID": 1,
  //   "ErrorNumber": 0, "ErrorMessage": "",
  //   "Value": "string" }

  String jsonretstr;

  AlpacaMsgPrintln("AS::put_commandstring");
  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr += ", \"Value\": \"hello\" }";
  AlpacaMsgPrint("put_commandstring: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}


//---------------------------------------------------
// PUT_CONNECT
// New Method
// HTTP_PUT /api/v1/focuser/0/connect
// Starts an asynchronous connect to the device.
// Platform 7 onward. The Connecting property will be
// true until device initialisation is complete.
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string" }
//---------------------------------------------------
void ALPACA_SERVER::put_connect() {
  // curl -X PUT "http://192.168.2.253:4040/api/v1/focuser/0/connect" -H "accept: application/json" -H "Content-Type: application/x-www-form-urlencoded' -d 'ClientID=1&ClientTransactionID=1"

  AlpacaMsgPrintln("PUT connect");

  String jsonretstr;

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  _connecting = true;

  // do some stuff

  _ConnectedState = true;

  // ok, back, connected, set _connecting false
  _connecting = false;

  jsonretstr = "{ " + addclientinfo(jsonretstr) + " }";
  AlpacaMsgPrint("_ConnectedState=");
  if (_ConnectedState) {
    AlpacaMsgPrintln(TALPACATRUE);
  } else {
    AlpacaMsgPrintln(TALPACAFALSE);
  }
  AlpacaMsgPrint("put_connect: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// GET_CONNECTED()
// HTTP_GET /api/v1/focuser/0/connected
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": true }
//---------------------------------------------------
void ALPACA_SERVER::get_connected() {
  // curl -X GET "http://192.168.2.253:4040/api/v1/focuser/0/connected?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"

  // URL  http://192.168.2.253:4040/api/v1/focuser/0/connected?ClientID=1&ClientTransactionID=1234"

  AlpacaMsgPrint("GET connected ");
  if (_ConnectedState == STATE_CONNECTED) {
    AlpacaMsgPrintln(TALPACATRUE);
    send_apianswer(TALPACA_GETCONNECTED, true);
  } else {
    AlpacaMsgPrintln(TALPACAFALSE);
    send_apianswer(TALPACA_GETCONNECTED, false);
  }
}

//---------------------------------------------------
// PUT_CONNECTED()
// HTTP_PUT /api/v1/focuser/0/connected
// Deprecated in favour of the newer non-blocking connect
// and disconnect methods, with the new connecting
// property serving as the completion property.
// Sets the connected state of the device
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "" }
//---------------------------------------------------
void ALPACA_SERVER::put_connected() {
  // curl -X PUT "192.168.2.253:4040/api/v1/focuser/0/connected" -H "accept: application/json" -H "Content-Type: application/x-www-form-urlencoded" -d "ClientID=77&ClientTransactionID=10&Connected=true"
  
  AlpacaMsgPrintln("PUT connected");

  String jsonretstr = "";

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr) + " }";

  AlpacaMsgPrint("put_connected: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// get_connecting()
// HTTP_GET /api/v1/focuser/0/connecting
// Get the Completion variable for the asynchronous
// Connect() and Disconnect() methods.
// Platform 7 onward. Returns true while the device
// is connecting or disconnecting.
//---------------------------------------------------
void ALPACA_SERVER::get_connecting() {
  // curl -X GET "http://192.168.2.253:4040/api/v1/focuser/0/connecting?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  
  // URL  http://192.168.2.253:4040/api/v1/focuser/0/connecting?ClientID=1&ClientTransactionID=1234
  // Alpaca response
  // { "ClientTransactionID":1234,"ServerTransactionID":1,"ErrorNumber":0,"ErrorMessage":"","Value":true }
  // { "ClientTransactionID":1234,"ServerTransactionID":1,"ErrorNumber":0,"ErrorMessage":"","Value":false }

  AlpacaMsgPrint("AS::get_connecting");
  // return focuser setting connecting
  send_apianswer(TALPACA_GETCONNECTING, _connecting);
}

//---------------------------------------------------
// description()
// HTTP_GET /api/v1/focuser/0/description
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": "string"  }
//---------------------------------------------------
void ALPACA_SERVER::get_description() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/description?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  
  // URL 192.168.2.253:4040/api/v1/focuser/0/description?ClientID=1&ClientTransactionID=1234

  AlpacaMsgPrintln("AS::get_description");
  send_apianswer("description", String(_ALPACA_DESCRIPTION));
}

//---------------------------------------------------
// GET_DEVICESTATE()
// Returns the device's operational state in a single call.
// HTTP_GET /api/v1/focuser/0/devicestate
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": [ { "Name": "string", "Value": "string" } ] }
//
// Platform 7 onward. Devices must return all operational
// values that are definitively known but can omit entries
// where values are unknown. Devices must NOT throw
// exceptions / return errors when values are not known.
// An empty list must be returned if no values are known.
// Client Applications must expect that, from time to time,
// some operational state values may not be present in the
// device response and must be prepared to handle “missing”
// values
//---------------------------------------------------
void ALPACA_SERVER::get_devicestate() {
  // curl -X GET "http://192.168.2.253:4040/api/v1/focuser/0/devicestate?ClientID=123&ClientTransactionID=1234" -H "accept: application/json"
  //
  // { "ClientTransactionID": 1, "ServerTransactionID": 1, "ErrorNumber": 0, "ErrorMessage": "string", "Value": [ { "Name": "string", "Value": "string" } ] }

  // Absolute, Position, IsMoving, maxStep, MaxIncrement, Temp, TempCompState, TempCompAvailable, StepSize
  AlpacaMsgPrintln("AS::get_devicestate");

  String jsonretstr;

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr += ", \"Value\": [ { ";
  jsonretstr += " \"Name\": \"absolute\", \"Value\": \"true\", ";
  jsonretstr += " \"Name\": \"position\", \"Value\": " + String(driverboard->getposition()) + ", ";
  jsonretstr += " \"Name\": \"maxincrement\", \"Value\": " + String(ControllerData->get_maxstep()) + ", ";
  jsonretstr += " \"Name\": \"maxstep\", \"Value\": " + String(ControllerData->get_maxstep()) + ", ";
  jsonretstr += " \"Name\": \"temp\", \"Value\": " + String(temp, 2) + ", ";
  if (tempcomp_state) {
    jsonretstr += " \"Name\": \"tempcomp\", \"Value\": true, ";
  } else {
    jsonretstr += " \"Name\": \"tempcomp\", \"Value\": false, ";
  }
  if (tempcomp_available) {
    jsonretstr += " \"Name\": \"tempcompavailable\", \"Value\": true, ";
  } else {
    jsonretstr += " \"Name\": \"tempcompavailable\", \"Value\": false, ";
  }
  jsonretstr += " \"Name\": \"stepsize\", \"Value\": " + String(ControllerData->get_stepsize(), 2) + " } ] }";

  AlpacaMsgPrint("get_devicestate: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// PUT_DISCONNECT
// New Method
// HTTP_PUT /api/v1/focuser/0/connect
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "" }
//---------------------------------------------------
void ALPACA_SERVER::put_disconnect() {
  // curl -X PUT "http://192.168.2.253:4040/api/v1/focuser/0/connect" -H "accept: application/json" -H "Content-Type: application/x-www-form-urlencoded" -d "ClientID=1&ClientTransactionID=1"

  AlpacaMsgPrintln("AS::put_disconnect");

  String jsonretstr;

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  _connecting = true;

  _ConnectedState = false;

  _connecting = false;

  jsonretstr = "{ " + addclientinfo(jsonretstr) + " }";
  AlpacaMsgPrint("put_disconnect: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// get_driverinfo()
// HTTP_GET /api/v1/focuser/0/driverinfo
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": "string" }
//---------------------------------------------------
void ALPACA_SERVER::get_driverinfo() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/driverinfo?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/driverinfo?ClientID=1&ClientTransactionID=1234
  // myFP2ESP9266 ALPACA SERVER (c) R. Brown. 2020-2025
  AlpacaMsgPrintln("AS::get_driverinfo");
  send_apianswer("driverinfo", String(_ALPACA_DRIVERINFO));
}

//---------------------------------------------------
// get_driverversion()
// HTTP_GET /api/v1/focuser/0/driverversion
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": "string" }
//---------------------------------------------------
void ALPACA_SERVER::get_driverversion() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/driverversion?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL  192.168.2.253:4040/api/v1/focuser/0/driverversion?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_driverversion");

  //String jsonretstr;

  //_ALPACA_ServerTransactionID++;
  //_ALPACA_ErrorNumber = 0;
  //_ALPACA_ErrorMessage = "";
  //getURLParameters();

  //jsonretstr = "{ " + addclientinfo(jsonretstr);
  //jsonretstr += ", \"Value\": \"" + String(major_version) + "\" }";
  //send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);

  AlpacaMsgPrint("driverversion: ");
  AlpacaMsgPrintln(String(major_version));
  send_apianswer("driverversion", String(major_version));  
}

//---------------------------------------------------
// get_interfaceversion()
// HTTP_GET /api/v1/focuser/0/interfaceversion
// /{device_type}/{device_number}/interfaceversion
// { "ClientTransactionID": 1,"ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": 1073741824 }
//---------------------------------------------------
void ALPACA_SERVER::get_interfaceversion() {
  // curl -X GET "http://192.168.2.253:4040/api/v1/focuser/0/interfaceversion?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL http://192.168.2.253:4040/api/v1/focuser/0/interfaceversion?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_interfaceversion");
  send_apianswer("driverinfo", _interfaceversion);
}

//---------------------------------------------------
// get_name()
// HTTP_GET /api/v1/focuser/0/name
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//    "Value": "string" }
//---------------------------------------------------
void ALPACA_SERVER::get_name() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/name?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/name?ClientID=1&ClientTransactionID=1234
  // DeviceName
  AlpacaMsgPrintln("AS::get_name");

  AlpacaMsgPrint("name:");
  AlpacaMsgPrintln(DeviceName);
  send_apianswer("name", String(DeviceName));
}

//---------------------------------------------------
// get_supportedactions()
// HTTP_GET /api/v1/focuser/0/supportedactions
// { "ClientTransactionID": 1, "ServerTransactionID": 1,
//   "ErrorNumber": 0, "ErrorMessage": "string",
//   "Value": [ "string" ] }
//---------------------------------------------------
void ALPACA_SERVER::get_supportedactions() {
  // curl -X GET "http://192.168.2.253:4040/api/v1/focuser/0/supportedactions?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  AlpacaMsgPrintln("AS::get_supportedactions");

  String jsonretstr;

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  // add transaction ID's
  jsonretstr = "{ " + addclientinfo(jsonretstr);
  // add supported actions
  jsonretstr += ", \"Value\": [ \"absolute\", \"ismoving\", \"maxstep\", \"maxincrement\", \"position\", \"stepsize\", \"tempcomp\", \"tempcompavailable\", \"temperature\", \"move\", \"halt\" ] }";
  AlpacaMsgPrint("supportedactions: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// get_absolute()
// HTTP_GET /api/v1/focuser/0/absolute
//---------------------------------------------------
void ALPACA_SERVER::get_absolute() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/absolute?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/absolute?ClientID=1&ClientTransactionID=1234

  // { "ClientTransactionID": 0, "ServerTransactionID": 0, "ErrorNumber": 0, "ErrorMessage": "string", "Value": true }
  AlpacaMsgPrintln("AS::get_absolute");
  send_apianswer("absolute", true);
}

//---------------------------------------------------
// GET_ISMOVING()
// HTTP_GET /api/v1/focuser/0/ismoving
//---------------------------------------------------
void ALPACA_SERVER::get_ismoving() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/ismoving?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/ismoving?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_ismoving");
  if (isMoving) {
    send_apianswer(TALPACA_GETISMOVING, true);
  } else {
    send_apianswer(TALPACA_GETISMOVING, false);
  }
}

//---------------------------------------------------
// get_maxincrement()
// HTTP_GET /api/v1/focuser/0/maxincrement
//---------------------------------------------------
void ALPACA_SERVER::get_maxincrement() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/maxincrement?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"
  // URL  http://192.168.2.253:4040/api/v1/focuser/0/maxincrement?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_maxincrement");
  send_apianswer("maxincrement", ControllerData->get_maxstep());
}

//---------------------------------------------------
// get_maxstep()
// HTTP_GET /api/v1/focuser/0/maxstep
//---------------------------------------------------
void ALPACA_SERVER::get_maxstep() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/maxstep?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"
  // URL  http://192.168.2.253:4040/api/v1/focuser/0/maxstep?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_maxstep");
  send_apianswer("maxstep", ControllerData->get_maxstep());
}

//---------------------------------------------------
// get_position()
// HTTP_GET /api/v1/focuser/0/position
//---------------------------------------------------
void ALPACA_SERVER::get_position() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/position?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/position?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_position");
  send_apianswer("position", driverboard->getposition());
}

//---------------------------------------------------
// get_stepsize()
// HTTP_GET /api/v1/focuser/0/stepsize
//---------------------------------------------------
void ALPACA_SERVER::get_stepsize() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/stepsize?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/stepsize?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_stepsize");

  String jsonretstr;

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr);
  jsonretstr += ", \"Value\": " + String(ControllerData->get_stepsize()) + " }";
  AlpacaMsgPrint("get_stepsize: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// GET_TEMPCOMP()
// HTTP_GET /api/v1/focuser/0/tempcomp
//---------------------------------------------------
void ALPACA_SERVER::get_tempcomp() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/tempcomp?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/tempcomp?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_tempcomp");

  // The state of temperature compensation mode
  // (if available), else always False.
  if (tempcomp_state == STATE_ENABLED) {
    send_apianswer(TALPACA_GETTEMPCOMP, true);
  } else {
    send_apianswer(TALPACA_GETTEMPCOMP, false);
  }
}

//---------------------------------------------------
// PUT_TEMPCOMP()
// HTTP_PUT /api/v1/focuser/0/tempcomp
//---------------------------------------------------
void ALPACA_SERVER::put_tempcomp() {
  // curl -X PUT "192.168.2.253:4040/api/v1/focuser/0/tempcomp" -H "accept: application/json" -H "Content-Type: application/x-www-form-urlencoded" -d "ClientID=&ClientTransactionID=&TempComp=true"
  // URL 192.168.2.253:4040/api/v1/focuser/0/tempcomp
  AlpacaMsgPrintln("AS::put_tempcomp");

  // look for parameter tempcomp=true or
  // tempcomp=false

  String jsonretstr = "";

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  if (tempcomp_available) {
    tempcomp_state = _TempCompState;
    jsonretstr = "{ " + addclientinfo(jsonretstr);
    jsonretstr = jsonretstr + " }";
    AlpacaMsgPrint("put_tempcomp:");
    AlpacaMsgPrintln(jsonretstr);
    send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
  } else {
    // no temp probe present, 
    // tempcomp_available is false, 
    // cannot set tempcomp state
    _ALPACA_ErrorNumber = ALPACA_NOTIMPLEMENTED;
    _ALPACA_ErrorMessage = TALPACA_NOTIMPLEMENTED;
    _TempCompState = false;
    tempcomp_state = false;
    jsonretstr = "{ " + addclientinfo(jsonretstr);
    jsonretstr = jsonretstr + " }";
    AlpacaMsgPrint("AS:put_tempcomp:");
    AlpacaMsgPrintln(jsonretstr);
    // 500
    send_reply(HTML_SERVERERROR, JSONAPPTYPE, jsonretstr);
  }
}

//---------------------------------------------------
// GET_TEMPCOMPAVAILABLE()
// HTTP_GET /api/v1/focuser/0/tempcompavailable
//---------------------------------------------------
void ALPACA_SERVER::get_tempcompavailable() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/tempcompavailable?ClientID=1&ClientTransactionID=1234" -H "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/tempcompavailable?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_tempcompavailable");

  if (tempcomp_available) {
    send_apianswer(TALPACA_GETTEMPCOMPAVAILABLE, true);
  } else {
    send_apianswer(TALPACA_GETTEMPCOMPAVAILABLE, false);
  }
}

//---------------------------------------------------
// get_temperature()
// HTTP_GET /api/v1/focuser/0/temperature
//---------------------------------------------------
void ALPACA_SERVER::get_temperature() {
  // curl -X GET "192.168.2.253:4040/api/v1/focuser/0/temperature?ClientID=1&ClientTransactionID=1234" -H  "accept: application/json"
  // URL 192.168.2.253:4040/api/v1/focuser/0/temperature?ClientID=1&ClientTransactionID=1234
  AlpacaMsgPrintln("AS::get_temperature");
  send_apianswer("name", temp);
}

//---------------------------------------------------
// PUT_HALT()
// HTTP_PUT /api/v1/focuser/0/halt
//---------------------------------------------------
void ALPACA_SERVER::put_halt() {
  // curl -X PUT "192.168.2.253:4040/api/v1/focuser/0/halt" -H "accept: application/json" -H "Content-Type: application/x-www-form-urlencoded" -d "ClientID=1&ClientTransactionID=2"
  // url 192.168.2.253:4040/api/v1/focuser/0/halt
  AlpacaMsgPrintln("AS::put_halt");

  String jsonretstr = "";

  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  getURLParameters();

  jsonretstr = "{ " + addclientinfo(jsonretstr) + " }";

  AlpacaMsgPrint("put_halt: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// PUT_MOVE()
// HTTP_PUT /api/v1/focuser/0/move
//---------------------------------------------------
void ALPACA_SERVER::put_move() {
  // curl -X PUT "192.168.2.253:4040/api/v1/focuser/0/move" -H  "accept: application/json" -H  "Content-Type: application/x-www-form-urlencoded" -d "Position=1000&ClientID=22&ClientTransactionID=33"
  // Response { "ClientTransactionID": 33,"ServerTransactionID": 40,"ErrorNumber": 0,"ErrorMessage": ""}
  // 
  // If an attempt is made to move the focuser 
  // beyond these limits (0 and maxStep), it will 
  // automatically stop at the limit (0 and maxStep). 
  // pos < 0 stop at 0
  // pos > maxStep stop at maxStep
  
  AlpacaMsgPrintln("AS::put_move");

  String jsonretstr = "";
  _ALPACA_ServerTransactionID++;
  _ALPACA_ErrorNumber = 0;
  _ALPACA_ErrorMessage = "";
  // get clientID and clienttransactionID
  // if arg == "move" then get "position"
  getURLParameters();

  // destination is in _pos
  // The focuser can step between 0 and MaxStep. If an attempt 
  // is made to move the focuser beyond these limits, it will 
  // automatically stop at the limit. 

  // RangeCheck(&_pos, 0L, ControllerData->get_maxstep());

  long cpos = driverboard->getposition();
  
  // Check for move below 0
  if (_pos < 0) {
    _pos = 0;
  }

  // check for move above maxStep
  if (_pos > ControllerData->get_maxstep()) {
    _pos = ControllerData->get_maxstep();
  }

  // check if move is to the same current position
  if (_pos == cpos) {
    // ignore move
  }
  else {
    // is a move to another new position
    ftargetPosition = _pos;
  }
  
  AlpacaMsgPrint("TargetPos: ");
  AlpacaMsgPrintln(String(ftargetPosition));

  jsonretstr = "{ " + addclientinfo(jsonretstr) + " }";

  AlpacaMsgPrint("put_move: ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(HTML_WEBPAGE, JSONAPPTYPE, jsonretstr);
}

//---------------------------------------------------
// GET_NOTFOUND()
// The device did not understand which operation was
// being requested or insufficient information was
// given to complete the operation. Check the error
// message for detailed information.
// 400  "text/plain"
//---------------------------------------------------
void ALPACA_SERVER::get_notfound() {
  String message = T_NOTFOUND;
  String jsonretstr = "";

  message += "URI: ";
  message += _alpacaserver->uri();

  // check for favicon.ico request
  String p = _alpacaserver->uri();
  AlpacaMsgPrint("AS:get_notfound ");
  AlpacaMsgPrintln(p);
  if (p == "/favicon.ico") {
    File ifile = LittleFS.open("/favicon.ico", "r");
    AlpacaMsgPrintln("send favicon.ico");
    _alpacaserver->sendHeader("Content-Disposition", "attachment; filename=favicon.ico;");
    _alpacaserver->streamFile(ifile, "image/x-icon");
    AlpacaMsgPrintln("favicon.ico sent");
    return;
  }

  message += "\nMethod: ";
  switch (_alpacaserver->method()) {
    case HTTP_GET: message += "GET"; break;
    case HTTP_POST: message += "POST"; break;
    case HTTP_PUT: message += "PUT"; break;
    case HTTP_DELETE: message += "DELETE"; break;
    default: message += "UNKNOWN_METHOD: " + String(_alpacaserver->method()); break;
  }

  message += "\nArguments: ";
  message += _alpacaserver->args();
  message += "\n";
  for (uint8_t i = 0; i < _alpacaserver->args(); i++) {
    message += " " + _alpacaserver->argName(i) + ": " + _alpacaserver->arg(i) + "\n";
  }

  AlpacaMsgPrint(T_NOTFOUND);
  AlpacaMsgPrintln(message);

  jsonretstr = message;
  AlpacaMsgPrint("get_notfound ");
  AlpacaMsgPrintln(jsonretstr);
  send_reply(BADREQUESTWEBPAGE, PLAINTEXTPAGETYPE, jsonretstr);
}


//---------------------------------------------------
// XHTML SUPPORT
//---------------------------------------------------

//---------------------------------------------------
// GET HEAP AND SEND TO WEB CLIENT
//---------------------------------------------------
void ALPACA_SERVER::get_heap() {
  // Send heap to client ajax request
  _alpacaserver->send(HTML_WEBPAGE, TEXTPAGETYPE, String(ESP.getFreeHeap()));
}

// -------------------------------------------------------
// GET SYSTEM UPDATE TIME SEND TO CLIENT
// -------------------------------------------------------
void ALPACA_SERVER::get_sut() {
  // Send value only to client ajax request
  get_systemuptime();
  _alpacaserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, systemuptime);
}


//---------------------------------------------------
// ALPACA SERVER END
//---------------------------------------------------
#endif
