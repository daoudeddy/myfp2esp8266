// -------------------------------------------------------
// myFP2ESP8266 DUCKDNS CLASS DEFINITION
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// duckdns.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#ifndef _duck_dns_h_
#define _duck_dns_h_

#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_DUCKDNS)

// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------

// -------------------------------------------------------
// CLASS
// -------------------------------------------------------
class DUCK_DNS {
public:
  DUCK_DNS();
  bool start();
  void stop(void);
  void updatenow(void);
  String get_ddns_ip(void);

private:
  bool _loaded = STATE_NOTLOADED;
  const char new_duckdns_ip[48] = "                                               ";
  const char old_duckdns_ip[48] = "                                               ";
  String cntlr_ip;
};

#endif 
#endif
