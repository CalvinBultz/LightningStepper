//Coded by: Calvin Bultz
//Date: 9/1/2022
//Version: 1.0

//Sketch uses 10478 bytes of program storage space.
//Global variables use 1145 bytes of dynamic memory

/*
  Notes:
-I used an Arduino Mega 2560 for this.
-This sketch is for the command controller which will listen to commands from the Arduino IDE, interpret the commands, pass them on or instruct the stepper controller, and respond with the stepper controller's responses if any.
-This sketch is intended to be run on an Arduino board with at least two Serial ports such as the Mega 2560.
-Refer to this link on Serial ports: https://www.arduino.cc/reference/en/language/functions/communication/serial/
-The Arduino library's keyword 'Serial' represents the USB or pins Tx0 and Rx0, 'Serial1' represents the pins Tx1 and Rx1.

-Before running this sketch I was in configuration 2 of figure 2 on the README file in the repository and setup the stepper controller with the following values.
currentPosition: 4022
maxPosition: 4023
minDelay: 1000
maxDelay: 10000

-Simple commands that can be sent through the IDE's serial monitor to the command controller.
Send: Forward:<command for stepper controller>. This forwards a command to the stepper controller
Send: MoveFullCW   This will instruct the stepper controller to move a full rotation CW or until it hits limits
Send: MoveFullCCW  This will instruct the stepper controller to move a full rotation CCW or until it hits limits
Send: MoveHalfCW   This will instruct the stepper controller to move a half rotation CW or until it hits limits
Send: MoveHalfCCW  This will instruct the stepper controller to move a half rotation CCW or until it hits limits
Send: GetMotorStats  This will return the current motor stats ex: Settings: currentPosition,maxPosition,minDelay,maxDelay
Send: SpeedTest   This will move to the half way position and then move the motor back and forth quickly.  
Send: Stop      This will stop the motor. Test it by commenting one of the Move statements wait for done and sendMessageToIDE function. Then stop it mid move.

-command format for the stepper controller
  Cmd 1-get the motor's settings.       Send: 1                           Replies: Strike(Settings: currentPosition,maxPosition,minDelay,maxDelay)
  Cmd 2-move to position.               Send: 2,speed,steps,direction     Replies:              
  Cmd 3 stop.                           Send: 3                           Replies:
    
  Notes:
  Speed is [1-100]   1 the slowest. 100 the fastest. Calculated from the minDelay and maxDelay.
  Direction 1=cw currentPosition increases, 2=ccw currentPosition decreases

-Degrees per Step
I set the maxPosition to 1 full rotation.
The maxPosition is counted from 0 so that means the number of steps to go around once was 4024
Therefore 360 degrees / 4024 steps = 0.08946322 degrees per step

-Serial Communications
Ensure baud rate is 9600
Ensure message is sent with the Message() block. This is default in the new IDE.
Ensure a newline character is sent at the end

-Warning!!: Physical limit switches should be used when controlling motors. For simplicity of code and the fact that my motor is setup with a safe full range of motion, the physical limit switches are not included.
Please read the disclamer on the README file in the repository.
*/

#pragma region Variables

//--Serial Reading
//Drive this low to interrupt the stepper controller so that it starts listening to the serial port
const int pin_CmdReady = 51;
//This pin is used by the stepper controller to indicate to the command controller that the stepper controller is...
//Low to High: ready for a message
//High to Low: processed a message. Ready to be interrupted again by the CmdReady pin
const int pin_Processing = 39;
int pin_Processing_Val = 0;
//Store the string read off the serial ports for processing
String msg = "";
//Used to continue checking the serial ports until a message arrives or the done pin changes
bool keepWaiting = true;
//Used for cmd parsing
int msgLength = 0;
//Used for cmd parsing
int commaIndex = 0;
//Used for cmd parsing
int indexOfP = 0;

//--IDE String commands. 
//This makes it easy to filter what message comes from the serial port.
String cmdFwrd = "Forward:";
String cmdMoveFullCW = "MoveFullCW";
String cmdMoveFullCCW = "MoveFullCCW";
String cmdMoveHalfCW = "MoveHalfCW";
String cmdMoveHalfCCW = "MoveHalfCCW";
String cmdGetMotorStats = "GetMotorStats";
String cmdSpeedTest = "SpeedTest";
String cmdStop = "Stop";

//--LightningStepper responses
//This makes it easy to filter what message comes from the serial port.
//Used to ensure the stepper controller is behaving as expected.
String msgMotorRunning = "Motor Running";
String msgSetupChoice = "read:";
String msgAutoStarup = "Auto setup initiated.";
String msgAutoSuccess = "Recieved minDelay:";
String msgExitingSetup = "Exiting the runSetup";
String msgSettings = "Settings:";

