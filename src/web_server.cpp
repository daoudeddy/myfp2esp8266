//---------------------------------------------------
// myFP2ESP8266 WEB SERVER CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// web_server.cpp
// Optional
// Index, Move forms
// NodeMCU 1.0 (ESP-12E Module)
//---------------------------------------------------


//---------------------------------------------------
// INCLUDES
//---------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(ENABLE_WEBSERVER)

#include <avr/pgmspace.h>
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Web Server messages 
// to be written to Serial port
//#define WEBSERVER_MSGPRINT 1

#ifdef WEBSERVER_MSGPRINT
#define WebSrvrMsgPrint(...) Serial.print(__VA_ARGS__)
#define WebSrvrMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define WebSrvrMsgPrint(...)
#define WebSrvrMsgPrintln(...)
#endif


//---------------------------------------------------
// EXTERN CLASSES
//---------------------------------------------------
// ControllerData
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// Driver board
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

#include "web_server.h"
extern WEB_SERVER *websrvr;


//---------------------------------------------------
// EXTERNS
//---------------------------------------------------


//---------------------------------------------------
// DEFINES
//---------------------------------------------------
bool wsindex_type;
bool wsmove_type;


//---------------------------------------------------
// Web Page Handlers
//---------------------------------------------------
void wsget_index(void) {
  wsindex_type = GeT;
  websrvr->get_index();
}

void wspost_index(void) {
  wsindex_type = PosT;
  websrvr->get_index();
}

void wsget_move(void) {
  wsmove_type = GeT;
  websrvr->get_move();
}

void wspost_move(void) {
  wsmove_type = PosT;
  websrvr->get_move();
}

void wsget_notfound(void) {
  websrvr->get_notfound();
}


//---------------------------------------------------
// XHTML
//---------------------------------------------------
void wsget_position(void) {
  websrvr->get_position();
}

void wsget_ismoving(void) {
  websrvr->get_ismoving();
}

void wsget_targetposition(void) {
  websrvr->get_targetposition();
}

void wsget_temperature(void) {
  websrvr->get_temperature();
}

void wsget_coilpower(void) {
  websrvr->get_coilpower();
}

void wsget_heap(void) {
  websrvr->get_heap();
}

void wsget_sut(void) {
  websrvr->get_sut();
}


//---------------------------------------------------
// CLASS
//---------------------------------------------------
WEB_SERVER::WEB_SERVER() {
  _loaded = STATE_NOTLOADED;
}


//---------------------------------------------------
// CREATE AND START THE WEBSERVER
// creates _webserver
//---------------------------------------------------
bool WEB_SERVER::start() {
  WebSrvrMsgPrint(T_WEBSERVER);
  WebSrvrMsgPrintln(T_START);

  // prevent any attempt to start if server is already started
  if (_loaded == STATE_LOADED) {
    return true;
  }

  // if server is not enabled then return
  if (ControllerData->get_websrvr_enable() == STATE_DISABLED) {
    WebSrvrMsgPrintln(T_DISABLED);
    return false;
  }

  _web_server = new ESP8266WebServer(WEBSERVERPORT);

  LittleFS.begin();
  if (!LittleFS.begin()) {
    WebSrvrMsgPrintln(T_FILESYSTEMERROR);
    return false;
  }

  WebSrvrMsgPrintln("add handlers");
  // Web pages
  _web_server->on("/", HTTP_GET, wsget_index);
  _web_server->on("/", HTTP_POST, wspost_index);
  _web_server->on("/index", HTTP_GET, wsget_index);
  _web_server->on("/index", HTTP_POST, wspost_index);
  _web_server->on("/move", HTTP_GET, wsget_move);
  _web_server->on("/move", HTTP_POST, wspost_move);

  // XHTML
  _web_server->on("/po", wsget_position);
  _web_server->on("/im", wsget_ismoving);
  _web_server->on("/ta", wsget_targetposition);
  _web_server->on("/tm", wsget_temperature);
  _web_server->on("/cp", wsget_coilpower);
  _web_server->on("/he", wsget_heap);
  _web_server->on("/su", wsget_sut);

  _web_server->onNotFound([]() {
    wsget_notfound();
  });

  _web_server->begin();
  _loaded = STATE_LOADED;
  WebSrvrMsgPrintln(T_RUNNING);
  return _loaded;
}

