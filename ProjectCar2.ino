#include "IRremote.h"
#include <Servo.h>

//PIN DEFINITIONS
  int receiver = 11;
  int pinMotor1A = 2; //Forward & Back
  int pinMotor1B = 3;
  int pinMotor1 = 6;
  int pinMotor2A = 4; //Left & Right
  int pinMotor2B = 5;
  int pinMotor2 = 9;
  int pinTrig = 12;
  int pinEcho = 13;
  int pinServo = A1;

//VARIABLES
  int motorSpeed1 = 90;  //0-100 Adjust accordingly
  int motorSpeed1B = 90;  //0-100 Adjust accordingly
  int motorInput1; //0-255 Calculated input for forward
  int motorInput1B; //0-255 Calculated input for back
  int motorSpeed2 = 100;  //0-100 Adjust accordingly
  int motorInput2; //0-255 Calculated input for turning
  int backDelay = 1300; 
  int scanDelay = 300;
  int stopDelay = 500;
  int forwardDelay = 600;
  int turnDelay = 900;
  int resultsIR = 0;
  float pingTime;
  float targetDistance[4];
  int targetTrip = 12;  //in inches
  float speedOfSound = 776.5;
  int pos = 110;
  int radarPos;
  

IRrecv irrecv(receiver); 
decode_results results;   
Servo radar;

void setup(){
  Serial.begin(9600);
  Serial.println("Test Begin");
  irrecv.enableIRIn(); // Start the receiver
  pinMode(pinMotor1A,OUTPUT);
  pinMode(pinMotor1B,OUTPUT);
  pinMode(pinMotor1,OUTPUT);
  pinMode(pinMotor2A,OUTPUT);
  pinMode(pinMotor2B,OUTPUT);
  pinMode(pinMotor2,OUTPUT);
  pinMode(pinTrig,OUTPUT);
  pinMode(pinEcho,INPUT);
  radar.attach(pinServo);
}

void loop(){
  if (irrecv.decode(&results)){ // Have we received an IR signal? 
    //Serial.println(results.value, HEX); // UN Comment to see raw values
    translateIR(); 
    irrecv.resume(); //receive the next value
  }  
  
  //Motor speed adjustment (add remote speed adjustment later)
  motorInput1 = map(motorSpeed1, 0, 100, 0, 255);
  motorInput1B= map(motorSpeed1B, 0, 100, 0, 255);
  motorInput2 = map(motorSpeed2, 0, 100, 0, 255);
  
  if (resultsIR == 5){
    AutoDrive();
  }

}//End void loop()


void translateIR(){ //This code is mostly for testing each function individually and for starting the AutoDrive feature 
  switch(results.value){   
    case 0xFFA25D:  //Stop all motors    
      Serial.println(" STOP ALL "); 
      resultsIR = 1;
      Stop();
    break;    
  
    case 0xFF629D:  //Go Forward
      Serial.println(" FORWARD ");  resultsIR = 2;
      Forward();
      delay(forwardDelay);
      Stop();
    break;

    case 0xFF22DD:  //Turn Wheels Left
      Serial.println(" LEFT "); resultsIR=4;
      Left();
      delay(turnDelay);
      Stop();
    break;

    case 0xFF02FD:  //Start AutoDrive Loop
      Serial.println(" START/STOP "); resultsIR=5; 
    break;
  
    case 0xFFC23D:  //Turn Wheels Right  
      Serial.println(" RIGHT ");  resultsIR=6;
      Right();
      delay(turnDelay);
      Stop();
    break;

    case 0xFFA857:  //Go Backward
      Serial.println(" BACK "); resultsIR=8;
      Back();
      Stop();
    break;

    case 0xFF30CF:  //Go Forward, Left
      Serial.println(" 1 ");  resultsIR=7; 
      Left();
      Forward();
      delay(forwardDelay);
      Stop(); 
    break;
    
    case 0xFF7A85:  //Go Forward, Right
      Serial.println(" 3 ");  resultsIR=9;
      Right(); 
      Forward();
      delay(forwardDelay);
      Stop();       
    break;

    case 0xFF42BD:  //Go Back, Left
      Serial.println(" 7 ");  resultsIR=13;
      Serial.println("Go Back and Left");
      Left();  
      Back();      
      Stop();          
    break;
    
    case 0xFF52AD:  //Go Back, Right
      Serial.println(" 9 ");  resultsIR=15;
      Right();
      Back();
      Stop();            
    break;

    default: 
      Serial.println(" ERROR   ");
  }//END Switch(Results.value)

  delay(500);
} //END translateIR


