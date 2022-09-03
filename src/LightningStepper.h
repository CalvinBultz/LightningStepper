/*
  LightningStepper.h - Library for controlling a unipolar stepper motor using the ULN2003 Driver Board.
  Created by Calvin Bultz, September 1, 2022.
  Released into the public domain.
  -Warning!!: Physical limit switches should be used when controlling motors. For simplicity of code and the fact that my motor is setup with a safe full range of motion, the physical limit switches are not included.
    Please read the disclamer on the README file in the repository.
*/
#ifndef LightningStepper_h
#define LightningStepper_h
#include "Arduino.h"
class LightningStepper
{
    public:
        LightningStepper(int pin_IN1, int pin_IN2, int pin_IN3, int pin_IN4, int pin_CmdReady, int pin_Done, int pin_Processing);
        void runSetup();
        void run();
    private:
        //--Stepper
        int stepper_pin1 = 2;
        int stepper_pin2 = 3;
        int stepper_pin3 = 4;
        int stepper_pin4 = 5;
        bool isHigh_pin1 = false;
        bool isHigh_pin2 = false;
        bool isHigh_pin3 = false;
        bool isHigh_pin4 = false;
        //Control the step angles.
        int stepKind = 8;

        //--Serial Reading    
        //This pin is used by the command controller or a button to indicate a message is ready. 
        //This is also used in various places to synchronize activity between controllers
        int pin_CmdReady = 12;
        int pin_CmdReady_Value = 0;
        //This pin is used by the stepper controller to indicate to the command controller that the stepper controller is...
        //Low to High: ready for a message
        //High to Low: processed a message. Ready to be interupted again by the CmdReady pin
        int pin_Processing = 10;
        //The message read from the serial port
        String msg = "";
        //Used to block and loop
        bool keepWaiting = true;
        //Used for cmd parsing
        int msgLength = 0;
        //Used for cmd parsing
        int commaIndex = 0;
        //Used for cmd parsing
        int indexOfP = 0;
        //Used for string comparison/parsing
        String msgIDEchunk = "Message(";

        //--Settings/Trackers
        // 1 = startUpManually , 2 = startUpAuto   (settings known)
        int launchMode = 0;
        //Delays
        String minDelayString = "";
        String maxDelayString = "";
        int minDelayInt = 0;
        int maxDelayInt = 0;
        String currentDelayString = "";
        int currentDelayInt = 0;
        //Direction 1 is cw, 2 is ccw
        String directionString = "";
        int directionInt = 0;
        //Steps
        String stepsString = "";
        int stepsInt = 0;
        //Speed
        String speedString = "";
        int speedInt = 0;
        //Cmd marks
        //1: get motors speed
        //2: move to postition
        //3: stop
        String cmdMarkString = "";
        int cmdMarkInt = 0;

        //--Postion/State
        String currentPositionString = "";
        int currentPositionInt = 0;
        String maxPositionString = "";
        int maxPositionInt = 0;
        //Once the motor reaches max or desired steps it will enter done loop where it just checks the CmdReay pin for message signals. It also does this on start
        bool done = true;
        int pin_Done = 11;        

        void waitForMessage(String p_msg);
        void sendMessage(String p_msg);
        String readSerial();
        String cleanMsg(String p_msg);
        void calculateDelay(int speedVal);
        void preSetupPrompt();
        void startUpAuto();
        void startUpManually();
        void processSettings();
        void processCmd();        
        void modulateStepper();     
        void stepCCW();
        void stepCW();
        void writePinHigh(int pin);
        void writePinLow(int pin);     
};

#endif
