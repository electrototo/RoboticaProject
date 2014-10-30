#include <AFMotor.h>
#include <Wire.h>
#include "HMC5883Llib.h"
#include <JsonParser.h>
#include <JsonGenerator.h>

// Compass section
Magnetometer mag;
boolean fail;
double heading;
// End compass section

// Input section
int left = 22;
int right = 24;

int arriba = 30;
int abajo = 28;

int led = 51;
// End input section

// Decission section
boolean turnRight = false;
boolean turnLeft = false;

boolean turnArriba = false; 
boolean turnAbajo = false;
// End decission section

// Motors section
AF_DCMotor baseMotor(1);
AF_DCMotor arribaba(2);
// End motors section

// Wtf section
float leftAngle, rightAngle, middleAngle;
boolean callRight, callLeft, pressed, calibrated, enter = false;
// End wtf section

// Serial input section
String content = "";
char character;
// End serial input section

// Time section
boolean activated = false;
boolean firstClick = false;
boolean leftIn = false;
boolean rightIn = false;
long time;
long elapsed = 0;
// End time section


//BACWARD = right

//TODO: Poner el código en bitbucket, github, etcétera... 
//TODO: Recibir mensajes mediante el serial, json... 
//TODO: Remover comentarios innecesario...
//TODO: Timer LISTOOOO
//TODO: Buscar una manera más eficiente para el server de graficar los datos.
void setup(){
  Serial.begin(9600);
  
  pinMode(left, INPUT);
  pinMode(right, INPUT);
  
  pinMode(arriba, INPUT);
  pinMode(abajo, INPUT);
  
  pinMode(led, OUTPUT);
  
  baseMotor.setSpeed(190);
  arribaba.setSpeed(190);
  
  if(digitalRead(right) == HIGH){
    turnLeft = true;
  } 
  
  if(digitalRead(left) == HIGH){
    turnRight = true;
  }
  
  if(digitalRead(left) == LOW && digitalRead(right) == LOW){
    turnRight = true;
  }
  
  if(digitalRead(arriba) == HIGH){
    turnAbajo = true;
  }
  
  if(digitalRead(abajo) == HIGH){
    turnArriba = true;
  }
  
  if(digitalRead(arriba) == LOW && digitalRead(abajo) == LOW){
    turnArriba = true;
  }
  
  if(mag.begin() != 0){
    //Serial.println("Error connecting to Magnetometer");
    fail = true; 
    return;
  }
  
  mag.setGain(HMC5833L_GAIN_1370);
}

void loop(){
  content = "";
  
  if(fail)
    return;
    
  int8_t ret = mag.readHeadingDeg(&heading);
  
  switch(ret){
    case HMC5833L_ERROR_GAINOVERFLOW:
      //Serial.println("Gain Overflow");
      return;
    case 0:
      break;
    default:
      //Serial.println("Failed to read Magnetometer");
      return; 
  }
  
    
  if(turnLeft){
    baseMotor.run(FORWARD);
  } else if (turnRight){
    baseMotor.run(BACKWARD);
  }
  
  if(turnArriba){
    arribaba.run(FORWARD);
  } else if (turnAbajo){
    arribaba.run(BACKWARD);
  }

  while (Serial.available() != 0){
    character = Serial.read();
    content.concat(character);
    delay(2); 
  }
  
  if (content != ""){
    if(content == "0"){
      baseMotor.setSpeed(0);
      arribaba.setSpeed(0);
    }
    
    if(content == "1"){
      baseMotor.setSpeed(190);
      arribaba.setSpeed(190);
    }
    
    //Serial.println(content);
  }
  
  if(calibrated){
    /*
    if(!enter){
      if(heading > (middleAngle - 1) && heading < (middleAngle + 1)){
        enter = true;
        
        baseMotor.setSpeed(0);
        digitalWrite(led, HIGH);
        
        delay(2000);
        
        digitalWrite(led, LOW);
        baseMotor.setSpeed(190); 
      }
    }
    else {
      if (heading < (middleAngle - 1.5) || heading > (middleAngle + 1.5)){
        enter = false;
      }
    } */
    
    ArduinoJson::Generator::JsonObject<2> baseSwitch;
    
    if(digitalRead(left) == HIGH){
      baseSwitch["left"] = "On";
    }
    else{
      baseSwitch["left"] = "Off";
    }
    
    if(digitalRead(right) == HIGH){
      baseSwitch["right"] = "On";
    }
    else{
      baseSwitch["right"] = "Off";
    }
    
    ArduinoJson::Generator::JsonObject<2> baseAngles;
    baseAngles["left"] = leftAngle;
    baseAngles["right"] = rightAngle;
    
    ArduinoJson::Generator::JsonObject<5> root;
    root["actualAngle"] = heading;
    root["switchStates"] = baseSwitch;
    root["baseAngles"] = baseAngles;
    root["calibrated"] = true;
    root["average"] = elapsed;
    
    Serial.println(root);
    
  }
  else{
    ArduinoJson::Generator::JsonObject<1> root;
    root["calibrated"] = false;
    
    Serial.println(root);
  }
  
  //Serial.println(heading);
  
  checkBase();
  checkArribaBa();
}

//------------------------FUNCTIONS------------------------//

void checkBase(){
  if(digitalRead(left) == HIGH){    
    turnLeft = false;
    turnRight = true;
    
    //Timer section 
    if(!leftIn){
      leftIn = true;
      
      if(!firstClick){
        firstClick = true;
        time = millis();
      }
      else if(firstClick){
        firstClick = false;
        elapsed = time - millis();
      }
    }
    //End timer section
    
    if(!callLeft){
      leftAngle = heading;
      callLeft = true;
    }
    
    if(callRight && callLeft){
      calibrated = true;
    }
    
    if(calibrated){
      middleAngle = (leftAngle + rightAngle) / 2;
      
      /*
      Serial.print(leftAngle);
      Serial.print(" Right angle: ");
      Serial.print(rightAngle);
      Serial.print(" Middle angle: ");
      Serial.println(middleAngle);
      */
    
    }
  } else {
   leftIn = false;
  } 
  
  if(digitalRead(right) == HIGH){
    turnLeft = true;
    turnRight = false;
    
    //Timer section 
    if(!rightIn){
      rightIn = true;
      
      if(!firstClick){
        firstClick = true;
        time = millis();
      }
      else if(firstClick){
        firstClick = false;
        elapsed = time - millis();
      }
    }
    //End timer section
    
    if(!callRight){
      rightAngle = heading;
      callRight = true;
    }
    
    if(callRight && callLeft){
      calibrated = true;
    }
    
    if(calibrated){
      middleAngle = (leftAngle + rightAngle) / 2;
      
      /*
      Serial.print(leftAngle);
      Serial.print(" Right angle: ");
      Serial.print(rightAngle);
      Serial.print(" Middle angle: ");
      Serial.println(middleAngle);
      */
    
    }
  }
  else {
    rightIn = false;
  }
}

void checkArribaBa(){
  if(digitalRead(arriba) == HIGH){
    turnArriba = false;
    turnAbajo = true;
  }
  
  if (digitalRead(abajo) == HIGH){
    turnArriba = true;
    turnAbajo = false;
  }
}
//----------------------------------------------------------//
