// -------------------------------------------------------
// myFP2ESP8266 TCPIP SERVER CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// tcpipserver.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#if !defined(_tcpip_server_h)
#define _tcpip_server_h

#include <Arduino.h>
#include "config.h"
#if defined(ENABLE_TCPIPSERVER)

#include <ESP8266WiFi.h>
#include "WiFiServer.h"


#define MAXCONNECTIONS 1


// -------------------------------------------------------
// CLASS
// -------------------------------------------------------
class TCPIP_SERVER {
public:
  TCPIP_SERVER();
  void begin();

  bool start(void);  // start the tcp/ip server
  void stop(void);   // stop the tcp/ip server
  bool loop(bool);   // check for new client
                     // and manage existing client

  void not_loaded(void);
  void send_reply(const char *);

  void build_reply(const char, bool);
  void build_reply(const char, const char *);
  void build_reply(const char, unsigned char);
  void build_reply(const char, float, int);
  void build_reply(const char, int);
  void build_reply(const char, long);
  void build_reply(const char, unsigned long);
  void build_reply(const char, String);
  void build_reply_board(const char, const char *);
  void build_reply_config(const char, const char *);
  void build_reply_string(const char, const char *);

  char *ftoa(char *, double, int);

private:
  void process_command();

  WiFiServer *_myserver;
  WiFiClient *_myclient = NULL;
  IPAddress _myclientIPAddress;
  char _clientipStr[16] = "000.000.000.000";
  bool _clientstatus = STATE_NOTCONNECTED;
  bool _loaded = STATE_NOTLOADED;
  bool _status = STATUS_STOPPED;
  bool _pwrdwn_status = STATE_OFF;
  const char _EOC = '#';      // 0x23   '#'  end of command
  const char _RTOKEN = '$';   // start of command
};


#endif
#endif  // #if !defined(_tcpip_server_h)
