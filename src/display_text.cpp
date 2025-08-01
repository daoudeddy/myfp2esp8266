// ----------------------------------------------------------------------
// myFP2ESP8266 TEXT DISPLAY CLASS (OLED 0.96", 128x64, 16 chars by 8 lines)
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2021. All Rights Reserved.
// display_text.cpp
// Optional
// NodeMCU 1.0 (ESP-12E Module)
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// INCLUDES
// ----------------------------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(DISPLAYTYPE)
#if (DISPLAYTYPE == TEXT_OLED12864)

#include <avr/pgmspace.h>
#include <Wire.h>

#include "display_text.h"


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Display Text messages to 
// be written to Serial port
//#define DISPLAYTEXTMSGS 1

#ifdef DISPLAYTEXTMSGS
#define DisplayTextPrint(...) Serial.print(__VA_ARGS__)
#define DisplayTextPrintln(...) Serial.println(__VA_ARGS__)
#else
#define DisplayTextPrint(...)
#define DisplayTextPrintln(...)
#endif


// ----------------------------------------------------------------------
// CLASSES
// ----------------------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA* ControllerData;

#include "driver_board.h"
extern DRIVER_BOARD* driverboard;


// ----------------------------------------------------------------------
// EXTERNS
// ----------------------------------------------------------------------
extern int mycontrollermode;


// ----------------------------------------------------------------------
// DEFINES
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// DATA
// ----------------------------------------------------------------------
// Display Text DT_

const char DT_T[4] = "T  ";
// Pg1
const char DT_POSITION[4] = "P  ";
//const char DT_TARGETPOSITION[4] = "T  ";  << use DT_T
const char DT_MAXSTEP[4] = "M  ";
const char DT_ISMOVING[8] = "Moving ";

// Pg2
//const char DT_TEMPERATURE[5] = "T  ";  << use DT_T
const char DT_TEMPCOMP[5]    = "TC ";
const char DT_MOTORSPEED[5]  = "MS ";
const char SPEED_SLOW[5]     = "Slow";
const char SPEED_MEDIUM[4]   = "Med";
const char SPEED_FAST[5]     = "Fast";
const char DT_STEPMODE[5]    = "SM ";

// Pg3
const char DT_COILPOWER[5]       = "CP  ";
const char DT_REVERSE[5]         = "REV ";
const char DT_DRIVERBOARD[5]     = "BRD ";
const char DT_FIRMWAREVERSION[5] = "VER ";

// Pg4
const char DT_BACKLASHINSTEPS[8]  = "BIN#  ";
const char DT_BACKLASHOUTSTEPS[8] = "BOU#  ";

// Pg5
const char DT_MODE[4]            = "MOD";
const char DT_MODELSERIAL[8]     = "LSERIAL";
const char DT_MODEACCESSPOINT[7] = "ACCPNT";
//const char T_STATION[9] = "STATION ";          << use this in defines.h
//const char DT_MODESTATION[8]     = "STATION";
const char DT_MODEERROR[6]       = "ERROR";
//const char T_SSID[6] = "SSID ";                << use this in defines.h
//const char DT_SSID[6]            = "SSID ";

// Pg6


// OLED_ADDR is defined in controller_defines.h
// sda, scl are in Controller->get_brdsda() and ControllerData->get_brdsck()


// ----------------------------------------------------------------------
// CLASS
// ----------------------------------------------------------------------
TEXT_DISPLAY::TEXT_DISPLAY(uint8_t addr)
  : _addr(addr) {
  _loaded = STATE_NOTLOADED;
}


