// -------------------------------------------------------
// myFP2ESP8266 GENERAL DEFINITIONS
// Copyright Robert Brown 2014-2025.
// Copyright Holger M, 2019-2021.
// All Rights Reserved.
// defines.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#ifndef _defines_h_
#define _defines_h_

#include <Arduino.h>
#include <avr/pgmspace.h>


// -------------------------------------------------------
// DEFINITIONS: DO NOT CHANGE
// -------------------------------------------------------
enum Focuser_States { State_Idle,
                      State_InitMove,
                      State_Backlash,
                      State_Moving,
                      State_FinishedMove,
                      State_DelayAfterMove,
                      State_EndMove
                    };

enum logo_num { nwifi,
                ntemp,
                nreboot
              };

enum Display_States { DisplayOff = 0, DisplayOn };

enum Controller_Modes { accesspoint = 1, localserial, station };

enum IPaddress_Modes { accesspointip = 1, serialip, dynamicip, staticip  };


// CONTROLLER MODES
#define ACCESSPOINT 1
#define LOCALSERIAL 2
#define STATION 3


// STATIC-DYNAMIC CONTROLLER TCP/IP ADDRESS
// mystationipaddressmode = IPADDRESSMODE
#define ACCESSPOINTIP  1
#define LOCALSERIALIP  2
#define DYNAMICIP      3
#define STATICIP       4


// WIFI CONFIG TYPES
#define READWIFICONFIG 1
#define MULTIAP 2  // reserved for future use

// BUFFER SIZES
#define BUFFER8LEN 8
#define BUFFER12LEN 12
#define BUFFER16LEN 16
#define BUFFER32LEN 32
#define BUFFER40LEN 40
#define BUFFER48LEN 48
#define BUFFER64LEN 64


// -------------------------------------------------------
// EXTERNS START
// -------------------------------------------------------
// methods for checking range of varaiables/settings
extern void RangeCheck(int *, int, int);
extern void RangeCheck(long *, long, long);
extern void RangeCheck(unsigned long *, unsigned long, unsigned long);
extern void RangeCheck(float *, float, float);

extern void get_systemuptime();
extern void software_Reboot(int);
extern long getrssi(void);


// FOCUSER SETTINGS
// position              // driverboard->getposition()
// maxincrement          // ControllerData->get_maxstep()
// maxstep               // ControllerData->get_maxstep()
extern bool connecting;  // Returns true while the device is connecting or disconnecting.
extern long ftargetPosition;
extern bool isMoving;
extern float temp;
extern bool tempcomp_state;
extern bool tempcomp_available;

// CONTROLLER SETTINGS
// States
extern bool alpacasrvr_status;
extern bool display_found;
extern bool display_status;
extern bool duckdns_status;
extern bool mngsrvr_status;
extern bool ota_status;
extern bool serialsrvr_status;
extern bool tcpipsrvr_status;
extern bool tempprobe_found;
extern bool tempprobe_status;
extern bool websrvr_status;

extern volatile bool halt_alert;
extern int _display_type;

// Colors READONLY
extern char HeaderColor[BUFFER8LEN];
extern char TitleColor[BUFFER8LEN];
extern char SubTitleColor[BUFFER8LEN];
extern char TextColor[BUFFER8LEN];
extern char BackColor[BUFFER8LEN];
extern char FooterColor[BUFFER8LEN];
// Controller settings READONLY
extern char project_author[BUFFER32LEN];
extern char project_name[BUFFER32LEN];
extern char major_version[BUFFER8LEN];
extern char minor_version[BUFFER8LEN];
extern char DeviceName[BUFFER12LEN];
extern char MDNSName[BUFFER12LEN];

extern char mySSID[BUFFER64LEN];
extern char myAPSSID[BUFFER64LEN];
extern char ipStr[BUFFER16LEN];
extern char systemuptime[BUFFER12LEN];
extern bool filesystemloaded;
extern int  myboardnumber;
extern bool focuserDirection;
extern int  myfixedstepmode;
extern int  mystepsperrev;


// -------------------------------------------------------
// EXTERNS END
// -------------------------------------------------------

// undef's
#undef DRVBRD
#undef FIXEDSTEPMODE
#undef STEPSPERREVOLUTION
#undef CONTROLLERMODE
#undef IPADDRESSMODE
#undef READWIFICONFIG
#undef ENABLE_ALPACASERVER
#undef ENABLE_TCPIPSERVER
#undef ENABLE_WEBSERVER
#undef ENABLE_MDNS
#undef ENABLE_PWRDOWN
#undef ENABLE_TEMPERATUREPROBE
#undef DISPLAYTYPE
#undef DRIVERTYPE
#undef ENABLE_DUCKDNS


