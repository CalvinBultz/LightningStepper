# LightningStepper
  This library is intended to be used on an Arduino microcontroller dedicated to controlling a stepper motor. It is intended to be able to run as fast as possible both modulating the stepper motor and listening for instructions. This library will keep track of the position of the stepper so that it can be used in a variety of applications such as joints that are limited in movement range, avoiding wire twisting, and printing applications. The driver board used is ULN2003 with a unipolar ROHS 28BYJ-48 stepper motor. A sketch of what the stepper controller would look like is below in figure 1.
![image](https://user-images.githubusercontent.com/62961062/187962426-64dd087f-8b12-4b27-abb2-20dabaf72682.png)

This library features a setup routine and a command routine. The setup routine prompts the user if they already know the settings or to go through the manual setup. It is recommended if you do not know the position settings to proceed with a manual setup in configuration 2 (figure 2). This will allow you to get the position of joint limits. After that you can proceed with configuration 1 (figure 2) to just supply the settings and move on. After setup, the stepper controller can be instructed to move the stepper to a position at a certain speed, request current settings, and to stop.
