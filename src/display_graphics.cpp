// -------------------------------------------------------
// myFP2ESP8266 GRAPHICS DISPLAY CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2021. All Rights Reserved.
// display_graphics.cpp
// Optional
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

// -------------------------------------------------------
// REQUIRES THE FOLLOWING LIBRARY
// -------------------------------------------------------
// Download and install the library from
//   https://github.com/ThingPulse/esp8266-oled-ssd1306


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(DISPLAYTYPE)
#if (DISPLAYTYPE == GRAPHIC_OLED12864)

#include <avr/pgmspace.h>
#include <Wire.h>
// bitmaps for display 
//    temp, boot-wifi, reboot
#include "images.h"
#include "display_graphics.h"


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Display Graphic messages to 
// be written to Serial port
//#define DISPLAYGRAPHICMSGS 1

#ifdef DISPLAYGRAPHICMSGS
#define DisplayGraphicPrint(...) Serial.print(__VA_ARGS__)
#define DisplayGraphicPrintln(...) Serial.println(__VA_ARGS__)
#else
#define DisplayGraphicPrint(...)
#define DisplayGraphicPrintln(...)
#endif


// -------------------------------------------------------
// EXTERN CLASSES
// -------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

#include "driver_board.h"
extern DRIVER_BOARD *driverboard;


// -------------------------------------------------------
// EXTERN SETTINGS
// -------------------------------------------------------
extern int mycontrollermode;


// -------------------------------------------------------
// DEFINES
// -------------------------------------------------------
#define MAX_WIDTH 127
#define MAX_HEIGHT 63


// -------------------------------------------------------
// images.h
// Holds icon images
// -------------------------------------------------------
// WiFi_Logo_bits[]
// i_wifi[]
// i_temp[]
// i_reboot


// -------------------------------------------------------
// DISPLAY CLASS CONSTRUCTOR
// -------------------------------------------------------
GRAPHIC_DISPLAY::GRAPHIC_DISPLAY(uint8_t addr)
  : _addr(addr) {
  _loaded = false;
}

// -------------------------------------------------------
// START DISPLAY
// -------------------------------------------------------
bool GRAPHIC_DISPLAY::start() {
  DisplayGraphicPrintln(T_DISPLAYGRAPHIC);
  DisplayGraphicPrintln(TUC_START);

  // if disabled then do not start
  if (ControllerData->get_display_enable() == STATE_DISABLED) {
    display_found = NOT_FOUND;
    return false;
  }

  if (_loaded == STATE_LOADED) {
    display_found = FOUND;
    return true;
  }

  // check if display is present
  Wire.beginTransmission(_addr);
  if (Wire.endTransmission() != 0) {
    DisplayGraphicPrintln(T_NOTFOUND);
    display_found = NOT_FOUND;
    return false;
  } 

  // display found
  DisplayGraphicPrintln(T_FOUND);

  // For SH1106
  _display = new SH1106Wire(_addr, 4, 5, GEOMETRY_128_64, I2C_ONE, 700000);
  // or
  // For SSD1306
  //_display = new SSD1306Wire(_addr, 4, 5);

  _display->init();

  display_found = FOUND;
  _loaded = STATE_LOADED;
  delay(1000);
  _display->flipScreenVertically();
  _display->setFont(ArialMT_Plain_10);
  _display->clear();
  _display->setTextAlignment(TEXT_ALIGN_CENTER);

  _display->drawString(0, 0, String(project_name) + String(major_version));
  // correction: Kenichi Aihara, 10-Oct-2024
  if (mycontrollermode == ACCESSPOINT) {
    _display->drawString(0, 12, T_SSID + String(myAPSSID));
  } else {
    _display->drawString(0, 12, T_SSID + String(mySSID));
  }
  // draw wifi logo
  //display_draw_xbm(nwifi, 34, 14);
  display_draw_xbm(nwifi);
  _display->display();
  
  delay(10);
  return true;
}

// --------------------------------------------------------
// STOP DISPLAY
// --------------------------------------------------------
void GRAPHIC_DISPLAY::stop(void) {
  DisplayGraphicPrintln(T_DISPLAYGRAPHIC);
  DisplayGraphicPrintln(TUC_STOP); 
  delete _display;
  _loaded = STATE_NOTLOADED;
}

// --------------------------------------------------------
// DISPLAY CLEAR
// --------------------------------------------------------
void GRAPHIC_DISPLAY::clear(void) {
  if (_loaded == STATE_LOADED) {
    DisplayGraphicPrintln("Display clear");
    _display->clear();
    delay(10);
  }
}

// --------------------------------------------------------
// DISPLAY OFF
// --------------------------------------------------------
void GRAPHIC_DISPLAY::off(void) {
  if (_loaded == STATE_LOADED) {
    DisplayGraphicPrint(T_DISPLAYGRAPHIC);
    DisplayGraphicPrintln(T_OFF);
    _display->displayOff();
    delay(10);
  }
}

// --------------------------------------------------------
// DISPLAY ON
// --------------------------------------------------------
void GRAPHIC_DISPLAY::on(void) {
  if (_loaded == STATE_LOADED) {
    DisplayGraphicPrint(T_DISPLAYGRAPHIC);
    DisplayGraphicPrintln(T_ON);
    _display->displayOn();
  }
  delay(10);
}