// -------------------------------------------------------
// COMMON DEFINES 
// -------------------------------------------------------
// AVAILABLE
#define NOT_AVAILABLE false
#define AVAILABLE     true

// FOUND
#define NOT_FOUND false
#define FOUND     true
// STATE ON OFF
#define STATE_OFF false
#define STATE_ON  true
// FILES
#define FILE_NOTFOUND false 
#define FILE_FOUND    true
// CONNECTION
#define STATE_NOTCONNECTED false
#define STATE_CONNECTED    true
// LOADED
#define STATE_NOTLOADED false
#define STATE_LOADED    true
// STATE ENABLED DISABLED
#define STATE_DISABLED  false
#define STATE_ENABLED   true
// STATUS
#define STATUS_STOPPED  false
#define STATUS_RUNNING  true
// TEMPERATURE MODE
#define FAHRENHEIT  false
#define CELSIUS     true
// Temperature Compensation Direction
#define TC_DIRECTION_IN  false
#define TC_DIRECTION_OUT true
// HTTP REQUESTS
#define GeT  true
#define PosT false 
// FOCUSER MOVE DIRECTION                   
#define moving_in false
#define moving_out !moving_in


// -------------------------------------------------------
// I2C
// SDA = GPIO4
// SCL = GPIO5
// -------------------------------------------------------
#define I2CDATAPIN 4
#define I2CCLKPIN  5


// -------------------------------------------------------
// ALPACA, MANAGEMENT and WEB SERVERS
// https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
// -------------------------------------------------------
#define HTML_WEBPAGE      200 
#define HTML_REDIRECTURL  301
#define BADREQUESTWEBPAGE 400
#define HTML_NOTFOUND     404
#define HTML_SERVERERROR  500


// -------------------------------------------------------
// DISPLAY (use I2Cscanner to find the correct address)
// -------------------------------------------------------
#define OLED_ADDR  0x3C // some displays maybe at 0x3D or 0x3F
#define DISPLAYPAGETIME 4 // default page time
// number of steps before updating position when moving if oledupdateonmove is 1
#define DISPLAYUPDATEONMOVE 15    
#define DISPLAYPAGETIMEDEFAULT 4
// Display types
#define DISPLAY_NONE       0
#define TEXT_OLED12864     1
#define LILYGO_OLED6432    2
#define GRAPHIC_OLED12864  3
// Display Driver types
#define _SSD1306_   1
#define _LILYGO_    2
#define _SSH1106_   3


// -------------------------------------------------------
// DUCKDNS
// -------------------------------------------------------
// check ip address every 2 minutes for an update
// value is in seconds. Read only.
#define DUCKDNS_REFRESHRATE 120  


// -------------------------------------------------------
// MOTOR
// -------------------------------------------------------
#define DEFAULTSTEPSIZE 50.0f
#define MINIMUMSTEPSIZE 0.001f
#define MAXIMUMSTEPSIZE 100.0f

#define DEFAULTPOSITION 5000U
// Default maxStep 80,000 steps
#define DEFAULTMAXSTEPS 80000U 
#define FOCUSERLOWERLIMIT 1024U 
// arbitary focuser limit 500,000 steps
#define FOCUSERUPPERLIMIT 500000U 

#define DEFAULT_MOTORSPEEDDELAY 4000U
#define DEFAULT_MOTORSPEEDDELAYMIN 500U
#define DEFAULT_MOTORSPEEDDELAYMAX 14000U
// motor speed
#ifndef SLOW
#define SLOW 0
#endif
#ifndef MED
#define MED 1
#endif
#ifndef FAST
#define FAST 2
#endif
// stepsize
#ifndef STEP1
#define STEP1 1
#endif
#ifndef STEP2
#define STEP2 2
#endif
#ifndef STEP4
#define STEP4 4
#endif
#ifndef STEP8
#define STEP8 8
#endif
#ifndef STEP16
#define STEP16 16
#endif
#ifndef STEP32
#define STEP32 32
#endif

// STEPMODES
// no longer required here

// MoveTimer (driver_board.cpp), ESP8266TimerInterrupt
// Cannot use Timer0, Only timer1 is available