//---------------------------------------------------
// STOP THE WEB SERVER
// deletes _webserver
//---------------------------------------------------
void WEB_SERVER::stop(void) {
  WebSrvrMsgPrintln(T_WEBSERVER);
  WebSrvrMsgPrintln(TUC_STOP);
  if (websrvr_status == STATUS_RUNNING) {
    _web_server->stop();
  }
  delete _web_server;
  _loaded = STATE_NOTLOADED;
}

//---------------------------------------------------
// CHECK FOR NEW CLIENTS OR EXISTING CLIENT REQUESTS
//---------------------------------------------------
void WEB_SERVER::loop() {
  // avoid a crash
  if (_loaded == STATE_NOTLOADED) {
    return;
  }
  _web_server->handleClient();
}

//---------------------------------------------------
// FILE SYSTEM NOT LOADED
//---------------------------------------------------
void WEB_SERVER::file_sys_error(void) {
  WebSrvrMsgPrint(T_WEBSERVER);
  WebSrvrMsgPrintln(T_FILESYSTEMERROR);
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, H_FSNOTLOADEDSTR);
}

//---------------------------------------------------
// SEND A REDIRECT PG TO CLIENT
//---------------------------------------------------
void WEB_SERVER::send_redirect(String pg) {
  _web_server->sendHeader("Location", pg);
  _web_server->send(HTML_REDIRECTURL);
}

//---------------------------------------------------
// SEND HEADER FOR XHTML TO CLIENT
//---------------------------------------------------
void WEB_SERVER::send_ACAOheader(void) {
  _web_server->sendHeader("Access-Control-Allow-Origin", "*");
}

//---------------------------------------------------
// XHTML Add AOC Header
//---------------------------------------------------
void WEB_SERVER::send_xhtml(String str) {
  send_ACAOheader();
  _web_server->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, str);
}

//---------------------------------------------------
// SEND JSON STRING TO CLIENT
//---------------------------------------------------
void WEB_SERVER::send_json(String str) {
  send_ACAOheader();
  _web_server->send(HTML_WEBPAGE, JSONTEXTPAGETYPE, str);
}

// ----------------------------------------------------------------------
// sends html header to client
// ----------------------------------------------------------------------
void WEB_SERVER::send_myheader(void) {
  _web_server->client().println("HTTP/1.1 200 OK");
  _web_server->client().println("Content-type:text/html");
  _web_server->client().println("Access-Control-Allow-Origin: *");
  _web_server->client().println("Control: no-cache");
  _web_server->client().println();
}

// ----------------------------------------------------------------------
// sends html page to web client
// ----------------------------------------------------------------------
void WEB_SERVER::send_mycontent(String str) {
  _web_server->client().print(str);
}

//---------------------------------------------------
// CONVERT THE FILE EXTENSION TO THE MIME TYPE
//---------------------------------------------------
String WEB_SERVER::get_contenttype(String filename) {
  String retval = "text/plain";
  if (filename.endsWith(".html")) {
    retval = "text/html";
  } else if (filename.endsWith(".css")) {
    retval = "text/css";
  } else if (filename.endsWith(".js")) {
    retval = "application/javascript";
  } else if (filename.endsWith(".ico")) {
    retval = "image/x-icon";
  }
  //retval = "application/octet-stream";
  return retval;
}


