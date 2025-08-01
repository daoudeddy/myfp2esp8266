// -------------------------------------------------------
// myFP2ESP8266 FOCUSER CONFIGURATION CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_data.cpp
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// DEFAULT CONFIGURATION
// ControllerData
// Controller Persistant Data  cntlr_config.jsn
// Controller Variable Data    cntlr_var.jsn
// Controller Board Data       board_config.jsn


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Controller Data messages to 
// be written to Serial port
//#define Controller_MsgPrint 1

#ifdef Controller_MsgPrint
#define ControllerPrint(...) Serial.print(__VA_ARGS__)
#define ControllerPrintln(...) Serial.println(__VA_ARGS__)
#else
#define ControllerPrint(...)
#define ControllerPrintln(...)
#endif


// -------------------------------------------------------
// CLASSES
// -------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

#include "driver_board.h"
extern DRIVER_BOARD *driverboard;


// -------------------------------------------------------
// EXTERNS
// -------------------------------------------------------
extern int _display_type;


// -------------------------------------------------------
// APPLICATION DATA
// For application settings see
//      defines/app_defines.h
// -------------------------------------------------------
#include "defines/app_defines.h"


// -------------------------------------------------------
// DUCKDNS DATA
// For duckdns settings see
//      defines/duckdns_defines.h
// -------------------------------------------------------
#include "defines/duckdns_defines.h"


// -------------------------------------------------------
// CONTROLLERSDATA DEFINES
// -------------------------------------------------------
#define DEFAULT_ZERO 0
#define DEFAULTCONFIGSIZE 2048
#define BOARDDATASIZE 512
#define BOARDVARDATASIZE 64


// -------------------------------------------------------
// CONTROLLER_DATA CLASS CONSTRUCTOR
// -------------------------------------------------------
CONTROLLER_DATA::CONTROLLER_DATA(void) {
  SnapShotMillis = millis();
  BoardSnapShotMillis = millis();
  ReqSaveData_var = false;    // Controller Variable Data
  ReqSaveData_per = false;    // Controller Persistant Data
  ReqSaveBoard_var = false;   // Controller Board Data

  ControllerPrint(T_CNTLRDATA);
  ControllerPrintln(TUC_START);  

  // mount Filesystem
  if (!LittleFS.begin()) {
    LittleFS.format();
    if (!LittleFS.begin()) {
      filesystemloaded = STATE_NOTLOADED;
      ControllerPrintln(T_FILESYSTEMERROR);
      software_Reboot(REBOOTDELAY);
    } else {
      filesystemloaded = STATE_LOADED;
    }
  }
  LoadConfiguration();
  delay(10);
};


// -------------------------------------------------------
// CACHE SETTINGS INTO CHAR BUFFERS
// From /defines/app_defines.h
// -------------------------------------------------------
void CONTROLLER_DATA::init_cachevars(void) {
  String tmp = DEFAULT_PROJECTAUTHOR;
  tmp.toCharArray(project_author, BUFFER12LEN);

  tmp = DEFAULT_PROJECTNAME;
  tmp.toCharArray(project_name, BUFFER12LEN);

  tmp = DEFAULT_MAJOR_VERSION;
  tmp.toCharArray(major_version, BUFFER12LEN);

  tmp = DEFAULT_MINOR_VERSION;
  tmp.toCharArray(minor_version, BUFFER12LEN);
}


