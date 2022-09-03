/*
  LightningStepper.h - Library for controlling a unipolar stepper motor using the ULN2003 Driver Board.
  Created by Calvin Bultz, September 1, 2022.
  Released into the public domain.
  -Warning!!: Physical limit switches should be used when controlling motors. For simplicity of code and the fact that my motor is setup with a safe full range of motion, the physical limit switches are not included.
    Please read the disclamer on the README file in the repository.
*/

#include "Arduino.h"
#include "LightningStepper.h"

LightningStepper::LightningStepper(int pin_IN1, int pin_IN2, int pin_IN3, int pin_IN4, int pin_CmdReady, int pin_Done, int pin_Processing)
{
	stepper_pin1 = pin_IN1;
	stepper_pin2 = pin_IN2;
	stepper_pin3 = pin_IN3;
	stepper_pin4 = pin_IN4;
	this->pin_CmdReady = pin_CmdReady;
	this->pin_Done = pin_Done;
    this->pin_Processing = pin_Processing;
}

#pragma region Utilities

void LightningStepper::waitForMessage(String p_msg)
{
    //Reset keepWaiting
    keepWaiting = true;
    //Reset msg 
    msg = "";
    while (keepWaiting == true) {
        //Keep adding chunks of a message until the whole thing arrives.
        msg = msg + Serial.readString();
        msgLength = msg.length();
        //remove any leading and trailing whitespace
        msg.trim();
        if (msgLength > 2) {
            //Look for the block() start
            indexOfP = msg.indexOf('(');
            if (indexOfP > 0) {
                //look for the block end
                indexOfP = msg.indexOf(')');
                if (indexOfP > 0) {
                    //Entire block arrived
                    //remove the block
                    msg = LightningStepper::cleanMsg(msg);
                    //Compare
                    if (msg.startsWith(p_msg)) {
                        //Match
                        keepWaiting = false;
                    }
                    else {
                        //Error. Send to IDE for debug. May need to turn off the Stepper Controller and turn back on.
                        LightningStepper::sendMessage("SC Error: " + msg);                        
                        //Reset the msg
                        msg = "";
                    }
                }
            }
        }
    }
}

void LightningStepper::sendMessage(String p_msg)
{
    //Cannot compile if Serial1 or Serial2 etc don't exist on the board. Must use the default Serial. The modulation of the stepper takes up a lot of the boards abillity to process other things anyway.
    //That being said you can easily use comments to enable and disable what Serial port the library uses.

    //Add the Strike() block so that parsing messages on the command controller is much easier.    
    Serial.println("Strike(" + p_msg + ")");
    Serial.flush();
    //Serial1.println("Strike(" + p_msg + ")");
    //Serial1.flush();
    //Serial2.println("Strike(" + p_msg + ")");
    //Serial2.flush();
    //Serial3.println("Strike(" + p_msg + ")"); 
    //Serial3.flush();
}

String LightningStepper::readSerial()
{
    //Cannot compile if Serial1 or Serial2 etc don't exist on the board. Must use the default Serial. The modulation of the stepper takes up a lot of the boards abillity to process other things anyway.
    //That being said you can easily use comments to enable and disable what Serial port the library uses.
    return Serial.readStringUntil('\n');
    //return Serial1.readStringUntil('\n');
    //return Serial2.readStringUntil('\n');
    //return Serial3.readStringUntil('\n');   
}

String LightningStepper::cleanMsg(String p_msg)
{

    //LightningStepper::printSerial("clean recieved: " + p_msg);
    if (p_msg.startsWith(msgIDEchunk))
    {
        //Clean it
        p_msg.remove(0, 8);
        indexOfP = p_msg.indexOf(')');
        p_msg.remove(indexOfP);
    }
    //LightningStepper::printSerial("clean produced: " + p_msg);
    return p_msg;
}

