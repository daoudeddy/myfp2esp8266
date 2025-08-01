// -------------------------------------------------------
// myFP2ESP8266 WEB SERVER CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// web_server.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#if !defined(_web_server_h_)
#define _web_server_h_

// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_WEBSERVER)

#undef DEBUG_ESP_HTTP_SERVER  // prevent messages from WiFiServer
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// -------------------------------------------------------
// SUPPORT FUNCTIONS
// -------------------------------------------------------


// -------------------------------------------------------
// CLASS
// -------------------------------------------------------
class WEB_SERVER {
  public:
    WEB_SERVER();
    bool start(void);
    void stop(void);
    void loop(void);
    void get_notfound(void);

    void send_redirect(String);
    void get_index(void);
    void get_move(void);
    void post_move(void);
    
    // xhtml
    void get_position(void);
    void get_ismoving(void);
    void get_targetposition(void);
    void get_temperature(void);
    void get_coilpower(void);
    void get_heap(void);
    void get_sut(void);

  private:
    void file_sys_error(void);
    void send_json(String);
    void send_xhtml(String);
    void send_ACAOheader(void);
    void send_myheader(void);
    void send_mycontent(String);
    String get_contenttype(String filename);

    ESP8266WebServer *_web_server;
    bool _loaded = STATE_NOTLOADED;

    const char WST_INDEX[8]     = { "/index " };
    const char WST_MOVE[7]      = { "/move " };
    const char WST_NOTFOUND[11] = { "/notfound " };
};


#endif
#endif
