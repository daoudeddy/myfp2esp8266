// -------------------------------------------------------
// myFP2ESP8266 TEXT DISPLAY CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2021. All Rights Reserved.
// display_text.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------

#if !defined(_text_display_h_)
#define _text_display_h_


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include "config.h"

#if defined(DISPLAYTYPE)
#if (DISPLAYTYPE == TEXT_OLED12864)

#include <mySSD1306Ascii.h>
#include <mySSD1306AsciiWire.h>


// Note: TEXT/GRAPHICS use the exact same class
// definition, but private members can be different.
// -------------------------------------------------------
// CLASS
// -------------------------------------------------------
class TEXT_DISPLAY {
  public:
    TEXT_DISPLAY(uint8_t addr);

    bool start(void);
    void stop(void);
    void update_page(long);
    void update_position(long);

    void clear(void);
    void on(void);
    void off(void);
    void display_setbrightness(uint8_t);
    
  private:
    void draw_main_update(long);
    void page1(long);
    void page2(void);
    void page3(void);
    void page4(void);
    void page5(void);
    void page6(void);

    uint8_t _addr;
    bool _loaded = STATE_NOTLOADED;
    SSD1306AsciiWire *_display;  
};

#endif
#endif    // #if defined(DISPLAYTYPE)
#endif    // #define _text_display_h_
