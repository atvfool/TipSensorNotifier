/*
 * Author: Andrew Hayden
 * Github: https://github.com/atvfool
 * 
 * The idea behind this project is to notify 1 or more numbers via text with coordinators with the device tips. 
 * The real world use case is to strap this thing to an ATV and to notify other if it flips over
 * The circuit:
 * -Arduino MKRGSM1400 board
 * -Arduino MKR IMU Shield attached
 */

// includes
#include <MKRIMU.h>
#include <MKRGSM.h>
#include <Arduino_MKRGPS.h>
#include "arduino_secrets.h" 

// constants
const float rollLimits[2] = {-50, 50};
const float pitchLimits[2] = {-80, 80};
const char PINNUMBER[] = SECRET_PINNUMBER;

// variables
float eulerAngleRollOffset = 0;
float eulerAnglePitchOffset = 0;
GSMLocation location;
GPRS gprs;
GSM gsmAccess;
GSM_SMS sms;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while(!Serial);

  Serial.println("Numbers to notify in case of an incident:");
  for( int i=0;i<sizeof(NUMBER_LIST_LENGTH)-1; i++){
    Serial.println(NUMBERS_TO_NOTIFY[i]);
  }

  // IMU Initialization
  if(!IMU.begin()){
    Serial.println("Failed to initialize IMU!");
     binkIMUError();
  }

  if(IMU.eulerAnglesAvailable()){
    float temp;
    IMU.readEulerAngles(temp, eulerAngleRollOffset, eulerAnglePitchOffset);
    eulerAngleRollOffset *= -1;
    eulerAnglePitchOffset *= -1;
  }

  // GMS Initialization
  Serial.println("SMS Messages Sender");

  // connection state
  bool connected = false;

  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (!connected) {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("GSM initialized");
  
  
  Serial.print("Roll Offset:");
  Serial.println(eulerAngleRollOffset);

  Serial.print("Pitch Offset:");
  Serial.println(eulerAnglePitchOffset);

  Serial.print('\t');
  Serial.print("Roll");
  Serial.print('\t');
  Serial.println("Pitch");
}

void loop() {
  // put your main code here, to run repeatedly:
  float heading, roll, pitch;

  if(IMU.eulerAnglesAvailable())
    IMU.readEulerAngles(heading, roll, pitch);

  Serial.print('\t');
  Serial.print(roll);
  Serial.print('\t');
  Serial.println(pitch);

  float adjustedRoll = roll + eulerAngleRollOffset;
  float adjustedPitch = pitch + eulerAnglePitchOffset;
  
  Serial.print("offset");
  Serial.print('\t');
  Serial.print(adjustedRoll);
  Serial.print('\t');
  Serial.println(adjustedPitch);

  // Seems like roll limits should be around 50 & -50
  // Seems like pitch limits should be around 80 & -80

  if(adjustRoll < rollLimits[0] || 
      adjustedRoll > rollLimits[1] || 
      adjustedPitch < pitchLimits[0] || 
      adjustedPitch > pitchLimits[1]){
        Serial.println("-------------EVENT RECORDED-------------");
        Serial.println("SENDING MESSAGE");
      }
  
  delay(3000);
  
}

void SendMessage(char[20] number, char[200] message){
  Serial.println("SENDING");
  Serial.println();
  Serial.println("Message:");
  Serial.println(message);

  // send the message
  sms.beginSMS(number);
  sms.print(message);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n");
}

void binkIMUError(){
  while(1){
    blinkError();
    blinkError();
    blinkError(); 
    delay(2000);
  }
}

void blinkError(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