//---------------------------------------------------
// HANDLER FOR /index
//---------------------------------------------------
void WEB_SERVER::get_index(void) {
  String tmp;
  String _WSpg;
  _WSpg.reserve(4608);  // 4540

  WebSrvrMsgPrintln(WST_INDEX);

  if (wsindex_type == PosT) {
    // if set focuser position
    String tmp = _web_server->arg("setpos");
    if (tmp != "") {
      // get new position value from text field pos
      // set new position only: this is NOT a move
      String fp = _web_server->arg("pos");
      if (fp != "") {
        long tp = 0;
        long maxp = ControllerData->get_maxstep();
        tp = fp.toInt();
        // range check the new position
        tp = (tp < 0) ? 0 : tp;
        ftargetPosition = (tp > maxp) ? maxp : tp;
        driverboard->setposition(ftargetPosition);
        ControllerData->set_fposition(ftargetPosition);
        goto Get_Handler;
      }
    }

    // if goto focuser position
    tmp = _web_server->arg("gotopos");
    if (tmp != "") {
      String fp = _web_server->arg("pos");
      if (fp != "") {
        long tp = (long)fp.toInt();
        // range check the new position
        RangeCheck(&tp, 0L, ControllerData->get_maxstep());
        ftargetPosition = tp;
      }
      goto Get_Handler;
    }

    // if update of maxsteps
    tmp = _web_server->arg("setmax");
    if (tmp != "") {
      String newmaxpos = _web_server->arg("max");
      if (newmaxpos != "") {
        long cpos = driverboard->getposition();
        long tp = newmaxpos.toInt();
        RangeCheck(&tp, cpos, FOCUSERUPPERLIMIT);
        ControllerData->set_maxstep(tp);
      }
      goto Get_Handler;
    }

    // if update Temperature Unit C/F
    tmp = _web_server->arg("tem");
    if (tmp != "") {
      if (tmp == "ce") {
        ControllerData->set_tempmode(CELSIUS);
      } else if (tmp == "fa") {
        ControllerData->set_tempmode(FAHRENHEIT);
      }
      goto Get_Handler;
    }

    // if update coilpower
    tmp = _web_server->arg("cpr");
    if (tmp != "") {
      if (tmp == "on") {
        ControllerData->set_coilpower_enable(STATE_ENABLED);
        driverboard->enablemotor();
      } else if (tmp == "off") {
        ControllerData->set_coilpower_enable(STATE_DISABLED);
        driverboard->releasemotor();
      }
      goto Get_Handler;
    }

    // if update motorspeed
    tmp = _web_server->arg("ms");
    if (tmp != "") {
      int mspd = tmp.toInt();
      RangeCheck(&mspd, SLOW, FAST);
      ControllerData->set_motorspeed(mspd);
      goto Get_Handler;
    }

    // if update reverse direction
    tmp = _web_server->arg("rd");
    if (tmp != "") {
      if (tmp == "on") {
        ControllerData->set_reverse_enable(STATE_ENABLED);
      } else if (tmp == "off") {
        ControllerData->set_reverse_enable(STATE_DISABLED);
      }
      goto Get_Handler;
    }

    // if a HALT request
    tmp = _web_server->arg("ha");
    if (tmp != "") {
      halt_alert = true;
      ftargetPosition = driverboard->getposition();
      isMoving = false;
      goto Get_Handler;
    }
  }

Get_Handler:

  File file = LittleFS.open("/index.html", "r");
  if (!file) {
    // index page not found
    _WSpg = H_FILENOTFOUNDSTR;
  } else {
    // index page found, send to client
    _WSpg = file.readString();
    file.close();

    // Web page colors
    _WSpg.replace("%PGT%", DeviceName);
    _WSpg.replace("%TXC%", TextColor);
    _WSpg.replace("%BKC%", BackColor);
    _WSpg.replace("%HEC%", HeaderColor);
    _WSpg.replace("%TIC%", TitleColor);
    _WSpg.replace("%STC%", SubTitleColor);

    // Current Position
    _WSpg.replace("%CPO%", String(driverboard->getposition()));

    // Target is a special case, we need to fill this in,
    // When a user clicks goto, then the index page is displayed
    // again and we need to pick up the target position and use
    // XHTML to update both current and target positions on the page
    // Check if prior command was a GOTO command

    tmp = _web_server->arg("gotopos");
    if (tmp != "") {
      _WSpg.replace("%POSI%", String(driverboard->getposition()));
      _WSpg.replace("%TAR%", String(ftargetPosition));
    } else {
      _WSpg.replace("%POSI%", String(ftargetPosition));
      _WSpg.replace("%TAR%", String(driverboard->getposition()));
    }

    // maxstep
    String str = String(ControllerData->get_maxstep());
    _WSpg.replace("%mnum%", str);

    // isMoving
    if (isMoving) {
      _WSpg.replace("%MOV%", T_TRUE);
    } else {
      _WSpg.replace("%MOV%", T_FALSE);
    }

    // Halt button
    // inline

    // temperature mode, celsius or fahrenheit
    if (ControllerData->get_tempmode() == CELSIUS) {
      String tpstr = String(temp, 2);
      _WSpg.replace("%TEM%", tpstr);
      _WSpg.replace("%TUN%", "C");
      // TM  -> ce or fa
      // TMB -> C or F
      _WSpg.replace("%TM%", "fa");
      _WSpg.replace("%TMB%", "F");
    } else {
      float ft = temp;
      ft = (ft * 1.8) + 32;
      String tpstr = String(ft, 2);
      _WSpg.replace("%TEM%", tpstr);
      _WSpg.replace("%TUN%", "F");
      _WSpg.replace("%TM%", "ce");
      _WSpg.replace("%TMB%", "C");
    }

    // coil power  %CPWR% = on off, %CPB% ENABLE DISABLE
    if (ControllerData->get_coilpower_enable() == STATE_ENABLED) {
      // state = On
      _WSpg.replace("%CPS%", T_ENABLED);
      _WSpg.replace("%CPWR%", "off");
      _WSpg.replace("%CPB%", TUC_DISABLE);
    } else {
      // state = Off
      _WSpg.replace("%CPS%", T_DISABLED);
      _WSpg.replace("%CPWR%", "on");
      _WSpg.replace("%CPB%", TUC_ENABLE);
    }

    // motorspeed
    switch (ControllerData->get_motorspeed()) {
      case 0:
        _WSpg.replace("%MSS%", T_CHECKED);
        _WSpg.replace("%MSM%", T_SPACE);
        _WSpg.replace("%MSF%", T_SPACE);
        break;
      case 1:
        _WSpg.replace("%MSS%", T_SPACE);
        _WSpg.replace("%MSM%", T_CHECKED);
        _WSpg.replace("%MSF%", T_SPACE);
        break;
      case 2:
        _WSpg.replace("%MSS%", T_SPACE);
        _WSpg.replace("%MSM%", T_SPACE);
        _WSpg.replace("%MSF%", T_CHECKED);
        break;
      default:
        _WSpg.replace("%MSS%", T_SPACE);
        _WSpg.replace("%MSM%", T_SPACE);
        _WSpg.replace("%MSF%", T_CHECKED);
        break;
    }

    // reverse direction  state %RDS%
    // for %RDO% on|off button %RDB% ENABLE|DISABLE
    if (ControllerData->get_reverse_enable() == STATE_ENABLED) {
      _WSpg.replace("%RDS%", T_ENABLED);
      _WSpg.replace("%RDO%", "off");
      _WSpg.replace("%RDB%", TUC_DISABLE);
    } else {
      _WSpg.replace("%RDS%", T_DISABLED);
      _WSpg.replace("%RDO%", "on");
      _WSpg.replace("%RDB%", TUC_ENABLE);
    }

    _WSpg.replace("%NAM%", ControllerData->get_brdname());
    _WSpg.replace("%VER%", String(major_version));
    _WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    _WSpg.replace("%SUT%", systemuptime);
  }

  WebSrvrMsgPrint(WST_INDEX);
  WebSrvrMsgPrintln(_WSpg.length());
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, _WSpg);
}


