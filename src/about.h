// -------------------------------------------------------
// myFP2ES8266 FOCUS CONTROLLER
// Copyright Robert Brown 2014-2025. All Rights Reserved.
// about.h
// -------------------------------------------------------


// -------------------------------------------------------
// Arduino IDE 2.3.4
// Arduino ESP8266 Core 3.1.2
// BOARD NODEMCU 1.0 (ESP-12E) ESP8266
// Flash Size 4MB (FS:2MB OTA:~1019KB)
// MMU 16KB Cache + 48KB IRAM and 2nd Heap (shared)
// Debug Level NONE
// Compiler Warnings DEFAULT
// -------------------------------------------------------


// -------------------------------------------------------
// PRINTED CIRCUIT BOARDS AND STRIPBOARDS
// -------------------------------------------------------
// https://sourceforge.net/projects/myfp2esp8266-focus-controller/files/Boards/
// For the following boards to use with ESP8266
// DRV8825, ULN2003, L293DMINI, L9110S, L298N, L293D Shield,
// WeMos D1 R3 Solderless
//   DRV8825
//   L298N
//   ULN2003
// DIY Solderless
//   DRV8825
//   L298N
//   ULN2003
// -------------------------------------------------------


// -------------------------------------------------------
// ENVIRONMENT
// -------------------------------------------------------
// Arduino IDE  2.3.4          
// Arduino ESP8266 Core 3.1.2   
// 	 https://arduino.esp8266.com/stable/package_esp8266com_index.json
// Arduino JSON 7.3.1           
//   https://github.com/bblanchon/ArduinoJson
// Dallas Temperature driver 4.0.3
//   https://github.com/milesburton/Arduino-Temperature-Control-Library
// esp8266-oled-ssd1306 4.4.0   
//   https://github.com/ThingPulse/esp8266-oled-ssd1306
// ESP8266TimerInterrupt 1.6.0  
//   https://github.com/khoih-prog/ESP8266TimerInterrupt
// OneWire 2.3.8
//   https://github.com/PaulStoffregen/OneWire
//
// Libraries are added using the Arduino IDE Library Manager
// ESP8266 Core is added using the Arduino IDE Boards Manager
//   see link above
// -------------------------------------------------------


// -------------------------------------------------------
// ARDUINO IDE SETTINGS
// YOU MUST SET THESE IN THE ARDUINO IDE
// -------------------------------------------------------
// Tools Board NodeMCU 1.0 (ESP-12E Module)
//   CPU Freq 160 MHz
//   Flash Size 4MB FS2MB OTA - 1019KB
//   Debug Level "None"
//   Erase Flash "Only Sketch"
//   MMU 16kb cache + 48KB IRAM and 2nd Heap (shared) 
//   Non 32-bit Access: "Use pgm_read macros for IRAM/PROGMEM"
// -------------------------------------------------------


// -------------------------------------------------------
// FILESYSTEM UPLOAD
// -------------------------------------------------------
// Filesystem is LittleFS
// See folder Arduino-v2-upload and follow the instructions 
// for download and install of the plug-in
// -------------------------------------------------------


// -------------------------------------------------------
// REQUIRED LIBRARIES
// -------------------------------------------------------
// ArduinoJSON
// Stepper Motor      myHalfStepperESP32
// Temperature Probe  DallasTemperature
// Text Oled Display  myOLED
// Graphic Display    see link above (esp8266-oled-ssd1306)
// ESP8266TimerInterrupt      
//   https://github.com/khoih-prog/ESP8266TimerInterrupt
// -------------------------------------------------------


// -------------------------------------------------------
// KNOWN COMPILER WARNINGS
// -------------------------------------------------------
// #warning set400kHz() disabled for this CPU.
// -------------------------------------------------------


// -------------------------------------------------------
// INTERFACES
// -------------------------------------------------------
// ASCOM ALPACA SERVER PORT 4040
// ASCOM DISCOVERY PORT 32337
// DUCKDNS
// MANAGEMENT SERVER PORT 6060 - Admin interface
// SERIAL PORT, 115200bps
// TCP/IP SERVER PORT 2020 - used by applications
// WEB SERVER PORT 80


// -------------------------------------------------------
// IMPORTANT
// -------------------------------------------------------
// Set Preferences of Arduino IDE, Compiler Warnings to Default
// -------------------------------------------------------


// -------------------------------------------------------
// DEFAULT CONFIGURATION
// -------------------------------------------------------
// The following are NOT defined in firmware. When the 
// controller firmware is generated (by the Arduino IDE) 
// and uploaded to the focus controller, most of these 
// servers are in a disabled/notstarted state.

// To use them you need to login to the Management Server 
// and enable/start
// -------------------------------------------------------
// ASCOM Remote Server [Port 4040] [Off] [Stopped]
// Management Server   [Port 6060] [On ] [Running]
// TCPIP Server        [Port 2020] [On ] [Stopped]
// Web Server          [Port 80]   [Off] [Stopped]
// -------------------------------------------------------