//--Settings/Trackers
String minDelayString = "";
String maxDelayString = "";
int minDelayInt = 0;
int maxDelayInt = 0;
//Speed 1-100
String speedString = "";
int speedInt = 0;
//Direction: 1 is cw, 2 is ccw
String directionString = "";
int directionInt = 0;
//Steps
String stepsString = "";
int stepsInt = 0;
//Postion
String currentPositionString = "";
int currentPositionInt = 0;
String maxPositionString = "";
int maxPositionInt = 0;
//Command controller can check this pin to determine if the stepper controller is busy or not.
const int pin_Done = 45;
int pin_Done_Val = 0;

#pragma endregion Variables


void setup() {
  
  //Setup the serial port with the IDE through usb cable
  Serial.setTimeout(1000);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for IDE port only.    
  }
  //Setup the serial port with the stepper controller through TX1 RX1 pins
  Serial1.setTimeout(1000);
  Serial1.begin(9600);
  

  //Set the pins
  pinMode(pin_CmdReady,OUTPUT);
  pinMode(pin_Done,INPUT);
  pinMode(pin_Processing,INPUT);

  //The stepper controller CmdReady pin is an INPUT_PULLUP
  //Set the command controller's CmdReady pin to High which means no commands ready.
  digitalWrite(pin_CmdReady, HIGH);
  

  //Wait for the user on the IDE to type go
  sendMessageToIDE("Command controller running. Send Message(Go) to setup the stepper controller.");
  waitForMessage_IDE("Go");

  //Every time the stepper controller is powered up the device must go through the setup routine
  //Be sure to record the motors settings before power down
  //The parameters are known so go through the setup in auto mode
  setupStepperController();
  
  sendMessageToIDE("Setup complete. Try out some of the IDE commands mentioned in the comments at the top of the sketch.");

  sendMessageToSC("Go");
}

void loop() { 
  
  //Read IDE.
  //Interpret msg
  msg = Serial.readStringUntil('\n');
  msgLength = msg.length();
  msg.trim();
  if(msgLength > 2){ 
    msg = cleanIDEMsg(msg);
    if(msg.startsWith(cmdFwrd)){
      forward();
    } 
    else if(msg.startsWith(cmdMoveFullCW)){
      moveFullCW();
    }
    else if(msg.startsWith(cmdMoveFullCCW)){
      moveFullCCW();
    }
    else if(msg.startsWith(cmdMoveHalfCW)){
      moveHalfCW();
    }
    else if(msg.startsWith(cmdMoveHalfCCW)){
      moveHalfCCW();
    }   
    else if(msg.startsWith(cmdGetMotorStats)){
      getMotorStats();
    } 
    else if(msg.startsWith(cmdStop)){
      stopTheMotor();
    }  
    else if(msg.startsWith(cmdSpeedTest)){
      speedTest();
    }  
  } 

  //Tip:If coding a command controller that is not UI driven like this one is and you need speed, place the message reading in a function. 
  //Tip:In place of the message reading, check the pin_Done for if the stepper controller has a message ready. Then call the message reading function.
  //Read the Stepper Controller. Pass on the message to IDE
  msg = Serial1.readStringUntil('\n');
  msgLength = msg.length();
  msg.trim();
  if(msgLength > 2){ 
    //Pass on the msg to IDE
    Serial.println(msg);
  } 
}

#pragma region Utilities

String cleanIDEMsg(String p_msg){   
  //Remove 'Message('
  p_msg.remove(0,8);
  indexOfP = p_msg.indexOf(')');
  //Remove ')'
  p_msg.remove(indexOfP);  
  return p_msg;
}

String cleanStepperControllerMsg(String p_msg){
  //Remove 'Strike('
  p_msg.remove(0,7);
  indexOfP = p_msg.indexOf(')');
  //Remove ')'
  p_msg.remove(indexOfP);
  return p_msg;
}


void waitForMessage_SC(String p_msg){
  //Reset keepWaiting
  keepWaiting = true;
  //Reset msg 
  msg = "";
  while(keepWaiting == true){
    //Keep adding chunks of a message until the whole thing arrives.
    msg = msg + Serial1.readStringUntil('\n');
    msgLength = msg.length();
    //remove any leading and trailing whitespace
    msg.trim();
    if(msgLength > 2){
      //Look for the block() start
      indexOfP = msg.indexOf('(');
      if(indexOfP > 0){
        //look for the block end
        indexOfP = msg.indexOf(')');
        if(indexOfP > 0){
          //Entire block arrived
          //remove the block
          msg = cleanStepperControllerMsg(msg);
          //Compare
          if(msg.startsWith(p_msg)){
            //Match
            keepWaiting = false;
          }
          else{
            //Error. Send to IDE for debug. May need to turn off the Stepper Controller and turn back on.
            sendErrorToIDE(msg);
            //Reset the msg
            msg = "";
          }
        }
      }          
    } 
  }  
}