// -------------------------------------------------------
// LOADS THE CONFIGURATION FROM FILES (CNTLR, BOARD, VAR)
// If configuration files are not found, or cannot be
// deserialised, then firmware will create Default
// Configurations for CNTLR, BOARD and VAR
// -------------------------------------------------------
bool CONTROLLER_DATA::LoadConfiguration() {
  // Focuser persistant data - Open cntlr_config.jsn file
  // for reading

  ControllerPrint(T_CNTLRDATA);
  ControllerPrintln("LoadConfiguration"); 

  if (LittleFS.exists(file_cntlr_config) == FILE_NOTFOUND) {
    ControllerPrintln("LoadDefaultPersistantData");
    LoadDefaultPersistantData();
  } else {
    String cdata;
    cdata.reserve(DEFAULTCONFIGSIZE);

    // Open file
    File cfile = LittleFS.open(file_cntlr_config, "r");
    cdata = cfile.readString();
    cfile.close();

    JsonDocument doc;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, cdata);
    if (error) {
      ControllerPrintln("LoadDefaultPersistantData");
      LoadDefaultPersistantData();
    } else {
      ControllerPrintln("LoadPersistantData");

      // maxStep
      maxstep = doc["maxstep"];

      // ALPACA
      alpacasrvr_enable = doc["alpaca_en"];

      // BACKLASH
      backlashsteps_in = doc["blin_steps"];
      backlashsteps_out = doc["blout_steps"];

      // COIL POWER
      coilpower_enable = doc["cp_en"];

      // DELAY AFTER MOVE
      delayaftermove_time = doc["dam_time"];

      // DEVICENAME
      devicename = doc["devname"].as<const char *>();
      snprintf(DeviceName, sizeof(DeviceName), "%s%c", devicename, 0x00);

      // DISPLAY
      display_enable = doc["d_en"];
      display_updateonmove = doc["d_updmove"];
      display_pageoption = doc["d_pgopt"].as<const char *>();

      // DUCKDNS
      duckdns_enable = doc["ddns_en"];
      duckdns_domain = doc["ddns_d"].as<const char *>();
      duckdns_token = doc["ddns_t"].as<const char *>();

      // MANAGEMENT SERVER
      mngsrvr_enable = doc["mngt_en"];

      // MDNS NAME
      mdnsname = doc["mdnsn"].as<const char *>();
      snprintf(MDNSName, sizeof(MDNSName), "%s%c", mdnsname, 0x00);

      // MOTORSPEED SLOW, MED, FAST
      motorspeed = doc["mspeed"];

      // POWERDOWN ENABLE
      powerdown_enable = doc["pwrdn"];
      // POWERDOWN TIME
      powerdown_time = doc["pwrdt"];

      // REVERSE
      reverse_enable = doc["rdir_en"];

      // STEPSIZE IN MICRONS
      stepsize = doc["ss_val"];

      // TCPIP Server
      tcpipsrvr_enable = doc["tcp_en"];

      // TEMPERATURE PROBE ENABLE
      tempprobe_enable = doc["t_en"];

      // TEMPERATURE COEFFICIENT VALUE: STEPS PER DEGREE
      tempcoefficient = doc["t_coe"];

      // TEMPERATURE DISPLAY MODE, CELCIUS=1, FAHRENHEIT=0
      tempmode = doc["t_mod"];

      // TEMP COMP DIRECTION
      tcdirection = doc["t_tcdir"];

      // TEMP COMP LOAD STATE
      tempcomp_onload = doc["tc_load"];

      // WEB Server
      websrvr_enable = doc["ws_en"];

      init_cachevars();
    }
  }

  // LOAD CONTROLLER BOARD DATA
  File bfile = LittleFS.open(file_board_config, "r");
  if (!bfile) {
    ControllerPrintln("LoadDefaultBoardData");
    LoadDefaultBoardData();
  } else {
    // board_config.jsn board data
    String bdata;
    bdata.reserve(BOARDDATASIZE);

    bdata = bfile.readString();
    bfile.close();

    JsonDocument doc_brd;
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_brd, bdata);
    if (error) {
      ControllerPrintln("LoadDefaultBoardData");
      LoadDefaultBoardData();
    } else {
      ControllerPrintln("LoadBoardData");
      board = doc_brd["board"].as<const char *>();
      maxstepmode = doc_brd["maxstepmode"];
      stepmode = doc_brd["stepmode"];
      enablepin = doc_brd["enpin"];
      steppin = doc_brd["steppin"];
      dirpin = doc_brd["dirpin"];
      temppin = doc_brd["temppin"];
      boardnumber = doc_brd["brdnum"];
      stepsperrev = doc_brd["stepsrev"];
      fixedstepmode = doc_brd["fixedsmode"];
      for (int i = 0; i < 4; i++) {
        boardpins[i] = doc_brd["brdpins"][i];
      }
      msdelay = doc_brd["msdelay"];
    }
  }

  // LOAD CONTROLLER VAR DATA : POSITION : DIRECTION
  // this uses stepmode which is in boardconfig file
  // so this must come after loading the board config

  // controller variable data (position, direction)
  File vfile = LittleFS.open(file_cntlr_var, "r");
  if (!vfile) {
    LoadDefaultVariableData();
  } else {
    String vdata;
    vdata.reserve(BOARDVARDATASIZE);

    vdata = vfile.readString();
    vfile.close();

    JsonDocument doc_var;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc_var, vdata);
    if (error) {
      LoadDefaultVariableData();
    } else {
      // get last position and last move direction
      fposition = doc_var["fpos"];
      focuserdirection = doc_var["fdir"];

      if (_display_type == GRAPHIC_OLED12864) {
        // round position to fullstep motor position,
        // holgers code
        // only applicable if using a GRAPHICS Display
        fposition = (fposition + stepmode / 2) / stepmode * stepmode;
      }
    }
  }
  return true;
}


// -------------------------------------------------------
// LOAD FOCUSER VARIABLE DATA - POSITION AND DIRECTION
// -------------------------------------------------------
void CONTROLLER_DATA::LoadDefaultVariableData() {
  // last focuser position and last focuser move direction
  fposition = DEFAULTPOSITION;
  focuserdirection = moving_in;
  SaveVariableConfiguration(fposition, focuserdirection);
  delay(10);
}