// -------------------------------------------------------
// EXTRA FEATURES NOT LISTED IN CONFIG.H
// Accessible from Management Server
// -------------------------------------------------------
// Backlash
//   In steps, Out steps
// Coil Power
//   State=disabled
//   Timeout=120s
// DelayAfterMove
//   State=disabled
//   Time interval=25ms
// Display
//   State=disabled
//   Brightness
//   Text Display Page Time=2
//   Page Options=all
//   Show pos when moving=enabled
// DuckDNS, refresh time=120s
// Focuser Presets, values=0
// Maxsteps limit
// Motor Speed=fast, Delay [values for fast, medium and slow]
// PowerDowm state, time delay (120s)
// Reverse Direction State=disabled
// Step mode [Full]
// Stepsize
//   Step size in microns=50.0
// Temperature Compensation
//   State=disabled
//   Direction
//   Coefficient=0
// Temperature probe
//   State=disabled
//   Default temperature Mode (C or F)
// -------------------------------------------------------


// -------------------------------------------------------
// NOT SUPPORTED BY ESP8266 FIRMWARE
// -------------------------------------------------------
// Backlash In-Out enable
// Delay After Move Enable
// DelayedDisplayUpdate
// DRV8825 StepMode in Software (fixed by jumpers)
// DuckDNS refreshtime
// Home Position Switch
// In Out LEDs
// InfraRed remote
// JoggingDirection
// JoggingState
// Joysticks
// Park
// Presets
// Push Buttons
// StepperPower
// Step size enable
// Temperature Probe Resolution (set to 0.25c)


// -------------------------------------------------------
// READ ONLY SETTINGS myFP2ESP8266
// Hint: Edit the default values then upload new firmware
// -------------------------------------------------------
// The following settings are READ-ONLY
// To change the value edit the setting in firmware then
// compile the firmware and upload to the Controller.
//
// AccessPoint 
//    SSID and Password
//    Network settings (IP, DNS, Gateway, Subnet mask)
// Station
//    SSID and Password
//    Network settings (IP, DNS, Gateway, Subnet mask)
//
// Display I2C Address
// Display page update time
// Display update on move refresh time
//
// DuckDNS Refresh Time
//
// FileSystem Save time (120s)
//
// I2C pins SDA SCL
//
// Management Server username, password
//
// Ports
//   Alpaca Server
//   TCPIP Server
//   Management Server
//   Web Server
//
// Reboot delay
//
// Serial Port Speed (11520)
//
// Temperature Pin
// Temperature Refresh interval
//
// Web Page Colors
// -------------------------------------------------------


// -------------------------------------------------------
// I PROGRAMMED THE CONTROLLER BUT THE ALPACA SERVER IS 
// NOT RUNNING.
// -------------------------------------------------------
// The default state is OFF
// 
// To start the ALPACA server, open a web browser and 
// access the Management server on port 6060
// 
// On the Servers page, it will show the ALPACA Server 
// as Disabled.
//
// Click the Enable button. The web page will refresh.
// 
// Click the Start button to start the server. The state 
// is saved in the controllers settings, so next time 
// the controller reboots, the ALPACA server will be 
// still enabled, thus the controller will start the 
// server at boot time.
//
// Same applies for
//   DuckDNS
//   Temperature Probe
//   Web Server
// -------------------------------------------------------


// -------------------------------------------------------
// I CHANGED A SETTING BUT THE CONTROLLER REVERTS TO THE
// PREVIOUS VALUE
// -------------------------------------------------------
// There is a 120 second wait time before a changed setting 
// is saved to the file-system.
// 
// If the controller is rebooted before the 120s delay 
// then the changed setting is not written.
//
// The setting can be saved if SAVE on the navbar is 
// clicked after changing the setting. 
// 
// HOWEVER this is NOT recommended as there is a limit 
// on the number of writes to the filesystem. The best 
// practice is to wait 3 minutes before rebooting or 
// power off.
// -------------------------------------------------------


// -------------------------------------------------------
// I PROGRAMMED THE CONTROLLER WITH A NEW DRIVERBOARD BUT 
// IT IS SHOWING THE OLDER DRIVERBOARD ON WEB PAGES
// -------------------------------------------------------
// You forgot to upload the filesystem after uploading the 
// new firmware.

// If you do not, the board_config.jsn file is not 
// re-created with the new driver board settings
// -------------------------------------------------------


// -------------------------------------------------------
// POWERDOWN
// See defines/app_defines.h
// Setting is in seconds (#define PWRDOWNDISPLAY 60L)
// -------------------------------------------------------
//Managed using MANAGEMENT SERVER /misc
// State  Enabled/Disabled
// Time   Interval (in seconds) to wait before applying POWERDOWN
//        Default 50s
//
// POWERDOWN DISABLED
// Nothing happens
//
// POWERDOWN ENABLED
// When the motor starts a move
//   Display (if enabled and running) is turned ON
// When the motor stops
//   Counter is started
//   When the counter expires Display is turned off  
// -------------------------------------------------------


// -------------------------------------------------------
// MDNS
// -------------------------------------------------------
// Default name = myfp28266.local
// Management Server = http://myfp28266.local:6060
// Alpaca Server     = http://myfp28266.local:4040
// Web Server        = http://myfp28266.local


// -------------------------------------------------------
// MDNS ISSUES
// -------------------------------------------------------
// MDNS Not working
//   Web Server MUST BE RUNNING
//   MDNS is setup at Boot Time only
//     This means you have to do the following
//     1. Start Arduino IDE to change firmware settings
//     2. In config.h enable
//        WEBSERVER
//        MDNS
//     3. Compile and Upload new firmware
//     4. Open Web browser to Management Server
//     5. On Servers pg
//        Enable Web Server
//        Start Web Server
//     6. Save
//     7. Reboot Controller
//
// When the Controller boots, it will Start Web Server, 
// and then MDNS will also start.
// -------------------------------------------------------

