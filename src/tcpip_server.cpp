// -------------------------------------------------------
// myFP2ESP8266 TCP/IP SERVER CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// tcpip_server.cpp
// Optional
// For ASCOM client via myFP2ESPASOM driver
// For Windows and Linux applications
// For INDI clients via myFP2 INDI driver using TCP/IP 
//    (no serial support)
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"
#if defined(ENABLE_TCPIPSERVER)
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiServer.h>


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable TCPIP Server messages 
// to be written to Serial port
//#define TCPIPSERVER_MsgPrint 1

#ifdef TCPIPSERVER_MsgPrint
#define TCPIPSrvr_MsgPrint(...) Serial.print(__VA_ARGS__)
#define TCPIPSrvr_MsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define TCPIPSrvr_MsgPrint(...)
#define TCPIPSrvr_MsgPrintln(...)
#endif


// -------------------------------------------------------
// EXTERN CLASSES
// -------------------------------------------------------
// ControllerData
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// Driver board
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;

// MANAGEMENT server
#include "management_server.h"
extern MANAGEMENT_SERVER *mngsrvr;

#include "tcpip_server.h"


// -------------------------------------------------------
// EXTERN METHODS
// -------------------------------------------------------
extern void display_off();
extern void display_on();
extern bool start_alpacaserver(void);
extern void stop_alpacaserver(void);
extern bool start_webserver();
extern void stop_webserver(void);


// -------------------------------------------------------
// EXTERNS SETTINGS
// -------------------------------------------------------
extern int _display_type;


// -------------------------------------------------------
// CLASS: TCPIP Server
// -------------------------------------------------------
TCPIP_SERVER::TCPIP_SERVER() {
}


// -------------------------------------------------------
// CREATE AND START THE TCP/IP SERVER
// -------------------------------------------------------
bool TCPIP_SERVER::start() {
  TCPIPSrvr_MsgPrint(T_TCPIPSERVER);
  TCPIPSrvr_MsgPrintln(T_START);
  // if the server is not enabled then return
  if (ControllerData->get_tcpipsrvr_enable() == STATE_DISABLED) {
    TCPIPSrvr_MsgPrintln(T_DISABLED);
    return false;
  }

  // prevent any attempt to start if already started
  if (_loaded == STATE_LOADED) {
    return true;
  }

  _myserver = new WiFiServer(TCPIPSERVERPORT);

  _myserver->begin(TCPIPSERVERPORT);
  _loaded = STATE_LOADED;
  _status = STATUS_RUNNING;
  _clientstatus = STATE_NOTCONNECTED;
  return _loaded;
}

// -------------------------------------------------------
// STOP THE TCP/IP SERVER
// This will stop and delete _myserver
// This must be done because start() creates _myserver
// -------------------------------------------------------
void TCPIP_SERVER::stop(void) {
  // can only stop a server that is _loaded
  if (_loaded == STATE_LOADED) {
    if (_clientstatus) {
      // stop a connected _myclient, drop resources
      _myclient->abort();
      _clientstatus = STATE_NOTCONNECTED;
    }

    // stop server
    _myserver->stop();
    delete _myserver;
    _loaded = STATE_NOTLOADED;
    _status = STATUS_STOPPED;
  }
}

// -------------------------------------------------------
// CHECKS FOR ANY NEW CLIENTS OR EXISTING CLIENT REQUESTS
// avoid using TCPIPSrvr_MsgPrint or debug code
// -------------------------------------------------------
bool TCPIP_SERVER::loop(bool pds) {
  static bool _clientstate = STATE_NOTCONNECTED;
  _pwrdwn_status = pds;

  // avoid a crash
  if (_loaded == STATE_NOTLOADED) {
    return false;
  }

  if (_clientstate) {
    if (_myclient->connected()) {
      // if client has send request
      while (_myclient->available()) {
        // process request
        process_command();
      }
      return true;
    } else {
      // not connected, stop client
      _myclient->abort();
      _myclient = NULL;
      _clientstate = STATE_NOTCONNECTED;
      return false;
    }
  }

  // check if any new connection
  WiFiClient newclient = _myserver->accept();
  if (newclient) {
    _clientstate = STATE_CONNECTED;
    // save new client
    _myclient = new WiFiClient(newclient);
    // Get IP of client
    _myclientIPAddress = newclient.remoteIP();
    snprintf(_clientipStr, sizeof(_clientipStr), "%i.%i.%i.%i", _myclientIPAddress[0], _myclientIPAddress[1], _myclientIPAddress[2], _myclientIPAddress[3]);
    return true;
  }
  return false;
}