// ----------------------------------------------------------------------
// DISPLAY START
// ----------------------------------------------------------------------
bool TEXT_DISPLAY::start() {
  // if disabled then do not start
  DisplayTextPrintln(T_DISPLAYTEXT);
  DisplayTextPrintln(TUC_START);

  if (ControllerData->get_display_enable() == STATE_DISABLED) {
    DisplayTextPrint(T_ERROR);
    DisplayTextPrintln(T_DISABLED);
    //display_found = NOT_FOUND;
    return false;
  }

  // prevent any attempt to start if already started
  if (_loaded == STATE_LOADED) {
    // means driver already loaded
    clear();
    on();
    display_found = FOUND;
    return true;
  }

  // check if display is present
  Wire.beginTransmission(_addr);
  if (Wire.endTransmission() != 0) {
    DisplayTextPrintln(T_NOTFOUND);
    display_found = NOT_FOUND;
    return false;
  }

  // display found
  DisplayTextPrintln(T_FOUND);

  _display = new SSD1306AsciiWire();
  _display->begin(&Adafruit128x64, _addr);

  display_found = FOUND;
  _loaded = STATE_LOADED;
  _display->set400kHz();
  _display->setFont(Adafruit5x7);
  // clear also sets cursor at 0,0
  _display->clear();
  // black on white
  _display->Display_Normal();
  _display->Display_On();
  // portrait, not rotated
  _display->Display_Rotate(0);
  _display->Display_Bright();
  _display->set1X();
  _display->println(project_name);
  _display->println(major_version);
  _display->println("BOOTING");
  return true;
}

// -------------------------------------------------------
// STOP DISPLAY
// -------------------------------------------------------
void TEXT_DISPLAY::stop(void) {
  DisplayTextPrintln(T_DISPLAYTEXT);
  DisplayTextPrintln(TUC_STOP);
  if (_loaded == STATE_LOADED) {
    clear();
    off();
  }
}

// -------------------------------------------------------
// DISPLAY CLEAR
// -------------------------------------------------------
void TEXT_DISPLAY::clear(void) {
  if (_loaded == STATE_LOADED) {
    _display->clear();
  }
}


// -------------------------------------------------------
// DISPLAY OFF
// -------------------------------------------------------
void TEXT_DISPLAY::off(void) {
  if (_loaded == STATE_LOADED) {
    _display->Display_Off();
  }
}

// -------------------------------------------------------
// DISPLAY ON
// -------------------------------------------------------
void TEXT_DISPLAY::on(void) {
  if (_loaded == STATE_LOADED) {
    _display->Display_On();
  }
}


// --------------------------------------------------------
// DISPLAY BRIGHTNESS
// --------------------------------------------------------
void TEXT_DISPLAY::display_setbrightness(uint8_t level) {
  if (display_status == STATUS_RUNNING) {
    _display->setContrast(level);
  }
}


// -------------------------------------------------------
// UPDATE DISPLAY PAGE
// -------------------------------------------------------
void TEXT_DISPLAY::update_page(long position) {
  if (_loaded == STATE_LOADED) {
    draw_main_update(position);
  }
}

// -------------------------------------------------------
// UPDATE POSITION
// -------------------------------------------------------
void TEXT_DISPLAY::update_position(long position) {
  if (_loaded == STATE_LOADED) {
    _display->setCursor(0, 0);
    _display->print(DT_POSITION);
    _display->print(position);
    _display->clearToEOL();
    _display->println();

    _display->print(DT_T);
    _display->print(ftargetPosition);
    _display->clearToEOL();
    _display->println();
  }
}

// -------------------------------------------------------
// UPDATE DISPLAY PAGE
// -------------------------------------------------------
void TEXT_DISPLAY::draw_main_update(long position) {
  DisplayTextPrintln(T_DISPLAYTEXT);
  DisplayTextPrintln(T_UPDATE);

  if (_loaded != STATE_LOADED) {
    DisplayTextPrintln(T_DISPLAYTEXT);
    DisplayTextPrintln(T_NOTLOADED);
    return;
  }

  static int page = 6;
  static int displaybitmask = 1;

  // get page options
  String mypage = ControllerData->get_display_pageoption();
  DisplayTextPrint("display:pageoption:");
  DisplayTextPrintln(mypage);
  for (int i = 0; i < (int)mypage.length(); i++) {
    page *= 2;
    if (mypage[i] == '1') {
      page++;
    }
  }

  // find the next page to display
  // mask off one bit at a time, skip page if not enabled
  while ((page & displaybitmask) == 0) {
    displaybitmask *= 2;
    // 63 = B111111
    if (displaybitmask > 63) {
      break;
    }
  }  // while ( (page & displaybitmask) == 0 )

  // bitmask has stopped at current page indicating 1
  // in page option string
  // ensure that mask does not exceed page value
  // 63 = B1111111
  if (displaybitmask > 63) {
    // reset bitmask
    displaybitmask = 1;
  }

  _display->clear();
  _display->set2X();

  // displaybitmask is now the page to display,
  // 1=pg1, 2=pg2, 4=pg3, 8=pg4 etc

  DisplayTextPrint("display:draw_main_update:bitmask = ");
  DisplayTextPrintln(displaybitmask);
  DisplayTextPrint("display:draw_main_update:");

  switch (displaybitmask) {
    case 1:
      DisplayTextPrintln("pg1");
      page1(position);
      break;
    case 2:
      DisplayTextPrintln("pg2");
      page2();
      break;
    case 4:
      DisplayTextPrintln("pg3");
      page3();
      break;
    case 8:
      DisplayTextPrintln("pg4");
      page4();
      break;
    case 16:
      DisplayTextPrintln(":pg5");
      page5();
      break;
    case 32:
      DisplayTextPrintln("pg6");
      page6();
      break;
    default:
      DisplayTextPrintln("pg num ERROR");
      page1(position);
      break;
  }
  // next page
  displaybitmask *= 2;
}

