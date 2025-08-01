// -------------------------------------------------------
// myFP2ESP8266 LOCALSERIAL CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// serial_comms.cpp
// Optional Configuration
// For ASCOM client via myFP2ASCOM driver
// For Windows and Linux applications (myFP2 Serial only)
// For INDI clients via myFP2 INDI driver using Serial connection
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// LOCALSERIAL is supported by the below (from myFP2)
//    myFP2ASCOM and myFP2ASCOM1 drivers
//    myFP2ASCOMApp
//    myfp2lusb Linux Application
//    myFP2W and myFP2Mini Application
//    MyFocuserPro2 INDI driver using Serial Port
//    Bluetooth is NOT Supported
//
// Do NOT use LOCALSERIAL with any of the myFP2ESP 
//    myFP2ESP32ASCOMApp
//    myfp2esp32l Linux Application
//    myFP2ESP32W Windows Application
//    MyFocuserPro2 INDI driver using Network
//
// WiFi IS DISABLED WHEN USING LOCALSERIAL
// -------------------------------------------------------


#include <Arduino.h>
#include "config.h"

#if defined(CONTROLLERMODE)
#if (CONTROLLERMODE == LOCALSERIAL)
#include <FS.h>
#include <LittleFS.h>
#include "serial_server.h"

#include "myQueue.h"
extern Queue<String> queue;


// -------------------------------------------------------
// EXTERN METHODS
// -------------------------------------------------------
extern void display_update(long);
extern void display_off(void);
extern void display_on(void);


// -------------------------------------------------------
// EXTERN CLASSES
// -------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// DRIVER BOARD
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;


// -------------------------------------------------------
// CRITICAL: DO NOT ENABLE ANY DEBUG TYPE CODE OR
// Serial.print()
// Serial.println()
// STATEMENTS. APPS WILL DISCONNECT IF YOU DO THIS.
// -------------------------------------------------------


// -------------------------------------------------------
// CRITICAL: DO NOT MESS WITH THIS BIT OF CODE
// -------------------------------------------------------
extern LOCAL_SERIAL *serialsrvr;

extern void serialEventRun(void) {
  serialsrvr->serialEvent();
}


// -------------------------------------------------------
// CLASS CONSTRUCTOR
// -------------------------------------------------------
LOCAL_SERIAL::LOCAL_SERIAL(HardwareSerial &serial)
  : _dev(serial) {
  //_dev = serial;
}

// -------------------------------------------------------
// START LOCAL SERIAL 
// -------------------------------------------------------
void LOCAL_SERIAL::start(uint32_t portspeed) {
  _dev.begin(portspeed);
  delay(600);  // give time for serial port to start
}

// -------------------------------------------------------
// CONVERT FLOAT TO ASCII STRING
// -------------------------------------------------------
char *LOCAL_SERIAL::ftoa(char *a, double f, int precision) {
  const long p[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000 };

  char *ret = a;
  long heiltal = (long)f;
  itoa(heiltal, a, 10);
  while (*a != '\0') a++;
  *a++ = '.';
  long desimal = abs((long)((f - heiltal) * p[precision]));
  itoa(desimal, a, 10);
  return ret;
}


// -------------------------------------------------------
// SEND RESPONSE TO CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::send_reply(const char *str) {
  _dev.print(str);
}

// -------------------------------------------------------
// BUILD A BOOL REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, const bool newstate) {
  char buff[BUFFER32LEN];
  if (newstate == false) {
    snprintf(buff, sizeof(buff), "%c%i%c", token, 0, _EOC);
  } else {
    snprintf(buff, sizeof(buff), "%c%i%c", token, 1, _EOC);
  }
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A CHAR ARRAY REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, const char *str) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%s%c", token, str, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A UNSIGNED CHAR REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, unsigned char data_val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%u%c", token, data_val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A FLOAT REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, float data_val, int decimalplaces) {
  char buff[BUFFER12LEN];
  switch (decimalplaces) {
    case 0:
      snprintf(buff, sizeof(buff), "%c%f%c", token, data_val, _EOC);
      break;
    case 1:
      snprintf(buff, sizeof(buff), "%c%.1f%c", token, data_val, _EOC);
      break;
    case 2:
      snprintf(buff, sizeof(buff), "%c%.2f%c", token, data_val, _EOC);
      break;
    case 3:
      snprintf(buff, sizeof(buff), "%c%.3f%c", token, data_val, _EOC);
      break;
    default:
      snprintf(buff, sizeof(buff), "%c%f%c", token, 2, _EOC);
      break;
  }
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A INTEGER REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, int val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%d%c", token, val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A LONG REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, long val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%lu%c", token, val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A UNSIGNED LONG REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, unsigned long val) {
  char buff[BUFFER32LEN];
  snprintf(buff, sizeof(buff), "%c%lu%c", token, val, _EOC);
  send_reply(buff);
}

