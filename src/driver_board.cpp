// -------------------------------------------------------
// myFP2ESP8266 BOARD DRIVER CLASS
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// Copyright Holger M, 2019-2021. All Rights Reserved.
// Copyright Paul P, 2021-2022. All Rights Reserved.
// driver_board.cpp
// Default 
// NodeMCU 1.0 (ESP-12E Module)
// -------------------------------------------------------


// -------------------------------------------------------
// NOT SUPPORTED
// -------------------------------------------------------
// Over The Air (OTA)
// Home Position Switch
// IN OUT LEDs
// Infra-red Remote
// Joystick
// Presets
// Push buttons
// TMC2209 and TMC2225


// -------------------------------------------------------
// INCLUDES
// -------------------------------------------------------
#include <Arduino.h>
#include <avr/pgmspace.h>
#include "config.h"


// -------------------------------------------------------
// DEBUGGING
// WARNING: DO NOT ENABLE DEBUGGING INFORMATION
// -------------------------------------------------------
// Remove comment to enable Driver Board messages to 
// be written to Serial port
//#define DRIVERBOARDMSGS 1

#ifdef DRIVERBOARDMSGS
#define DrvBrdMsgPrint(...) Serial.print(__VA_ARGS__)
#define DrvBrdMsgPrintln(...) Serial.println(__VA_ARGS__)
#else
#define DrvBrdMsgPrint(...)
#define DrvBrdMsgPrintln(...)
#endif


// -------------------------------------------------------
// ESP8266TIMER INTERRUPT LIBRARY
// version 1.6.0
// These define's must be placed at the beginning 
// before #include "ESP8266TimerInterrupt.h"
// -------------------------------------------------------
#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0

// Select a Timer Clock
#define USING_TIM_DIV1 false   // for shortest and most accurate timer
#define USING_TIM_DIV16 false  // for medium time medium accurate timer
#define USING_TIM_DIV256 true  // for longest timer but least accurate. Default

// https://github.com/khoih-prog/ESP8266TimerInterrupt
#include "ESP8266TimerInterrupt.h"

// Shared between interrupt handler and driverboard class
volatile bool stepdir;  // direction of steps to move
ESP8266Timer ITimer;    // timer 1 steps the motor


// -------------------------------------------------------
// CLASSES
// -------------------------------------------------------
#include "controller_data.h"
extern CONTROLLER_DATA *ControllerData;

// DRIVER BOARD
#include "driver_board.h"
extern DRIVER_BOARD *driverboard;


// -------------------------------------------------------
// EXTERN SETTINGS
// -------------------------------------------------------
// Used to signal move is completed
extern volatile bool timerSemaphore;

// number of steps to move the motor
extern volatile uint32_t stepcount;

// HALT cmd from App or ASCOM driver to TCP/IP Server
// Also Alpaca Server, Web Server
extern volatile bool halt_alert;  


// -------------------------------------------------------
// DEFINES
// -------------------------------------------------------


// -------------------------------------------------------
// BASIC RULE FOR SETTING STEPMODE setstepmode()
// -------------------------------------------------------
// DRIVER_BOARD->setstepmode(xx); // sets the physical pins
// and 
// ControllerData->set_brdstepmode(xx); // save stepmode


// -------------------------------------------------------
// ASM CODE to generate a 2uS delay 
// Required by A4998, DRV8825 for Step Pulse
// -------------------------------------------------------
inline void asm2uS() __attribute__((always_inline));

inline void asm1uS() {
  asm volatile(
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t"
    "nop \n\t" ::);
}


// -------------------------------------------------------
// TIMER INTERRUPT SERVICE ROUTINE (ISR)
// STEP MOTOR
// -------------------------------------------------------
void IRAM_ATTR TimerHandler() {
  static bool mjob = false;  // state of motor job

  if (stepcount && !(halt_alert)) {
    // move motor (byte direction)
    driverboard->movemotor(stepdir, true);
    // decrement steps to move
    stepcount--;
    if (stepcount == 0 || halt_alert) {
      mjob = true;
    }
  } else {
    // stepcount could be 0, halt_alert could be true
    if (mjob == true) {
      stepcount = 0;         // if hps_alert was asserted
      mjob = false;          // wait, and do nothing
      timerSemaphore = true; // signal move complere
    }
  }
}


// -------------------------------------------------------
// DRIVER_BOARD CLASS
// -------------------------------------------------------
DRIVER_BOARD::DRIVER_BOARD() {
}


