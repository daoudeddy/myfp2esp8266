// -------------------------------------------------------
// myFP2ESP8266 FOCUSER CONFIGURATION CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// controller_data.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------
#include <Arduino.h>
#include "boarddefs.h"
#include "config.h"


// -------------------------------------------------------
// CONTROLLER_DATA CLASS
// -------------------------------------------------------
class CONTROLLER_DATA {
public:
  CONTROLLER_DATA(void);
  bool LoadConfiguration(void);

  // attempt to load a board config file [DRVBRD] 
  // immediately after a firmware reprogram
  bool LoadBrdConfigStart(String);
  void LoadDefaultBoardData(void);

  bool SaveConfiguration(long, byte);
  bool SaveVariableConfiguration(long, bool);
  bool SavePersitantConfiguration(void);
  bool SaveBoardConfiguration(void);
  bool SaveNow(long, bool);

  void SetFocuserDefaults(void);

  // create a board config from a json string 
  // used by Management Server
  // Legacy
  bool CreateBoardConfigfromjson(String);

  void init_cachevars(void);

  long get_fposition(void);
  long get_maxstep(void);
  byte get_focuserdirection(void);
  void set_fposition(long);
  void set_maxstep(long);
  void set_focuserdirection(byte);

  // SERVERS AND SERVICES
  bool get_alpacasrvr_enable(void);
  bool get_duckdns_enable(void);
  bool get_mngsrvr_enable(void);
  bool get_tcpipsrvr_enable(void);
  bool get_websrvr_enable(void);

  void set_alpacasrvr_enable(bool);
  void set_duckdns_enable(bool);
  void set_mngsrvr_enable(bool);
  void set_tcpipsrvr_enable(bool);
  void set_websrvr_enable(bool);

  // ALPACA

  // BACKLASH
  byte get_backlashsteps_in(void);
  byte get_backlashsteps_out(void);
  void set_backlashsteps_in(byte);
  void set_backlashsteps_out(byte);

  // COIL POWER enabled = ON, disabled = OFF
  bool get_coilpower_enable(void);
  void set_coilpower_enable(bool);

  // DELAY AFTER MOVE
  byte get_delayaftermove_time(void);
  void set_delayaftermove_time(byte);

  // Device Name
  String get_devicename(void);
  void set_devicename(String);

  // DISPLAY
  bool get_display_enable(void);
  void set_display_enable(bool);
  bool get_display_updateonmove(void);
  void set_display_updateonmove(bool);
  String get_display_pageoption(void);
  void set_display_pageoption(String);

  // DUCKDNS
  String get_duckdns_domain(void);
  String get_duckdns_token(void);
  void set_duckdns_domain(String);
  void set_duckdns_token(String);

  // MDNS Name
  String get_mdnsname(void);
  void set_mdnsname(String);

  // MOTORSPEED
  byte get_motorspeed(void);
  void set_motorspeed(byte);

  // POWERDOWN
  // Handles Display
  bool get_powerdown_enable(void);
  void set_powerdown_enable(bool);
  int  get_powerdown_time(void);
  void set_powerdown_time(int);

  // REVERSE
  bool get_reverse_enable(void);
  void set_reverse_enable(bool);

  // STEPSIZE
  float get_stepsize(void);
  void set_stepsize(float);

  // TEMPERATURE PROBE
  bool get_tempprobe_enable(void);
  void set_tempprobe_enable(bool);
  bool get_tempmode(void);  // C or F
  void set_tempmode(bool);
  int get_tempcoefficient(void);
  void set_tempcoefficient(int);
  bool get_tcdirection(void);
  void set_tcdirection(bool);
  bool get_tempcomp_onload(void);
  void set_tempcomp_onload(bool);

  // BOARD CONFIGURATIONS
  String get_brdname(void);
  int get_brdmaxstepmode(void);
  int get_brdstepmode(void);
  int get_brdenablepin(void);
  int get_brdsteppin(void);
  int get_brddirpin(void);
  int get_brdtemppin(void);
  int get_brdboardpins(int);
  int get_brdstepsperrev(void);
  int get_brdfixedstepmode(void);
  unsigned long get_brdmsdelay(void);
  int get_brdnumber(void);
  int get_fixedstepmode(void);
  int get_stepsperrev(void);