// -------------------------------------------------------
// LOAD DEFAULT FOCUSER PERSISTANT DATA SETTINGS
// (when no config file exists eg; after an upload)
// Creates a default config setting file and saves
// it to the filesystem LittleFS
// -------------------------------------------------------
void CONTROLLER_DATA::LoadDefaultPersistantData() {
  maxstep = DEFAULTMAXSTEPS;

  // Set the initial defaults for a controller

  // ALPACA
  alpacasrvr_enable = STATE_DISABLED;

  // DUCKDNS from /defines/duckdns_defines.h
  duckdns_enable = STATE_DISABLED;
  duckdns_domain = String(duckdnsdomain);
  duckdns_token = String(duckdnstoken);

  mngsrvr_enable = STATE_ENABLED;
  tcpipsrvr_enable = STATE_DISABLED;
  websrvr_enable = STATE_DISABLED;

  // BACKLASH
  backlashsteps_in = DEFAULT_ZERO;
  backlashsteps_out = DEFAULT_ZERO;

  // COIL POWER
  coilpower_enable = STATE_DISABLED;

  // DELAY AFTER MOVE
  delayaftermove_time = 25;

  devicename = String(DEFAULT_DEVICENAME);
  snprintf(DeviceName, sizeof(DeviceName), "%s%c", devicename, 0x00);

  // DISPLAY
  display_enable = STATE_DISABLED;
  display_updateonmove = STATE_ENABLED;
  display_pageoption = "111111";  // six pages

  mdnsname = String(DEFAULT_MDNSNAME);
  snprintf(MDNSName, sizeof(MDNSName), "%s%c", mdnsname, 0x00);

  // MOTORSPEED
  motorspeed = FAST;

  // powerdown enable
  powerdown_enable = STATE_DISABLED;
  powerdown_time = 60L;

  // REVERSE
  reverse_enable = STATE_DISABLED;

  // STEPSIZE
  stepsize = DEFAULTSTEPSIZE;

  // TEMPERATURE PROBE
  tempprobe_enable = STATE_DISABLED;
  tempcoefficient = DEFAULT_ZERO;
  tempmode = CELSIUS;
  tcdirection = TC_DIRECTION_IN;
  tempcomp_onload = STATE_DISABLED;

  SavePersitantConfiguration();
  delay(10);
}


// -------------------------------------------------------
// LOAD DEFAULT BOARD DATA SETTINGS
// -------------------------------------------------------
void CONTROLLER_DATA::LoadDefaultBoardData() {
  // we are here because board_config.jsn was not found
  // we can load the default board configuration from
  // DRVBRD defined - DefaultBoardName in .ino file

  // Driver board data - Open the specific board
  // config .jsn file for reading

  // cannot use boardnumber because the value has not
  // been set yet
  // Load the board file from /boards, make up filename first

  String bfile = "/boards/" + String(myboardnumber) + ".jsn";

  // attempt to load the specified board config
  // file from /boards
  if (LoadBrdConfigStart(bfile) == true) {
    return;
  } else {
    // a board config file could not be loaded,
    // so create a dummy one
    board = "Unknown";
    maxstepmode = -1;
    stepmode = 1;
    enablepin = -1;
    steppin = -1;
    dirpin = -1;
    temppin = -1;
    // captured from controller_config.h
    boardnumber = myboardnumber;
    fixedstepmode = myfixedstepmode;
    stepsperrev = mystepsperrev;
    for (int i = 0; i < 4; i++) {
      boardpins[i] = -1;
    }
    msdelay = DEFAULT_MOTORSPEEDDELAY;
  }
  SaveBoardConfiguration();
  delay(10);
}


// -------------------------------------------------------
// RESET FOCUSER SETTINGS TO DEFAULTS
// TCPIP_SERVER.CPP CASE 42:
// -------------------------------------------------------
void CONTROLLER_DATA::SetFocuserDefaults(void) {
  ControllerPrint(T_CNTLRDATA);
  ControllerPrintln(T_LOADCONFIG); 

  LittleFS.remove(file_cntlr_config);
  LittleFS.remove(file_board_config);
  LittleFS.remove(file_cntlr_var);

  LoadDefaultPersistantData();
  LoadDefaultBoardData();
  LoadDefaultVariableData();
  delay(10);
}