// 128x64
// _display->set1X();
// 16 chars per line
// 8 rows
// _display->set2X();
// 8 chars per line
// 4 rows

// P 80000  = 7 chars
// T 22.75c = 9 chars

// -------------------------------------------------------
// PAGE 1
// Position
// Target
// maxStep
// Moving MoveDirection
// -------------------------------------------------------
void TEXT_DISPLAY::page1(long position) {
  if (_loaded == STATE_LOADED) {
    _display->home();

    // Line 1 of 4: Position
    _display->print(DT_POSITION);
    _display->print(position);
    _display->clearToEOL();
    _display->println();

    // Line 2 of 4: Target Position
    _display->print(DT_T);
    _display->print(ftargetPosition);
    _display->clearToEOL();
    _display->println();

    // Line 3 of 4: maxStep
    _display->print(DT_MAXSTEP);
    _display->print(ControllerData->get_maxstep());
    _display->clearToEOL();
    _display->println();

    // Line 4 of 4: isMoving and IN | OUT
    if (isMoving) {
      _display->print(DT_ISMOVING);
      if (ControllerData->get_focuserdirection()) {
        _display->print("O");
      } else {
        _display->print("I");
      }
    } else {
      _display->print(T_STOPPED);
    }
    _display->clearToEOL();
  }
}