void LightningStepper::calculateDelay(int speedVal)
{
    float m = ((float)minDelayInt - (float)maxDelayInt) / ((float)100 - (float)0);
    float y = (m * (float)speedVal) + (float)maxDelayInt;
    currentDelayInt = round(y);
    //LightningStepper::sendMessage("currentDelay: " + String(currentDelayInt));
}
#pragma endregion Utilities

#pragma region Commands

//This method sets up the pins, speed, and position tracking.
void LightningStepper::runSetup()
{
    //This code will not compile if Serial1 or Serial2 etc don't exist on the board. 
    //That being said you can easily use comments to enable and disable what Serial port the code uses.
    //The modulation of the stepper takes up a lot of the boards abillity to process other things so do so with this understanding.


    //Optimize timeout for speedy short messages
    Serial.setTimeout(1000);
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect.    
    }
    /*
    //Optimize timeout for speedy short messages
    Serial1.setTimeout(100);
    Serial1.begin(9600);
    while (!Serial1)
    {
      ; // wait for serial port to connect.
    }

    //Optimize timeout for speedy short messages
    Serial2.setTimeout(100);
    Serial2.begin(9600);
    while (!Serial2)
    {
      ; // wait for serial port to connect.
    }

    //Optimize timeout for speedy short messages
    Serial3.setTimeout(100);
    Serial3.begin(9600);
    while (!Serial3)
    {
      ; // wait for serial port to connect.
    }
    */

    //Initialize pins
    pinMode(stepper_pin1, OUTPUT);
    pinMode(stepper_pin2, OUTPUT);
    pinMode(stepper_pin3, OUTPUT);
    pinMode(stepper_pin4, OUTPUT);
    digitalWrite(stepper_pin1, LOW);
    digitalWrite(stepper_pin2, LOW);
    digitalWrite(stepper_pin3, LOW);
    digitalWrite(stepper_pin4, LOW);
    pinMode(pin_CmdReady, INPUT_PULLUP);
    pinMode(pin_Done, OUTPUT);
    pinMode(pin_Processing, OUTPUT);

    //Wait for the CMDReady pin signal. This eliminates the issue where the command controller would have to power up first.
    keepWaiting = true;
    while (keepWaiting == true)
    {
        //Check the pin. Remember this is INPUT_PULLUP so a value of 0 is the signal
        pin_CmdReady_Value = digitalRead(pin_CmdReady);
        if (pin_CmdReady_Value == 0)
        {
            //Proceed
            keepWaiting = false;
        }
    }



    //Prompt the user for either the manual setup or the auto setup
    LightningStepper::preSetupPrompt();

    if (launchMode == 1)
    {
        LightningStepper::startUpManually();
    }
    else if (launchMode == 2)
    {
        LightningStepper::startUpAuto();
    }
    else
    {
        LightningStepper::sendMessage("SC Error: setup code wrong");
    }
    
    LightningStepper::sendMessage("Exiting the runSetup routine. Type Go to proceed to run");
    //Wait for go
    LightningStepper::waitForMessage("Go");
    //Go ahead and set the done pin high for the first loop. The command controller always checks this before sending a command unless it needs to interupt.
    digitalWrite(pin_Done, HIGH);
    //Go ahead and set the processing pin low for the first loop.
    digitalWrite(pin_Processing, LOW);
}