void waitForMessage_IDE(String p_msg){
  //Reset keepWaiting
  keepWaiting = true;
  //Reset msg 
  msg = "";
  while(keepWaiting == true){
    //Keep adding chunks of a message until the whole thing arrives.
    msg = msg + Serial.readStringUntil('\n');
    msgLength = msg.length();
    //remove any leading and trailing whitespace
    msg.trim();
    if(msgLength > 2){
      //Look for the block() start
      indexOfP = msg.indexOf('(');
      if(indexOfP > 0){
        //look for the block end
        indexOfP = msg.indexOf(')');
        if(indexOfP > 0){
          //Entire block arrived
          //remove the block
          msg = cleanIDEMsg(msg);
          //Compare
          if(msg.startsWith(p_msg)){
            //Match
            keepWaiting = false;
          }
          else{
            //Error. Send to IDE for debug. May need to turn off the Stepper Controller and turn back on.
            sendErrorToIDE(msg);
            //Reset the msg
            msg = "";
          }
        }
      }          
    } 
  }  
}

void processMotorSettings(String p_msg){
  
  //Get each chunk of info and assign to variables
  //Remove the desciptor "Settings: " 
  p_msg.remove(0,10);

  //Separate first chunk. The currentPosition
  commaIndex = p_msg.indexOf(",");
  currentPositionString = p_msg.substring(0, commaIndex);  
  //Remove the first chunk. 
  p_msg.remove(0, (commaIndex + 1));

  //Separate the second chunk. The maxPosition
  commaIndex = p_msg.indexOf(",");
  maxPositionString = p_msg.substring(0, commaIndex);
  //Remove the second chunk.
  p_msg.remove(0, (commaIndex + 1));  

  //Separate the 3rd chunk. The minDelay
  commaIndex = p_msg.indexOf(",");
  minDelayString = p_msg.substring(0, commaIndex);
  //Remove the 3rd chunk
  p_msg.remove(0, (commaIndex + 1));

  //The 5th chunk is all that remains. The maxDelay  
  maxDelayString = p_msg;

  //Convert and assign all metrics to variables  
  currentPositionInt = currentPositionString.toInt();
  maxPositionInt = maxPositionString.toInt();  
  minDelayInt = minDelayString.toInt();
  maxDelayInt = maxDelayString.toInt();            
}

void sendErrorToIDE(String p_msg){
  p_msg = "ccFoundError(" + p_msg + ")";
  Serial.println(p_msg);
}

void sendMessageToIDE(String p_msg){
  p_msg = "CommandController(" + p_msg + ")";
  Serial.println(p_msg);
  Serial.flush();
}

void sendMessageToSC(String p_msg){
  p_msg = "Message(" + p_msg + ")";
  Serial1.println(p_msg);
  Serial1.flush();
}

#pragma endregion Utilities




#pragma region Commands
//Basic commands
void forward(){
  //Clean the forward command block.
  //Remove 'Forward('
  msg.remove(0,8);
  indexOfP = msg.indexOf(')');
  //Remove ')'
  msg.remove(indexOfP);
  
  //Send it to the stepper controller using the proper technique
  sendCommandToStepperController_NoInterrupts(msg);
}
void moveFullCW(){
  sendCommandToStepperController_NoInterrupts("2,90,4023,1");
  //Wait for the done pin to go high. Then send IDE a message the motor stopped.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  
  sendMessageToIDE("Done moving.");
}
void moveFullCCW(){
  sendCommandToStepperController_NoInterrupts("2,90,4023,2");
  //Wait for the done pin to go high. Then send IDE a message the motor stopped.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  
  sendMessageToIDE("Done moving.");
}
void moveHalfCW(){
  sendCommandToStepperController_NoInterrupts("2,90,2012,1");
  //Wait for the done pin to go high. Then send IDE a message the motor stopped.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  
  sendMessageToIDE("Done moving.");
}
void moveHalfCCW(){
  sendCommandToStepperController_NoInterrupts("2,90,2012,2");
  //Wait for the done pin to go high. Then send IDE a message the motor stopped.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  
  sendMessageToIDE("Done moving.");
}

void getMotorStats(){
  sendCommandToStepperController_NoInterrupts("1");
  //Wait for the done pin to go high. Then send IDE a message the motor stopped.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }    
  sendMessageToIDE("currentPosition: " + currentPositionString + " maxPosition: " + maxPositionString + " minDelay: " + minDelayString + " maxDelay: " + maxDelayString);
}

void speedTest(){

  //Move to the half way position from power up. This waits for done. Just setting up the speed test.
  moveHalfCCW();
    
  //Use Iterrupts for max speed. 
  //Rotate back and forth
  sendCommandToStepperController_InterruptsOk("2,100,1000,1");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,2");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,1");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,2");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,1");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,2");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,1");
  delay(1000);
  sendCommandToStepperController_InterruptsOk("2,100,1000,2");

}

