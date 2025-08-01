// -------------------------------------------------------
// myFP2ESP8266 MANAGEMENT SERVER CLASS DEFINITIONS
// Copyright Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// management_server.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#undef DEBUG_ESP_HTTP_SERVER
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// -------------------------------------------------------
// MANAGEMENT SERVER CLASS
// -------------------------------------------------------
class MANAGEMENT_SERVER {
public:
  MANAGEMENT_SERVER();
  bool start(void);
  void stop(void);
  void loop(void); 
  void load_navbar(void);
  void get_navbar(void);
  bool is_hexdigit(char);

  // admin
  void get_servers(void);
  void get_duckdns(void);
  void get_backlash(void);
  void get_display(void);
  void get_temp(void);
  void get_links(void);
  void get_misc(void);
  void get_motor(void);

  void get_reboot(void);
  void get_wait(void);
  
  // file handlers
  void get_notfound(void);
  void get_filelist(void);
  void get_deletefile(void);
  void handler_postdeletefile();
  void get_uploadfile(void);
  void handler_postuploadstart(void);
  void handler_postuploadfile(void);
  void handler_fileread(String);
  void handler_success(void);
  void handler_saveconfig(void);
  void handler_notsavedconfig(void);
  void get_cntlrconfig(void);
  void get_cntlrvar(void);
  void get_boardconfig(void);

  String get_uri(void);
  
  // xhtml
  void get_position(void);
  void get_ismoving(void);
  void get_rssi(void);
  void get_targetposition(void);
  void get_heap(void);
  void get_sut(void);

private:
  bool check_access(void);
  void Send_NoPage(void);
  void send_json(String);
  void send_redirect(String);
  String get_contenttype(String);
  void ListAllFilesInDir(String);

  bool _loaded = STATE_NOTLOADED;
  String _navbar;   
  String _filelist;
  String _errormsg;
  File _fsUploadFile;
  ESP8266WebServer *mserver;

  const char T_SERVERS[10]  = "/servers ";
  const char T_DUCKDNS[10]  = "/duckdns ";
  const char T_BACKLASH[11] = "/backlash ";
  const char T_DISPLAY[10]  = "/display ";
  const char T_TEMP[8]      = "/temp " ;
  const char T_MISC[7]      = "/misc ";
  const char T_MOTOR[8]     = "/motor ";
  const char T_SYSTEM[9]    = "/system ";
  const char T_DELETE[9]    = "/delete ";
  const char T_LIST[7]      = "/list ";
  const char T_LINKS[8]     = "/links ";
  const char T_NOTFOUND[11] = "/notfound ";
  const char T_UPLOAD[9]    = "/upload ";
  const char T_CONFIGSAVED[14] = "/configsaved "; 
  const char T_CONFIGNOTSAVED[17] = "/confignotsaved ";
  const char T_SUCCESS[10]  = "/success "; 
  const char T_DELETEOK[11] = "/deleteok "; 
  
};