// -------------------------------------------------------
// SAVE THE CONFIGURATION TO FILES
// Called externally from loop() to save config
// files after time elapsed
// -------------------------------------------------------
bool CONTROLLER_DATA::SaveConfiguration(long currentPosition, byte DirOfTravel) {
  bool state = false;

  if (isMoving == true) {
    ControllerPrintln("CD-Save err isMoving");
    return false;
  }

  if (fposition != currentPosition || focuserdirection != DirOfTravel) {
    fposition = currentPosition;
    focuserdirection = DirOfTravel;
    ReqSaveData_var = true;
    SnapShotMillis = millis();
  }

  unsigned long x = millis();

  if ((SnapShotMillis + DEFAULTSAVETIME) < x || SnapShotMillis > x) {
    if (ReqSaveData_per == true) {
      if (SavePersitantConfiguration() == true) {
        state = true;
        ReqSaveData_per = false;
      } else {
        ControllerPrintln(T_ERROR);
      }
    }

    // save variable data - position and direction
    if (ReqSaveData_var == true) {
      ControllerPrintln("data_var save");
      if (SaveVariableConfiguration(fposition, focuserdirection) == true) {
        state = true;
        ReqSaveData_var = false;
      } else {
        ControllerPrintln(T_ERROR);
      }
    }

    // save board_config data
    if (ReqSaveBoard_var == true) {
      ControllerPrintln("brd_var save");
      if (SaveBoardConfiguration() == true) {
        state = true;
        ReqSaveBoard_var = false;
      } else {
        ControllerPrintln(T_ERROR);
      }
    }
  }
  delay(10);
  return state;
}


// -------------------------------------------------------
// SAVE CONFIGURATION FILES IMMEDIATELY (LIKE IN THE CASE FOR REBOOT()
// -------------------------------------------------------
bool CONTROLLER_DATA::SaveNow(long focuser_position, bool focuser_direction) {
  SaveBoardConfiguration();
  SaveVariableConfiguration(focuser_position, focuser_direction);
  delay(10);
  return SavePersitantConfiguration();
}


// -------------------------------------------------------
// SAVE VARIABLE DATA (POSITION, DIR TRAVEL) SETTINGS TO CNTLR_VAR.JSN
// -------------------------------------------------------
bool CONTROLLER_DATA::SaveVariableConfiguration(long focuser_position, bool focuser_direction) {
  LittleFS.begin();

  // Delete existing file
  LittleFS.remove(file_cntlr_var);

  // Open file for writing
  File vfile = LittleFS.open(file_cntlr_var, "w");
  if (!vfile) {
    return false;
  }

  JsonDocument doc;

  // Set the values in the document
  fposition = focuser_position;
  focuserdirection = focuser_direction;

  doc["fpos"] = fposition;
  doc["fdir"] = focuserdirection;

  // save settings to file
  if (serializeJson(doc, vfile) == 0) {
    vfile.close();
    return false;
  }
  vfile.close();
  return true;
}


// -------------------------------------------------------
// SAVE FOCUSER CONTROLLER (PERSISTENT) DATA TO FILE CNTLR_CONFIG.JSN
// -------------------------------------------------------
bool CONTROLLER_DATA::SavePersitantConfiguration() {
  LittleFS.begin();

  // remove existing file
  LittleFS.remove(file_cntlr_config);

  JsonDocument doc;

  doc["maxstep"] = maxstep;

  // SERVERS - SERVICES
  doc["alpaca_en"] = alpacasrvr_enable;

  // backlash
  doc["blin_steps"] = backlashsteps_in;
  doc["blout_steps"] = backlashsteps_out;

  // coil power
  doc["cp_en"] = coilpower_enable;

  // delay after move
  doc["dam_time"] = delayaftermove_time;

  // devicename
  doc["devname"] = devicename;

  // display
  doc["d_en"] = display_enable;
  doc["d_updmove"] = display_updateonmove;
  doc["d_pgopt"] = display_pageoption;

  // duckdns
  doc["ddns_en"] = duckdns_enable;
  doc["ddns_d"] = duckdns_domain;
  doc["ddns_t"] = duckdns_token;

  // Management Server
  doc["mngt_en"] = mngsrvr_enable;

  // mdns name
  doc["mdnsn"] = mdnsname;

  // motorspeed
  doc["mspeed"] = motorspeed;

  // powerdown enable
  doc["pwrdn"] = powerdown_enable;

  // powerdown time
  doc["pwrdt"] = powerdown_time;

  // reverse
  doc["rdir_en"] = reverse_enable;

  // stepsize
  doc["ss_val"] = stepsize;

  // TCPIP Server
  doc["tcp_en"] = tcpipsrvr_enable;

  // temperature probe
  doc["t_en"] = tempprobe_enable;
  doc["t_coe"] = tempcoefficient;
  doc["t_mod"] = tempmode;
  doc["t_tcdir"] = tcdirection;
  doc["tc_load"] = tempcomp_onload;

  // WEB Server
  doc["ws_en"] = websrvr_enable;

  // Open file for writing
  File cfile = LittleFS.open(file_cntlr_config, "w");
  if (!cfile) {
    return false;
  }

  if (serializeJson(doc, cfile) == 0) {
    cfile.close();
    return false;
  }

  cfile.close();
  return true;
}


