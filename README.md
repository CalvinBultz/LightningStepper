# LightningStepper
  This library is intended to be used on an Arduino microcontroller dedicated to controlling a stepper motor. It is intended to be able to run as fast as possible both modulating the stepper motor and listening for commands. This library will keep track of the position of the stepper so that it can be used in a variety of applications such as joints that are limited in movement range, avoiding wire twisting, and printing applications. The driver board used is ULN2003 with a unipolar ROHS 28BYJ-48 stepper motor. A sketch of the stepper controller example is below in figure 1.

![image](https://user-images.githubusercontent.com/62961062/188285039-6227c018-6c86-4c67-95e7-7fe9cae280a5.png)

This library features a setup routine and a run routine. The setup routine prompts the user if they already know the settings or to go through the manual setup. It is recommended if you do not know the position settings to proceed with a manual setup in configuration 2 (figure 2). This will allow you to get the position of joint limits. Physical limit switches are recommended. After that you can proceed with configuration 1 (figure 2) to just supply the settings and move on. After setup, the stepper controller can be instructed to move the stepper to a position at a certain speed, request current settings, and to stop.

![image](https://user-images.githubusercontent.com/62961062/188285131-95b021be-756a-48e6-ab5e-2542d37f27f8.png)

Nearly as important as the library, there is an example sketch included called LightningStepper_CommandController. This demonstrates how to code a command controller in configuration 1 (figure 2) to talk to the stepper controller. In summary, the commands the stepper controller receives once setup are below.

  Commands:

Cmd 1- get the motor's details.  
Send: 1      
Replies: Strike(Settings: currentPosition,maxPosition,currentDelay,minDelay,maxDelay)

Cmd 2- move to position.            
Send: 2,speed,steps,direction     
No Reply              

Cmd 3- stop.             
Send: 3         
No Reply

  Command Notes:

Speed is [1-100]. 1 being the slowest. 100 being the fastest.
Direction 1=cw (currentPosition increases), 2=ccw (currentPosition decreases)
There are 3 pins used for interrupts and logic. Refer to the command controller example.

  Serial Communication Notes:

  This library requires the newline characters ‘\n’ sent after every message received. In response, this library will add the ‘\r’ carriage return and newline characters ‘\n’ sent after every message. It uses println described in the link below.
https://www.arduino.cc/reference/en/language/functions/communication/serial/println/ .
The baud rate needs to be set to 9600 and all messages received need to be inside a Message() block. If you are in configuration 2 (figure 2), the Arduino IDE serial monitor has a drop down to select “newline”. Ensure you do so. Also, the IDE will send Message(<whatever you typed>) automatically. The LightningStepper library also uses a block around transmissions as it makes serial communication parsing easier. The block of all messages from this library arrive as Strike(<the response>).
  
  Download instructions:
  
This is intended to be used with the Arduino IDE and can be downloaded through library manager as described in the link.
https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries
  
  Software Bill of materials:

-Arduino.h   Version at 9/1/2022. This is the Arduino Programming Language and to my understanding it is updated along with the Arduino IDE. The version of the IDE at time of the LightningStepper library creation was Arduino IDE 2.0.0-beta.3
  
-LightningStepper.h  Version 1.0 Created 9/1/2022 By Calvin Bultz
  
-LightningStepper.cpp  Version 1.0 Created 9/1/2022 By Calvin Bultz
  
-LightningStepper_StepperController.ino Version 1.0 Created 9/1/2022 By Calvin Bultz
  
-LightningStepper_CommandController.ino Version 1.0 Created 9/1/2022 By Calvin Bultz

  Disclaimer:
  
This disclaimer applies to the content in this Github repository, LightningStepper found at https://github.com/CalvinBultz/LightningStepper.git. More specifically, this applies to the library’s LightningStepper.h file, the library’s LightningStepper.cpp file, the example Arduino sketch LightningStepper_CommandController.ino file, the example Arduino sketch LightningStepper_StepperController.ino file, and the README.md file. These files contain code, instructions, and configurations that can be used at your own risk. These files contain code, instructions, and configurations that are “as-is”.  The files themselves are “as-is”.  Calvin Bultz has provided these files in the repository on a “as-is” basis and makes no warranties regarding any information, content, or licenses provided on or through it and disclaims liability for damages resulting from using or referring to the content of this repository, LightningStepper. If you do use these files, use caution as you are making things move and interacting with delicate components. Also, please follow all applicable rules, licenses, and regulations that apply. 

