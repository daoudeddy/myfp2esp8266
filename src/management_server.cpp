// -------------------------------------------------------
// myFP2ESP8266 MANAGEMENT SERVER CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// management_server.cpp
// Default Configuration
// NodeMCU 1.0 (ESP-12E Module) ESP8266
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDE FILES
// -------------------------------------------------------
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Management Server messages 
// to be written to Serial port
//#define MANGEMENTSERVER_MsgPrint 1

#ifdef MANGEMENTSERVER_MsgPrint
#define MngSrvrMsgPrint(...) Serial.print(__VA_ARGS__)
#define MngSrvrMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define MngSrvrMsgPrint(...)
#define MngSrvrMsgPrintln(...)
#endif

// -------------------------------------------------------
// EXTERN CLASSES
// -------------------------------------------------------
// Controller
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// Driver Board
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

// Management Server defines
#include "defines/management_defines.h"
#include "management_server.h"
extern MANAGEMENT_SERVER *mngsrvr;


// -------------------------------------------------------
// EXTERN METHODS
// -------------------------------------------------------
// Alpaca Server
extern bool start_alpacaserver(void);
extern void stop_alpacaserver(void);
// Display
extern bool display_start();
extern void display_stop();
extern void display_clear();
// DuckDNS
extern bool duckdns_start(void);
extern void duckdns_stop(void);
extern String duckdns_getip(void);
// TCP/IP Server
extern bool start_tcpipserver(void);
extern void stop_tcpipserver(void);
// Temperature Probe
extern bool start_temperature_probe(void);
extern void stop_temperature_probe(void);
// Web Server
extern bool start_webserver(void);
extern void stop_webserver(void);


// -------------------------------------------------------
// EXTERN SETTINGS
// -------------------------------------------------------
// Controller
extern int mycontrollermode;
extern int mystationipaddressmode;
// Display
extern int _display_type;
// Management Server defines
extern const char *admin_username;
extern const char *admin_password;


// -------------------------------------------------------
// MANAGEMENT SERVER DEFINITIONS
// -------------------------------------------------------
#define NAVBARSIZE 2040  // 2027
// Page refresh time following a Management service reboot
// page time (s) between next page refresh
#define MAXSIZECUSTOMBRD 300
// temporary page when reboot occurs
#define RebootStr "<html><meta http-equiv=\"refresh\" content=\"15; url='/'\"><head><title>Management Server></title></head><body><p>Please wait, controller rebooting.</p></body></html>"

// DISPLAY
// display.html
// Page Display Time
#define DPT2Y "<option value=\"2\" selected>2</option>"
#define DPT2N "<option value=\"2\">2</option>"

#define DPT4Y "<option value=\"4\" selected>4</option>"
#define DPT4N "<option value=\"4\">4</option>"

#define DPT6Y "<option value=\"6\" selected>6</option>"
#define DPT6N "<option value=\"6\">6</option>"

#define DPT8Y "<option value=\"8\" selected>8</option>"
#define DPT8N "<option value=\"8\">8</option>"

#define DPT10Y "<option value=\"10\" selected>10</option>"
#define DPT10N "<option value=\"10\">10</option>"

bool servers_type;
bool duckdns_type;
bool backlash_type;
bool display_type;
bool temp_type;
bool misc_type;
bool motor_type;


// -------------------------------------------------------
// helper handlers .on for mserver
// -------------------------------------------------------
void ms_getservers() {
  servers_type = GeT;
  mngsrvr->get_servers();
}

void ms_postservers() {
  servers_type = PosT;
  mngsrvr->get_servers();
}

void ms_getduckdns() {
  duckdns_type = GeT;
  mngsrvr->get_duckdns();
}

void ms_postduckdns() {
  duckdns_type = PosT;
  mngsrvr->get_duckdns();
}

void ms_getbacklash() {
  backlash_type = GeT;
  mngsrvr->get_backlash();
}

void ms_postbacklash() {
  backlash_type = PosT;
  mngsrvr->get_backlash();
}

void ms_getdisplay() {
  display_type = GeT;
  mngsrvr->get_display();
}

void ms_postdisplay() {
  display_type = PosT;
  mngsrvr->get_display();
}

void ms_gettemp() {
  temp_type = GeT;
  mngsrvr->get_temp();
}

void ms_posttemp() {
  temp_type = PosT;
  mngsrvr->get_temp();
}

void ms_getmisc() {
  misc_type = GeT;
  mngsrvr->get_misc();
}

void ms_postmisc() {
  misc_type = PosT;
  mngsrvr->get_misc();
}

void ms_getmotor() {
  motor_type = GeT;
  mngsrvr->get_motor();
}

void ms_postmotor() {
  motor_type = PosT;
  mngsrvr->get_motor();
}

void msget_reboot() {
  mngsrvr->get_reboot();
}

// handler reboot
void msget_wait() {
  mngsrvr->get_wait();
}

void msget_navbar() {
  mngsrvr->get_navbar();
}

void msget_notfound() {
  mngsrvr->get_notfound();
}

String ms_geturi(void) {
  return mngsrvr->get_uri();
}

void ms_deletefile() {
  mngsrvr->get_deletefile();
}

void ms_postdeletefile() {
  mngsrvr->handler_postdeletefile();
}

void ms_getfilelist() {
  mngsrvr->get_filelist();
}

void ms_uploadfile() {
  mngsrvr->get_uploadfile();
}

void ms_postuploadstart(void) {
  mngsrvr->handler_postuploadstart();
}

void ms_postuploadfile(void) {
  mngsrvr->handler_postuploadfile();
}

void ms_success(void) {
  mngsrvr->handler_success();
}

//void ms_fail(void) {
//mngsrvr->handler_fail();
//}

void ms_saveconfig(void) {
  mngsrvr->handler_saveconfig();
}

void ms_getlinks(void) {
  mngsrvr->get_links();
}

void ms_cntlrconfig(void) {
  mngsrvr->get_cntlrconfig();
}

void ms_cntlrvar(void) {
  mngsrvr->get_cntlrvar();
}

void ms_boardconfig(void) {
  mngsrvr->get_boardconfig();
}

// XHTML
void ms_rssi() {
  mngsrvr->get_rssi();
}

void ms_getposition() {
  mngsrvr->get_position();
}

void ms_getismoving() {
  mngsrvr->get_ismoving();
}

void ms_gettargetposition() {
  mngsrvr->get_targetposition();
}

void ms_getheap() {
  mngsrvr->get_heap();
}

void ms_getsut() {
  mngsrvr->get_sut();
}


// -------------------------------------------------------
// MANAGEMENT SERVER CLASS
// -------------------------------------------------------
MANAGEMENT_SERVER::MANAGEMENT_SERVER() {
  _loaded = STATE_NOTLOADED;
  _navbar.reserve(NAVBARSIZE);
  _filelist.reserve(1024);  // 922
}


// -------------------------------------------------------
// LOAD AND CACHE NAVIGATION FOOTER
// -------------------------------------------------------
void MANAGEMENT_SERVER::load_navbar() {
  // cache the navbar for admin pages, len: 1702
  File nfile = LittleFS.open("/navbar.html", "r");
  if (!nfile) {
    _navbar = T_SPACE;
    Send_NoPage();
    return;
  } else {
    _navbar = nfile.readString();
    nfile.close();
    MngSrvrMsgPrint("ms:navbar len: ");
    MngSrvrMsgPrintln(_navbar.length());
  }
}