// -------------------------------------------------------
// SAVE BOARD DATA TO FILE BOARD_CONFIG.JSN
// -------------------------------------------------------
bool CONTROLLER_DATA::SaveBoardConfiguration() {
  LittleFS.remove(file_board_config);

  // Open file for writing
  File bfile = LittleFS.open(file_board_config, "w");
  if (!bfile) {
    return false;
  } else {
    JsonDocument doc_brd;

    // Set the values in the document
    doc_brd["board"] = board;
    doc_brd["maxstepmode"] = maxstepmode;
    doc_brd["stepmode"] = stepmode;
    doc_brd["enpin"] = enablepin;
    doc_brd["steppin"] = steppin;
    doc_brd["dirpin"] = dirpin;
    doc_brd["temppin"] = temppin;
    doc_brd["brdnum"] = boardnumber;
    doc_brd["stepsrev"] = stepsperrev;
    doc_brd["fixedsmode"] = fixedstepmode;
    for (int i = 0; i < 4; i++) {
      doc_brd["brdpins"][i] = boardpins[i];
    }
    doc_brd["msdelay"] = msdelay;

    if (serializeJson(doc_brd, bfile) == 0) {
      bfile.close();
      delay(10);
      return false;
    }
    bfile.close();
  }
  delay(10);
  return true;
}


// -------------------------------------------------------
// LOAD BOARD DATA CONFIGURATION
// -------------------------------------------------------
bool CONTROLLER_DATA::LoadBrdConfigStart(String brdfile) {
  File bfile = LittleFS.open(brdfile, "r");
  if (!bfile) {
    return false;
  } else {
    // read file and deserialize
    String fdata;
    fdata.reserve(BOARDDATASIZE);
    fdata = bfile.readString();
    bfile.close();

    JsonDocument doc_brd;

    DeserializationError jerror = deserializeJson(doc_brd, fdata);
    if (jerror) {
      delay(10);
      return false;
    }

    // save the brd_data just read from board
    // config file (brdfile) into board_config.jsn
    // Set the board values from doc_brd
    board = doc_brd["board"].as<const char *>();
    maxstepmode = doc_brd["maxstepmode"];
    stepmode = doc_brd["stepmode"];
    enablepin = doc_brd["enpin"];
    steppin = doc_brd["steppin"];
    dirpin = doc_brd["dirpin"];
    temppin = doc_brd["temppin"];
    boardnumber = doc_brd["brdnum"];

    // brdstepsperrev comes from STEPSPERREVOLUTION
    // and will be different so must override the
    // default setting in the board files
    switch (myboardnumber) {
      case PRO2EULN2003:
      case PRO2EL298N:
      case PRO2EL293DMINI:
      case PRO2EL9110S:
      case PRO2EL293DNEMA:
      case PRO2EL293D28BYJ48:
        stepsperrev = mystepsperrev;
        // override STEPSPERREVOLUTION from controller_config.h
        break;
      default:
        stepsperrev = doc_brd["stepsrev"];
        break;
    }
    // myfixedstepmode comes from FIXEDSTEPMODE
    // and will be different so must override
    // the default setting in the board files
    switch (myboardnumber) {
      case WEMOSDRV8825H:
      case WEMOSDRV8825:
      case PRO2EDRV8825:
        fixedstepmode = myfixedstepmode;
        // override FIXEDSTEPMODE from controller_config.h
        break;
      default:
        fixedstepmode = doc_brd["fixedsmode"];
        break;
    }
    for (int i = 0; i < 4; i++) {
      boardpins[i] = doc_brd["brdpins"][i];
    }
    msdelay = doc_brd["msdelay"];
    SaveBoardConfiguration();
    delay(10);
    return true;
  }
  return false;
}


// -------------------------------------------------------
// CREATE BOARD CONFIGURATION FROM JSON STRING
// legacy orphaned code
// DO NOT TRY THIS
// kept here in case I change my mind about something
// -------------------------------------------------------
bool CONTROLLER_DATA::CreateBoardConfigfromjson(String jsonstr) {
  // generate board configuration from json string
  JsonDocument doc_brd;

  // Deserialize the JSON document
  DeserializationError jerror = deserializeJson(doc_brd, jsonstr);
  if (jerror) {
    LoadDefaultBoardData();
    return false;
  } else {
    board = doc_brd["board"].as<const char *>();
    maxstepmode = doc_brd["maxstepmode"];
    stepmode = doc_brd["stepmode"];
    enablepin = doc_brd["enpin"];
    steppin = doc_brd["steppin"];
    dirpin = doc_brd["dirpin"];
    temppin = doc_brd["temppin"];
    boardnumber = doc_brd["brdnum"];
    // brdstepsperrev comes from STEPSPERREVOLUTION
    // and will be different so must override the
    // default setting in the board files
    switch (myboardnumber) {
      case PRO2EULN2003:
      case PRO2EL298N:
      case PRO2EL293DMINI:
      case PRO2EL9110S:
      case PRO2EL293DNEMA:
      case PRO2EL293D28BYJ48:
        stepsperrev = mystepsperrev;
        // override STEPSPERREVOLUTION from
        // controller_config.h
        break;
      default:
        stepsperrev = doc_brd["stepsrev"];
        break;
    }
    // myfixedstepmode comes from FIXEDSTEPMODE and
    // will be different so must override the
    // default setting in the board files
    switch (myboardnumber) {
      case WEMOSDRV8825H:
      case WEMOSDRV8825:
      case PRO2EDRV8825:
        fixedstepmode = myfixedstepmode;
        // override FIXEDSTEPMODE in
        // controller_config.h
        break;
      default:
        fixedstepmode = doc_brd["fixedsmode"];
        break;
    }
    for (int i = 0; i < 4; i++) {
      boardpins[i] = doc_brd["brdpins"][i];
    }
    msdelay = doc_brd["msdelay"];
    SaveBoardConfiguration();
    delay(10);
    return true;
  }
}