void stopTheMotor(){
  sendCommandToStepperController_InterruptsOk("3");
  //Wait for the done pin to go high. Then send IDE a message the motor stopped.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  
  sendMessageToIDE("Motor Stopped");  
}

void sendCommandToStepperController_NoInterrupts(String p_cmd){
  //Wait for the done pin to go high. Then send the message.
  //The stepper controller keeps the done pin state high unless it is busy.  
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Done_Val = digitalRead(pin_Done);
    if(pin_Done_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  

  //Pull the pin_CmdReady pin low to indicate a message.
  digitalWrite(pin_CmdReady, LOW);

  //Wait for the stepper controller to reply it has started processing. Low to High transition
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Processing_Val = digitalRead(pin_Processing);
    if(pin_Processing_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  

  //Go ahead and remove the CmdReady pin signal to prevent an double cmd read
  digitalWrite(pin_CmdReady, HIGH);
  //Stepper controller ready for message, send it over
  sendMessageToSC(p_cmd);
  //If using the get settings command, wait for the settings
  if(p_cmd == "1"){
    waitForMessage_SC(msgSettings);
    //Convert and assign values to variables
    processMotorSettings(msg);
  }

  //Wait for the stepper controller to reply it has finished processing. High to low transition
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Processing_Val = digitalRead(pin_Processing);
    if(pin_Processing_Val == 0){
      //Done
      keepWaiting = false;
    }
  }   
  //The stepper controller can now be interrupted with CMDReady
}

void sendCommandToStepperController_InterruptsOk(String p_cmd){
  //Send the message regardless of the done pin. This is great for a stop command or live movement. 

  //Pull the pin_CmdReady pin low to indicate a message.
  digitalWrite(pin_CmdReady, LOW);

  //Wait for the stepper controller to reply it has started processing. Low to High transition
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Processing_Val = digitalRead(pin_Processing);
    if(pin_Processing_Val == 1){
      //Done
      keepWaiting = false;
    }
  }  

  //Go ahead and remove the CmdReady pin signal to prevent an double cmd read
  digitalWrite(pin_CmdReady, HIGH);
  //Stepper controller ready for message, send it over
  sendMessageToSC(p_cmd);
  //If using the get settings command, wait for the settings
  if(p_cmd == "1"){
    waitForMessage_SC(msgSettings);
    //Convert and assign values to variables
    processMotorSettings(msg);
  }

  //Wait for the stepper controller to reply it has finished processing. High to low transition
  keepWaiting = true;
  while(keepWaiting == true){
    pin_Processing_Val = digitalRead(pin_Processing);
    if(pin_Processing_Val == 0){
      //Done
      keepWaiting = false;
    }
  }   
  //The stepper controller can now be interrupted with CMDReady
}


void setupStepperController(){
  sendMessageToIDE("Command Controller is setting up the Stepper Controller");

  //Drive the CMDReady pin low to signal the stepper controller to enter startup where it request setup mode
  digitalWrite(pin_CmdReady, LOW);

  //Wait for the stepper controller to power up and request setup mode
  //ex: Strike(Motor Running. To manually setup a motor reply '1', to auto setup a motor reply '2')
  waitForMessage_SC(msgMotorRunning);
  sendMessageToIDE("motor running");
  //Remove the signal from the CMDReady pin
  digitalWrite(pin_CmdReady, HIGH);
  
  //Reply 2 for auto setup.
  sendMessageToSC("2"); 
  //Wait for the stepper controller to reply
  //Strike("read: 2") 
  waitForMessage_SC(msgSetupChoice);
  sendMessageToIDE(msg);
  //Wait for the stepper controller to reply
  //Strike("Auto setup initiated. Please specify: minDelay,maxDelay,currentPosition,MaxPosition")
  waitForMessage_SC(msgAutoStarup);
  sendMessageToIDE(msg);
  //Reply with the settings
  //Notes:
  //Delays are in microseconds ex: delayMicroseconds(10000);.
  //16383 is the largest possible delay.
  //Delay of 1000 for minDelay is about as fast as the motor can turn with no load. This may need to be tweaked.
  //Delay of 10000 for maxDelay is slow. 

  //Before running this sketch I was in configuration 2 and setup the stepper controller with the following values
  //currentPosition: 4022
  //maxPosition: 4023
  //minDelay: 1000
  //maxDelay: 10000
  //Parameters: minDelay,maxDelay,currentPosition,MaxPosition
  sendMessageToSC("1000,10000,4022,4023");

  //Wait for the stepper controller to reply
  //Strike(Recieved minDelay... 
  waitForMessage_SC(msgAutoSuccess);

  waitForMessage_SC(msgExitingSetup);
     
}
#pragma endregion Commands