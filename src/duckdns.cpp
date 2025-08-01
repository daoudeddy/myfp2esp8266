// -------------------------------------------------------
// myFP2ESP8266 DUCKDNS CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// duckdns.cpp
// Optional
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_DUCKDNS)

// https://github.com/ayushsharma82/EasyDDNS
#include <EasyDDNS.h>
#include "duckdns.h"


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable boot messages to be written
// to Serial port
//#define DUCKDNS_MsgPrint 1

#ifdef DUCKDNS_MsgPrint
#define DuckdnsMsgPrint(...) Serial.print(__VA_ARGS__)
#define DuckdnsMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define DuckdnsMsgPrint(...)
#define DuckdnsMsgPrintln(...)
#endif


// -------------------------------------------------------
// EXTERN CLASSES
// -------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;


// -------------------------------------------------------
// EXTERN SETTINGS
// -------------------------------------------------------


// -------------------------------------------------------
// CLASS CONSTRUCTOR
// REFRESH IS HANDLED BY LIB - JUST SET THE REFRESH VALUE
// -------------------------------------------------------
DUCK_DNS::DUCK_DNS() {
  _loaded = STATE_NOTLOADED;
  cntlr_ip.reserve(24);
}


// -------------------------------------------------------
// START DUCKDNS
// -------------------------------------------------------
bool DUCK_DNS::start(void) {
  DuckdnsMsgPrint(T_DUCKDNS);
  DuckdnsMsgPrintln(TUC_START);
  // prevent any attempt to start if server is already started
  if (_loaded == STATE_LOADED) {
    DuckdnsMsgPrintln(T_RUNNING);
    return true;
  }

  // if server is not enabled then return
  if (ControllerData->get_duckdns_enable() == STATE_DISABLED) {
    DuckdnsMsgPrintln(T_DISABLED);
    return false;
  }

  // Set DDNS Service Name to "duckdns"
  EasyDDNS.service("duckdns");

  String domain = ControllerData->get_duckdns_domain();
  int domainlen = domain.length() + 1;
  String token = ControllerData->get_duckdns_token();
  int tokenlen = token.length() + 1;
  char ddd[domainlen];
  char ddt[tokenlen];
  domain.toCharArray(ddd, domainlen);
  token.toCharArray(ddt, tokenlen);
  // Enter ddns Domain & Token | Example - "esp.duckdns.org","1234567"
  EasyDDNS.client(ddd, ddt);
  DuckdnsMsgPrint("domain: ");
  DuckdnsMsgPrintln(ddd);
  DuckdnsMsgPrint("token: ");
  DuckdnsMsgPrintln(ddt);

  // convert refreshrate to milliseconds
  EasyDDNS.update(DUCKDNS_REFRESHRATE * 1000);

  // Get Notified when your IP changes
  EasyDDNS.onUpdate([&](const char* old_duckdns_ip, const char* new_duckdns_ip) {
    DuckdnsMsgPrint("ddns:ipchange:");
    DuckdnsMsgPrintln(new_duckdns_ip);
    old_duckdns_ip = new_duckdns_ip;
    cntlr_ip = String(new_duckdns_ip);
    DuckdnsMsgPrint("old_duckdns_ip:");
    DuckdnsMsgPrintln(old_duckdns_ip);
    DuckdnsMsgPrint("new_duckdns_ip:");
    DuckdnsMsgPrintln(new_duckdns_ip);
  });
  _loaded = STATE_LOADED;
  duckdns_status = STATUS_RUNNING;
  this->updatenow();
  return true;
}


// -------------------------------------------------------
// STOP DUCKDNS
// -------------------------------------------------------
void DUCK_DNS::stop() {
  _loaded = false;
  duckdns_status = STATUS_STOPPED;
  DuckdnsMsgPrint(T_DUCKDNS);
  DuckdnsMsgPrintln(TUC_STOP);  
}


// -------------------------------------------------------
// GET IP ADDRESS FROM EASYDNS SERVER
// -------------------------------------------------------
String DUCK_DNS::get_ddns_ip() {
  return cntlr_ip;
}


// -------------------------------------------------------
// UPDATE DUCKDNS
// -------------------------------------------------------
void DUCK_DNS::updatenow() {
  DuckdnsMsgPrint(T_DUCKDNS);
  DuckdnsMsgPrintln(T_UPDATE);

  if (_loaded == STATE_LOADED) {
    EasyDDNS.update(false);      // use global ip
    DuckdnsMsgPrint("ip:");
    DuckdnsMsgPrintln(this->get_ddns_ip());
  }
  else {
    DuckdnsMsgPrintln(T_ERROR);
  }
}

#endif