// -------------------------------------------------------
// START
// -------------------------------------------------------
void DRIVER_BOARD::start(long startposition) {
  do {
    timerSemaphore = false;
    stepcount = 0;

    // get board number and cache it locally here
    _boardnum = ControllerData->get_brdnumber();

    if ((_boardnum == WEMOSDRV8825) || (_boardnum == PRO2EDRV8825) || (_boardnum == PRO2EDRV8825S) || (_boardnum == PRO2EDRV8825DS)) {
      pinMode(ControllerData->get_brdenablepin(), OUTPUT);
      pinMode(ControllerData->get_brddirpin(), OUTPUT);
      pinMode(ControllerData->get_brdsteppin(), OUTPUT);
      digitalWrite(ControllerData->get_brdenablepin(), 1);
      // fixed step mode
    }
    
  if ((_boardnum == PRO2EULN2003) || (_boardnum == PRO2EULN2003S) || (_boardnum == PRO2EULN2003DS) \
    || (_boardnum == PRO2EL298N)  || (_boardnum == PRO2EL298NS)   || (_boardnum == PRO2EL298NDS) \
    || (_boardnum == PRO2EL293DMINI) || (_boardnum == PRO2EL9110S) ) {
      // IN1, IN2, IN3, IN4
      _inputpins[0] = ControllerData->get_brdboardpins(0);
      _inputpins[1] = ControllerData->get_brdboardpins(1);
      _inputpins[2] = ControllerData->get_brdboardpins(2);
      _inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++) {
        pinMode(_inputpins[i], OUTPUT);
      }
      // for boards that support half stepper
#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N) || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) || (DRVBRD == PRO2EULN2003S) || (DRVBRD == PRO2EULN2003DS))
      myhstepper = new HalfStepper(ControllerData->get_brdstepsperrev(), _inputpins[0], _inputpins[1], _inputpins[2], _inputpins[3]);  // ok
#endif
      // restore step mode
      setstepmode(ControllerData->get_brdstepmode());
    }

    if (_boardnum == PRO2EL293DNEMA) {
      // Motor Shield IN2, IN3, IN1, IN4
      _inputpins[0] = ControllerData->get_brdboardpins(1);
      _inputpins[1] = ControllerData->get_brdboardpins(2);
      _inputpins[2] = ControllerData->get_brdboardpins(0);
      _inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++) {
        pinMode(_inputpins[i], OUTPUT);
      }
#if (DRVBRD == PRO2EL293DNEMA)
      // Motor Shield with NEMA motor
      mystepper = new Stepper(ControllerData->get_brdstepsperrev(), _inputpins[0], _inputpins[1], _inputpins[2], _inputpins[3]);  // DONE
#endif
      // restore step mode
      setstepmode(ControllerData->get_brdstepmode());
    }
    if (_boardnum == PRO2EL293D28BYJ48) {
      // Motor Shield with 28BYJ-48 motor
      // IN2, IN3, IN1, IN4 mystepper.h
      _inputpins[0] = ControllerData->get_brdboardpins(0);
      _inputpins[1] = ControllerData->get_brdboardpins(2);
      _inputpins[2] = ControllerData->get_brdboardpins(1);
      _inputpins[3] = ControllerData->get_brdboardpins(3);
      for (int i = 0; i < 4; i++) {
        pinMode(_inputpins[i], OUTPUT);
      }
#if (DRVBRD == PRO2EL293D28BYJ48)
      // Motor Shield with 28BYJ-48 motor
      mystepper = new Stepper(ControllerData->get_brdstepsperrev(), ControllerData->get_brdboardpins(1), ControllerData->get_brdboardpins(2), _inputpins[0], _inputpins[3]);  // DONE
#endif
      // restore step mode
      setstepmode(ControllerData->get_brdstepmode());
    }
  } while (0);

  // For all boards do the following
  // set default focuser position to same as ControllerData
  _focuserposition = startposition;
  delay(10);
}

// -------------------------------------------------------
// DESTRUCTOR
// -------------------------------------------------------
DRIVER_BOARD::~DRIVER_BOARD() {
#if ((DRVBRD == PRO2EULN2003)   || (DRVBRD == PRO2EL298N) \
  || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) \
  || (DRVBRD == PRO2EL298NS)    || (DRVBRD == PRO2EULN2003S) \
  || (DRVBRD == PRO2EL298NDS)   || (DRVBRD == PRO2EULN2003DS))
  delete myhstepper;
#endif
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
  // Motor Shield
  delete mystepper;
#endif
}

// -------------------------------------------------------
// ENABLE MOTOR
// Must be called whenever the motor is required to step
// For all DRV8825 boards
// -------------------------------------------------------
void DRIVER_BOARD::enablemotor(void) {
  if ((_boardnum == WEMOSDRV8825)   || (_boardnum == PRO2EDRV8825) \
    || (_boardnum == PRO2EDRV8825S) || (_boardnum == PRO2EDRV8825DS) ){
    digitalWrite(ControllerData->get_brdenablepin(), 0);
  }
  // boards require 1ms before stepping can occur
  delay(1);
}

