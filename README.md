# LightningStepper
  This library is intended to be used on an Arduino microcontroller dedicated to controlling a stepper motor. It is intended to be able to run as fast as possible both modulating the stepper motor and listening for instructions. This library will keep track of the position of the stepper so that it can be used in a variety of applications such as joints that are limited in movement range, avoiding wire twisting, and printing applications. The driver board used is ULN2003 with a unipolar ROHS 28BYJ-48 stepper motor. A sketch of what the stepper controller would look like is below in figure 1.
![image](https://user-images.githubusercontent.com/62961062/187962426-64dd087f-8b12-4b27-abb2-20dabaf72682.png)

This library features a setup routine and a command routine. The setup routine prompts the user if they already know the settings or to go through the manual setup. It is recommended if you do not know the position settings to proceed with a manual setup in configuration 2 (figure 2). This will allow you to get the position of joint limits. After that you can proceed with configuration 1 (figure 2) to just supply the settings and move on. After setup, the stepper controller can be instructed to move the stepper to a position at a certain speed, request current settings, and to stop.
![image](https://user-images.githubusercontent.com/62961062/187963050-2b6200fd-7d73-456b-bc7e-75c22da09f01.png)

Nearly as important as the library, there is an example sketch included called LightningStepper_CommandController. This demonstrates how to code a command controller in configuration 1 (figure 2) to talk to the stepper controller. In summary, the commands the stepper controller receives once setup are below.

Commands:

Cmd 1-get the motor's details.  
Send: 1      
Replies: Strike(Settings: currentPosition,maxPosition,currentDelay,minDelay,maxDelay)
Cmd 2-move to position.            
Send: 2,speed,steps,direction     
No Reply              
Cmd 3 stop.             
Send: 3         
No Reply

Command Notes:
Speed is [1-100]   1 the slowest. 100 the fastest.

Direction 1=cw currentPosition increases, 2=ccw currentPosition decreases

Serial Communication Notes:	

  This library requires the newline characters ‘\n’ sent after every message. This library will add the ‘\r’ carriage return and newline characters ‘\n’ sent after every message. It uses println described in the link below.
https://www.arduino.cc/reference/en/language/functions/communication/serial/println/ .
  If you are in configuration 2, the new Arduino IDE serial monitor has a drop down to select “newline”. Ensure you do so. Also, the IDE will send Message(<whatever you typed>). This is fine. The program filters for this and can handle communications with or without the Message() block. The library has adopted this same block as it makes serial communication parsing easier. The block of all messages from this library arrive as Strike(<the response>).
  
Download instructions:
  
  
This is intended to be used with the Arduino IDE and can be downloaded through library manager as described in the link.
https://docs.arduino.cc/software/ide-v1/tutorials/installing-libraries

Software Bill of materials:
  
-Arduino.h   Version at 9/1/2022. This is the Arduino Programming Language and to my understanding it is updated along with the Arduino IDE. The version of the IDE at time of the LightningStepper library creation was Arduino IDE 2.0.0-beta.3
-LightningStepper.h  Version 1.0 Created 9/1/2022
-LightningStepper.cpp  Version 1.0 Created 9/1/2022

Conclusion:
  
Use this library, example sketches, configurations, and code at your own risk. Exercise extreme care as you are making things move and interacting with delicate components. Also, please follow all applicable rules and regulations that apply. Lastly, if you chose to use this library, thank you! 