//---------------------------------------------------
// HANDLER FOR /move
//---------------------------------------------------
void WEB_SERVER::get_move(void) {
  String _WSpg;
  _WSpg.reserve(4864);  // 4765

  WebSrvrMsgPrintln(WST_MOVE);

  if (wsmove_type == PosT) {
    // if focuser is moving then cannot change values
    if (isMoving == true) {
      goto Get_Handler;
    }

    // if a HALT request
    if (_web_server->arg("ha") != "") {
      halt_alert = true;
      ftargetPosition = driverboard->getposition();
      isMoving = false;
      goto Get_Handler;
    }

    long pos = 0;

    // Check the move buttons
    String mv = _web_server->arg("mvl500");
    WebSrvrMsgPrint("/move : button: ");
    WebSrvrMsgPrintln(mv);
    if (_web_server->arg("mvl500") != "") {
      pos = -500;
    } else if (_web_server->arg("mvl100") != "") {
      pos = -100;
    } else if (_web_server->arg("mvl10") != "") {
      pos = -10;
    } else if (_web_server->arg("mvl1") != "") {
      pos = -1;
    } else if (_web_server->arg("mvp1") != "") {
      pos = 1;
    } else if (_web_server->arg("mvp10") != "") {
      pos = 10;
    } else if (_web_server->arg("mvp100") != "") {
      pos = 100;
    } else if (_web_server->arg("mvp500") != "") {
      pos = 500;
    }

    if (pos != 0) {
      // a move button was pressed, so now process the move
      WebSrvrMsgPrintln(T_MOVETO);
      WebSrvrMsgPrintln(pos);
      // get current focuser position
      long curpos = driverboard->getposition();
      // get maxsteps
      long maxpos = ControllerData->get_maxstep();
      // calculate target
      long target = curpos + pos;
      // range check target position
      RangeCheck(&target, 0, maxpos);
      WebSrvrMsgPrint(T_TARGET);
      WebSrvrMsgPrintln(target);
      // apply the move
      ftargetPosition = target;
      goto Get_Handler;
    }

    // so it was not a move button. check for goto
    String fp = _web_server->arg("pos");
    if (fp != "") {
      long tp = 0;
      long max = (long)ControllerData->get_maxstep();
      tp = fp.toInt();
      RangeCheck(&tp, 0, max);
      WebSrvrMsgPrint(T_GOTO);
      WebSrvrMsgPrintln(tp);
      // apply the move
      ftargetPosition = tp;
      goto Get_Handler;
    }
  }  // end of move_post

Get_Handler:

  File file = LittleFS.open("/move.html", "r");
  if (!file) {
    // move page not found
    _WSpg = H_FILENOTFOUNDSTR;
  } else {
    // move page found, send to client
    _WSpg = file.readString();
    file.close();

    // Web page colors
    _WSpg.replace("%PGT%", DeviceName);
    _WSpg.replace("%TXC%", TextColor);
    _WSpg.replace("%BKC%", BackColor);
    _WSpg.replace("%TIC%", TitleColor);
    _WSpg.replace("%HEC%", HeaderColor);
    _WSpg.replace("%STC%", SubTitleColor);

    String pos = String(driverboard->getposition());

    _WSpg.replace("%CPO%", pos);
    _WSpg.replace("%TAR%", String(ftargetPosition));
    if (isMoving == true) {
      _WSpg.replace("%MOV%", T_TRUE);
    } else {
      _WSpg.replace("%MOV%", T_FALSE);
    }

    // now the buttons -500 to +500, each button is its own form

    // position and goto position button
    // Position [value span id POS2 %CP%] Input Field %PI%
    // button %BP%
    _WSpg.replace("%CP%", pos);

    // halt button, inline html

    _WSpg.replace("%NAM%", ControllerData->get_brdname());
    _WSpg.replace("%VER%", String(major_version));
    _WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    _WSpg.replace("%SUT%", systemuptime);
  }
  WebSrvrMsgPrint(WST_MOVE);
  WebSrvrMsgPrintln(_WSpg.length());
  send_myheader();
  send_mycontent(_WSpg);
  //_web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, _WSpg);
}