// -------------------------------------------------------
// Controller Config Persistant Data get and set methods
// -------------------------------------------------------
// FOCUSER POSITION
long CONTROLLER_DATA::get_fposition() {
  return fposition;
}

void CONTROLLER_DATA::set_fposition(long fposition) {
  fposition = fposition;
}

// FOCUSER DIRECTION
byte CONTROLLER_DATA::get_focuserdirection() {
  return focuserdirection;
}

void CONTROLLER_DATA::set_focuserdirection(byte newdir) {
  focuserdirection = newdir;
}

// MAXSTEPS
long CONTROLLER_DATA::get_maxstep() {
  return maxstep;
}

void CONTROLLER_DATA::set_maxstep(long newval) {
  StartDelayedUpdate(maxstep, newval);
}

// ALPACA SERVER ENABLE
bool CONTROLLER_DATA::get_alpacasrvr_enable(void) {
  return alpacasrvr_enable;
}

void CONTROLLER_DATA::set_alpacasrvr_enable(bool newstate) {
  StartDelayedUpdate(alpacasrvr_enable, newstate);
}

// BACKLASH
byte CONTROLLER_DATA::get_backlashsteps_in(void) {
  return backlashsteps_in;
}

void CONTROLLER_DATA::set_backlashsteps_in(byte newval) {
  StartDelayedUpdate(backlashsteps_in, newval);
}

byte CONTROLLER_DATA::get_backlashsteps_out(void) {
  return backlashsteps_out;
}

void CONTROLLER_DATA::set_backlashsteps_out(byte newval) {
  StartDelayedUpdate(backlashsteps_out, newval);
}

// COILPOWER
bool CONTROLLER_DATA::get_coilpower_enable(void) {
  return coilpower_enable;
  // state of coil power, 0 = !enabled, 1= enabled
}

void CONTROLLER_DATA::set_coilpower_enable(bool newstate) {
  StartDelayedUpdate(coilpower_enable, newstate);
}

// DELAY AFTER MOVE
byte CONTROLLER_DATA::get_delayaftermove_time(void) {
  return delayaftermove_time;
}

void CONTROLLER_DATA::set_delayaftermove_time(byte newtime) {
  StartDelayedUpdate(delayaftermove_time, newtime);
}

// DEVICENAME
String CONTROLLER_DATA::get_devicename(void) {
  return devicename;
}

void CONTROLLER_DATA::set_devicename(String newstr) {
  String sav = newstr.substring(0, 10);  // 11 chars
  StartDelayedUpdate(devicename, sav);
  // update cached var
  snprintf(DeviceName, sizeof(DeviceName), "%s%c", sav, 0x00);
}

// DISPLAY
bool CONTROLLER_DATA::get_display_enable(void) {
  return display_enable;
}

void CONTROLLER_DATA::set_display_enable(bool newstate) {
  StartDelayedUpdate(display_enable, newstate);
}


bool CONTROLLER_DATA::get_display_updateonmove(void) {
  return display_updateonmove;
}

void CONTROLLER_DATA::set_display_updateonmove(bool newstate) {
  StartDelayedUpdate(display_updateonmove, newstate);
}

String CONTROLLER_DATA::get_display_pageoption(void) {
  String tmp = display_pageoption;
  while (tmp.length() < 6) {
    tmp = tmp + "0";
  }
  tmp = tmp + "";
  display_pageoption = tmp;
  return display_pageoption;
}

void CONTROLLER_DATA::set_display_pageoption(String newoption) {
  String tmp = newoption;
  while (tmp.length() < 6) {
    tmp = tmp + "0";
  }
  tmp = tmp + "";
  display_pageoption = tmp;
  StartDelayedUpdate(display_pageoption, newoption);
}