// -------------------------------------------------------
// SEND REPLY TO CLIENT
// -------------------------------------------------------
void TCPIP_SERVER::send_reply(const char *str) {
  // if client is still connected
  if (_myclient->connected()) {
    // send reply
    _myclient->print(str);
  }
}

// -------------------------------------------------------
// Build a bool reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, bool state) {
  char buff[BUFFER32LEN];
  if (state == false) {
    snprintf(buff, sizeof(buff), "%c%i%c", token, 0, _EOC);
  } else {
    snprintf(buff, sizeof(buff), "%c%i%c", token, 1, _EOC);
  }
  send_reply(buff);
}

// -------------------------------------------------------
// Build a char array reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, const char *str) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%s%c", token, str, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// Build a unsigned char reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, unsigned char data_val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%u%c", token, data_val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// Build a float reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, const float data_val, int decimalplaces) {
  char buff[BUFFER32LEN];
  String tmpstr = String(data_val, decimalplaces);  // Eric T, Nov 2024
  snprintf(buff, sizeof(buff), "%c%s%c", token, tmpstr, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// Build a integer reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, int data_val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%i%c", token, data_val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// Build a string reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, String str) {
  char buff[BUFFER32LEN];
  char tmp[30];
  str.toCharArray(tmp, str.length());
  snprintf(buff, str.length() + 1, "%c%s%c", token, tmp, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// Build a long reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, long data_val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%ld%c", token, data_val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// Build a unsigned long reply to a client
// -------------------------------------------------------
void TCPIP_SERVER::build_reply(const char token, unsigned long data_val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%lu%c", token, data_val, _EOC);
  send_reply(buff);
}


// -------------------------------------------------------
// PROCESS A CLIENT COMMAND REQUEST
// -------------------------------------------------------
void TCPIP_SERVER::process_command() {
  String receiveString = "";
  String WorkString = "";
  byte bval;
  int cmdvalue;
  int ival;
  long lval;

  String drvbrd = ControllerData->get_brdname();
  receiveString = _myclient->readStringUntil(_EOC);
  receiveString = receiveString + '#' + "";

  TCPIPSrvr_MsgPrint("receiveString = ");
  TCPIPSrvr_MsgPrintln(receiveString);

  String cmdstr = receiveString.substring(1, 3);
  TCPIPSrvr_MsgPrint("cmdstr = ");
  TCPIPSrvr_MsgPrintln(cmdstr);

  if (cmdstr[0] == 'A') {
    cmdvalue = 100 + (cmdstr[1] - '0');  // can only use digits A0-A9
  } else if (cmdstr[0] == 'B') {
    cmdvalue = 110 + (cmdstr[1] - '0');  // can only use digits B0-B9
  } else if (cmdstr[0] == 'C') {
    cmdvalue = 120 + (cmdstr[1] - '0');  // can only use digits C0-C9
  } else {
    cmdvalue = cmdstr.toInt();
  }

  WorkString = receiveString.substring(3, receiveString.length() - 1);
  TCPIPSrvr_MsgPrint("1: WorkString = ");
  TCPIPSrvr_MsgPrintln(WorkString);

  switch (cmdvalue) {
    case 0:
      // Get focuser position
      build_reply('P', driverboard->getposition());
      break;

    case 1:
      // ismoving
      build_reply('I', isMoving);
      break;

    case 2:
      // Get controller status
      build_reply('E', "OK");
      break;

    case 3:
      // Get firmware version
      build_reply('F', major_version);
      break;

    case 4:
      // Get get_brdname + version number
      {
        char buff[BUFFER32LEN];
        char tempstr[20];
        String brdname = ControllerData->get_brdname();
        brdname.toCharArray(tempstr, brdname.length() + 1);
        snprintf(buff, sizeof(buff), "%s%c%c%s", tempstr, '\r', '\n', major_version);
        build_reply('F', buff);
      }
      break;

    case 5:
      // Set new target position to xxxxxx (and focuser initiates
      // immediate move to xxxxxx)
      // only if not already moving
      if (isMoving == false) {
        ftargetPosition = WorkString.toInt();
        RangeCheck(&ftargetPosition, 0L, ControllerData->get_maxstep());
        isMoving = true;
      }
      break;

    case 6:
      // Get Temperature
      build_reply('Z', temp, 3);
      break;

    case 7:
      // Set maxSteps
      lval = (long) WorkString.toInt();
      RangeCheck(&lval, driverboard->getposition() + 1, FOCUSERUPPERLIMIT);
      // check new maxStep against focuser position
      ControllerData->set_maxstep(lval);
      break;

    case 8:
      // Get maxStep
      build_reply('M', ControllerData->get_maxstep());
      break;

    case 9:
      // Get _inoutledmode, pulse or move
      // not supported on esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 10:
      // Get maxIncrement
      build_reply('Y', ControllerData->get_maxstep());
      break;

    case 11:
      // Get coil power
      build_reply('O', ControllerData->get_coilpower_enable());
      break;

    case 12:
      // Set coil power enable
      ival = WorkString.toInt();
      if (ival == 0) {
        ControllerData->set_coilpower_enable(STATE_DISABLED);
        driverboard->releasemotor();
      } else {
        ControllerData->set_coilpower_enable(STATE_ENABLED);
        driverboard->enablemotor();
      }
      break;

    case 13:
      // Get reverse direction setting, 00 off, 01 on
      build_reply('R', ControllerData->get_reverse_enable());
      break;

    case 14:
      // Set reverse direction
      if (isMoving == false) {
        ival = WorkString.toInt();
        if (ival == 0) {
          ControllerData->set_reverse_enable(STATE_DISABLED);
        } else {
          ControllerData->set_reverse_enable(STATE_ENABLED);
        }
      }
      break;

    case 15:
      // Set motor speed
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 2);
      ControllerData->set_motorspeed((byte)ival);
      break;

    case 16:
      // Set display to celsius
      // temperature display mode, Celsius=1, Fahrenheit=0
      ControllerData->set_tempmode(CELSIUS);
      break;

    case 17:
      // Set display to fahrenheit
      // temperature display mode, Celsius=1, Fahrenheit=0
      ControllerData->set_tempmode(FAHRENHEIT);
      break;

    case 18:
      // Set Stepsize Enable state
      // not supported on esp8266 - always enabled
      break;

    case 19:
      // Set the step size value - double type, eg 2.1
      {
        float tempstepsize = WorkString.toFloat();
        RangeCheck(&tempstepsize, MINIMUMSTEPSIZE, MAXIMUMSTEPSIZE);
        ControllerData->set_stepsize(tempstepsize);
      }
      break;

    case 20:
      // Set the temperature resolution for temperature probe
      // ignored for esp8266
      break;

    case 21:
      // Get temp probe resolution
      build_reply('Q', DEFAULTTEMPRESOLUTION);
      break;

    case 22:
      // Set temperature coefficient steps value to xxx
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 400);
      ControllerData->set_tempcoefficient(ival);
      break;

    case 23:
      // Set enable tempcomp
      ival = WorkString.toInt();
      if (tempcomp_available == STATE_ENABLED) {
        (ival == 1) ? tempcomp_state = STATE_ENABLED : tempcomp_state = STATE_DISABLED;
      } else {
        tempcomp_state = STATE_DISABLED;
        tempcomp_available = STATE_DISABLED;
      }
      break;

    case 24:
      // Get status of temperature compensation (enabled | disabled)
      build_reply('1', tempcomp_state);
      break;

    case 25:
      // Get temperature compensation available
      build_reply('A', tempcomp_available);
      break;

    case 26:
      // Get temperature coefficient steps/degree
      build_reply('B', ControllerData->get_tempcoefficient());
      break;

    case 27:
      // stop a move - like a Halt
      halt_alert = true;
      break;

    case 28:
      // home the motor to position 0
      if (isMoving == false) {
        ftargetPosition = 0;
      }
      break;

    case 29:
      // Get stepmode
      build_reply('S', ControllerData->get_brdstepmode());
      break;

    // -------------------------------------------------------
    // Basic rule for setting stepmode
    // DRIVER_BOARD->setstepmode(xx); // sets physical pins
    // ControllerData->set_brdstepmode(xx);  // saves stepmode
    case 30:
      // Set step mode
      {
        ival = WorkString.toInt();
        int brdnum = ControllerData->get_brdnumber();
        if (brdnum == PRO2EULN2003 || brdnum == PRO2EL298N || brdnum == PRO2EL293DMINI || brdnum == PRO2EL9110S) {
          RangeCheck(&ival, 1, 2);
        } else if (brdnum == WEMOSDRV8825 || brdnum == PRO2EDRV8825) {
          // stepmopde is set by jumpers
          ival = (int)ControllerData->get_brdfixedstepmode();
        } else if (brdnum == PRO2EL293DNEMA || brdnum == PRO2EL293D28BYJ48) {
          ival = STEP1;
        }
      }
      ControllerData->set_brdstepmode((int)ival);
      driverboard->setstepmode((int)ival);
      break;

    case 31:
      // Set focuser position
      if (isMoving == false) {
        lval = (long) WorkString.toInt();
        RangeCheck(&lval, 0L, ControllerData->get_maxstep());
        ftargetPosition = lval;
        driverboard->setposition(lval);
        ControllerData->set_fposition(lval);
      }
      break;

    case 32:
      // Get if stepsize is enabled
      // esp8266 always enabled
      build_reply('U', 1);
      break;

    case 33:
      // Get stepsize
      build_reply('T', ControllerData->get_stepsize(), 2);
      break;

    case 34:
      // Get the time that a display page is shown for
      build_reply('X', DISPLAYPAGETIME);
      break;

    case 35:
      // Set display page time, in seconds, integer, 2-10
      // not supported on esp8266
      break;

    case 36:
      // :360# Disable Display
      // :361# Enable Display
      ival = WorkString.toInt();
      if (ival == 1) {
        ControllerData->set_display_enable(STATE_ENABLED);
      } else {
        ControllerData->set_display_enable(STATE_DISABLED);
      }
      if (display_found == true) {
        (ControllerData->get_display_enable() == true) ? display_on() : display_off();
      }
      break;

    case 37:
      // Get display enable status
      build_reply('D', ControllerData->get_display_enable());
      break;

    case 38:
      // Get temperature mode 1=Celsius, 0=Fahrenheight
      build_reply('b', ControllerData->get_tempmode());
      break;

    case 39:
      // Get the new motor position (target) XXXXXX
      build_reply('N', ftargetPosition);
      break;

    case 40:
      // reboot controller with 2s delay
      software_Reboot(2000);
      break;

    case 41:
      // Set in-out-led-mode (pulsed or move)
      // not supported on esp8266
      break;

    case 42:
      // reset focuser defaults
      if (isMoving == false) {
        ControllerData->SetFocuserDefaults();
        ftargetPosition = ControllerData->get_fposition();
        driverboard->setposition(ftargetPosition);
        ControllerData->set_fposition(ftargetPosition);
      }
      break;

    case 43:
      // Get motorspeed
      build_reply('C', ControllerData->get_motorspeed());
      break;

    case 44:
      // Get Powerdown enable state
      build_reply(_RTOKEN, ControllerData->get_powerdown_enable());
      break;

    case 45:
      // Set PowerDown enable state
      ival = WorkString.toInt();
      if (ival == 0) {
        ControllerData->set_powerdown_enable(false);
      } else {
        ControllerData->set_powerdown_enable(true);
      }
      break;

    case 46:
      // Get in-out led enable state
      // not supported on esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 47:
      // Set in-out led enable state
      // not supported on esp8266
      break;

    case 48:
      // save settings to file
      // do not do this if focuser is moving
      if (isMoving == false) {
        // need to save position setting
        ControllerData->set_fposition(driverboard->getposition());
        // save the focuser settings immediately
        ControllerData->SaveNow(driverboard->getposition(), driverboard->getdirection());
      }
      break;

    case 49:
      // aXXXXX
      build_reply('a', "b552efd");
      break;

    case 50:
      // Get if Home Position Switch enabled, 0 = no, 1 = yes
      // not supported on esp8266
      build_reply('l', 0);
      break;

    case 51:
      // Get Wifi Controller IP Address
      build_reply(_RTOKEN, ipStr);
      break;

    case 52:
      // Get PowerDown state
      build_reply(_RTOKEN, _pwrdwn_status);
      break;

    case 53:
      // Get number of display pages
      // deprecated
      build_reply(_RTOKEN, 0);
      break;

    case 54:
      // Controller SSID
      build_reply(_RTOKEN, mySSID);
      break;

    case 55:
      // Get motorspeed delay for current speed setting
      build_reply('0', ControllerData->get_brdmsdelay());
      break;

    case 56:
      // Set motorspeed delay
      lval = WorkString.toInt();
      RangeCheck(&ival, DEFAULT_MOTORSPEEDDELAYMIN, DEFAULT_MOTORSPEEDDELAYMAX);
      ControllerData->set_brdmsdelay(ival);
      break;

    case 57:
      // Get pushbutton enable state
      // not supported on esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 58:
      // Set pushbutton enable state
      // not supported on esp8266
      break;

    case 59:
      // Get powerdown time
      build_reply(_RTOKEN, ControllerData->get_powerdown_time());
      break;

    case 60:
      // Set powerdown time interval in seconds (30-120)
      ival = WorkString.toInt();
      RangeCheck(&ival, 30, 120);
      ControllerData->set_powerdown_time(ival);
      break;

    case 61:
      // Set update of position on oled when moving (0=disable, 1=enable)
      ival = WorkString.toInt();
      (ival == 0) ? ControllerData->set_display_updateonmove(STATE_DISABLED) : ControllerData->set_display_updateonmove(STATE_ENABLED);
      break;

    case 62:
      // Get update of position on oled when moving (00=disable, 01=enable)
      build_reply('L', ControllerData->get_display_updateonmove());
      break;

    case 63:
      // Get status of home position switch
      // not supported on esp8266
      build_reply('H', 0);
      break;

    case 64:
      // move a specified number of steps
      if (isMoving == false) {
        lval = WorkString.toInt() + driverboard->getposition();
        RangeCheck(&lval, 0L, ControllerData->get_maxstep());
        ftargetPosition = lval;
        isMoving = true;
      }
      break;

    case 65:
      // Set jogging state enable/disable
      // not supported esp8266
      break;

    case 66:
      // Get jogging state enabled/disabled
      // not supported esp8266
      build_reply('K', 0);
      break;

    case 67:
      // Set jogging direction, 0=IN, 1=OUT
      // not supported esp8266
      break;

    case 68:
      // Get jogging direction, 0=IN, 1=OUT
      // not supported esp8266
      build_reply('V', 0);
      break;

    case 69:
      // Get push button steps
      // not supported on esp8266
      build_reply('?', 1);
      break;

    case 70:
      // Set push buttons steps [1-max] where max = stepsize / 2
      // not supported on esp8266
      break;

    case 71:
      // Set delayaftermove time value in milliseconds [0-250]
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 255);
      ControllerData->set_delayaftermove_time((byte)ival);
      break;

    case 72:
      // Get delayaftermove_status value in milliseconds
      build_reply('3', ControllerData->get_delayaftermove_time());
      break;

    case 73:
      // Set disable/enable backlash IN (going to lower focuser position)
      // not supported on ESP8266
      break;

    case 74:
      // Get backlash in enabled status
      // If backlash steps in > 0
      if (ControllerData->get_backlashsteps_in() > 0) {
        build_reply('4', 1);
      }
      else {
        build_reply('4', 0);
      }      
      break;

    case 75:
      // Set disable/enable backlash OUT (going to higher position)
      // not supported on ESP8266
      break;

    case 76:
      // Get backlash OUT enabled status
      if (ControllerData->get_backlashsteps_out() > 0) {
        build_reply('4', 1);
      }
      else {
        build_reply('4', 0);
      }  
      break;

    case 77:
      // Set backlash in steps [0-255]
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 255);
      ControllerData->set_backlashsteps_in((byte)ival);
      break;

    case 78:
      // Get backlash steps IN
      build_reply('6', ControllerData->get_backlashsteps_in());
      break;

    case 79:
      // Set backlash OUT steps
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 255);
      ControllerData->set_backlashsteps_out((byte)ival);
      break;

    case 80:
      // Get backlash steps OUT
      build_reply('7', ControllerData->get_backlashsteps_out());
      break;

    case 81:
      // Get STALL_VALUE (for TMC2209 stepper modules)
      // not supported on esp8266
      build_reply('8', 0);
      break;

    case 82:
      // Set STALL_VALUE (for TMC2209 stepper modules)
      // not supported on esp8266
      break;

    case 83:
      // Get if there is a temperature probe
      build_reply('c', tempprobe_found);
      break;

    case 84:
      // myFP2N reserved
      break;

    case 85:
      // Get delay after move enable state
      // esp8266 - if dam-time < 0, then 1 else 0
      if (ControllerData->get_delayaftermove_time() > 0) {
        build_reply(_RTOKEN, 1);
      }
      else {
        build_reply(_RTOKEN, 0);
      }
      break;

    case 86:
      // Set delay after move enable state
      // esp8266 - enabled if dam-time is non 0
      break;

    case 87:
      // Get tc direction
      build_reply('k', ControllerData->get_tcdirection());
      break;

    case 88:
      // Set tc direction
      ival = WorkString.toInt();
      (ival == 0) ? ControllerData->set_tcdirection(STATE_DISABLED) : ControllerData->set_tcdirection(STATE_ENABLED);
      break;

    case 89:
      // Get stepper power (reads from A7) - only valid if hardware circuit is added (1=stepperpower ON)
      // not supported on esp8266
      build_reply('9', 1);
      break;

    case 90:
      // Set focuser preset x [0-9] with position value yyyy [unsigned long]
      // not supported
      break;

    case 91:
      // Get focuser preset [0-9]
      // not supported
      build_reply(_RTOKEN, 0L);
      break;

    case 92:
      // Set Display page display option (6 pgs)
      {
        // If empty (no args) - fill with default display string
        if (WorkString == "") {
          WorkString = "111111";
        }

        // if display option length less than 6
        // then pad with leading 0's
        while (WorkString.length() < 6) {
          WorkString = '0' + WorkString;
        }

        // do not allow display strings that exceed 
        // length of buffer (0-5, 6 digits)
        if (WorkString.length() > 6) {
          WorkString[6] = 0x00;
        }
        ControllerData->set_display_pageoption(WorkString);
      }
      break;

    case 93:
      // Get Display page option
      {
        // return as string of 01's
        char buff[10];
        memset(buff, 0, 10);
        String answer = ControllerData->get_display_pageoption();
        // should be 6 digits (pgs 1-6)
        // copy to buff
        int i;
        for (i = 0; i < (int) answer.length(); i++) {
          buff[i] = answer[i];
        }
        buff[i] = 0x00;
        build_reply('l', buff);
      }
      break;

    case 94:
      // GET TMC Interpolation state (0=disabled, 1-enabled)
      // not supported on esp8266
      build_reply('n', 0);      
      break;

    case 95:
      // SET TMC Interpolation state (0=disabled, 1-enabled)
      // not supported on esp8266
      break;

    case 96:
      // Get firmware Major version, Minor Version
      {
        char buff[BUFFER32LEN];
        snprintf(buff, sizeof(buff), "%s,%s", major_version, minor_version);
        build_reply('F', buff);
      }
      break;

    case 97:
      // Get Display Type, NONE=0, 1=TEXT,  2=LILYGO, 3=GRAPHIC
      build_reply('n', _display_type);
      break;

    case 98:
      // Get network strength dbm
      {
        long rssi = getrssi();
        build_reply(_RTOKEN, rssi);
      }
      break;

    case 99:
      // Set home position switch enable state, 0 or 1, disabled or enabled
      // not supported on esp8266
      break;

    // :A0-A9
    case 100:
      // Get joystick1 enable state
      // not supported on esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 101:
      // Set joystick1 enable state (0=stopped, 1=started)
      // not supported on esp8266
      break;

    case 102:
      // Get joystick2 enable state
      // not supported on esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 103:
      // Set joystick2 enable state (0=stopped, 1=started)
      // not supported on esp8266
      break;

    case 104:
      // Get temp probe enabled state
      build_reply(_RTOKEN, ControllerData->get_tempprobe_enable());
      break;

    case 105:
      // Set temp probe enabled state
      ival = WorkString.toInt();
      (ival == 0) ? ControllerData->set_tempprobe_enable(STATE_DISABLED) : ControllerData->set_tempprobe_enable(STATE_ENABLED);
      break;

    case 106:
      // Get ALPACA Server enabled state
      build_reply(_RTOKEN, ControllerData->get_alpacasrvr_enable());
      break;

    case 107:
      // Set ALPACA Server enabled state
      ival = WorkString.toInt();
      if (ival == 0) {
        ControllerData->set_alpacasrvr_enable(STATE_DISABLED);
        if (alpacasrvr_status) {
          // stop and disable
          stop_alpacaserver();
        }
        alpacasrvr_status = STATUS_STOPPED;
      } else {
        ControllerData->set_alpacasrvr_enable(STATE_ENABLED);
      }
      break;

    case 108:
      // Get ALPACA Server Start/Stop status
      build_reply(_RTOKEN, alpacasrvr_status);
      break;

    case 109:
      // Set ALPACA Server Start/Stop
      ival = WorkString.toInt();
      if (ival == 0) {
        // stop the alpaca server
        if (alpacasrvr_status == STATUS_RUNNING) {
          stop_alpacaserver();
        }
        alpacasrvr_status = STATUS_STOPPED;
      } else {
        // start alpaca server
        if (ControllerData->get_alpacasrvr_enable() == STATE_ENABLED) {
          alpacasrvr_status = start_alpacaserver();
          if (alpacasrvr_status == STATUS_STOPPED) {
            TCPIPSrvr_MsgPrintln("tcp:109:err alpaca!start");
          }
        }
      }
      break;

    // :B0 to :B9
    case 110:
      // Get Web Server enabled state
      build_reply(_RTOKEN, ControllerData->get_websrvr_enable());
      break;

    case 111:
      // Set Web Server enabled state
      ival = WorkString.toInt();
      if (ival == 0) {
        if (websrvr_status == STATUS_RUNNING) {
          stop_webserver();
        }
        websrvr_status = STATUS_STOPPED;
        ControllerData->set_websrvr_enable(STATE_DISABLED);
      } else {
        ControllerData->set_websrvr_enable(STATE_ENABLED);
      }
      break;

    case 112:
      // Get Web Server Start/Stop status
      build_reply(_RTOKEN, websrvr_status);
      break;

    case 113:
      // Set Web Server Start/Stop
      ival = WorkString.toInt();
      if (ival == 0) {
        if (websrvr_status == STATUS_RUNNING) {
          stop_webserver();
        }
        websrvr_status = STATUS_STOPPED;
      } else {
        // start, check enable
        if (ControllerData->get_websrvr_enable() == STATE_ENABLED) {
          // enabled, then start
          websrvr_status = start_webserver();
          if (websrvr_status == STATUS_STOPPED) {
            TCPIPSrvr_MsgPrintln("E:tcp:113:wsrvr!start");
          }
        }
      }
      break;

    case 114:  
      // Get Management Server enabled state
      build_reply(_RTOKEN, ControllerData->get_mngsrvr_enable());
      break;

    case 115:  
      // Set Management Server enabled state
      ival = WorkString.toInt();
      if (ival == 0) {
        if (mngsrvr_status == STATUS_RUNNING) {
          // stop the server first
          mngsrvr->stop();
          mngsrvr_status = STATUS_STOPPED;
        }
        ControllerData->set_mngsrvr_enable(STATE_DISABLED);
      } else {
        ControllerData->set_mngsrvr_enable(STATE_ENABLED);
      }
      break;

    case 116:
      // Get Management Server Start/Stop status
      build_reply(_RTOKEN, mngsrvr_status);
      break;

    case 117:
      // Set  Start/Stop Management Server 
      ival = WorkString.toInt();
      if (ival == 0) {
        if (mngsrvr_status == STATUS_RUNNING) {
          mngsrvr->stop();
        }
        mngsrvr_status = STATUS_STOPPED;
      } else {
        // start
        if (ControllerData->get_mngsrvr_enable() == STATE_ENABLED) {
          mngsrvr_status = mngsrvr->start();
          if (mngsrvr_status == STATUS_STOPPED) {
            TCPIPSrvr_MsgPrintln("E:tcp:117:mngsrvr!start");
          }
        }
      }
      break;

    case 118:
      // Get cntlr_config.jsn
      {
        File dfile = LittleFS.open("/cntlr_config.jsn", "r");
        if (!dfile) {
          build_reply(_RTOKEN, 0);
          return;
        } else {
          String cdata = dfile.readString();
          dfile.close();
          int len = cdata.length();
          char tempstr[len + 2];
          char cd[len + 4];
          cdata.toCharArray(tempstr, cdata.length() + 1);
          snprintf(cd, len + 2, "%c%s%c", _RTOKEN, tempstr, _EOC);
          send_reply(cd);
        }
      }
      break;

    case 119:
      // Get board_config.jsn
      {
        File dfile = LittleFS.open("/board_config.jsn", "r");
        if (!dfile) {
          build_reply(_RTOKEN, 0);
          return;
        } else {
          String cdata = dfile.readString();
          dfile.close();
          int len = cdata.length();
          char tempstr[len + 2];
          char cd[len + 4];
          cdata.toCharArray(tempstr, cdata.length() + 1);
          snprintf(cd, len + 2, "%c%s%c", _RTOKEN, tempstr, _EOC);
          send_reply(cd);
        }
      }
      break;

    // :C0 to :C9    
    case 120:
      // Get OTA State (0=Disabled, 1=Enabled)
      build_reply(_RTOKEN, 0);
      break;
   
    case 121:
      // Set OTA State
      // Not supported on ESP8266
      break;

    case 122:
      // Get OTA Status (0=Stopped, 1=Running)
      build_reply(_RTOKEN, 0);
      break;

    case 123:
      // Set OTA Status (0=Stopped, 1=Running)
      // Not supported on ESP8266
      break;

    case 124:
      // Get brightness level (0=off, 255=max)
      // Not supported on ESP8266
      build_reply(_RTOKEN, 255);
      break;

    case 125:
      // Set OTA Status (0=Stopped, 1=Running)
      // Not supported on ESP8266
      break;

    default:
      TCPIPSrvr_MsgPrint("tcpip cmd err: ");
      TCPIPSrvr_MsgPrintln(cmdvalue);
      break;
  }
}

#endif
