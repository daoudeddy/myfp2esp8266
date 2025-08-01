// -------------------------------------------------------
// myFP2ESP8266 BOARD DRIVER CLASS DEFINITIONS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// Copyright Paul P, 2021-2022. All Rights Reserved. TMC22xx code
// driver_board.h
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------
#ifndef _driver_board_h
#define _driver_board_h

// required for DRVBRD
#include <Arduino.h>
#include "config.h"

#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N)  || (DRVBRD == PRO2EL293DMINI) \
  || (DRVBRD == PRO2EL9110S)  || (DRVBRD == PRO2EL298NS) || (DRVBRD == PRO2EL298NDS) \
  || (DRVBRD == PRO2EULN2003S)  || (DRVBRD == PRO2EULN2003DS) )
#include <myHalfStepperESP32.h>
//#include <myStepperESP32.h>
#endif

#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
#include <myStepperESP32.h>
#endif


// -------------------------------------------------------
// DRIVER BOARD CLASS : DO NOT CHANGE
// -------------------------------------------------------
class DRIVER_BOARD {
public:
  DRIVER_BOARD();
  ~DRIVER_BOARD(void);
  void start(long);
  void initmove(bool, long);
  void movemotor(bool, bool);
  void end_move(void);  // prior name was halt()

  // get
  long getposition(void);
  bool getdirection(void);

  // set
  void enablemotor(void);
  void releasemotor(void);
  void setposition(long);
  void setstepmode(int);

private:
#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N)  || (DRVBRD == PRO2EL293DMINI) \
  || (DRVBRD == PRO2EL9110S)  || (DRVBRD == PRO2EL298NS) || (DRVBRD == PRO2EL298NDS) \
  || (DRVBRD == PRO2EULN2003S)  || (DRVBRD == PRO2EULN2003DS) )
  HalfStepper* myhstepper;
  //Stepper* mystepper;
#endif
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
  Stepper* mystepper;
#endif

  // clock frequency used to generate 2us delay for ESP32 160Mhz/240Mhz
  unsigned int _clock_frequency;
  long _focuserposition;

  // input pins for driving stepper boards
  int _inputpins[4];
  // board number from mySetupData
  int _boardnum;
  int _reverse;   // ControllerData->get_reverse_enable()
  int _dirpin;    // ControllerData->get_brddirpin()
  int _enablepin; // ControllerData->get_brdenablepin()
  int _steppin;   // ControllerData->get_brdsteppin()
  
};


#endif