// DUCKDNS
bool CONTROLLER_DATA::get_duckdns_enable(void) {
  return duckdns_enable;
}

void CONTROLLER_DATA::set_duckdns_enable(bool newstate) {
  StartDelayedUpdate(duckdns_enable, newstate);
}

String CONTROLLER_DATA::get_duckdns_domain(void) {
  return duckdns_domain;
}

void CONTROLLER_DATA::set_duckdns_domain(String newdomain) {
  StartDelayedUpdate(duckdns_domain, newdomain);
}

String CONTROLLER_DATA::get_duckdns_token(void) {
  return duckdns_token;
}

void CONTROLLER_DATA::set_duckdns_token(String newtoken) {
  StartDelayedUpdate(duckdns_token, newtoken);
}

// MDNS Name
String CONTROLLER_DATA::get_mdnsname(void) {
  return mdnsname;
}

void CONTROLLER_DATA::set_mdnsname(String newstr) {
  String sav = newstr.substring(0, 10);
  StartDelayedUpdate(mdnsname, sav);
  // update cached var
  snprintf(MDNSName, sizeof(MDNSName), "%s%c", sav, 0x00);
}

// MANAGEMENT SERVER
bool CONTROLLER_DATA::get_mngsrvr_enable(void) {
  return mngsrvr_enable;
}

void CONTROLLER_DATA::set_mngsrvr_enable(bool newstate) {
  StartDelayedUpdate(mngsrvr_enable, newstate);
}

// MOTOR SPEED
byte CONTROLLER_DATA::get_motorspeed(void) {
  return motorspeed;
}

void CONTROLLER_DATA::set_motorspeed(byte newval) {
  StartDelayedUpdate(motorspeed, newval);
}

// POWERDOWN
bool CONTROLLER_DATA::get_powerdown_enable(void) {
  return powerdown_enable;
}

void CONTROLLER_DATA::set_powerdown_enable(bool newstate) {
  StartDelayedUpdate(powerdown_enable, newstate);
}

// POWERDOWN Enable
int CONTROLLER_DATA::get_powerdown_time(void) {
  return powerdown_time;
}

// POWERDOWN Time
void CONTROLLER_DATA::set_powerdown_time(int newstate) {
  StartDelayedUpdate(powerdown_time, newstate);
}

// REVERSE
bool CONTROLLER_DATA::get_reverse_enable(void) {
  return reverse_enable;
  // state of reverse direction, 0 = !enabled, 1= enabled
}

void CONTROLLER_DATA::set_reverse_enable(bool newstate) {
  StartDelayedUpdate(reverse_enable, newstate);
}

// STEPMODE
// Handled by DriverBoard

// STEPSIZE
float CONTROLLER_DATA::get_stepsize(void) {
  return stepsize;
  // this is the actual measured focuser
  // stepsize in microns and is reported
  // to ASCOM, so must be valid

  // the amount in microns that the focuser
  // tube moves in one step of the motor
}

void CONTROLLER_DATA::set_stepsize(float newval) {
  StartDelayedUpdate(stepsize, newval);
}

// TCPIP SERVER
bool CONTROLLER_DATA::get_tcpipsrvr_enable(void) {
  return tcpipsrvr_enable;
}

void CONTROLLER_DATA::set_tcpipsrvr_enable(bool newstate) {
  StartDelayedUpdate(tcpipsrvr_enable, newstate);
}

// TEMPERATURE
bool CONTROLLER_DATA::get_tempmode(void) {
  return tempmode;
  // temperature display mode, Celcius=1, Fahrenheit=0
}

void CONTROLLER_DATA::set_tempmode(bool newmode) {
  StartDelayedUpdate(tempmode, newmode);
  // temperature display mode, Celcius=1, Fahrenheit=0
}

bool CONTROLLER_DATA::get_tempprobe_enable(void) {
  return tempprobe_enable;
}

void CONTROLLER_DATA::set_tempprobe_enable(bool newstate) {
  StartDelayedUpdate(tempprobe_enable, newstate);
}

int CONTROLLER_DATA::get_tempcoefficient(void) {
  return tempcoefficient;
  // steps per degree temperature coefficient value (maxval=256)
}

void CONTROLLER_DATA::set_tempcoefficient(int newval) {
  StartDelayedUpdate(tempcoefficient, newval);
  // steps per degree temperature coefficient value (maxval=256)
}

bool CONTROLLER_DATA::get_tcdirection(void) {
  return tcdirection;
}

void CONTROLLER_DATA::set_tcdirection(bool newdirection) {
  StartDelayedUpdate(tcdirection, newdirection);
}

bool CONTROLLER_DATA::get_tempcomp_onload(void) {
  return tempcomp_onload;
}

void CONTROLLER_DATA::set_tempcomp_onload(bool newstate) {
  StartDelayedUpdate(tcdirection, newstate);
}