// --------------------------------------------------------
// HANDLER FOR WRITING DISPLAY PAGE
// --------------------------------------------------------
void GRAPHIC_DISPLAY::update_page(long position) {
  if (_loaded == STATE_LOADED) {
    DisplayGraphicPrint(T_DISPLAYGRAPHIC);
    DisplayGraphicPrintln(T_UPDATE);
    draw_main_update(position);
  }
  delay(10);
}


// --------------------------------------------------------
// UPDATE POSITION
// writes focuser position at specific location  
// on display when the focuser is moving
// pixels: x = 0 - 127, y = 0 - 63
// --------------------------------------------------------
void GRAPHIC_DISPLAY::update_position(long position) {
  int x = 0;
  int y = 32;

  if (_loaded == STATE_LOADED) {
    char buff[12];
    DisplayGraphicPrintln("Display update position");
    snprintf(buff, sizeof(buff), "%ld", position);
    _display->drawString(x, y, buff);
    _display->display();
  }
  delay(10);
}

// --------------------------------------------------------
// UPDATE MAIN PAGES
// There is only 1 page at present
// --------------------------------------------------------
void GRAPHIC_DISPLAY::draw_main_update(long position) {
  char buffer[80];

  DisplayGraphicPrintln(T_DISPLAYGRAPHIC);
  DisplayGraphicPrintln(T_UPDATE);

  if (_loaded != STATE_LOADED) {
    DisplayGraphicPrintln(T_DISPLAYTEXT);
    DisplayGraphicPrintln(T_NOTLOADED);    
    return;
  }

  _display->clear();
  _display->setTextAlignment(TEXT_ALIGN_CENTER);
  _display->setFont(ArialMT_Plain_24);

  char dir = (ControllerData->get_focuserdirection() == moving_in) ? '<' : '>';
  snprintf(buffer, sizeof(buffer), "%lu:%i %c", driverboard->getposition(), (int)(driverboard->getposition() % ControllerData->get_brdstepmode()), dir);
  _display->drawString(64, 28, buffer);
  _display->setFont(ArialMT_Plain_10);
  snprintf(buffer, sizeof(buffer), "µSteps: %i MaxPos: %lu", ControllerData->get_brdstepmode(), ControllerData->get_maxstep());
  _display->drawString(64, 0, buffer);
  snprintf(buffer, sizeof(buffer), "TargetPos:  %lu", ftargetPosition);
  _display->drawString(64, 12, buffer);

  _display->setTextAlignment(TEXT_ALIGN_LEFT);

  snprintf(buffer, sizeof(buffer), "TEMP: %.2f C", temp);
  _display->drawString(54, 54, buffer);

  snprintf(buffer, sizeof(buffer), "BL: %i", ControllerData->get_backlashsteps_out());
  _display->drawString(0, 54, buffer);

  snprintf(buffer, sizeof(buffer), "%c", _heartbeat[++_count_hb % 4]);
  _display->drawString(8, 14, buffer);

  _display->display();

  delay(10);
}


// TODO
// There are more icons available!
// reboot icon in images.h i_reboot[]
// temperature icon in images.h i_temp[]
// TODO add motor moving icon - when a move occurs, 
//   show target position and motor icon with heartbeat
// enum logo_num { nwifi, ntemp, nreboot };


// --------------------------------------------------------
// DISPLAY DRAW XBM ICON
// write bitmap id:num at pos x, pos y
// logonum is the ID of a logo from the images.h file
// --------------------------------------------------------
//void GRAPHIC_DISPLAY::display_draw_xbm(logo_num num, int16_t x, int16_t y) {
void GRAPHIC_DISPLAY::display_draw_xbm(logo_num num) {
  if (_loaded == STATE_LOADED) {
    // make all icons 40 pixels high
    // x becomes right aligned, passed value of x is ignored
    // y becomes MAX_HEIGHT - image height - 1 = 63 - 40 = 23 - 1 = 22
    // ((MAX_HEIGHT - wifi_height) - 1)
    DisplayGraphicPrintln("Display draw icon");
    switch (num) {
      case nwifi:
        _display->drawXbm(((MAX_WIDTH - wifi_width) - 2), ((MAX_HEIGHT - wifi_height) - 1), wifi_width, wifi_height, i_wifi);
        break;
      // temperature is bottom left aligned
      case ntemp:
        _display->drawXbm(2, ((MAX_HEIGHT - temp_height) - 1), temp_width, temp_height, i_temp);  // draw temperature
        break;
      // reboot is centered
      case nreboot:
        // x pos is (max width / 2) - (reboot_width / 2)
        // y pos is (max height / 2) - (reboot_height / 2)
        _display->drawXbm(((MAX_WIDTH / 2) - ((reboot_width / 2) - 1)), ((MAX_HEIGHT / 2) - ((reboot_height / 2) - 1)), reboot_width, reboot_height, i_reboot);  // draw reboot icon
        break;
    }
    _display->display();
    delay(10);
  }
}

#endif  // #if (DISPLAYTYPE == GRAPHIC_OLED12864)
#endif  // #if defined(DISPLAYTYPE)