//---------------------------------------------------
// GET NOTFOUND AND SEND TO WEB CLIENT
//---------------------------------------------------
void WEB_SERVER::get_notfound(void) {
  String _WSpg;
  _WSpg.reserve(3456);  // 3341

  // can we get server args to determine the filename?
  String p = _web_server->uri();
  WebSrvrMsgPrint(WST_NOTFOUND);
  WebSrvrMsgPrintln(p);

  // get the MIME type
  String contenttype = get_contenttype(p);
  WebSrvrMsgPrint("ContentType=");
  WebSrvrMsgPrintln(contenttype);

  if (p == "/favicon.ico") {
    File ifile = LittleFS.open("/favicon.ico", "r");
    WebSrvrMsgPrintln("notfound: send favicon.ico");
    _web_server->sendHeader("Content-Disposition", "attachment; filename=favicon.ico;");
    _web_server->streamFile(ifile, "image/x-icon");
    WebSrvrMsgPrintln("notfound: favicon.ico sent");
  }

  // not found
  File nfile = LittleFS.open("/notfound.html", "r");
  if (!nfile) {
    _WSpg = H_FILENOTFOUNDSTR;
  } else {
    _WSpg = nfile.readString();
    nfile.close();
    // Web page colors
    _WSpg.replace("%PGT%", DeviceName);
    _WSpg.replace("%TXC%", TextColor);
    _WSpg.replace("%BKC%", BackColor);
    _WSpg.replace("%TIC%", TitleColor);
    _WSpg.replace("%HEC%", HeaderColor);
    _WSpg.replace("%STC%", SubTitleColor);

    _WSpg.replace("%IP%", ipStr);
    _WSpg.replace("%POR%", String(WEBSERVERPORT));

    _WSpg.replace("%NAM%", ControllerData->get_brdname());
    _WSpg.replace("%VER%", String(major_version));
    _WSpg.replace("%HEA%", String(ESP.getFreeHeap()));
    get_systemuptime();
    _WSpg.replace("%SUT%", systemuptime);

    WebSrvrMsgPrint(WST_NOTFOUND);
    WebSrvrMsgPrintln(_WSpg.length());
    _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, _WSpg);
    return;
  }
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, _WSpg);
  return;
}