void AutoDrive(){
  //LOCAL VARIABLES
    int autoValue;
 
  //START SCAN: SERVO & ULTRASONIC SENSOR    
    int x;
    for (x=0; x<3; x++){
      radarPos=x+1;
      pos=map(radarPos, 1, 3, 190, 65); //radar 70deg to 180deg mapped to 1 - 3
      radar.write(pos);
      delay(scanDelay);
      Scan();
      targetDistance[x]=speedOfSound*pingTime*63360/2;  //in Inches
    }
    radarPos = 2;
    pos=map(radarPos, 1, 3, 190, 65);
    radar.write(pos); //For centering sensor
    Serial.println("Target Distance (L,F,R): ");
    int i;
    for (i=0; i<3; i++){  
      Serial.println(targetDistance[i]);
    }
    delay(scanDelay); 

  //START SCAN: TESTS 
    if(targetDistance[1] < targetTrip){
      autoValue = 2;
      Serial.println("Forward Obstruction");     
    }
    else if(targetDistance[0]< targetTrip) {
      autoValue = 3;  
      Serial.println("Left Obstruction");     
    }
    else if(targetDistance[2]< targetTrip){  
      autoValue = 4;    
      Serial.println("Right Obstruction");    
    }  
    else  {
      autoValue = 1;
      Serial.println("No Obstruction");
    }

  //OBSTRUCTION CASES
    switch(autoValue){
    case 1: //No Obstruction Forward, Forward
      Forward();
      delay(forwardDelay);
      Stop();
    break;
    
    case 2: //Obstruction Forward, Left & Back
      Stop();     
      Left();   
      Back();  
      Stop();
    break; 
    
    case 3: //Obstruction Left, slight right   
      Right();           
      Forward();
      delay(turnDelay);
      Stop();
    break;
    
    case 4: //Obstruction Right, slightly left
      Left();           
      Forward();
      delay(turnDelay);
      Stop();
    break;
    
    default:
      Serial.println("AutoDrive Error");
      
    } //end of switch(autoValue)  
} //end of AutoDrive()

//CONTROL FUNCTIONS
  void Forward(){
    digitalWrite(pinMotor1A,HIGH);  //Go Forward
    digitalWrite(pinMotor1B,LOW);
    analogWrite(pinMotor1,motorInput1);
    Serial.println("Forward");
  }
  
  void Left(){
    digitalWrite(pinMotor2A,LOW); //Left
    digitalWrite(pinMotor2B,HIGH);
    analogWrite(pinMotor2,motorInput2);  
    Serial.println("Left");     
  }
  
  void Right(){
    digitalWrite(pinMotor2A,HIGH); //Right
    digitalWrite(pinMotor2B,LOW);     
    analogWrite(pinMotor2,motorInput2);  
    Serial.println("Right");       
  }
  
  void Back(){
    digitalWrite(pinMotor1A,LOW);  //Go Back
    digitalWrite(pinMotor1B,HIGH);
    analogWrite(pinMotor1,motorInput1B); 
    Serial.println("Back");
    delay(backDelay);   
  }
  
  void Stop(){
    digitalWrite(pinMotor1,LOW);
    digitalWrite(pinMotor2,LOW);
    Serial.println("Stop");
    delay(stopDelay);
  }
  
  void Scan(){
    digitalWrite(pinTrig,LOW);
    delayMicroseconds(2000);
    digitalWrite(pinTrig,HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTrig,LOW);
  
    pingTime=pulseIn(pinEcho,HIGH);
    pingTime=pingTime/1000000/3600.;
  }

