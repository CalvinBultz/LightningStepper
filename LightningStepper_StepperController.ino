//Coded by: Calvin Bultz
//Date: 9/1/2022
//Version: 1.0

#include <LightningStepper.h>

//-Warning!!: Physical limit switches should be used when controlling motors. For simplicity of code and the fact that my motor is setup with a safe full range of motion, the physical limit switches are not included.
//Please read the disclamer on the README file in the repository.

//Sketch uses 12678  bytes of program storage space.
//Global variables use 970 bytes of dynamic memory

//Serial Communications:
//Ensure baud rate is 9600
//Ensure message is sent with the Message() block. This is default in the new IDE.
//Ensure newline is sent. 

//Instantiate the LightningStepper object. The parameters are: LightningStepper(int pin_IN1, int pin_IN2, int pin_IN3, int pin_IN4, int pin_CmdReady, int pin_Done, int pin_Processing);
//IN1, IN2, etc are the pins with wires going to the driver board. You should see labels for IN1, IN2, etc.
//pin_CmdReady signals a command is ready for the stepper controller. This is an INPUT_PULLUP pin. This means its home is +5V and to signal you must drive it low to ground.
//Do this with wires and a button or setting your command controller to do so.
//The pin_Done will produce +5V when the stepper controller is done running the command sent to it. This can go to an LED or to your command controller for an iterrupt in it's logic.
//pin_Processing is used by the command controller logic. See the example sketch LightningStepper_CommandController for details.
LightningStepper myStepper(2,3,4,5,12,11,10);

void setup() {
  //Call the RunSetup method on the LightningStepper object.
  //Send the CmdReady pin low to start the setup prompt.
  //Either reply to manually setup or set to known parameters with auto setup. 
  myStepper.runSetup();
}

void loop() {
  //Call the Run method on the LightningStepper object.
  //This is intended to be called in a looping manner as fast as possible.
  //Drive pin_CmdReady low to start a new command or interupt and existing command.
  //After that send Serial commands through the IDE with a serial cable or with a command controller and wires connecting TX and RX.
  //Interupts with pin_Processing is used by the command controller logic. See the example sketch LightningStepper_CommandController for details.
  myStepper.run();
}