void LightningStepper::preSetupPrompt()
{
    LightningStepper::sendMessage("Motor Running. To manually setup a motor reply '1', to auto setup a motor reply '2'");
    
    //Reset keepWaiting
    keepWaiting = true;
    //Reset msg 
    msg = "";
    while (keepWaiting == true)
    {
        //Keep adding chunks of a message until the whole thing arrives.
        msg = msg + LightningStepper::readSerial();
        msgLength = msg.length();
        //remove any leading and trailing whitespace
        msg.trim();        
        if (msgLength > 2)
        {
            //Look for the block() start
            indexOfP = msg.indexOf('(');
            if (indexOfP > 0) 
            {
                //look for the block end
                indexOfP = msg.indexOf(')');
                if (indexOfP > 0) 
                {
                    //Entire block arrived
                    //remove the block
                    msg = LightningStepper::cleanMsg(msg);
                    //Compare
                    if (msg == "1")
                    {
                        keepWaiting = false;
                        launchMode = 1;
                        LightningStepper::sendMessage("read: " + msg);
                    }
                    else if (msg == "2")
                    {
                        keepWaiting = false;
                        launchMode = 2;
                        LightningStepper::sendMessage("read: " + msg);
                    }
                    else
                    {
                        LightningStepper::sendMessage("Error 1");
                        //Loop again
                        //Reset 
                        msg = "";
                    }                    
                }
            }            
        }
    }
}

void LightningStepper::startUpAuto()
{
    LightningStepper::sendMessage("Auto setup initiated. Please specify: minDelay,maxDelay,currentPosition,MaxPosition");

    //Reset keepWaiting
    keepWaiting = true;
    //Reset msg 
    msg = "";
    while (keepWaiting == true)
    {
        //Keep adding chunks of a message until the whole thing arrives.
        msg = msg + LightningStepper::readSerial();
        msgLength = msg.length();
        //remove any leading and trailing whitespace
        msg.trim();
        if (msgLength > 2)
        {
            //Look for the block() start
            indexOfP = msg.indexOf('(');
            if (indexOfP > 0)
            {
                //look for the block end
                indexOfP = msg.indexOf(')');
                if (indexOfP > 0)
                {
                    //Entire block arrived
                    //remove the block
                    msg = LightningStepper::cleanMsg(msg);
                    //Process msg settings
                    LightningStepper::processSettings();
                    keepWaiting = false;
                }
            }
        }        
    }
    LightningStepper::sendMessage("Recieved minDelay: " + minDelayString + " maxDelay: " + maxDelayString + " currentPosition: " + currentPositionString + " maxPosition: " + maxPositionString);
}

void LightningStepper::startUpManually()
{

    LightningStepper::sendMessage("Manual setup initiated. Please specify: minDelay,maxDelay");

    //--Process min and max delay setting
    //Reset keepWaiting
    keepWaiting = true;
    //Reset msg 
    msg = "";
    while (keepWaiting == true)
    {
        //Keep adding chunks of a message until the whole thing arrives.
        msg = msg + LightningStepper::readSerial();
        msgLength = msg.length();
        //remove any leading and trailing whitespace
        msg.trim();
        if (msgLength > 2)
        {
            //Look for the block() start
            indexOfP = msg.indexOf('(');
            if (indexOfP > 0)
            {
                //look for the block end
                indexOfP = msg.indexOf(')');
                if (indexOfP > 0)
                {
                    //Entire block arrived
                    //remove the block
                    msg = LightningStepper::cleanMsg(msg);
                    //Process msg settings
                    //Separate first chunk
                    commaIndex = msg.indexOf(",");
                    minDelayString = msg.substring(0, commaIndex);
                    //Remove the first chunk
                    msg.remove(0, (commaIndex + 1));
                    //Separate second chunk
                    commaIndex = msg.indexOf(",");
                    maxDelayString = msg.substring(0, commaIndex);
                    minDelayInt = minDelayString.toInt();
                    maxDelayInt = maxDelayString.toInt();

                    keepWaiting = false;
                }
            }
        }
    }
    LightningStepper::sendMessage("Recieved minDelay: " + minDelayString + " maxDelay: " + maxDelayString);
    

    //--Set the ccw stop    
    LightningStepper::sendMessage("Type 'Go' to move. To stop, drive the pin_CmdReady low thus setting the zero position");
    //Wait for go    
    LightningStepper::waitForMessage("Go");

    //Wait for CmdReady pin. Stop and set zero
    keepWaiting = true;
    while (keepWaiting == true)
    {
        pin_CmdReady_Value = digitalRead(pin_CmdReady);
        if (pin_CmdReady_Value == 0)
        {
            LightningStepper::sendMessage("The zero position has been set");
            LightningStepper::sendMessage("Type 'Go' to move. To stop, drive the pin_CmdReady low thus setting the max position");
            currentPositionInt = 0;
            keepWaiting = false;
        }
        else
        {
            //Modulate stepper
            LightningStepper::stepCCW();
            delayMicroseconds(3000);
        }
    }
    //Wait for go
    LightningStepper::waitForMessage("Go");

    //--Set the cw stop aka max position
    //Wait for pin 30. Stop and set maxposition
    keepWaiting = true;
    while (keepWaiting == true)
    {
        pin_CmdReady_Value = digitalRead(pin_CmdReady);
        if (pin_CmdReady_Value == 0)
        {
            maxPositionInt = currentPositionInt;
            maxPositionString = String(maxPositionInt);
            LightningStepper::sendMessage("Max position: " + maxPositionString);            
            //Move one step so its not right on max
            LightningStepper::stepCCW();
            currentPositionInt--;
            keepWaiting = false;
        }
        else
        {
            //Modulate stepper
            LightningStepper::stepCW();
            currentPositionInt++;
            delayMicroseconds(3000);
        }
    }    
}

