// -------------------------------------------------------
// myFP2ESP8266 GRAPHIC DISPLAY CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2021. All Rights Reserved.
// display_graphics.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#if !defined(_display_graphic_h_)
#define _display_graphic_h_

// requires esp8266-oled-ssd1306 library
// https://github.com/ThingPulse/esp8266-oled-ssd1306


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(DISPLAYTYPE)
#if (DISPLAYTYPE == GRAPHIC_OLED12864)

// assume SH1106 display
#include <SH1106Wire.h>  // for the OLED 128x64 1.3" display SH1106 driver
// or
// assume SSD1306 display
//#include <SSD1306Wire.h>  // for the OLED 128x64 1.3" display SSD1306 driver


// -------------------------------------------------------
// DEFINITIONS
// -------------------------------------------------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// OLED_ADDR found in controller_defines.h
#define HEARTBEAT1 0x7C //  |
#define HEARTBEAT2 0x2F //  /
#define HEARTBEAT3 0x2D //  - 
#define HEARTBEAT4 0x5C //  \

// Note: TEXT/GRAPHICS use the exact same class definition
// but private members can be different.
// -------------------------------------------------------
// CLASS
// -------------------------------------------------------
class GRAPHIC_DISPLAY {
  public:
    GRAPHIC_DISPLAY(uint8_t addr);

    bool start(void);
    void stop(void);
    void update_page(long);
    void update_position(long);

    void clear(void);
    void on(void);
    void off(void);

    void display_draw_xbm(logo_num);

  private:
    void draw_main_update(long);

    uint8_t _addr;
    bool _loaded = STATE_NOTLOADED;
    byte _count_hb = 0;
    const char _heartbeat[4] = { HEARTBEAT1, HEARTBEAT2, HEARTBEAT3, HEARTBEAT4 };
    
    SH1106Wire *_display;
    // or
    //SSD1306Wire *_display;
};


#endif  // #if (DISPLAYTYPE == GRAPHIC_OLED12864)
#endif  // #if defined(DISPLAYTYPE)
#endif  // #if !defined(_display_graphic_h_)