// -------------------------------------------------------
// START MANAGEMENT SERVER
// -------------------------------------------------------
bool MANAGEMENT_SERVER::start() {
  // check if server already created, if not, create one
  if (_loaded == STATE_NOTLOADED) {
    mserver = new ESP8266WebServer(MNGSERVERPORT);
  }

  LittleFS.begin();
  if (!LittleFS.begin()) {
    // error
    MngSrvrMsgPrintln("Mngsrvr:LittleFS error");
    return false;
  }

  // cache the navbar for admin pages, len: 956
  load_navbar();

  // admin pages
  mserver->on("/", HTTP_GET, ms_getservers);
  mserver->on("/", HTTP_POST, ms_postservers);
  mserver->on("/ln", msget_navbar);  // reload navbar
  mserver->on("/servers", HTTP_GET, ms_getservers);
  mserver->on("/servers", HTTP_POST, ms_postservers);
  mserver->on("/duckdns", HTTP_GET, ms_getduckdns);
  mserver->on("/duckdns", HTTP_POST, ms_postduckdns);
  mserver->on("/backlash", HTTP_GET, ms_getbacklash);
  mserver->on("/backlash", HTTP_POST, ms_postbacklash);
  mserver->on("/display", HTTP_GET, ms_getdisplay);
  mserver->on("/display", HTTP_POST, ms_postdisplay);
  mserver->on("/temp", HTTP_GET, ms_gettemp);
  mserver->on("/temp", HTTP_POST, ms_posttemp);
  mserver->on("/misc", HTTP_GET, ms_getmisc);
  mserver->on("/misc", HTTP_POST, ms_postmisc);
  mserver->on("/motor", HTTP_GET, ms_getmotor);
  mserver->on("/motor", HTTP_POST, ms_postmotor);

  // links to files on SourceForge
  mserver->on("/links", ms_getlinks);
  // reboot page
  mserver->on("/wait", msget_wait);
  mserver->on("/reboot", msget_reboot);
  // file handling pages
  mserver->on("/delete", HTTP_GET, ms_deletefile);
  mserver->on("/delete", HTTP_POST, ms_postdeletefile);
  mserver->on("/list", HTTP_GET, ms_getfilelist);
  mserver->on("/save", ms_saveconfig);
  mserver->on("/success", ms_success);

  mserver->on("/cntlr_config.jsn", ms_cntlrconfig);
  mserver->on("/cntlr_var.jsn", ms_cntlrvar);
  mserver->on("/board_config.jsn", ms_boardconfig);

  mserver->on("/uri", ms_geturi);

  mserver->on("/upload", HTTP_GET, ms_uploadfile);
  mserver->on(
    "/upload", HTTP_POST, []() {
      ms_postuploadstart();
    },
    ms_postuploadfile);

  // XHTML
  mserver->on("/he", ms_getheap);
  mserver->on("/im", ms_getismoving);
  mserver->on("/po", ms_getposition);
  mserver->on("/rssi", HTTP_GET, ms_rssi);
  mserver->on("/su", ms_getsut);
  mserver->on("/ta", ms_gettargetposition);

  // not found
  mserver->onNotFound([]() {
    msget_notfound();
  });

  mserver->begin();
  _loaded = true;
  return _loaded;
}

// -------------------------------------------------------
// STOP MANAGEMENT SERVER
// -------------------------------------------------------
void MANAGEMENT_SERVER::stop(void) {
  if (mngsrvr_status == STATUS_RUNNING) {
    mserver->stop();
  }
  if (mserver) {
    delete mserver;
  }
  _loaded = STATE_NOTLOADED;
  mngsrvr_status = STATUS_STOPPED;
}

// -------------------------------------------------------
// CHECK FOR CLIENTS
// -------------------------------------------------------
void MANAGEMENT_SERVER::loop() {
  // avoid a crash
  if (_loaded == STATE_NOTLOADED) {
    return;
  }

  if (mngsrvr_status == STATUS_RUNNING) {
    mserver->handleClient();
  }
}

// -------------------------------------------------------
// CHECK AUTHENTICATION
// -------------------------------------------------------
bool MANAGEMENT_SERVER::check_access(void) {
  if (_loaded == STATE_NOTLOADED) {
    return false;
  }

  if (!mserver->authenticate(admin_username, admin_password)) {
    mserver->requestAuthentication();
    return false;
  }
  return true;
}

// -------------------------------------------------------
// SEND PAGE NOT FOUND TO CLIENT
// -------------------------------------------------------
void MANAGEMENT_SERVER::Send_NoPage(void) {
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, H_FILENOTFOUNDSTR);
}

// -------------------------------------------------------
// SEND JSON STRING TO CLIENT
// -------------------------------------------------------
void MANAGEMENT_SERVER::send_json(String str) {
  mserver->sendHeader("Access-Control-Allow-Origin", "*");
  mserver->send(HTML_WEBPAGE, JSONTEXTPAGETYPE, str);
}

// -------------------------------------------------------
// SEND A REDIRECT PG TO CLIENT
// -------------------------------------------------------
void MANAGEMENT_SERVER::send_redirect(String pg) {
  mserver->sendHeader("Location", pg);
  mserver->send(HTML_REDIRECTURL);
}

// -------------------------------------------------------
// CONVERT THE FILE EXTENSION TO THE MIME TYPE
// Multipurpose Internet Mail Extensions
// -------------------------------------------------------
String MANAGEMENT_SERVER::get_contenttype(String filename) {
  String retval = "text/plain";
  if (filename.endsWith(".html")) {
    retval = "text/html";
  } else if (filename.endsWith(".css")) {
    retval = "text/css";
  } else if (filename.endsWith(".js")) {
    retval = "application/javascript";
  } else if (filename.endsWith(".json")) {
    retval = "text/json";
  } else if (filename.endsWith(".jsn")) {
    retval = "text/json";
  } else if (filename.endsWith(".ico")) {
    retval = "image/x-icon";
  } else {
    retval = "application/octet-stream";
  }
  return retval;
}


// ------------------------------------------------------
// CHECK IF DIGIT IS HEX
// ------------------------------------------------------
bool MANAGEMENT_SERVER::is_hexdigit(char c) {
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


// -------------------------------------------------------
// LOAD NEW NAVIGATION BAR FOOTER FILE AND CACHE NAVBAR
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_navbar() {
  load_navbar();
  send_redirect("/");
}

// -------------------------------------------------------
// REBOOT CONTROLLER
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_reboot() {
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, RebootStr);
  software_Reboot(REBOOTDELAY);
}