  // set boardconfig
  void set_brdname(String);
  void set_brdmaxstepmode(int);
  void set_brdstepmode(int);
  void set_brdenablepin(int);
  void set_brdsteppin(int);
  void set_brddirpin(int);
  void set_brdtemppin(int);
  void set_brdboardpins(int);
  void set_brdstepsperrev(int);
  void set_brdfixedstepmode(int);
  void set_brdmsdelay(unsigned long);
  void set_brdnumber(int);
  void set_fixedstepmode(int);
  void set_stepsperrev(int);

private:
  void LoadDefaultPersistantData(void);
  void LoadDefaultVariableData(void);
  void LoadBoardConfiguration(void);
  void SetDefaultBoardData(void);

  void StartDelayedUpdate(bool &, bool);
  void StartDelayedUpdate(byte &, byte);
  void StartDelayedUpdate(int &, int);
  void StartDelayedUpdate(unsigned int &, unsigned int);
  void StartDelayedUpdate(long &, long);
  void StartDelayedUpdate(unsigned long &, unsigned long);
  void StartDelayedUpdate(float &, float);
  void StartDelayedUpdate(String &, String);

  void StartBoardDelayedUpdate(byte &, byte);
  void StartBoardDelayedUpdate(int &, int);
  void StartBoardDelayedUpdate(unsigned long &, unsigned long);
  void StartBoardDelayedUpdate(float &, float);
  void StartBoardDelayedUpdate(String &, String);

  bool ReqSaveData_var;   // Flag request save variable data
  bool ReqSaveData_per;   // Flag request save persitant data
  bool ReqSaveBoard_var;  // Flag request save board config

  // Controller JSON configuration
  const String file_cntlr_config = "/cntlr_config.jsn";  
  // variable JSON setup data, position and direction
  const String file_cntlr_var = "/cntlr_var.jsn";        
  // board JSON configuration
  const String file_board_config = "/board_config.jsn";  

  long fposition;          // last focuser position
  long maxstep;            // max steps
  byte focuserdirection;   // last focuser move direction

  unsigned long SnapShotMillis;
  unsigned long BoardSnapShotMillis;

  // Loaded at boot time, if enabled is 1 then an 
  // attempt will be made to "start" and "run"

  // ALPACA
  bool alpacasrvr_enable;

  byte backlashsteps_in;     
  byte backlashsteps_out;

  bool coilpower_enable; // 0=off, 1=stay on
  
  byte delayaftermove_time; // milliseconds to wait after a move 

  String devicename;

  // DISPLAY
  bool display_enable;
  bool display_updateonmove;  // update position when moving
  String display_pageoption;  // which pages to show/hide
  
  // DUCKDNS
  bool duckdns_enable;
  String duckdns_domain;
  String duckdns_token;

  // MDNSNAME
  String mdnsname;

  // MOTORSPEED
  byte motorspeed;  // slow, medium or fast

  // POWERDOWN DISPLAY
  bool powerdown_enable;
  int powerdown_time;

  bool mngsrvr_enable;

  // REVERSE
  bool reverse_enable;  // if true, motor direction is reversed

  // STEPSIZE
  float stepsize;
  // the step size in microns, ie 7.2 - value * 10,
  // so real stepsize = stepsize / 10 (maxval = 25.6)

  bool tcpipsrvr_enable;

  bool tempprobe_enable;

  // TEMP MODE C/F
  bool tempmode;  // Celcius=1, Fahrenheit=0

  // TEMP COEFFIENT
  int tempcoefficient;  // steps per degree (Celcius)

  byte tcdirection;  // direction to move when tempcomp is ON
  byte tempcomp_onload;  // set tempcomp on bootup

  bool websrvr_enable;

  // DATASET BOARD CONFIGURATION
  String board;
  int maxstepmode;
  int stepmode;
  int enablepin;
  int steppin;
  int dirpin;
  int temppin;
  int boardnumber;
  int fixedstepmode;
  int stepsperrev;
  int boardpins[4];
  unsigned long msdelay;

  // these capture compile time settings and 
  // are required to initialize a board correctly
  // defined controller_config.h
  int myboardnumber = DRVBRD; 
  int myfixedstepmode = FIXEDSTEPMODE; 
  int mystepsperrev = STEPSPERREVOLUTION; 

  const char *T_LOADCONFIG = "LOAD CONFIG";
  
  /*
        { "board":"PRO2EDRV8825","brdnum":20,"maxstepmode":32,"stepmode":1,"enpin":14,"steppin":33,"dirpin":32,
        "temppin":13,"brdnum":60,"stepsrev":-1,"fixedsmode":-1,"brdpins":[27,26,25,-1],"mspeed":4000 }
  */

};