// WEB SERVER
bool CONTROLLER_DATA::get_websrvr_enable(void) {
  return websrvr_enable;
}

void CONTROLLER_DATA::set_websrvr_enable(bool newstate) {
  StartDelayedUpdate(websrvr_enable, newstate);
}


// -------------------------------------------------------
// Board Data get and set methods
// -------------------------------------------------------
String CONTROLLER_DATA::get_brdname() {
  return board;
}

int CONTROLLER_DATA::get_brdmaxstepmode() {
  return maxstepmode;
}

int CONTROLLER_DATA::get_brdstepmode() {
  return stepmode;
}

int CONTROLLER_DATA::get_brdenablepin() {
  return enablepin;
}

int CONTROLLER_DATA::get_brdsteppin() {
  return steppin;
}

int CONTROLLER_DATA::get_brddirpin() {
  return dirpin;
}

int CONTROLLER_DATA::get_brdtemppin() {
  return temppin;
}

int CONTROLLER_DATA::get_brdboardpins(int pinnum) {
  return boardpins[pinnum];
}

int CONTROLLER_DATA::get_brdnumber() {
  return boardnumber;
}

int CONTROLLER_DATA::get_brdstepsperrev() {
  return stepsperrev;
}

int CONTROLLER_DATA::get_brdfixedstepmode() {
  return fixedstepmode;
}

unsigned long CONTROLLER_DATA::get_brdmsdelay() {
  return msdelay;
}

// set
void CONTROLLER_DATA::set_brdname(String newstr) {
  StartBoardDelayedUpdate(board, newstr);
}

void CONTROLLER_DATA::set_brdmaxstepmode(int newval) {
  StartBoardDelayedUpdate(maxstepmode, newval);
}

void CONTROLLER_DATA::set_brdstepmode(int newval) {
  StartBoardDelayedUpdate(stepmode, newval);
}

void CONTROLLER_DATA::set_brdenablepin(int pinnum) {
  StartBoardDelayedUpdate(enablepin, pinnum);
}

void CONTROLLER_DATA::set_brdsteppin(int pinnum) {
  StartBoardDelayedUpdate(steppin, pinnum);
}

void CONTROLLER_DATA::set_brddirpin(int pinnum) {
  StartBoardDelayedUpdate(dirpin, pinnum);
}

void CONTROLLER_DATA::set_brdtemppin(int pinnum) {
  StartBoardDelayedUpdate(temppin, pinnum);
}

void CONTROLLER_DATA::set_brdboardpins(int pinnum) {
  StartBoardDelayedUpdate(boardpins[pinnum], pinnum);
}

void CONTROLLER_DATA::set_brdnumber(int newval) {
  StartBoardDelayedUpdate(boardnumber, newval);
}

void CONTROLLER_DATA::set_brdfixedstepmode(int newval) {
  StartBoardDelayedUpdate(fixedstepmode, newval);
}

void CONTROLLER_DATA::set_brdstepsperrev(int stepsrev) {
  StartBoardDelayedUpdate(stepsperrev, stepsrev);
}

void CONTROLLER_DATA::set_brdmsdelay(unsigned long newval) {
  StartBoardDelayedUpdate(msdelay, newval);
}


// -------------------------------------------------------
// Delayed Write routines which update the focuser setting
// with the new value, then sets a flag for when the
// focuser persistant data should be written to file
// -------------------------------------------------------
void CONTROLLER_DATA::StartDelayedUpdate(bool &org_data, bool new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(byte &org_data, byte new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(int &org_data, int new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(unsigned int &org_data, unsigned int new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(long &org_data, long new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(unsigned long &org_data, unsigned long new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(float &org_data, float new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartDelayedUpdate(String &org_data, String new_data) {
  if (org_data != new_data) {
    ReqSaveData_per = true;
    SnapShotMillis = millis();
    org_data = new_data;
  }
}


// -------------------------------------------------------
// Delayed Write routines which update the Board setting with the
// new value, then sets a flag for when the board data should be written to file
// -------------------------------------------------------
void CONTROLLER_DATA::StartBoardDelayedUpdate(int &org_data, int new_data) {
  if (org_data != new_data) {
    ReqSaveBoard_var = true;
    BoardSnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(unsigned long &org_data, unsigned long new_data) {
  if (org_data != new_data) {
    ReqSaveBoard_var = true;
    BoardSnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(byte &org_data, byte new_data) {
  if (org_data != new_data) {
    ReqSaveBoard_var = true;
    BoardSnapShotMillis = millis();
    org_data = new_data;
  }
}

void CONTROLLER_DATA::StartBoardDelayedUpdate(String &org_data, String new_data) {
  if (org_data != new_data) {
    ReqSaveBoard_var = true;
    BoardSnapShotMillis = millis();
    org_data = new_data;
  }
}