// -------------------------------------------------------
// HANDLER FOR /servers
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_servers(void) {
  String AdminPg;
  String msg;

  AdminPg.reserve(3840);  // 3707

  MngSrvrMsgPrintln(T_SERVERS);

  if (!check_access()) {
    return;
  }

  // post handler
  if (servers_type == PosT) {

    // ALPACA Server name=ascome, value=enable or value=disable
    msg = mserver->arg("ascome");
    if (msg != "") {
      if (msg == TLC_ENABLE) {
        ControllerData->set_alpacasrvr_enable(STATE_ENABLED);
      } else if (msg == TLC_DISABLE) {
        if (alpacasrvr_status == STATUS_RUNNING) {
          stop_alpacaserver();
        }
        alpacasrvr_status = STATUS_STOPPED;
        ControllerData->set_alpacasrvr_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // ALPACA Server start/stop service,
    // name=ascoms, value=start or value=stop
    msg = mserver->arg("ascoms");
    if (msg != "") {
      if (msg == TLC_START) {
        if (alpacasrvr_status == STATUS_RUNNING) {
          goto Get_Handler;
        }
        if (ControllerData->get_alpacasrvr_enable() == STATE_ENABLED) {
          alpacasrvr_status = start_alpacaserver();
        }
      } else if (msg == TLC_STOP) {
        if (alpacasrvr_status == STATUS_RUNNING) {
          stop_alpacaserver();
        }
        alpacasrvr_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }

    // TCP/IP Server ENABLE | DISABLE
    msg = mserver->arg("tcpe");
    if (msg != "") {
      if (msg == TLC_ENABLE) {
        ControllerData->set_tcpipsrvr_enable(STATE_ENABLED);
      } else if (msg == TLC_DISABLE) {
        if (tcpipsrvr_status == STATUS_RUNNING) {
          stop_tcpipserver();
        }
        tcpipsrvr_status = STATUS_STOPPED;
        ControllerData->set_tcpipsrvr_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // TCP/IP Server name=tcps, START | STOP
    msg = mserver->arg("tcps");
    if (msg != "") {
      if (msg == TLC_START) {
        if (tcpipsrvr_status == STATUS_RUNNING) {
          goto Get_Handler;
        }
        if (ControllerData->get_tcpipsrvr_enable() == STATE_ENABLED) {
          tcpipsrvr_status = start_tcpipserver();
        }
      } else if (msg == TLC_STOP) {
        if (tcpipsrvr_status == STATUS_RUNNING) {
          stop_tcpipserver();
        }
        tcpipsrvr_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }

    // WEB Server name=webe, value=enable or value=disable
    msg = mserver->arg("webe");
    if (msg != "") {
      if (msg == TLC_ENABLE) {
        ControllerData->set_websrvr_enable(STATE_ENABLED);
      } else if (msg == TLC_DISABLE) {
        if (websrvr_status == STATUS_RUNNING) {
          stop_webserver();
        }
        websrvr_status = STATUS_STOPPED;
        ControllerData->set_websrvr_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // Web Server name=webs, value=start or value=stop
    msg = mserver->arg("webs");
    if (msg != "") {
      if (msg == TLC_START) {
        if (websrvr_status == STATUS_RUNNING) {
          goto Get_Handler;
        }
        if (ControllerData->get_websrvr_enable() == STATE_ENABLED) {
          // enabled
          websrvr_status = start_webserver();
        }
      } else if (msg == TLC_STOP) {
        if (websrvr_status == STATUS_RUNNING) {
          stop_webserver();
        }
        websrvr_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }
  }

Get_Handler:

  File file = LittleFS.open("/servers.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    // ALPACA Server ENABLE | DISABLE
    if (ControllerData->get_alpacasrvr_enable() == STATE_ENABLED) {
      AdminPg.replace("%ASS%", T_ENABLED);
      AdminPg.replace("%ASE%", TLC_DISABLE);
      AdminPg.replace("%ASEB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%ASS%", T_DISABLED);
      AdminPg.replace("%ASE%", TLC_ENABLE);
      AdminPg.replace("%ASEB%", TUC_ENABLE);
    }

    // ALPACA Server START | STOP
    if (alpacasrvr_status) {
      AdminPg.replace("%ASST%", T_RUNNING);
      AdminPg.replace("%AES%", TLC_STOP);
      AdminPg.replace("%AESB%", TUC_STOP);
    } else {
      AdminPg.replace("%ASST%", T_STOPPED);
      AdminPg.replace("%AES%", TLC_START);
      AdminPg.replace("%AESB%", TUC_START);
    }

    // ALPACA port ReadOnly
    AdminPg.replace("%APO%", String(ALPACASERVERPORT));

    // TCP/IP Server ENABLE | DISABLE
    if (ControllerData->get_tcpipsrvr_enable() == STATE_ENABLED) {
      AdminPg.replace("%TCE%", T_ENABLED);
      AdminPg.replace("%TCPE%", TLC_DISABLE);
      AdminPg.replace("%TCEB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%TCE%", T_DISABLED);
      AdminPg.replace("%TCPE%", TLC_ENABLE);
      AdminPg.replace("%TCEB%", TUC_ENABLE);
    }

    // TCP/IP server START | STOP
    if (tcpipsrvr_status == STATUS_RUNNING) {
      AdminPg.replace("%TCPS%", T_RUNNING);
      AdminPg.replace("%TCS%", TLC_STOP);
      AdminPg.replace("%TCSB%", TUC_STOP);
    } else {
      AdminPg.replace("%TCPS%", T_STOPPED);
      AdminPg.replace("%TCS%", TLC_START);
      AdminPg.replace("%TCSB%", TUC_START);
    }

    // TCP/IP port ReadOnly
    AdminPg.replace("%TPO%", String(TCPIPSERVERPORT));

    // WEB Server ENABLE | DISABLE
    if (ControllerData->get_websrvr_enable() == STATE_ENABLED) {
      AdminPg.replace("%WSS%", T_ENABLED);
      AdminPg.replace("%WSE%", TLC_DISABLE);
      AdminPg.replace("%WSEB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%WSS%", T_DISABLED);
      AdminPg.replace("%WSE%", TLC_ENABLE);
      AdminPg.replace("%WSEB%", TUC_ENABLE);
    }

    // WEB server START | STOP
    if (websrvr_status == STATUS_RUNNING) {
      AdminPg.replace("%WBS%", T_RUNNING);
      AdminPg.replace("%WBO%", TLC_STOP);
      AdminPg.replace("%WBOB%", TUC_STOP);
    } else {
      AdminPg.replace("%WBS%", T_STOPPED);
      AdminPg.replace("%WBO%", TLC_START);
      AdminPg.replace("%WBOB%", TUC_START);
    }

    // Webserver Port ReadOnly
    AdminPg.replace("%WPO%", String(WEBSERVERPORT));

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_SERVERS);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// DUCKDNS
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_duckdns(void) {
  String AdminPg;
  String msg;

  AdminPg.reserve(3392);  // 3320

  MngSrvrMsgPrintln(T_DUCKDNS);

  if (!check_access()) {
    return;
  }

  // post handler
  if (duckdns_type == PosT) {
    // DUCKDNS ENABLE | DISABLE
    msg = mserver->arg("dse");
    if (msg != "") {
      if (msg == TLC_ENABLE) {
        ControllerData->set_duckdns_enable(STATE_ENABLED);
      } else if (msg == TLC_DISABLE) {
        // in case server is running, stop server
        if (duckdns_status == STATUS_RUNNING) {
          duckdns_stop();
        }
        duckdns_status = STATUS_STOPPED;
        ControllerData->set_duckdns_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // DUCKDNS START | STOP
    msg = mserver->arg("dss");
    if (msg != "") {
      if (msg == TLC_START) {
        if (duckdns_status == STATUS_RUNNING) {
          goto Get_Handler;
        }
        // if server is enabled
        if (ControllerData->get_duckdns_enable() == STATE_ENABLED) {
          duckdns_status = duckdns_start();
        }
        goto Get_Handler;
      } else if (msg == TLC_STOP) {
        // in case server is running, stop server
        if (duckdns_status == STATUS_RUNNING) {
          duckdns_stop();
        }
        duckdns_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }

    // DUCKDNS DOMAIN NAME
    msg = mserver->arg("setddnsdom");
    if (msg != "") {
      String dom = mserver->arg("ddomain");
      if (dom != "") {
        ControllerData->set_duckdns_domain(dom);
      }
      goto Get_Handler;
    }

    // DUCKDNS TOKEN
    msg = mserver->arg("setddnstok");
    if (msg != "") {
      String dtok = mserver->arg("dtoken");
      if (dtok != "") {
        ControllerData->set_duckdns_token(dtok);
      }
      goto Get_Handler;
    }
  }  // end of post handler

Get_Handler:

  File file = LittleFS.open("/duckdns.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    // DUCKDNS, ENABLE DISABLE
    if (ControllerData->get_duckdns_enable() == STATE_ENABLED) {

      AdminPg.replace("%DUS%", T_ENABLED);
      AdminPg.replace("%DSE%", TLC_DISABLE);
      AdminPg.replace("%DSB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%DUS%", T_DISABLED);
      AdminPg.replace("%DSE%", TLC_ENABLE);
      AdminPg.replace("%DSB%", TUC_ENABLE);
    }

    // DUCKDNS, STATE START-STOP
    if (duckdns_status == STATUS_RUNNING) {
      AdminPg.replace("%DSAT%", T_RUNNING);
      AdminPg.replace("%DSO%", TLC_STOP);
      AdminPg.replace("%DSATB%", TUC_STOP);
    } else {
      AdminPg.replace("%DSAT%", T_STOPPED);
      AdminPg.replace("%DSO%", TLC_START);
      AdminPg.replace("%DSATB%", TUC_START);
    }

    // duckdns domain
    AdminPg.replace("%ddom%", ControllerData->get_duckdns_domain());

    // duckdns token
    AdminPg.replace("%ddtok%", ControllerData->get_duckdns_token());

    // duckdns refresh time
    long refreshtime = DUCKDNS_REFRESHRATE * 1000;
    AdminPg.replace("%DRT%", String(refreshtime));

    // duckdns ip
    if (duckdns_status == STATUS_RUNNING) {
      AdminPg.replace("%DIP%", duckdns_getip());
    } else {
      AdminPg.replace("%DIP%", "---");
    }

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_DUCKDNS);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// BACKLASH
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_backlash(void) {
  String AdminPg;
  String msg;

  AdminPg.reserve(3008);  // 2922

  MngSrvrMsgPrintln(T_BACKLASH);

  if (!check_access()) {
    return;
  }

  // post handler
  if (backlash_type == PosT) {
    String msg;

    // backlash in steps, setblis, blis,
    msg = mserver->arg("setblis");
    if (msg != "") {
      String st = mserver->arg("blis");
      byte steps = (byte) st.toInt();
      ControllerData->set_backlashsteps_in(steps);
      goto Get_Handler;
    }

    // backlash out steps, setbos, bos
    msg = mserver->arg("setblos");
    if (msg != "") {
      String st = mserver->arg("blos");
      byte steps = (byte) st.toInt();
      ControllerData->set_backlashsteps_out(steps);
      goto Get_Handler;
    }

  }  // end of post handler

Get_Handler:

  File file = LittleFS.open("/backlash.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%STC%", SubTitleColor);
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);

    AdminPg.replace("%HDR%", DeviceName);

    AdminPg.replace("%blinum%", String(ControllerData->get_backlashsteps_in()));

    AdminPg.replace("%blonum%", String(ControllerData->get_backlashsteps_out()));

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }

  MngSrvrMsgPrint(T_BACKLASH);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// DISPLAY
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_display(void) {
  String AdminPg;
  String msg;

  AdminPg.reserve(4160);  // 4089, page varies in size

  MngSrvrMsgPrintln(T_DISPLAY);

  if (!check_access()) {
    return;
  }

  // post handler
  if (display_type == PosT) {
    // display state ENABLED DISABLED
    msg = mserver->arg("en");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_display_enable(STATE_ENABLED);
      } else if (msg == TLC_OFF) {
        // check if display is running, if so stop the display
        if (display_status == STATUS_RUNNING) {
          display_clear();
          display_stop();
        }
        ControllerData->set_display_enable(STATE_DISABLED);
        display_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }

    // display state start RUNNING STOPPED
    msg = mserver->arg("st");
    if (msg != "") {
      if (msg == TLC_START) {
        if (display_status == STATUS_RUNNING) {
          goto Get_Handler;
        }
        if (ControllerData->get_display_enable() == STATE_ENABLED) {
          display_status = display_start();
        }
      } else if (msg == TLC_STOP) {
        // if display is running, stop the display
        if (display_status == STATUS_RUNNING) {
          display_clear();
          display_stop();
        }
        display_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }

    // display update position when moving
    msg = mserver->arg("up");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_display_updateonmove(STATE_ENABLED);
      } else if (msg == TLC_OFF) {
        ControllerData->set_display_updateonmove(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // display page option, pg1 is at [5], 111111b
    String pageoption = ControllerData->get_display_pageoption();
    if (pageoption == "") {
      pageoption = "000001";
      ControllerData->set_display_pageoption(pageoption);
    }

    // make sure there are 6 digits, pad leading 0's if necessary
    while (pageoption.length() < 6) {
      pageoption = '0' + pageoption;
    }

    // when a form is submitted, only checkboxes which are 
    // currently checked are submitted to the server, with
    // the value on by default
    msg = mserver->arg("spo");
    if (msg != "") {
      msg = mserver->arg("pg1");
      if (msg == TLC_ON) {
        pageoption[5] = '1';
      }
      else {
        pageoption[5] = '0';
      }

      msg = mserver->arg("pg2");
      if (msg == TLC_ON) {
        pageoption[4] = '1';
      }
      else {
        pageoption[4] = '0';
      }

      msg = mserver->arg("pg3");
      if (msg == TLC_ON) {
        pageoption[3] = '1';
      }
      else {
        pageoption[3] = '0';
      }

      msg = mserver->arg("pg4");
      if (msg == TLC_ON) {
        pageoption[2] = '1';
      }
      else {
        pageoption[2] = '0';
      }

      msg = mserver->arg("pg5");
      if (msg == TLC_ON) {
        pageoption[1] = '1';
      }
      else {
        pageoption[1] = '0';
      }

      msg = mserver->arg("pg6");
      if (msg == TLC_ON) {
        pageoption[0] = '1';
      }
      else {
        pageoption[0] = '0';
      }
      ControllerData->set_display_pageoption(pageoption);
      goto Get_Handler;
    }
  }

Get_Handler:

  File file = LittleFS.open("/display.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%STC%", SubTitleColor);
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);

    AdminPg.replace("%HDR%", DeviceName);

    // DisplayType
    switch (_display_type) {
      case TEXT_OLED12864:
        MngSrvrMsgPrintln(T_DISPLAYTEXT);
        AdminPg.replace("%TY%", "Text");
        break;
      case LILYGO_OLED6432:
        MngSrvrMsgPrintln(T_DISPLAYLILYGO);
        AdminPg.replace("%TY%", "LilyGo");
        break;
      case GRAPHIC_OLED12864:
        MngSrvrMsgPrintln(T_DISPLAYGRAPHIC);
        AdminPg.replace("%TY%", "Graphic");
        break;
      default:
        MngSrvrMsgPrintln(T_DISPLAYNONE);
        AdminPg.replace("%TY%", "N/A");
        break;
    }

    // DISPLAY State Enabled Disabled
    if (ControllerData->get_display_enable() == STATE_ENABLED) {
      AdminPg.replace("%DST%", T_ENABLED);
      AdminPg.replace("%DSE%", TLC_OFF);
      AdminPg.replace("%DSB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%DST%", T_DISABLED);
      AdminPg.replace("%DSE%", TLC_ON);
      AdminPg.replace("%DSB%", TUC_ENABLE);
    }

    // DISPLAY Start/Stop 
    if (display_status) {
      AdminPg.replace("%DSS%", T_RUNNING);
      AdminPg.replace("%DS%", TLC_STOP);
      AdminPg.replace("%DB%", TUC_STOP);
    } else {
      AdminPg.replace("%DSS%", T_STOPPED);
      AdminPg.replace("%DS%", TLC_START);
      AdminPg.replace("%DB%", TUC_START);
    }

    // DISPLAY Update Position when moving
    // Enable/Disable
    if (ControllerData->get_display_updateonmove() == STATE_ENABLED) {
      AdminPg.replace("%DSPM%", T_ENABLED);
      AdminPg.replace("%SPE%", TLC_OFF);
      AdminPg.replace("%SPB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%DSPM%", T_DISABLED);
      AdminPg.replace("%SPE%", TLC_ON);
      AdminPg.replace("%SPB%", TUC_ENABLE);
    }

    // DISPLAY page options
    // %P1% to %P6%
    // need to get page options and then set each checkbox
    String pageoption = ControllerData->get_display_pageoption();

    // now build the page option html code
    // start with page1, which is right most bit
    // if 0, then unchecked, else if 1 then checked
    // read it in reverse order
    // 111111 6 pages P1-P6, [5] to [0]
    // pageoption[] index 5-0, index5 = pg1, index 0 = pg6;
    if (pageoption[5] == '0') {
      AdminPg.replace("%CHK1%", " ");
    }
    else {
      AdminPg.replace("%CHK1%", "Checked");
    }

    if (pageoption[4] == '0') {
      AdminPg.replace("%CHK2%", " ");
    }
    else {
      AdminPg.replace("%CHK2%", "Checked");
    }

    if (pageoption[3] == '0') {
      AdminPg.replace("%CHK3%", " ");
    }
    else {
      AdminPg.replace("%CHK3%", "Checked");
    }

    if (pageoption[2] == '0') {
      AdminPg.replace("%CHK4%", " ");
    }
    else {
      AdminPg.replace("%CHK4%", "Checked");
    }

    if (pageoption[1] == '0') {
      AdminPg.replace("%CHK5%", " ");
    }
    else {
      AdminPg.replace("%CHK5%", "Checked");
    }

    if (pageoption[0] == '0') {
      AdminPg.replace("%CHK6%", " ");
    }
    else {
      AdminPg.replace("%CHK6%", "Checked");
    }

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_DISPLAY);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// TEMP
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_temp(void) {
  String AdminPg;
  String msg;

  AdminPg.reserve(4160);  // 4106

  MngSrvrMsgPrintln(T_TEMP);

  if (!check_access()) {
    return;
  }

  // post handler
  if (temp_type == PosT) {
    // Temperature Probe ENABLE/DISABLE, tps, enabled. not enabled
    msg = mserver->arg("tps");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_tempprobe_enable(STATE_ENABLED);
      } else if (msg == TLC_OFF) {
        // if running then stop it first
        if (tempprobe_status == STATUS_RUNNING) {
          stop_temperature_probe();
        }
        tempprobe_status = STATUS_STOPPED;
        ControllerData->set_tempprobe_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // PROBE START-STOP
    msg = mserver->arg("tpru");
    if (msg != "") {
      if (msg == "start") {
        if (ControllerData->get_brdtemppin() != -1) {
          if (ControllerData->get_tempprobe_enable() == STATE_ENABLED) {
            tempprobe_status = start_temperature_probe();
          } 
        }
      } else if (msg == "stop") {
        if (tempprobe_status) {
          stop_temperature_probe();
        }
        tempprobe_status = STATUS_STOPPED;
      }
      goto Get_Handler;
    }

    // Temperature probe celsius/fahrenheit
    msg = mserver->arg("tm");
    if (msg != "") {
      if (msg == "cel") {
        ControllerData->set_tempmode(CELSIUS);
      } else if (msg == "fah") {
        ControllerData->set_tempmode(FAHRENHEIT);
      }
      goto Get_Handler;
    }

    // Temperature Compensation Direction
    msg = mserver->arg("tcd");
    if (msg != "") {
      if (msg == "in") {
        ControllerData->set_tcdirection(TC_DIRECTION_IN);
      } else if (msg == "out") {
        ControllerData->set_tcdirection(TC_DIRECTION_OUT);
      }
      goto Get_Handler;
    }

    // Temperature Coefficient 0-100
    msg = mserver->arg("settc");
    if (msg != "") {
      String st = mserver->arg("tce");
      int tc = st.toInt();
      RangeCheck(&tc, 0, 100);
      ControllerData->set_tempcoefficient(tc);
      goto Get_Handler;
    }

    // Temp Comp On Load
    String msg = mserver->arg("tcl");
    if (msg != "") {
      if (msg == TLC_ENABLE) {
        ControllerData->set_tempcomp_onload(STATE_ENABLED);
      } else if (msg == TLC_DISABLE) {
        ControllerData->set_tempcomp_onload(STATE_DISABLED);
      }
      goto Get_Handler;
    }
  }  // end of post handler

Get_Handler:

  File file = LittleFS.open("/temp.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    // Temperature Probe State Enable/Disable
    if (ControllerData->get_tempprobe_enable() == STATE_ENABLED) {
      AdminPg.replace("%TPS%", T_ENABLED);
      AdminPg.replace("%TPE%", TLC_OFF);
      AdminPg.replace("%TPEB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%TPS%", T_DISABLED);
      AdminPg.replace("%TPE%", TLC_ON);
      AdminPg.replace("%TPEB%", TUC_ENABLE);
    }

    // Temperature Probe Status Start/Stop
    if (tempprobe_status == STATUS_RUNNING) {
      AdminPg.replace("%TPR%", T_RUNNING);
      AdminPg.replace("%TPG%", TLC_STOP);
      AdminPg.replace("%TPGB%", TUC_STOP);
    } else {
      AdminPg.replace("%TPR%", T_STOPPED);
      AdminPg.replace("%TPG%", TLC_START);
      AdminPg.replace("%TPGB%", TUC_START);
    }

    // Temperature Mode %TPM%, %BTPM%
    // Celcius=1, Fahrenheit=0
    if (ControllerData->get_tempmode() == CELSIUS) {
      // celsius - Change to Fahrenheit
      AdminPg.replace("%TPM%", T_CELSIUS);
      AdminPg.replace("%TM%", "fah");
      AdminPg.replace("%TMB%", "F");
    } else {
      // Fahrenheit - change to celsius
      AdminPg.replace("%TPM%", T_FAHRENHEIT);
      AdminPg.replace("%TM%", "cel");
      AdminPg.replace("%TMB%", "C");
    }

    float tp = temp;
    if (ControllerData->get_tempmode() == FAHRENHEIT) {
      tp = (tp * 1.8) + 32;
    }

    AdminPg.replace("%TEV%", String(tp, 2));

    if (ControllerData->get_tempmode() == FAHRENHEIT) {
      AdminPg.replace("%TEM%", "F");
    } else {
      AdminPg.replace("%TEM%", "C");
    }

    // Temperature Compensation Direction value %TCD% %BTCD%
    // TC_DIRECTION_IN or TC_DIRECTION_OUT
    if (ControllerData->get_tcdirection() == TC_DIRECTION_IN) {
      AdminPg.replace("%TCD%", T_IN);
      AdminPg.replace("%TCO%", "out");
      AdminPg.replace("%TCOB%", "OUT");
    } else {
      AdminPg.replace("%TCD%", T_OUT);
      AdminPg.replace("%TCO%", "in");
      AdminPg.replace("%TCOB%", "IN");
    }

    // Temp Comp Coefficent
    AdminPg.replace("%tcnum%", String(ControllerData->get_tempcoefficient()));

    // Temp Comp On Load
    if (ControllerData->get_tempcomp_onload() == STATE_ENABLED) {
      AdminPg.replace("%TOL%", T_ENABLED);
      AdminPg.replace("%TCL%", TLC_DISABLE);
      AdminPg.replace("%TCLB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%TOL%", T_DISABLED);
      AdminPg.replace("%TCL%", TLC_ENABLE);
      AdminPg.replace("%TCLB%", TUC_ENABLE);
    }

    // Temp Comp Available
    if (tempcomp_available) {
      AdminPg.replace("%TCA%", T_ENABLED);
    } else {
      AdminPg.replace("%TCA%", T_DISABLED);
    }

    // Temp Comp State
    if (tempcomp_state) {
      AdminPg.replace("%TCS%", T_ENABLED);
    } else {
      AdminPg.replace("%TCS%", T_DISABLED);
    }

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_TEMP);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// MISC
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_misc(void) {
  String AdminPg;

  AdminPg.reserve(3904);  // 3816

  MngSrvrMsgPrintln(T_MISC);

  if (!check_access()) {
    return;
  }

  // post handler
  if (misc_type == PosT) {
    String msg;

    // DeviceName
    msg = mserver->arg("dna");
    if (msg != "") {
      ControllerData->set_devicename(msg);    
      goto Get_Handler;
    }

    // MDNSName
    msg = mserver->arg("mdna");
    if (msg != "") {
      ControllerData->set_mdnsname(msg);    
      goto Get_Handler;
    }

    // PowerDown ENABLE/DISABLE 
    msg = mserver->arg("pwr");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_powerdown_enable(STATE_ENABLED);
      } else if (msg == TLC_OFF) {
        ControllerData->set_powerdown_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // PowerDown Time
    msg = mserver->arg("setpdc");
    if (msg != "") {
      String st = mserver->arg("pdt");
      int pdt = st.toInt();
      RangeCheck(&pdt, 30, 120);
      ControllerData->set_powerdown_time(pdt);
      goto Get_Handler;
    }   
  }  // end of post handler

Get_Handler:

  File file = LittleFS.open("/misc.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    // Controller Mode
    if (mycontrollermode == ACCESSPOINT) {
      AdminPg.replace("%MOD%", T_ACCESSPOINT);
    } else {
      AdminPg.replace("%MOD%", T_STATION);
    }

    // local serial cannot get here because wifi is not running
    // Static IP
    if (mystationipaddressmode == STATICIP) {
      AdminPg.replace("%IPS%", T_ON);
    } else {
      AdminPg.replace("%IPS%", T_OFF);
    }

    // Device Name
    AdminPg.replace("%DVN%", ControllerData->get_devicename());

    // MDNS Name
    AdminPg.replace("%MDN%", ControllerData->get_mdnsname());

    // PowerDown enable
    if (ControllerData->get_powerdown_enable()) {
      AdminPg.replace("%PDS%", T_ENABLED);
      AdminPg.replace("%HPWR%", TLC_OFF);
      AdminPg.replace("%PDN%", TUC_DISABLE);
    } else {
      AdminPg.replace("%PDS%", T_DISABLED);
      AdminPg.replace("%HPWR%", TLC_ON);
      AdminPg.replace("%PDN%", TUC_ENABLE);
    }

    // PowerDownDisplay Time
    AdminPg.replace("%PNTI%", String(ControllerData->get_powerdown_time()));
    
    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_MISC);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// MOTOR
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_motor(void) {
  String AdminPg;

  AdminPg.reserve(4992);  // 4948
  MngSrvrMsgPrintln(T_MOTOR);

  if (!check_access()) {
    return;
  }

  // post handler
  if (motor_type == PosT) {
    String msg;
    String fp;
    long tp;

    // position set
    msg = mserver->arg("setpos");
    if (msg != "") {
      // get new position value from text field pos
      // this is NOT a move
      fp = mserver->arg("pos");
      if (fp != "") {
        //long maxp = ControllerData->get_maxstep();
        tp = (long)fp.toInt();
        RangeCheck(&tp, 0L, ControllerData->get_maxstep());
        ftargetPosition = tp;
        driverboard->setposition(ftargetPosition);
        ControllerData->set_fposition(ftargetPosition);
        goto Get_Handler;
      }
    }

    // position goto
    msg = mserver->arg("gopos");
    if (msg != "") {
      // get new position value from text field pos
      // set target position: this is a move
      fp = mserver->arg("pos");
      if (fp != "") {
        tp = (long)fp.toInt();
        RangeCheck(&tp, 0L, ControllerData->get_maxstep());
        ftargetPosition = tp;
        isMoving = true;
        goto Get_Handler;
      }
    }

    // maxsteps // EH Feb 2025, Jimboh
    msg = mserver->arg("setmax");
    if (msg != "") {
      String newmaxpos = mserver->arg("max");
      if (newmaxpos != "") {
        // get maxStep arg
        tp = newmaxpos.toInt();
        // test if below or equal to position
        long cpos = driverboard->getposition();
        RangeCheck(&tp, cpos, FOCUSERUPPERLIMIT);
        ControllerData->set_maxstep(tp);
      }
      goto Get_Handler;
    }

    // coil power enable cpst on off
    msg = mserver->arg("cpst");
    if (msg != "") {
      if (msg == TLC_OFF) {
        ControllerData->set_coilpower_enable(STATE_DISABLED);
        driverboard->releasemotor();
      } else if (msg = TLC_ON) {
        ControllerData->set_coilpower_enable(STATE_ENABLED);
        driverboard->enablemotor();
      }
      goto Get_Handler;
    }

    // update delay after move
    msg = mserver->arg("dam");
    if (msg != "") {
      int dam = msg.toInt();
      RangeCheck(&dam, 0, 255);
      ControllerData->set_delayaftermove_time(dam);
    }

    // update motorspeed
    msg = mserver->arg("ms");
    if (msg != "") {
      int mspd = msg.toInt();
      RangeCheck(&mspd, SLOW, FAST);
      ControllerData->set_motorspeed(mspd);
      goto Get_Handler;
    }

    // motor speed delay 500-14000
    msg = mserver->arg("setmsd");
    if (msg != "") {
      String msd = mserver->arg("msd");
      if (msd != "") {
        unsigned long newdelay = (unsigned long) msd.toInt();
        RangeCheck(&newdelay, DEFAULT_MOTORSPEEDDELAYMIN, DEFAULT_MOTORSPEEDDELAYMAX);
        ControllerData->set_brdmsdelay(newdelay);
      }
      goto Get_Handler;
    }

    // reverse direction rdst on off
    msg = mserver->arg("rdst");
    if (msg != "") {
      if (msg == TLC_ON) {
        ControllerData->set_reverse_enable(STATE_ENABLED);
      } else if (msg = TLC_OFF) {
        ControllerData->set_reverse_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // stepmode
    msg = mserver->arg("setsm");
    if (msg != "") {
      if (msg == "half") {
        driverboard->setstepmode(STEP2);
      } else if (msg == "full") {
        driverboard->setstepmode(STEP1);
      }
      // ignore all others because sm is FIXED
      goto Get_Handler;
    }

    // stepsize value
    msg = mserver->arg("setss");
    if (msg != "") {
      String st = mserver->arg("ssv");
      float steps = st.toFloat();
      RangeCheck(&steps, MINIMUMSTEPSIZE, MAXIMUMSTEPSIZE);
      ControllerData->set_stepsize(steps);
      goto Get_Handler;
    }
  }  // end of post handler

Get_Handler:

  File file = LittleFS.open("/motor.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    // position
    AdminPg.replace("%CPO%", String(driverboard->getposition()));
    //AdminPg.replace("%posval%", String(driverboard->getposition()));

    // maxsteps
    AdminPg.replace("%maxval%", String(ControllerData->get_maxstep()));

    // Coil Power
    if (ControllerData->get_coilpower_enable() == STATE_ENABLED) {
      AdminPg.replace("%CPS%", T_ENABLED);
      AdminPg.replace("%CPV%", TLC_OFF);
      AdminPg.replace("%CPSB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%CPS%", T_DISABLED);
      AdminPg.replace("%CPV%", TLC_ON);
      AdminPg.replace("%CPSB%", TUC_ENABLE);
    }

    // delay after move
    AdminPg.replace("%damnum%", String(ControllerData->get_delayaftermove_time()));

    // motorspeed
    switch (ControllerData->get_motorspeed()) {
      case 0:
        AdminPg.replace("%MSS%", T_CHECKED);
        AdminPg.replace("%MSM%", T_SPACE);
        AdminPg.replace("%MSF%", T_SPACE);
        break;
      case 1:
        AdminPg.replace("%MSS%", T_SPACE);
        AdminPg.replace("%MSM%", T_CHECKED);
        AdminPg.replace("%MSF%", T_SPACE);
        break;
      case 2:
        AdminPg.replace("%MSS%", T_SPACE);
        AdminPg.replace("%MSM%", T_SPACE);
        AdminPg.replace("%MSF%", T_CHECKED);
        break;
      default:
        AdminPg.replace("%MSS%", T_SPACE);
        AdminPg.replace("%MSM%", T_SPACE);
        AdminPg.replace("%MSF%", T_CHECKED);
        break;
    }

    // motor speed delay value %MSD%
    AdminPg.replace("%msdnum%", String(ControllerData->get_brdmsdelay()));

    // Reverse Direction
    if (ControllerData->get_reverse_enable() == STATE_ENABLED) {
      AdminPg.replace("%RDS%", T_ENABLED);
      AdminPg.replace("%RV%", TLC_OFF);
      AdminPg.replace("%RVB%", TUC_DISABLE);
    } else {
      AdminPg.replace("%RDS%", T_DISABLED);
      AdminPg.replace("%RV%", TLC_ON);
      AdminPg.replace("%RVB%", TUC_ENABLE);
    }

    // step mode v314 code
    // Build based on Fixed, Step1-2 and Step1-256

    int sm = ControllerData->get_brdstepmode();
    // %SMV% current step mode setting
    // %SMN% hidden  desired setting "full" or "half"
    // %SMB% button text H or F or ---

    // halfstepper, Stepmode1 and Stepmode2
    if ((myboardnumber == PRO2EULN2003) || (myboardnumber == PRO2EL293DNEMA) || (myboardnumber == PRO2EL298N) || (myboardnumber == PRO2EL293DMINI) || (myboardnumber == PRO2EL9110S)) {
      // half stepper boards, Button switches bewteen Step1, Step2
      if (sm == 1) {
        // Current value [%SMV%]
        AdminPg.replace("%SMV%", String(sm));
        // SMN
        AdminPg.replace("%SMN%", "half");
        // button SMB
        AdminPg.replace("%SMB%", "1/2");
      } else {
        // Current value [%SMV%]
        AdminPg.replace("%SMV%", String(sm));
        // SMN
        AdminPg.replace("%SMN%", "full");
        // button SMB
        AdminPg.replace("%SMB%", "FULL");
      }
    } else {
      // all other boards have a fixed step mode
      // hardware jumpers set step mode on the driver board
      // step mode is set to the config.h value of
      //    FIXEDSTEPMODE
      // so to change it, user must edit config.h and
      // upload the new firmware to the Controller

      // Current value [%SMV%]
      AdminPg.replace("%SMV%", String(sm));
      // SMN
      AdminPg.replace("%SMN%", "fixed");
      // button SMB
      AdminPg.replace("%SMB%", "---");
    }

    // step size value
    String ssv = String(ControllerData->get_stepsize());
    AdminPg.replace("%ssnum%", ssv);

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_MOTOR);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// WAIT: HANDLER FOR REBOOT
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_wait() {
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, RebootStr);
}

// -------------------------------------------------------
// DELETE FILE
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_deletefile() {
  String AdminPg;
  AdminPg.reserve(2540);  // 2425

  MngSrvrMsgPrintln(T_DELETE);

  if (!check_access()) {
    return;
  }

  File file = LittleFS.open("/delete.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_DELETE);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}

// -------------------------------------------------------
// LISTS ALL FILES IN FILE SYSTEM
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_filelist(void) {
  MngSrvrMsgPrintln(T_LIST);
  if (!check_access()) {
    return;
  }
  _filelist = "{ ";
  ListAllFilesInDir("/");
  // erase last ,
  _filelist = _filelist.substring(0, _filelist.length() - 2);
  _filelist += "  }";

  MngSrvrMsgPrint(T_LIST);
  MngSrvrMsgPrintln(_filelist.length());
  // send _filelist
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, JSONTEXTPAGETYPE, _filelist);
}

// -------------------------------------------------------
// LISTS ALL FILES IN FILE SYSTEM DIRECTORY
// -------------------------------------------------------
void MANAGEMENT_SERVER::ListAllFilesInDir(String dir_path) {
  Dir dir = LittleFS.openDir(dir_path);
  while (dir.next()) {
    if (dir.isFile()) {
      //_filelist += "\"file\":";
      //_filelist += "\"" + String(dir_path + dir.fileName()) + "\", ";
      _filelist += "\"" + String(dir_path + dir.fileName()) + "\": ";
      _filelist += String(dir.fileSize()) + ", ";

      MngSrvrMsgPrint("File ");
      MngSrvrMsgPrint(dir_path);
      MngSrvrMsgPrintln(dir.fileName());
    }
    if (dir.isDirectory()) {
      // print directory names
      _filelist += "\"dir\":";
      _filelist += "\"" + String(dir_path + dir.fileName()) + "\", ";
      // recursive file listing inside new directory
      ListAllFilesInDir(dir_path + dir.fileName() + "/");
    }
  }
}


// -------------------------------------------------------
// LISTS ALL LINKS TO PROJECT SITE
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_links(void) {
  String AdminPg;
  AdminPg.reserve(2345);  // 3382

  MngSrvrMsgPrintln(T_LINKS);

  if (!check_access()) {
    return;
  }

  File file = LittleFS.open("/links.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_LINKS);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// -------------------------------------------------------
// cntlrconfig
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_cntlrconfig(void) {
  String AdminPg;

  AdminPg.reserve(2048);

  File file = LittleFS.open("/cntlr_config.jsn", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    // read contents into string
    AdminPg = file.readString();
    file.close();
    send_json(AdminPg);
  }
}

// -------------------------------------------------------
// cntlrvar
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_cntlrvar(void) {
  String AdminPg;

  AdminPg.reserve(64);

  File file = LittleFS.open("/cntlr_var.jsn", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    // read contents into string
    AdminPg = file.readString();
    file.close();
    send_json(AdminPg);
  }
}

// -------------------------------------------------------
// boardconfig
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_boardconfig(void) {
  String AdminPg;

  AdminPg.reserve(512);

  File file = LittleFS.open("/board_config.jsn", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    // read contents into string
    AdminPg = file.readString();
    file.close();
    send_json(AdminPg);
  }
}

// -------------------------------------------------------
// NOT FOUND
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_notfound(void) {
  String AdminPg;
  AdminPg.reserve(2304);  // 2126

  MngSrvrMsgPrintln(T_NOTFOUND);

  if (!check_access()) {
    return;
  }

  String p = mserver->uri();
  MngSrvrMsgPrint("-uri ");
  MngSrvrMsgPrintln(p);

  // get the MIME type
  String contenttype = get_contenttype(p);

  if (p == "/favicon.ico") {
    File ifile = LittleFS.open("/favicon.ico", "r");
    MngSrvrMsgPrintln("notfound: send favicon.ico");
    mserver->sendHeader("Content-Disposition", "attachment; filename=favicon.ico;");
    mserver->streamFile(ifile, "image/x-icon");
    MngSrvrMsgPrintln("notfound: favicon.ico sent");
  }

  //send file not found back to user
  File file = LittleFS.open("/adminnotfound.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    // using file "adminnotfound", read contents into string
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
    MngSrvrMsgPrint(T_NOTFOUND);
    MngSrvrMsgPrintln(AdminPg.length());
    mserver->sendHeader("Cache-Control", "no-cache");
    mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
  }
}


// -------------------------------------------------------
// UPLOAD FILE
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_uploadfile(void) {
  String AdminPg;
  AdminPg.reserve(2432);  // 2244

  MngSrvrMsgPrintln(T_UPLOAD);

  if (!check_access()) {
    return;
  }

  File file = LittleFS.open("/upload.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_UPLOAD);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->sendHeader("Cache-Control", "no-cache");
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}


// ------------------------------------------------------
// SAVE CONFIG TO FILESYSTEM
// ------------------------------------------------------
void MANAGEMENT_SERVER::handler_saveconfig(void) {
  String AdminPg;
  AdminPg.reserve(2176);  // 2152

  MngSrvrMsgPrintln(T_CONFIGSAVED);

  if (!check_access()) {
    return;
  }

  // save the focuser settings immediately
  if (ControllerData->SaveNow(driverboard->getposition(), driverboard->getdirection()) == true) {
    File file = LittleFS.open("/configsaved.html", "r");
    if (!file) {
      Send_NoPage();
      return;
    } else {
      AdminPg = file.readString();
      file.close();

      // Web page colors
      AdminPg.replace("%TXC%", TextColor);
      AdminPg.replace("%BKC%", BackColor);
      AdminPg.replace("%TIC%", TitleColor);
      AdminPg.replace("%HEC%", HeaderColor);
      AdminPg.replace("%STC%", SubTitleColor);

      AdminPg.replace("%HDR%", DeviceName);

      AdminPg += _navbar;

      // footer
      AdminPg.replace("%FTR%", FooterColor);
      AdminPg.replace("%NAM%", ControllerData->get_brdname());
      AdminPg.replace("%VER%", String(major_version));
      AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
      get_systemuptime();
      AdminPg.replace("%SUT%", systemuptime);
      AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
    }

    MngSrvrMsgPrint(T_CONFIGSAVED);
    MngSrvrMsgPrintln(AdminPg.length());
    mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
    return;
  } else {
    // config save error
    File file = LittleFS.open("/confignotsaved.html", "r");
    if (!file) {
      Send_NoPage();
      return;
    } else {
      AdminPg = file.readString();
      file.close();

      // Web page colors
      AdminPg.replace("%TXC%", TextColor);
      AdminPg.replace("%BKC%", BackColor);
      AdminPg.replace("%TIC%", TitleColor);
      AdminPg.replace("%HEC%", HeaderColor);
      AdminPg.replace("%STC%", SubTitleColor);

      AdminPg.replace("%HDR%", DeviceName);

      AdminPg += _navbar;

      // footer
      AdminPg.replace("%FTR%", FooterColor);
      AdminPg.replace("%NAM%", ControllerData->get_brdname());
      AdminPg.replace("%VER%", String(major_version));
      AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
      get_systemuptime();
      AdminPg.replace("%SUT%", systemuptime);
      AdminPg.replace("%WiFi%", String(WiFi.RSSI()));

      MngSrvrMsgPrint(T_CONFIGNOTSAVED);
      MngSrvrMsgPrintln(AdminPg.length());
      mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
    }
  }
}


// -------------------------------------------------------
// IF REQUESTED OPERATION WAS SUCCESSFUL, DISPLAY SUCCESS HTML PAGE
// -------------------------------------------------------
void MANAGEMENT_SERVER::handler_success(void) {
  String AdminPg;
  AdminPg.reserve(2304);  // 2219

  MngSrvrMsgPrintln(T_SUCCESS);

  if (!check_access()) {
    return;
  }

  File file = LittleFS.open("/success.html", "r");
  if (!file) {
    Send_NoPage();
    return;
  } else {
    AdminPg = file.readString();
    file.close();

    // Web page colors
    AdminPg.replace("%TXC%", TextColor);
    AdminPg.replace("%BKC%", BackColor);
    AdminPg.replace("%TIC%", TitleColor);
    AdminPg.replace("%HEC%", HeaderColor);
    AdminPg.replace("%STC%", SubTitleColor);

    AdminPg.replace("%HDR%", DeviceName);

    AdminPg += _navbar;

    // footer
    AdminPg.replace("%FTR%", FooterColor);
    AdminPg.replace("%NAM%", ControllerData->get_brdname());
    AdminPg.replace("%VER%", String(major_version));
    AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    AdminPg.replace("%SUT%", systemuptime);
    AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
  }
  MngSrvrMsgPrint(T_SUCCESS);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}

// -------------------------------------------------------
// DELETE FILE (POST)
// -------------------------------------------------------
void MANAGEMENT_SERVER::handler_postdeletefile() {
  String AdminPg;
  AdminPg.reserve(2304);  // deleteok 2222

  MngSrvrMsgPrintln(T_DELETEOK);

  if (!check_access()) {
    return;
  }

  // check server arguments, df has filename
  String df = mserver->arg("fname");
  if (df != "") {
    // df = " / " + df;
    if (df[0] != '/') {
      df = '/' + df;
    }
    // load the deleteok.html file
    File file = LittleFS.open("/deleteok.html", "r");
    if (!file) {
      Send_NoPage();
      return;
    } else {
      AdminPg = file.readString();
      file.close();

      // Web page colors
      AdminPg.replace("%TXC%", TextColor);
      AdminPg.replace("%BKC%", BackColor);
      AdminPg.replace("%TIC%", TitleColor);
      AdminPg.replace("%HEC%", HeaderColor);
      AdminPg.replace("%STC%", SubTitleColor);

      AdminPg.replace("%HDR%", DeviceName);

      AdminPg.replace("%FIL%", df);

      if (!LittleFS.exists(df)) {
        AdminPg.replace("%STA%", "err File not found");
      } else {
        if (LittleFS.remove(df)) {
          AdminPg.replace("%STA%", "deleted.");
        } else {
          AdminPg.replace("%STA%", "Error File delete");
        }
      }
      AdminPg += _navbar;

      // footer
      AdminPg.replace("%FTR%", FooterColor);
      AdminPg.replace("%NAM%", ControllerData->get_brdname());
      AdminPg.replace("%VER%", String(major_version));
      AdminPg.replace("%HEA%", String(ESP.getFreeHeap()));
      get_systemuptime();
      AdminPg.replace("%SUT%", systemuptime);
      AdminPg.replace("%WiFi%", String(WiFi.RSSI()));
    }
  } else {
    // null argument has been passed
    AdminPg = H_FILENOTFOUNDSTR;
  }
  MngSrvrMsgPrint(T_DELETEOK);
  MngSrvrMsgPrintln(AdminPg.length());
  mserver->send(HTML_WEBPAGE, TEXTPAGETYPE, AdminPg);
}

// -------------------------------------------------------
// HANDLES REQUEST TO UPLOAD FILE
// -------------------------------------------------------
void MANAGEMENT_SERVER::handler_postuploadstart(void) {
  if (_loaded == false) {
    return;
  }
  // inform client to send data now
  mserver->send(HTML_WEBPAGE);
}

// -------------------------------------------------------
// WRITES THE UPLOAD FILE
// -------------------------------------------------------
void MANAGEMENT_SERVER::handler_postuploadfile(void) {
  if (!check_access()) {
    return;
  }

  HTTPUpload &upload = mserver->upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    _fsUploadFile = LittleFS.open(filename, "w");
    _errormsg = "File " + filename;
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (_fsUploadFile) {
      _fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (_fsUploadFile) {
      // If the file was successfully created
      _fsUploadFile.close();
      mngsrvr->handler_success();
      return;
    } else {
      mserver->send(HTML_SERVERERROR, String(HTML_WEBPAGE), "Err: upload file");
    }
  }
}

// -------------------------------------------------------
// GET URI
// -------------------------------------------------------
String MANAGEMENT_SERVER::get_uri(void) {
  String p = mserver->uri();
  return p;
}


// -------------------------------------------------------
// GET NETWORK SIGNAL STRENGTH
// xhtml
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_rssi(void) {
  if (!check_access()) {
    return;
  }
  long rssi = getrssi();
  mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, String(rssi));
}

// -------------------------------------------------------
// GET POSITION AND SEND TO CLIENT
// xhtml
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_position() {
  // Send position value only to client ajax request
  mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, String(driverboard->getposition()));
}

// -------------------------------------------------------
// GET ISMOVING
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_ismoving() {
  if (isMoving == true) {
    mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, "True");
  } else {
    mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, "False");
  }
}

// -------------------------------------------------------
// GET TARGET POSITION
// xhtml
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_targetposition() {
  mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, String(ftargetPosition));
}

// -------------------------------------------------------
// GET HEAP
// xhtml
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_heap() {
  mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, String(ESP.getFreeHeap()));
}

// -------------------------------------------------------
// GET SUT
// xhtml
// -------------------------------------------------------
void MANAGEMENT_SERVER::get_sut() {
  get_systemuptime();
  mserver->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, String(systemuptime));
}