//This method listens for commands and runs the stepper motor.
void LightningStepper::run()
{
    //Check the pin for if there is a msg to read or not. This is way faster than checking the serial input. 
    pin_CmdReady_Value = digitalRead(pin_CmdReady);
    if (pin_CmdReady_Value == 0)
    {
        //The serial input is ready. Note this pin is configured as input_Pullup
        LightningStepper::processCmd();
    }
    else
    {
        if (done == true)
        {
            //Do nothing. 
        }
        else
        {
            //modulate stepper one step. 
            LightningStepper::modulateStepper();
        }
    }
}

void LightningStepper::processSettings() 
{
    //Separate first chunk
    commaIndex = msg.indexOf(",");
    minDelayString = msg.substring(0, commaIndex);
    //Remove the first chunk
    msg.remove(0, (commaIndex + 1));

    //Separate second chunk
    commaIndex = msg.indexOf(",");
    maxDelayString = msg.substring(0, commaIndex);
    //Remove the second chunk
    msg.remove(0, (commaIndex + 1));

    //Separate 3rd chunk
    commaIndex = msg.indexOf(",");
    currentPositionString = msg.substring(0, commaIndex);
    //Remove the 3rd chunk
    msg.remove(0, (commaIndex + 1));

    //Separate 4th chunk
    commaIndex = msg.indexOf(",");
    maxPositionString = msg.substring(0, commaIndex);
    //Remove the 3rd chunk
    msg.remove(0, (commaIndex + 1));

    //Convert all strings to ints
    minDelayInt = minDelayString.toInt();
    maxDelayInt = maxDelayString.toInt();
    currentPositionInt = currentPositionString.toInt();
    maxPositionInt = maxPositionString.toInt();
}