// -------------------------------------------------------
// BUILD A STRING REPLY TO A CLIENT
// -------------------------------------------------------
void LOCAL_SERIAL::build_reply(const char token, String str) {
  char buff[BUFFER32LEN];
  char tmp[30];
  str.toCharArray(tmp, str.length());
  snprintf(buff, str.length() + 1, "%c%s%c", token, tmp, _EOC);
  send_reply(buff);
}


// -------------------------------------------------------
// HANDLER FOR SERIAL COMMANDS
// DO NOT UNCOMMENT ANY LINES STARTING //Serial.
// GET commands do not return any response
// -------------------------------------------------------
void LOCAL_SERIAL::process_cmd(bool PowerDownStatus) {
  String receiveString = "";
  String WorkString = "";
  byte bval;
  int cmdval;
  int ival;
  long lvar;
  float fval;

  if (queue.count() == 0) {
    return;
  }

  receiveString = (String)queue.pop();
  //Serial.println();
  //Serial.print("process_cmd: receiveString = ");
  //Serial.println(receiveString);

  String cmdstr = receiveString.substring(0, 2);

  if (receiveString[0] == 'A') {
    cmdval = 100 + (receiveString[1] - '0');  // only use digits A0-A9
  } else if (receiveString[0] == 'B') {
    cmdval = 110 + (receiveString[1] - '0');  // only use digits B0-B9
  } else if (receiveString[0] == 'C') {
    cmdval = 120 + (receiveString[1] - '0');  // only use digits C0-C9
  } else {
    cmdval = cmdstr.toInt();
  }
  //Serial.print("process_cmd: cmd = ");
  //Serial.println(cmdval);

  WorkString = receiveString.substring(2, receiveString.length());
  //Serial.print("WorkString = ");
  //Serial.println(WorkString);

  switch (cmdval) {
    // all commands are in this switch()
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
      // Get firmware version, also used by INDI
      build_reply('F', major_version);
      break;

    case 4:
      // Get board name and major version: Safe, INDI does not use
      {
        char buff[32];
        char tempstr[20];
        String brdname = ControllerData->get_brdname();
        brdname.toCharArray(tempstr, brdname.length() + 1);
        snprintf(buff, sizeof(buff), "%s%c%c%s", tempstr, '\r', '\n', major_version);
        build_reply('F', buff);
      }
      break;

    case 5:
      // :05xxxxxx# Set new target position to xxxxxx
      // (and focuser initiates immediate move to xxxxxx)
      // only if not already moving
      if (isMoving == false) {
        lvar = WorkString.toInt();
        //Serial.print("ss: target: ");
        //Serial.println(lvar);
        //Serial.println("Rangecheck");
        lvar = (lvar < 0) ? 0 : lvar;
        lvar = (lvar > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : lvar;
        //Serial.println("After Rangecheck");
        //Serial.print("ss: target: ");
        //Serial.println(lvar);  

        ftargetPosition = lvar;
        isMoving = true;
        delay(5);
      }
      break;

    case 6:
      // Get temperature
      build_reply('Z', temp, 3);
      break;

    case 7:  // Set maxStep
      lvar = (long) WorkString.toInt();
      RangeCheck(&lvar, driverboard->getposition() + 1, FOCUSERUPPERLIMIT);
      ControllerData->set_maxstep(lvar);
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
      // Get Coilpower state
      build_reply('O', ControllerData->get_coilpower_enable());
      break;

    case 12:
      // Set Coil power state
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
      // Get Reverse direction setting
      build_reply('R', ControllerData->get_reverse_enable());
      break;

    case 14:
      // Set Reverse direction
      if (isMoving == false) {
        ival = WorkString.toInt();
        if (ival == 1) {
          ControllerData->set_reverse_enable(STATE_ENABLED);
        } else {
          ControllerData->set_reverse_enable(STATE_DISABLED);
        }
      }
      break;

    case 15:
      // Set Motorspeed
      bval = (byte)WorkString.toInt() & 3;
      ControllerData->set_motorspeed(bval);
      break;

    case 16:
      // Set Display to Celsius
      // temperature display mode, Celsius=1, Fahrenheit=0
      ControllerData->set_tempmode(true);
      break;

    case 17:
      // Set Display to Fahrenheit
      // temperature display mode, Celsius=1, Fahrenheit=0
      ControllerData->set_tempmode(false);
      break;

    case 18:
      // Set Stepsize enable state
      // not supported on esp8266 - always enabled
      break;

    case 19:
      // :19xxxx# Set Step size value, double type, eg 2.1
      // 0.001-50.0
      fval = (float)WorkString.toFloat();
      RangeCheck(&fval, MINIMUMSTEPSIZE, MAXIMUMSTEPSIZE);
      ControllerData->set_stepsize(fval);
      break;

    case 20:
      // Set Temperature resolution for temperature probe
      // ignored for esp8266
      break;

    case 21:
      // Get Temperature probe resolution
      build_reply('Q', DEFAULTTEMPRESOLUTION);
      break;

    case 22:
      // Set Temperature compensation value to xxx
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 400);
      ControllerData->set_tempcoefficient(ival);
      break;

    case 23:
      // Set Temperature compensation ON (1) or OFF (0)
      if (tempprobe_status == STATUS_RUNNING) {
        ival = WorkString.toInt();
        if (ival == 0) {
          tempcomp_state = STATE_DISABLED;
        } else {
          tempcomp_state = STATE_ENABLED;
        }
      }
      break;

    case 24:
      // Get status of temperature compensation (enabled | disabled)
      build_reply('1', tempcomp_state);
      break;

    case 25:
      // Get IF temperature compensation is available
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
      // :28# home the motor to position 0
      if (isMoving == false) {
        ftargetPosition = 0;
        isMoving = true;
      }
      break;

    case 29:
      // Get stepmode
      build_reply('S', ControllerData->get_brdstepmode());
      break;

    case 30:
      // Set step mode
      {
        ival = WorkString.toInt();
        int brdnum = ControllerData->get_brdnumber();
        if (brdnum == PRO2EULN2003 || brdnum == PRO2EL298N || brdnum == PRO2EL293DMINI || brdnum == PRO2EL9110S) {
          ival = (int)(ival & 3);  // STEP1 - STEP2
        } else if (brdnum == WEMOSDRV8825 || brdnum == PRO2EDRV8825) {
          ival = (int)ControllerData->get_brdfixedstepmode();  // stepmopde set by jumpers
        } else if (brdnum == PRO2EL293DNEMA || brdnum == PRO2EL293D28BYJ48) {
          ival = STEP1;
        } else {
          //Serial.print("tcp err brd ");
          //Serial.println(brdnum);
        }
      }
      ControllerData->set_brdstepmode((int)ival);
      driverboard->setstepmode((int)ival);
      break;

    case 31:
      // Set focuser position
      if (isMoving == false) {
        lvar = (long) WorkString.toInt();
        RangeCheck(&lvar, 0, ControllerData->get_maxstep());
        driverboard->setposition(lvar);
        ControllerData->set_fposition(lvar);
        ftargetPosition = lvar;
      }
      break;

    case 32:
      // Get stepsize enabled
      // not supported on esp8266 - always enabled
      build_reply('U', 1);
      break;

    case 33:
      // Get stepsize value
      build_reply('T', ControllerData->get_stepsize(), 2);
      break;

    case 34:
      // Get the time that a display screen is shown for
      // response is in milliseconds 2000 - 10000
      lvar = DISPLAYPAGETIME * 1000;
      build_reply('X', lvar);
      break;

    case 35:
      // Set length of time a display page is shown for in seconds
      // received value is in seconds 2-10
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
      // :38# Dxx# Get Temperature mode 1=Celsius, 0=Fahrenheight
      build_reply('b', ControllerData->get_tempmode());
      break;

    case 39:
      // Get the new motor position (target) XXXXXX
      build_reply('N', ftargetPosition);
      break;

    case 40:
      // reset controller
      software_Reboot(2000);
      break;

    case 41:
      // :41# Set in-out LED MODE (pulsed or move)
      // ignored for esp8266
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
      // Get PowerDown enable state
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
      // Get in-out LED enable state
      // not supported esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 47:
      // Set in-out LED enable state
      // not supported esp8266
      break;

    case 48:
      // save settings
      // do not do this if focuser is moving
      if (isMoving == false) {
        // need to save position setting
        ControllerData->set_fposition(driverboard->getposition());
        // save the focuser settings immediately
        ControllerData->SaveNow(driverboard->getposition(), driverboard->getdirection());
      }
      break;

    case 49:
      // Get Focuser Type
      build_reply('a', "b552efd");
      break;

    case 50:
      // Get if HOMEPOSITIONSWITCH is enabled in firmware
      // not supported on esp8266
      build_reply('l', 0);
      break;

    case 51:
      // return ESP8266Wifi Controller IP Address
      // meaningless because controller is in serial mode
      build_reply(_RTOKEN, ipStr);
      break;

    case 52:
      // Get PowerDown state
      build_reply(_RTOKEN, PowerDownStatus);
      break;

    case 53:
      // :53#  $x#  Get number of display pages
      build_reply(_RTOKEN, 0);
      break;

    case 54:
      // return ESP32 Controller SSID
      build_reply('g', "SERIAL");
      break;

    case 55:
      // Get motorspeed delay for current speed setting
      build_reply('0', ControllerData->get_brdmsdelay());
      break;

    case 56:
      // Set motorspeed delay for current speed setting
      ival = WorkString.toInt();
      ival = (ival < 1000) ? 1000 : ival;
      ControllerData->set_brdmsdelay(ival);
      break;

    case 57:
      // Get pushbutton enable state
      // not supported esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 58:
      // Set pushbutton enable state (0 or 1)
      // not supported esp8266
      break;

    case 59:
      // Get PowerDown time
      build_reply(_RTOKEN, ControllerData->get_powerdown_time());
      break;

    case 60:
      // Set powerdown time interval in seconds (30-120)
      ival = WorkString.toInt();
      RangeCheck(&ival, 30, 120);
      ControllerData->set_powerdown_time(ival);
      break;

    case 61:
      // Set update of position on lcd when moving (0=disable, 1=enable)
      ival = (byte)WorkString.toInt();
      if (ival == 1) {
        ControllerData->set_display_updateonmove(STATE_ENABLED);
      } else {
        ControllerData->set_display_updateonmove(STATE_DISABLED);
      }
      break;

    case 62:
      // Get update of position on lcd when moving (00=disable, 01=enable)
      build_reply('L', ControllerData->get_display_updateonmove());
      break;

    case 63:  // Get status of home position switch (0=off, 1=closed, position 0)
      // not supported on esp8266
      build_reply('H', 0);
      break;

    case 64:
      // move a specified number of steps
      if (isMoving == false) {
        lvar = WorkString.toInt() + driverboard->getposition();
        lvar = (lvar < 0) ? 0 : lvar;
        ftargetPosition = (lvar > ControllerData->get_maxstep()) ? ControllerData->get_maxstep() : lvar;
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
      // Set push button steps
      // not supported esp8266
      break;

    case 71:
      // Set DelayAfterMove in milliseconds
      ival = WorkString.toInt();
      RangeCheck(&ival, 0, 255);
      ControllerData->set_delayaftermove_time((byte)ival);
      break;

    case 72:
      // Get DelayAfterMove
      build_reply('3', ControllerData->get_delayaftermove_time());
      break;

    case 73:
      // Disable/enable backlash IN (going to lower focuser position)
      // not supported by ESP8266
      break;

    case 74:
      // Get backlash in enabled status
      if (ControllerData->get_backlashsteps_in() > 0) {
        build_reply('4', 1);
      }
      else {
        build_reply('4', 0);
      }  
      break;

    case 75:
      // Disable/enable backlash OUT (going to lower focuser position)
      // not supported by ESP8266
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
      // Set backlash in steps
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
      // not supported esp8266
      build_reply('8', 0);
      break;

    case 82:
      // Set STALL_VALUE (for TMC2209 stepper modules)
      // not supported esp8266
      break;

    case 83:
      // Get if there is a temperature probe
      build_reply('c', tempprobe_found);
      break;

    case 84:
      // Set Nextion Page
      // not supported esp8266
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
      // esp8266 - enabled if dam-time > 0
      break;

    case 87:
      // Get tc direction
      build_reply('c', ControllerData->get_tcdirection());
      break;

    case 88:
      // Set tc direction
      ival = WorkString.toInt();
      if (ival == 1) {
        ControllerData->set_tcdirection(STATE_ENABLED);
      } else {
        ControllerData->set_tcdirection(STATE_DISABLED);
      }
      break;

    case 89:
      // Get stepper power
      // not supported esp8266
      build_reply('9', 1);
      break;

    case 90:
      // Set preset x [0-9] with position value yyyy [unsigned long]
      // not supported on esp8266
      break;

    case 91:
      // Get focuserpreset [0-9]
      // not supported on esp8266
      build_reply(_RTOKEN, 0);
      break;

    case 92:
      // Set display page display option (8 digits, index of 0-7)
      // pages for esp8266 are different
      // Hint: Before using LocalSerial, set the pages by
      // programming controller using ACCESSPOINT or STATION 
      // then use the Management Server to set the display
      // option.
        // If empty (no args) - fill with default display string
        if (WorkString == "") {
          WorkString = "111111";
        }

        // if display option length less than 6, pad with leading 0's
        if (WorkString.length() < 6) {
          while (WorkString.length() < 6) {
            WorkString = '0' + WorkString;
          }
        }

        // do not allow display strings that exceed length of buffer (0-7, 8 digits)
        if (WorkString.length() > 6) {
          WorkString[6] = 0x00;
        }
        ControllerData->set_display_pageoption(WorkString);
      break;

    case 93:
      // Get display page option
      {
        // return as string of 01's
        char buff[10];
        String answer = ControllerData->get_display_pageoption();
        //Serial.println();
        //Serial.print("serial:page option: ");
        //Serial.println(answer);
        int i;
        for (i = 0; i < (int)answer.length(); i++) {
          buff[i] = answer[i];
        }
        buff[i] = 0x00;
        build_reply('l', buff);
      }
      break;

    case 94:
      // Set DelayedDisplayUpdate (0=disabled, 1-enabled)
      // not supported on esp8266
      break;

    case 95:
      // Get DelayedDisplayUpdate (0=disabled, 1-enabled)
      // not supported on esp8266
      build_reply('n', 0);
      break;

    case 96:
      // :96#   FString#  Get firmware Major version, Minor Version
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
      // Set HPSW enable
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
      if (ival == 1) {
        ControllerData->set_tempprobe_enable(STATE_ENABLED);
      } else {
        ControllerData->set_tempprobe_enable(STATE_DISABLED);
      } 
      break;

    case 106:
      // Get ALPACA Server enabled state
      // not supported when using LOCALSERIAL
      build_reply(_RTOKEN, ControllerData->get_alpacasrvr_enable());
      break;

    case 107:
      // Set ALPACA Server enabled state
      // not supported when using LOCALSERIAL
      break;

    case 108:
      // Get ALPACA Server Start/Stop status
      build_reply(_RTOKEN, alpacasrvr_status);
      break;

    case 109:
      // myFP2ESP32 set ASCOM ALPACA Server Start/Stop
      // this will start or stop the ASCOM server
      // not supported when using LOCALSERIAL
      break;

    // :B0 to :B9
    case 110:
      // Get Web Server enabled state
      build_reply(_RTOKEN, ControllerData->get_websrvr_enable());
      break;

    case 111:
      // Set Web Server enabled state
      // not supported when using LOCALSERIAL
      break;

    case 112:
      // Get Web Server Start/Stop status
      build_reply(_RTOKEN, 0);
      break;

    case 113:
      // Set Web Server Start/Stop - this will start or stop the ASCOM server
      // not supported when using LOCALSERIAL
      break;

    case 114:
      // Get Management Server enabled state
      build_reply(_RTOKEN, ControllerData->get_mngsrvr_enable());
      break;

    case 115:
      // Set Management Server enabled state
      // not supported when using LOCALSERIAL
      break;

    case 116:
      // Get Management Server Start/Stop status
      build_reply(_RTOKEN, mngsrvr_status);
      break;

    case 117:
      // Set Management Server Start/Stop - this will start or stop the Management server
      // not supported when using LOCALSERIAL
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

    case 120:
      // Get OTA State (0=Disabled, 1=Enabled) #
      // TODO
      break;

    case 121:
      // Set OTA State

      break;

    case 122:
      // Get OTA Status (0=Stopped, 1=Running)

      break;

    case 123:
      // Set OTA Status

      break;

    default:
      //Serial.print("tcpip cmd err: ");
      //Serial.println(cmdval);
      break;
  }
}

// -------------------------------------------------------
// CLEAR SERIAL PORT RECEIVE BUFFER
// -------------------------------------------------------
void LOCAL_SERIAL::clearSerialPort() {
  while (_dev.available())
    _dev.read();
}

// -------------------------------------------------------
// HANDLER SERIAL INCOMING DATA
// -------------------------------------------------------
void LOCAL_SERIAL::serialEvent(void) {
  // : starts the command
  // # ends the command

  while (_dev.available()) {
    char inChar = _dev.read();    
    switch (inChar) {
      case ':' :  // start
        line = "";
        break;
      case '#' :  // eoc
        queue.push(line);
        break;
      default:  // anything else
        line = line + inChar;
        break;
    }
  }
}

#endif  // #if (CONTROLLERMODE == LOCALSERIAL)
#endif  // if defined(CONTROLLERMODE)
