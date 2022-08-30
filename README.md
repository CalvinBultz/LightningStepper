# LightningStepper
  This library is intended to be used on an Arduino microcontroller dedicated to controlling a stepper motor. It is intended to be able to run as fast as possible both modulating the stepper motor and listening for instructions. This library will keep track of the position of the stepper so that it can be used in a variety of applications such as joints that are limited in movement range, avoiding wire twisting, and printing applications. The driver board used is ULD2003 with a 28BYJ-48 stepper. A sketch of what the stepper controller would look like is below in figure 1.
![StepperController_Sketch](https://user-images.githubusercontent.com/62961062/187461104-c171f575-a68b-4306-ad2e-efa37dbb52f2.jpg)

  The library features a setup routine and a command routine. The setup routine prompts the user if they already know the settings or to go through the manual setup. It is recommended if you do not know the position settings to proceed with a manual setup in configuration 2 (figure 2). This will allow you to get the position of joint limits. After that you can proceed with configuration 1 (figure 2) to just supply the settings and move on. After setup, the command routine can be used to instruct the controller to move the stepper to a position at a certain speed, request current settings, and to stop mid command.

![LightningStepper_LogicalDiagram](https://user-images.githubusercontent.com/62961062/187461511-234f4707-908e-4d93-a091-7bb13695cb58.jpeg)

Serial Communication Notes:	
  This library requires the newline characters ‘\n’ sent after every message. This library will add the ‘\r’ carriage return and newline characters ‘\n’ sent after every message. It uses println described in the link below.
https://www.arduino.cc/reference/en/language/functions/communication/serial/println/
If you are in configuration 2, the new Arduino IDE serial monitor has a drop down to select “newline”. Ensure you do so. Also, the IDE will send Message(<whatever you typed>). This is fine. The program filters for this and can handle communications with or without the Message() block. The library has adopted this same block as it makes serial communication parsing easier. The block of all messages from this library arrive as Strike(<the response>).