void LightningStepper::processCmd()
{
    /*
        Commands:
        Cmd 1-get the motor's details.        Send: 1                           Replies: Strike(Settings: currentPosition,maxPosition,currentDelay,minDelay,maxDelay)
        Cmd 2-move to position.               Send: 2,speed,steps,direction     Replies:
        Cmd 3 stop.                           Send: 3                           Replies:

        Notes:
        Speed is [0-100]   1 the slowest. 100 the fastest. 
        Direction 1=cw currentPosition increases, 2=ccw currentPosition decreases

        pin_Processing:
        Low to High: stepper controller ready for a message/cmd
        High to Low: stepper controller processed message and ready for another interupt from pin_CmdReady
    */
    

    //--Let the command controller know that the stepper controller has started processing.
    //The command controller will then start the serial transmission
    digitalWrite(pin_Processing, HIGH);
    
    //--Enter msg checker loop
    //Reset keepWaiting
    keepWaiting = true;
    //Reset msg 
    msg = "";
    while (keepWaiting == true)
    {
        //Keep adding chunks of a message until the whole thing arrives.
        msg = msg + LightningStepper::readSerial();
        msgLength = msg.length();
        //remove any leading and trailing whitespace
        msg.trim();
        if (msgLength > 2)
        {
            //Look for the block() start
            indexOfP = msg.indexOf('(');
            if (indexOfP > 0)
            {
                //look for the block end
                indexOfP = msg.indexOf(')');
                if (indexOfP > 0)
                {
                    //Entire block arrived
                    //remove the Message() block
                    msg = LightningStepper::cleanMsg(msg);
                    //Process cmd
                    //Either a single digit or digits with commas
                    msgLength = msg.length();
                    if (msgLength > 1)
                    {
                        //Separate first chunk. The cmd
                        commaIndex = msg.indexOf(",");
                        cmdMarkString = msg.substring(0, commaIndex);
                    }
                    else
                    {
                        cmdMarkString = msg;
                    }

                    //Analyze cmdMark and Determine if further processing is needed
                    //cmds marks
                    //1: get motors details
                    //2: move to postition
                    //3: stop
                    if (cmdMarkString == "3")
                    {
                        //Stop. 

                        //Program enters done loop. The user must then apply a voltage to send another cmd
                        done = true;
                        //Set the done pin high
                        digitalWrite(pin_Done, HIGH);
                    }
                    else if (cmdMarkString == "1")
                    {
                        //Reply with motor details

                        //Get strings of all metrics
                        currentPositionString = String(currentPositionInt);
                        maxPositionString = String(maxPositionInt);
                        speedString = String(speedInt);
                        minDelayString = String(minDelayInt);
                        maxDelayString = String(maxDelayInt);
                        LightningStepper::sendMessage("Settings: " + currentPositionString + "," + maxPositionString + "," + minDelayString + "," + maxDelayString);
                        done = true;
                        //Set the done pin high
                        digitalWrite(pin_Done, HIGH);
                    }
                    else if (cmdMarkString == "2")
                    {
                        //Move to position

                        //Process more chunks
                        //Remove the first chunk. The cmd.
                        msg.remove(0, (commaIndex + 1));

                        //Separate the second chunk. The speed
                        commaIndex = msg.indexOf(",");
                        speedString = msg.substring(0, commaIndex);
                        //Remove the second chunk.
                        msg.remove(0, (commaIndex + 1));

                        //Separate the 3rd chunk. The steps
                        commaIndex = msg.indexOf(",");
                        stepsString = msg.substring(0, commaIndex);
                        //Remove the 3rd chunk
                        msg.remove(0, (commaIndex + 1));

                        //All that is left is the 4th chunk. The direction
                        directionString = msg;
                        //Set all metrics                 
                        speedInt = speedString.toInt();
                        stepsInt = stepsString.toInt();
                        directionInt = directionString.toInt();
                        //Turn 0-100 speed into microsecond delay
                        //Calculates and sets the currentDelay used in the modulation loop.
                        LightningStepper::calculateDelay(speedInt);

                        //The stepper controller is not done until it steps the requested amount. It can be interupted before done in the run routine.
                        done = false;
                        //Set the done pin low meaning it is not done 
                        digitalWrite(pin_Done, LOW);
                    }
                    //Stop listening for serial messages/commands
                    keepWaiting = false;
                }
                else 
                {
                    //Loop...Keep reading and adding chunks
                }
            }
        }
    }

    //The command controller cannot interupt yet with the CmdReady pin.
    //Let the command controller know that the stepper controller is finished processing
    digitalWrite(pin_Processing, LOW);    
}

#pragma endregion Commands

#pragma region StepperControl

