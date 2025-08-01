# myFP2E Firmware for ESP32 and ESP8266
This is the firmware for the myFocuserPro2E ESP8266 and ESP32 focus controller projecta on Sourceforge.
https://sourceforge.net/projects/arduinoascomfocuserpro2diy/files/myFocuserPro2E/
https://sourceforge.net/projects/myfocuserpro2-esp32/

(c) Robert Brown, 2019. All rights reserved.
(c) Holger Manz, 2019. All rights reserved.

The source code is located in the src folder. 


#HalfStepperESP32.h and HalfStepperESP32.cpp
This file manages the stepping modes for the ULN2003, L298N, L293DMINI and L9110S driver boards. It is a modified version of the HalfStepper library from Tom Biuso. DO NOT MODIFY THESE FILES. It has been modified to work with ESP8266 and ESP32 chips.



# HalfStepper

#### An Arduino library to enable half-stepping and single/dual phasing for step motors.
---

HalfStepper decorates (wraps) the Stepper library to provide half-step states and has options for single/dual phasing as well as multiple coil sequencing options.
<br/>
<br/>This library is able to drive motors directly or through drivers with motor control or H-bridge chips, such as the L293, L298, and L9110.

---

## License
Copyright � 2016 Tom Biuso.
<br />This library is released under the GNU Lesser General Public License v3.0.
<br />See LICENSE file for detailed terms and LGPL v3.0 license text.
<br />
<br />Based upon the Stepper Arduino library.
<br />Copyright � Arduino LLC, Sebastian Gassner, Noah Shibley.
<br />Also released under the GNU LGPL.



