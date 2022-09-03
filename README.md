# LightningStepper
This library is intended to be used on an Arduino microcontroller dedicated to controlling a stepper motor. It is intended to be able to run as fast as possible both modulating the stepper motor and listening for commands. This library will keep track of the position of the stepper so that it can be used in a variety of applications such as joints that are limited in movement range, avoiding wire twisting, and printing applications. The driver board used is ULN2003 with a unipolar ROHS 28BYJ-48 stepper motor. A sketch of the stepper controller example is below in figure 1.