// -------------------------------------------------------
// PORTS
// DO NOT CHANGE MNGSERVERPORT
// TCPIP SERVER is required for Windows+ASCOM+LINUX+INDI apps
// -------------------------------------------------------
#define ALPACASERVERPORT 4040      // ALPACA port
#define ALPACADISCOVERYPORT 32227  // ALPACA Discovery UDP
#define MNGSERVERPORT 6060         // Management Server Port
#define TCPIPSERVERPORT 2020       // TCPIP Server Port
#define WEBSERVERPORT 80           // Web Server port


// -------------------------------------------------------
// LOCALSERIAL
// -------------------------------------------------------
// 9600, 14400, 19200, 28800, 38400, 57600, 115200
#define SERIALPORTSPEED  115200

// number of commands in queue
#define MAXCOMMAND  10


// -------------------------------------------------------
// TEMPERATURE PROBE
// -------------------------------------------------------
#define DEFAULTTEMPREFRESHTIME  3000 // in ms  
#define DEFAULTTEMPRESOLUTION  10
#define DEFAULTLASTTEMP  20.0f
#define DEFAULTEMPPIN  10
// Set the default DS18B20 resolution to 0.25 of a degree 9=0.5, 10=0.25, 11=0.125, 12=0.0625

#define REBOOTDELAY  2000  // in ms, delay before reboot occurs
#define DEFAULTSAVETIME  120000L // in ms, 120 seconds


// -------------------------------------------------------
// DO NOT CHANGE
// -------------------------------------------------------
extern const char TEXTPAGETYPE[10];
extern const char PLAINTEXTPAGETYPE[11];
extern const char JSONTEXTPAGETYPE[10];
extern const char JSONAPPTYPE[17];

extern const char T_GET[5];
extern const char T_POST[6];
extern const char T_PUT[5];

extern const char H_FILENOTFOUNDSTR[196];
extern const char H_FSNOTLOADEDSTR[211];
extern const char T_FILESYSTEMERROR[28];

extern const char T_ACCESSPOINT[13];
extern const char T_STATION[9];
extern const char T_LOCALSERIAL[13];
extern const char T_CELSIUS[8];
extern const char T_FAHRENHEIT[11];
//extern const char T_SERIAL[7];
extern const char T_SSID[6];
extern const char T_DEVICENAME[12]; 
extern const char T_NO[3];
extern const char T_YES[4];

extern const char T_OK[3];
extern const char T_NOTOK[4];

extern const char T_DISABLED[9];
extern const char T_ENABLED[8];

extern const char T_FOUND[8];
extern const char T_NOTFOUND[12];

extern const char T_IN[3];
extern const char T_OUT[4];

extern const char TUC_START[7];
extern const char TLC_START[6];
extern const char T_START[7];
extern const char T_ERRSTART[11];

extern const char TUC_STOP[6];
extern const char TLC_STOP[5];
extern const char T_STOP[6];
extern const char TUC_STOPPED[8];
extern const char T_STOPPED[8];

extern const char T_RUNNING[8];
extern const char TUC_RUNNING[8];

extern const char T_MOVETO[7];
extern const char T_TARGET[9];
extern const char T_GOTO[6];
extern const char T_TRUE[5];
extern const char T_FALSE[6];
extern const char T_ADDRESS[9];
extern const char TLC_ENABLE[7];
extern const char TUC_ENABLE[7];
extern const char TLC_DISABLE[8];
extern const char TUC_DISABLE[8];

extern const char T_OFF[4];
extern const char TLC_OFF[4];
extern const char T_ON[3];
extern const char TLC_ON[3];

extern const char T_ERROR[7];
extern const char T_SPACE[2];
extern const char T_SELECTED[9];
extern const char T_CHECKED[8];
extern const char T_CHECK[7];

extern const char T_PORT[6];
extern const char T_ALPACASERVER[13];
extern const char T_CNTLRDATA[16];
extern const char T_DISPLAY[9];
extern const char T_DISPLAYTEXT[13];
extern const char T_DISPLAYGRAPHIC[16];
extern const char T_DISPLAYNONE[13];
extern const char T_DISPLAYLILYGO[15];
extern const char T_DRIVERBOARD[14];
extern const char T_DUCKDNS[9];
extern const char T_MNGMNTSERVER[13];
extern const char T_MDNS[6];
extern const char T_TCPIPSERVER[12];
extern const char T_TEMPPROBE[11]; 
extern const char T_WEBSERVER[10];
extern const char T_NOTSUPPORTED[15];
extern const char T_NOTLOADED[11];
extern const char T_UPDATE[7];
//extern const char T_CHECKLOOP[15];


#endif  // _controller_defines_h
