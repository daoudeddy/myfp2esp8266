// -------------------------------------------------------
// myFP2ESP8266 GENERAL DEFINITIONS CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// defines.cpp
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#include <Arduino.h>
#include <avr/pgmspace.h>
#include "defines.h"


// -------------------------------------------------------
// CONTROLLER DATA REFERENCES
// -------------------------------------------------------
const char TEXTPAGETYPE[10] = "text/html";
const char PLAINTEXTPAGETYPE[11] = "text/plain";
const char JSONTEXTPAGETYPE[10] = "text/json";
const char JSONAPPTYPE[17] = "application/json";

const char T_GET[5]  = "GET ";
const char T_POST[6] = "POST ";
const char T_PUT[5]  = "PUT ";

const char H_FILENOTFOUNDSTR[196] = "<html><head><title>myFP2ESP8266</title></head><body><p>myFP2ESP8266</p><p>File not found</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>";
const char H_FSNOTLOADEDSTR[211] = "<html><head><title>myFP2ESP8266</title></head><body><p>myFP2ESP8266</p><p>err: File-system not started.</p><p><form action=\"/\" method=\"GET\"><input type=\"submit\" value=\"HOMEPAGE\"></form></p></body></html>";
const char T_FILESYSTEMERROR[28] = "ERROR FileSystem not loaded";

const char T_ACCESSPOINT[13] = "ACCESSPOINT ";
const char T_STATION[9] = "STATION ";
const char T_LOCALSERIAL[13] = "LOCALSERIAL ";
const char T_CELSIUS[8] = "Celsius";
const char T_FAHRENHEIT[11] = "Fahrenheit";
//const char T_SERIAL[7] = "Serial";
const char T_SSID[6] = "SSID ";
const char T_DEVICENAME[12] = "DeviceName ";
const char T_NO[3] = "No";
const char T_YES[4] = "Yes";

const char T_OK[3] = "OK";
const char T_NOTOK[4] = "!OK";

const char T_DISABLED[9] = "Disabled";
const char T_ENABLED[8] = "Enabled";

const char T_FOUND[8] = " found ";
const char T_NOTFOUND[12] = "err !found ";

const char T_IN[3] = "In";
const char T_OUT[4] = "Out";

const char TUC_START[7] = "START ";
const char TLC_START[6] = "start";
const char T_START[7] = "Start ";
const char T_ERRSTART[11] = "err start ";

const char TUC_STOP[6] = "STOP ";
const char TLC_STOP[5] = "stop";
const char T_STOP[6] = "Stop ";
const char TUC_STOPPED[8] = "STOPPED";
const char T_STOPPED[8] = "Stopped";

const char T_RUNNING[8] = "Running";
const char TUC_RUNNING[8] = "RUNNING";

const char T_MOVETO[7] = " Move ";
const char T_TARGET[9] = "Target ";
const char T_GOTO[6] = "Goto ";
const char T_TRUE[5] = "True";
const char T_FALSE[6] = "False";
const char T_ADDRESS[9] = "Address ";
const char TLC_ENABLE[7] = "enable";   
const char TUC_ENABLE[7] = "ENABLE";
const char TLC_DISABLE[8] = "disable";
const char TUC_DISABLE[8] = "DISABLE";

const char T_OFF[4] = "OFF";
const char TLC_OFF[4] = "off";
const char T_ON[3] = "ON";
const char TLC_ON[3] = "on";

const char T_ERROR[7] = "Error ";
const char T_SPACE[2]  = " ";
const char T_SELECTED[9] = "Selected";
const char T_CHECKED[8] = "Checked";
const char T_CHECK[7] = "CHECK ";

const char T_PORT[6] = "PORT ";
const char T_ALPACASERVER[13] = "ALPACA-SRVR ";  // txt names
const char T_CNTLRDATA[16]    = "CONTROLLERDATA ";
const char T_DISPLAY[9]       = "DISPLAY ";
const char T_DISPLAYTEXT[13]  = "DISPLAYTEXT ";
const char T_DISPLAYGRAPHIC[16] = "DISPLAYGRAPHIC ";
const char T_DISPLAYNONE[13]    = "DISPLAYNONE ";
const char T_DISPLAYLILYGO[15]  = "DISPLAYLILYGO ";
const char T_DRIVERBOARD[14]  = "DRIVER BOARD ";
const char T_DUCKDNS[9]       = "DUCKDNS ";
const char T_MNGMNTSERVER[13] = "MNGMNT-SRVR ";
const char T_MDNS[6]          = "MDNS ";
const char T_TCPIPSERVER[12]  = "TCPIP-SRVR ";
const char T_TEMPPROBE[11]    = "TEMPPROBE ";
const char T_WEBSERVER[10]    = "WEB-SRVR ";
const char T_NOTSUPPORTED[15] = "NOT SUPPORTED ";
const char T_NOTLOADED[11]    = "NOT LOADED";
const char T_UPDATE[7]        = "UPDATE";
