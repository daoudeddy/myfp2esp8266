//---------------------------------------------------
// myFP2ESP8266 ALPACA SERVER CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// alpaca_server.h
// NodeMCU 1.0 (ESP-12E Module)
//---------------------------------------------------

#ifndef _alpaca_server_h
#define _alpaca_server_h

#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_ALPACASERVER)

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Required for ALPACA DISCOVERY PROTOCOL
#include <WiFiUdp.h>


//---------------------------------------------------
// CLASS
//---------------------------------------------------
class ALPACA_SERVER {
public:
  ALPACA_SERVER(void);

  bool start(void);
  void stop(void);
  void loop(void);

  void get_home(void);
  void get_focusersetup(void);
  void get_notfound(void);
  void file_sys_error(void);

  // ASCOM Methods Common To All Devices
  void put_action(void);
  void put_commandblind(void);
  void put_commandbool(void);
  void put_commandstring(void);

  void put_connect(void);
  void get_connected(void);
  void put_connected(void);
  void get_connecting(void);

  void get_description(void);
  void get_devicestate(void);
  void put_disconnect(void);
  void get_driverinfo(void);
  void get_driverversion(void);
  void get_interfaceversion(void);
  void get_name(void);
  void get_supportedactions(void);

  // FOCUSER SPECIFIC 
  void get_absolute(void);
  void get_maxstep(void);
  void get_maxincrement(void);
  void get_temperature(void);
  void get_position(void);
  void put_halt(void);
  void get_ismoving(void);
  void get_stepsize(void);
  void get_tempcomp(void);
  void put_tempcomp(void);
  void get_tempcompavailable(void);
  void put_move(void);

  // MANAGEMENT API
  void get_man_apiversions(void);
  void get_man_description(void);
  void get_man_configureddevices(void);

  // xhtml
  void get_heap(void);
  void get_sut(void);

private:
  String addclientinfo(String);
  void check_Alpaca_Discovery(void);
  void getURLParameters(void);
  void sendmycontent(String);
  void sendmyheader(void);
  void send_reply(int, String, String);
  void send_setup(void);
  void send_apianswer(String, bool);
  void send_apianswer(String, int);
  void send_apianswer(String, float);
  void send_apianswer(String, long);
  void send_apianswer(String, String);

  bool _loaded = STATE_NOTLOADED;
  bool _discoverystatus = STATUS_STOPPED;
  bool _TempCompState = STATE_DISABLED;
  bool _connecting = false;
  bool _ConnectedState = STATE_NOTCONNECTED;
  char _packetBuffer[255] = { 0 };
  int _ALPACA_ErrorNumber = 0;
  const int _interfaceversion = 3; 
  unsigned int _ALPACA_ServerTransactionID = 0;
  unsigned int _ALPACA_ClientID = 0;
  unsigned int _ALPACA_ClientTransactionID = 0;
  long _pos = 0L;
 
  String _ALPACA_ErrorMessage = "";

  ESP8266WebServer *_alpacaserver;

  const char *_ALPACA_GUID = "CAA62EC5-7A54-490C-9E9A-24B747DC8C9C";
  const char *_ALPACA_DESCRIPTION = "ALPACA Server for myFP2ESP8266 controllers";
  const char *_ALPACA_DRIVERINFO = "myFP2ESP8266 ALPACA SERVER (c) R. Brown. 2020-2025";
  const char *_ALPACA_SERVER_VERSION = "1.3";
  const char *TALPACA_NOTIMPLEMENTED = "AS:not implemented";
  // Discovery
  const char *TALPACA_DISCOVERY = "DISCOVERY ";
  const char *TALPACA_PKTSMALL = "Pkt too small ";
  const char *TALPACA_PKTINVALID = "Pkt invalid ";
  const char *TALPACA_PKTRESPONSE = "Response ";

  const char *TALPACA_ADDCLIENTINFO = "AS:addclientinfo: ";
  const char *TALPACA_ACTION = "AS:action: ";
  const char *TALPACA_CONNECTEDSTATE = "AS:ConnectedState:";
  const char *TALPACA_CONNECT = "AS:Connect: ";
  const char *TALPACA_GETCONNECTED = "AS:get_connected: ";
  const char *TALPACA_PUTCONNECTED = "AS:put_connected: ";
  const char *TALPACA_GETCONNECTING = "AS:Connecting: ";

  const char *TALPACA_GETSETUP = "AS:get /ascomsetup.html ";

  const char *ALPACA_MANAGEMENTINFO = "{ \"ServerName\": \"myFP2ESP8266\", \"Manufacturer\": \"R. Brown\", \"ManufacturerVersion\": \"%VNUM%\", \"Location\": \"New Zealand\" }";
  const char *ALPACA_NOTFOUNDSTR = "<html><head><title>ALPACA Server</title></head><body><p>File system not started</p></body></html>";

  const char *T_GET = "GET ";
  const char *T_POST = "POST ";
  const char *T_PUT = "PUT ";

  const char *TALPACA_GETISMOVING = "AS:get_ismoving ";
  const char *TALPACA_GETTEMPCOMP = "AS:get_tempcomp ";
  const char *TALPACA_GETTEMPCOMPAVAILABLE = "AS:get_tempcomp available ";

  const char *TALPACAFALSE = "false ";
  const char *TALPACATRUE = "true ";
};

#endif

#endif