void LightningStepper::modulateStepper() {
    //Direction 1 = cw currentPosition increases, 2 = ccw currentPosition decreases
    //Check if it has reached limts factor in direction for if it is at 0 but is going up
    if (currentPositionInt == 0 && directionInt == 1) 
    {
        //At limit but will move back in bounds. Free to move
        //Check if the steps are 0 to know if it has reached the requested position 
        if (stepsInt <= 0)
        {
            //Finished Instructions
            done = true;
            //Set the done pin high
            digitalWrite(pin_Done, HIGH);
        }
        else if (stepsInt > 0)
        {
            //Keep going
            //Determine Direction
            if (directionInt == 1)
            {
                //cw is considered positive heading away from the zero position towards max position
                //Modulate
                LightningStepper::stepCW();
                //Requested steps will go down by one
                stepsInt--;
                //Postion moves positive
                currentPositionInt++;
            }
            else if (directionInt == 2)
            {
                //ccw is considered negative heading towards zero and away from max position
                LightningStepper::stepCCW();
                //Requested steps will go down by one
                stepsInt--;
                //Positon moves negative
                currentPositionInt--;
            }
            //Delay for speed
            delayMicroseconds(currentDelayInt);
        }

    }
    else if (currentPositionInt == maxPositionInt && directionInt == 2) 
    {
        //At limit but will move back in bounds. Free to move
        //Check if the steps are 0 to know if it has reached the requested position 
        if (stepsInt <= 0)
        {
            //Finished Instructions
            done = true;
            //Set the done pin high
            digitalWrite(pin_Done, HIGH);
        }
        else if (stepsInt > 0)
        {
            //Keep going
            //Determine Direction
            if (directionInt == 1)
            {
                //cw is considered positive heading away from the zero position towards max position
                //Modulate
                LightningStepper::stepCW();
                //Requested steps will go down by one
                stepsInt--;
                //Postion moves positive
                currentPositionInt++;
            }
            else if (directionInt == 2)
            {
                //ccw is considered negative heading towards zero and away from max position
                LightningStepper::stepCCW();
                //Requested steps will go down by one
                stepsInt--;
                //Positon moves negative
                currentPositionInt--;
            }
            //Delay for speed
            delayMicroseconds(currentDelayInt);
        }
    }
    else if (currentPositionInt < maxPositionInt && currentPositionInt > 0)
    {
        //Not at limit. Free to move
        //Check if the steps are 0 to know if it has reached the requested position 
        if (stepsInt <= 0)
        {
            //Finished Instructions
            done = true;
            //Set the done pin high
            digitalWrite(pin_Done, HIGH);
        }
        else if (stepsInt > 0)
        {
            //Keep going
            //Determine Direction
            if (directionInt == 1)
            {
                //cw is considered positive heading away from the zero position towards max position
                //Modulate
                LightningStepper::stepCW();
                //Requested steps will go down by one
                stepsInt--;
                //Postion moves positive
                currentPositionInt++;
            }
            else if (directionInt == 2)
            {
                //ccw is considered negative heading towards zero and away from max position
                LightningStepper::stepCCW();
                //Requested steps will go down by one
                stepsInt--;
                //Positon moves negative
                currentPositionInt--;
            }
            //Delay for speed
            delayMicroseconds(currentDelayInt);
        }

    }
    else if (currentPositionInt >= maxPositionInt || currentPositionInt <= 0)
    {
        //Ran into a limit. Stop Moving
        done = true;
        //Set the done pin high
        digitalWrite(pin_Done, HIGH);
    }
}