// -------------------------------------------------------
// PAGE 2
// Temperature, C | F
// TempComp State ON | OFF
// MotorSpeed
// StepMode
// -------------------------------------------------------
void TEXT_DISPLAY::page2(void) {
  if (_loaded == STATE_LOADED) {
    _display->home();

    // Line 1 of 4: Temperature + c|f
    _display->print(DT_T);
    float tp = temp;
    if (ControllerData->get_tempmode() == FAHRENHEIT) {
      tp = (tp * 1.8) + 32;
    }
    _display->print(String(tp, 2));
    if (ControllerData->get_tempmode() == CELSIUS) {
      _display->print("c");
    } else {
      _display->print("f");
    }
    _display->clearToEOL();
    _display->println();

    // Line 2 of 4: TempComp State
    _display->print(DT_TEMPCOMP);
    if (tempcomp_state) {
      _display->print(T_ON);
    } else {
      _display->print(T_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // Line 3 of 4: MOTOR SPEED
    int speed = ControllerData->get_motorspeed();
    _display->print(DT_MOTORSPEED);
    switch (speed) {
      case 0:
        _display->print(SPEED_SLOW);
        break;
      case 1:
        _display->print(SPEED_MEDIUM);
        break;
      case 2:
        _display->print(SPEED_FAST);
        break;
    }
    _display->clearToEOL();
    _display->println();

    // Line 4 of 4: STEP MODE
    _display->print(DT_STEPMODE);
    _display->print(ControllerData->get_brdstepmode());
    _display->clearToEOL();
  }
}


// -------------------------------------------------------
// PAGE 3
// Coil Power
// Reverse
// DrvBrd
// Firmware revision
// -------------------------------------------------------
void TEXT_DISPLAY::page3(void) {
  if (_loaded == STATE_LOADED) {
    _display->home();

    // Line 1 of 4: Coil power
    _display->print(DT_COILPOWER);
    if (ControllerData->get_coilpower_enable() == STATE_ENABLED) {
      _display->print(T_ON);
    } else {
      _display->print(T_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // Line 2 of 4: Reverse direction
    _display->print(DT_REVERSE);
    if (ControllerData->get_reverse_enable() == STATE_ENABLED) {
      _display->print(T_ON);
    } else {
      _display->print(T_OFF);
    }
    _display->clearToEOL();
    _display->println();

    // Line 3 of 4: DRVBRD
    //_display->print(DT_DRIVERBOARD);
    String name = ControllerData->get_brdname();
    String bname = name.substring(0, 15);
    _display->print(bname);
    _display->clearToEOL();
    _display->println();

    // Line 4 of 4: Firmware Version
    //_display->print(DT_FIRMWAREVERSION);
    _display->print(major_version);
    _display->print("-");
    _display->print(minor_version);
    _display->clearToEOL();
  }
}


// -------------------------------------------------------
// PAGE 4
// BACKLASH IN-OUT ENABLE, BACKLASH IN-OUT STEPS
// -------------------------------------------------------
void TEXT_DISPLAY::page4(void) {
  if (_loaded == STATE_LOADED) {
    _display->home();

    // Line 1 of 2: BACKLASHIN STEPS
    _display->print(DT_BACKLASHINSTEPS);
    int steps = ControllerData->get_backlashsteps_in();
    _display->print(steps);
    _display->clearToEOL();
    _display->println();

    // Line 2 of 2: BACKLASHOUT STEPS
    _display->print(DT_BACKLASHOUTSTEPS);
    steps = ControllerData->get_backlashsteps_out();
    _display->print(steps);
    _display->clearToEOL();
  }
}


// -------------------------------------------------------
// PAGE 5
// MODE SERIAL | ACCESSPOINT | STATION
// SSID
// DEVICENAME
// MDNS NAME
// -------------------------------------------------------
void TEXT_DISPLAY::page5(void) {
  if (_loaded == STATE_LOADED) {
    _display->home();

    String ssID;

    // Line 1 of 4: MODE SERIAL | ACCESSPOINT | STATION
    //_display->print(DT_MODE);
    if (mycontrollermode == LOCALSERIAL) {
      _display->print(DT_MODELSERIAL);
      ssID = "---";
    } else if (mycontrollermode == ACCESSPOINT) {
      _display->print(DT_MODEACCESSPOINT);
      ssID = String(myAPSSID);
    } else if (mycontrollermode == STATION) {
      _display->print(T_STATION);
      ssID = String(mySSID);
    } else {
      _display->print(DT_MODEERROR);
      ssID = "---";
    }
    _display->clearToEOL();
    _display->println();

    //ssID = ssID.substring(0, 5);

    // Line 2 of 4: SSID
    _display->print(T_SSID);
    _display->print(ssID.substring(0, 6));
    _display->clearToEOL();
    _display->println();

    // Line 3 of 4: DEVICENAME
    _display->print(DeviceName);
    _display->clearToEOL();
    _display->println();

    // Line 4 of 4: MDNS NAME
    _display->print(MDNSName);
    _display->clearToEOL();
  }
}


// -------------------------------------------------------
// PAGE 6
// IP
// HEAP
// SUT
// ???
// -------------------------------------------------------
void TEXT_DISPLAY::page6(void) {
  if (_loaded == STATE_LOADED) {
    _display->home();

    _display->set1X();

    // Line 1 of 4: IP
    _display->print(ipStr);
    _display->clearToEOL();
    _display->println();

    _display->println();

    _display->set2X();

    // Line 2 of 4: HEAP
    _display->print(String(ESP.getFreeHeap()));
    _display->clearToEOL();
    _display->println();

    // Line 3 of 4: SUT
    get_systemuptime();
    _display->print(systemuptime);
    _display->clearToEOL();
    _display->println();
  }
}

#endif  // #if (DISPLAYTYPE == TEXT_OLED12864)
#endif  // #if defined(DISPLAYTYPE)