//---------------------------------------------------
// get position and send to web client
// xhtml
//---------------------------------------------------
void WEB_SERVER::get_position() {
  // Send position value only to client ajax request
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, String(driverboard->getposition()));
}

//---------------------------------------------------
// get ismoving and send to web client
// xhtml
//---------------------------------------------------
void WEB_SERVER::get_ismoving() {
  // Send isMoving value only to client ajax request
  if (isMoving == true) {
    _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, T_TRUE);
  } else {
    _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, T_FALSE);
  }
}

//---------------------------------------------------
// get target position and send to web client
// xhtml
//---------------------------------------------------
void WEB_SERVER::get_targetposition() {
  // Send targetPosition value only to client ajax request
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, String(ftargetPosition));
}

//---------------------------------------------------
// get temperature and send to web client
// xhtml
//---------------------------------------------------
void WEB_SERVER::get_temperature() {
  // Send temperature value only to client ajax request
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, String(temp, 2));
}

//---------------------------------------------------
// get coil power state and send to web client
// xhtml
//---------------------------------------------------
void WEB_SERVER::get_coilpower() {
  // Send coil power enabled state to client ajax request
  if (ControllerData->get_coilpower_enable() == true) {
    _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, T_ENABLED);
  } else {
    _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, T_DISABLED);
  }
}

//---------------------------------------------------
// GET HEAP AND SEND TO WEB CLIENT
// xhtml
//---------------------------------------------------
void WEB_SERVER::get_heap() {
  // Send heap to client ajax request
  _web_server->send(HTML_WEBPAGE, TEXTPAGETYPE, String(ESP.getFreeHeap()));
}

// -------------------------------------------------------
// GET SYSTEM UPDATE TIME SEND TO CLIENT
// xhtml
// -------------------------------------------------------
void WEB_SERVER::get_sut() {
  // Send value only to client ajax request
  get_systemuptime();
  _web_server->send(HTML_WEBPAGE, PLAINTEXTPAGETYPE, systemuptime);
}


#endif