//Counter Clockwise. Just adjust the device doing the commanding if this needs to flip direction.
void LightningStepper::stepCCW()
{
    if (isHigh_pin4 == false && isHigh_pin3 == false && isHigh_pin2 == false && isHigh_pin1 == true)
    {

        //currentstep = "A";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == false && isHigh_pin2 == true && isHigh_pin1 == true)
    {
        //currentstep = "AB";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == false && isHigh_pin2 == true && isHigh_pin1 == false)
    {
        //currentstep = "B";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == true && isHigh_pin2 == true && isHigh_pin1 == false)
    {
        //currentstep = "BC";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == true && isHigh_pin2 == false && isHigh_pin1 == false)
    {
        //currentstep = "C";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinHigh(4);
            return;
        }
    }
    else if (isHigh_pin4 == true && isHigh_pin3 == true && isHigh_pin2 == false && isHigh_pin1 == false)
    {
        //currentstep = "CD";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
    }
    else if (isHigh_pin4 == true && isHigh_pin3 == false && isHigh_pin2 == false && isHigh_pin1 == false)
    {
        //currentstep = "D";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
    }
    else if (isHigh_pin4 == true && isHigh_pin3 == false && isHigh_pin2 == false && isHigh_pin1 == true)
    {
        //currentstep = "DA";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else
    {
        //just set currentstep = "DA"
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
}

//Clockwise. Just adjust the device doing the commanding if this needs to flip direction.
void LightningStepper::stepCW()
{
    if (isHigh_pin4 == false && isHigh_pin3 == false && isHigh_pin2 == false && isHigh_pin1 == true)
    {
        //currentstep = "A";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == false && isHigh_pin2 == true && isHigh_pin1 == true)
    {
        //currentstep = "AB";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == false && isHigh_pin2 == true && isHigh_pin1 == false)
    {
        //currentstep = "B";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == true && isHigh_pin2 == true && isHigh_pin1 == false)
    {
        //currentstep = "BC";
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == false && isHigh_pin3 == true && isHigh_pin2 == false && isHigh_pin1 == false)
    {
        //currentstep = "C";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == true && isHigh_pin3 == true && isHigh_pin2 == false && isHigh_pin1 == false)
    {
        //currentstep = "CD";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
    else if (isHigh_pin4 == true && isHigh_pin3 == false && isHigh_pin2 == false && isHigh_pin1 == false)
    {
        //currentstep = "D";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinHigh(4);
            return;
        }
    }
    else if (isHigh_pin4 == true && isHigh_pin3 == false && isHigh_pin2 == false && isHigh_pin1 == true)
    {
        //currentstep = "DA";
        if (stepKind == 4)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinHigh(3);
            LightningStepper::writePinHigh(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinLow(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinHigh(4);
            return;
        }
    }
    else
    {
        if (stepKind == 4)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinHigh(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
        if (stepKind == 8)
        {
            LightningStepper::writePinHigh(1);
            LightningStepper::writePinLow(2);
            LightningStepper::writePinLow(3);
            LightningStepper::writePinLow(4);
            return;
        }
    }
}

//Write the digital pin high
void LightningStepper::writePinHigh(int pin)
{
    switch (pin)
    {
    case 1:
        digitalWrite(stepper_pin1, HIGH);
        isHigh_pin1 = true;
        break;
    case 2:
        digitalWrite(stepper_pin2, HIGH);
        isHigh_pin2 = true;
        break;
    case 3:
        digitalWrite(stepper_pin3, HIGH);
        isHigh_pin3 = true;
        break;
    case 4:
        digitalWrite(stepper_pin4, HIGH);
        isHigh_pin4 = true;
        break;
    }
}

//Write the digital pin low
void LightningStepper::writePinLow(int pin) {
    switch (pin)
    {
    case 1:
        digitalWrite(stepper_pin1, LOW);
        isHigh_pin1 = false;
        break;
    case 2:
        digitalWrite(stepper_pin2, LOW);
        isHigh_pin2 = false;
        break;
    case 3:
        digitalWrite(stepper_pin3, LOW);
        isHigh_pin3 = false;
        break;
    case 4:
        digitalWrite(stepper_pin4, LOW);
        isHigh_pin4 = false;
        break;
    }
}
#pragma endregion StepperControl