// -------------------------------------------------------
// RELEASE MOTOR
// Turns off coil power current to the motor.
// -------------------------------------------------------
void DRIVER_BOARD::releasemotor(void) {
  // all DRV8825 boards
  if ( (_boardnum == WEMOSDRV8825)  || (_boardnum == PRO2EDRV8825) \
    || (_boardnum == PRO2EDRV8825S) || (_boardnum == PRO2EDRV8825DS)) {
    digitalWrite(ControllerData->get_brdenablepin(), 1);
  } 
  else if ( (_boardnum == PRO2EULN2003) || (_boardnum == PRO2EL298N) \
    || (_boardnum == PRO2EL293DMINI) || (_boardnum == PRO2EL9110S) \
    || (_boardnum == PRO2EL293DNEMA) || (_boardnum == PRO2EL293D28BYJ48) \
    || (_boardnum == PRO2EL298NS) || (_boardnum == PRO2EL298NDS) \
    || (_boardnum == PRO2EULN2003S) || (_boardnum == PRO2EULN2003DS) ) {
    digitalWrite(_inputpins[0], 0);
    digitalWrite(_inputpins[1], 0);
    digitalWrite(_inputpins[2], 0);
    digitalWrite(_inputpins[3], 0);
  }
}

// -------------------------------------------------------
// INIT MOVE
// This enables the move timer and sets the leds for the required mode
// -------------------------------------------------------
void DRIVER_BOARD::initmove(bool mdir, long steps) {
  stepcount = steps;
  stepdir = mdir;
  enablemotor();
  timerSemaphore = false;
  _reverse = ControllerData->get_reverse_enable();
  _dirpin = ControllerData->get_brddirpin();
  _enablepin = ControllerData->get_brdenablepin();
  _steppin = ControllerData->get_brdsteppin();

  DrvBrdMsgPrint("DB-initmove: ");
  DrvBrdMsgPrintln(steps);
  DrvBrdMsgPrint("direction: ");
  if (stepdir == moving_in) {
    DrvBrdMsgPrintln(T_IN);
  }
  else {
    DrvBrdMsgPrintln(T_OUT);
  }

  // get current board speed delay value
  int spd = ControllerData->get_motorspeed();

  unsigned long msdelay = ControllerData->get_brdmsdelay();

  // determine motor step interrupt rate
  switch (spd) {
    case 0:  // slow, 1/3rd the speed
      msdelay *= 3;
      break;
    case 1:  // med, 1/2 the speed
      msdelay *= 2;
      break;
    case 2:  // fast
      //msdelay *= 1000;
      break;
  }

  // msdelay is the interval between timer events
  if (ITimer.attachInterruptInterval(msdelay, TimerHandler) == false) {
    DrvBrdMsgPrintln("err ITimer");
  }
  delay(10);
}

// -------------------------------------------------------
// MOVE MOTOR
// DO NOT ADD ANY BEGUG/PRINT CODE IN THIS FUNCTION
// -------------------------------------------------------
void DRIVER_BOARD::movemotor(bool ddir, bool updateposition) {
  stepdir = ddir;

  if ((_boardnum == WEMOSDRV8825) || (_boardnum == PRO2EDRV8825) \
   || (_boardnum == PRO2EDRV8825S) || (_boardnum == PRO2EDRV8825DS) ) {
    // for all DRV8825 boards
    if ( _reverse ) {
      digitalWrite(_dirpin, !stepdir);
    } else {
      digitalWrite(_dirpin, stepdir);
    }
    digitalWrite(_enablepin, 0);  // Enable Motor Driver
    digitalWrite(_steppin, 1);    // Step pin on
    asm1uS();                     // Need 2uS delay for DRV8825
    asm1uS();
    asm1uS();
    digitalWrite(_steppin, 0);
  } else if ((_boardnum == PRO2EULN2003) || (_boardnum == PRO2EL298N) \
    || (_boardnum == PRO2EL293DMINI) || (_boardnum == PRO2EL9110S) \
    || (_boardnum == PRO2EL298NS) || (_boardnum == PRO2EL298NDS) \
    || (_boardnum == PRO2EULN2003S) || (_boardnum == PRO2EULN2003DS) ) {
    if (stepdir == moving_in) {
      // access to myhstepper must be protected because it may be undefined
      if (_reverse) {
#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N) \
    || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) \
    || (DRVBRD == PRO2EL298NS) || (DRVBRD == PRO2EL298NDS) \
    || (DRVBRD == PRO2EULN2003S) || (DRVBRD == PRO2EULN2003DS) )
        myhstepper->step(1);
#endif
      } else {
#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N) \
    || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) \
    || (DRVBRD == PRO2EL298NS) || (DRVBRD == PRO2EL298NDS) \
    || (DRVBRD == PRO2EULN2003S) || (DRVBRD == PRO2EULN2003DS) )
        myhstepper->step(-1);
#endif
      }
    } // step direction is moving out
    else {
      if (_reverse) {
#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N) \
    || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) \
    || (DRVBRD == PRO2EL298NS) || (DRVBRD == PRO2EL298NDS) \
    || (DRVBRD == PRO2EULN2003S) || (DRVBRD == PRO2EULN2003DS) )
        myhstepper->step(-1);
#endif
      } else {
#if ((DRVBRD == PRO2EULN2003) || (DRVBRD == PRO2EL298N) \
    || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) \
    || (DRVBRD == PRO2EL298NS) || (DRVBRD == PRO2EL298NDS) \
    || (DRVBRD == PRO2EULN2003S) || (DRVBRD == PRO2EULN2003DS) )
        myhstepper->step(1);
#endif
      }
    }
    asm1uS();
    asm1uS();
  } else if (_boardnum == PRO2EL293DNEMA || _boardnum == PRO2EL293D28BYJ48) {
    // handle motor shield board
    if (stepdir == moving_in) {
      if (_reverse) {
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
        mystepper->step(1);
#endif
      } else {
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
        mystepper->step(-1);
#endif
      }
    } else {
      if (_reverse) {
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
        mystepper->step(-1);
#endif
      } else {
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
        mystepper->step(1);
#endif
      }
    }
    asm1uS();
    asm1uS();
  }

  // adjust position
  if (updateposition) {
    (stepdir == moving_in) ? _focuserposition-- : _focuserposition++;
  }
}


// -------------------------------------------------------
// END MOVE
// when a move has completed, we need to detach/disable movetimer
// prior name was halt()
// -------------------------------------------------------
void DRIVER_BOARD::end_move(void) {
  ITimer.detachInterrupt();
  DrvBrdMsgPrintln("Move done");
  delay(10);
}


// -------------------------------------------------------
// Basic rule for setting stepmode in this order
// Set DRIVER_BOARD->setstepmode(xx);
// this sets the physical pins
// driver->setstepmode(xx) also writes to ControllerData->set_brdstepmode(xx);
// -------------------------------------------------------
void DRIVER_BOARD::setstepmode(int smode) {
  DrvBrdMsgPrint("DB:stepmode:");
  DrvBrdMsgPrintln(smode);
  
  do {
    // handle all drv8825 boards
#if ((DRVBRD == WEMOSDRV8825) || (DRVBRD == PRO2EDRV8825) \
    || (DRVBRD == PRO2EDRV8825S) || (DRVBRD == PRO2EDRV8825DS) )
    // stepmode is set in hardware jumpers, cannot set by software
    // ignore request
#endif
#if ((DRVBRD == PRO2EULN2003)   || (DRVBRD == PRO2EL298N) \
  || (DRVBRD == PRO2EL293DMINI) || (DRVBRD == PRO2EL9110S) \
  || (DRVBRD == PRO2EL298NS)    || (DRVBRD == PRO2EL298NDS) \
  || (DRVBRD == PRO2EULN2003S)  || (DRVBRD == PRO2EULN2003DS) )
    // all half-stepper boards
    switch (smode) {
      case STEP1:
        myhstepper->SetSteppingMode(SteppingMode::FULL);
        break;
      case STEP2:
        myhstepper->SetSteppingMode(SteppingMode::HALF);
        break;
      default:
        smode = STEP1;
        myhstepper->SetSteppingMode(SteppingMode::FULL);
        break;
    }
    // update boardconfig.jsn
    ControllerData->set_brdstepmode(smode);
#endif
#if ((DRVBRD == PRO2EL293DNEMA) || (DRVBRD == PRO2EL293D28BYJ48))
    // handle motor shield, full steps only
    ControllerData->set_brdstepmode(STEP1);
#endif
  } while (0);
  delay(10);
}


// -------------------------------------------------------
// GET DIRECTION OF MOVE
// driverboard->getdirection()
// -------------------------------------------------------
bool DRIVER_BOARD::getdirection(void) {
  return stepdir;
}


// -------------------------------------------------------
// GET POSITION
// driverboard->position()
// -------------------------------------------------------
long DRIVER_BOARD::getposition(void) {
  return _focuserposition;
}


// -------------------------------------------------------
// SET POSITION
// driverboard->setdirection(21345)
// -------------------------------------------------------
void DRIVER_BOARD::setposition(long newpos) {
  _focuserposition = newpos;
}
