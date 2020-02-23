#include <Wire.h>
#include "DS1307.h"
DS1307 clock;//define a object of DS1307 class


int LED_PIN = 3;
int brg;
int delta = 10;

void setup() {

  // PWM setup, see https://etechnophiles.com/change-frequency-pwm-pins-arduino-uno/
//  TCCR0B = TCCR0B & B11111000 | B00000010; // D5 & D6 PWM frequency  7812.50 Hz
//  TCCR0B = TCCR0B & B11111000 | B00000001; // D5 & D6 PWM frequency 62500.00 Hz

  TCCR2B = TCCR2B & B11111000 | B00000001; // D3 & D11 PWM frequency 31372.55 Hz
  pinMode(LED_PIN, OUTPUT);
//  analogWrite(LED_PIN, 128);
  analogWrite(LED_PIN, 64);

  // bluetooth setup
  Serial.begin(9600);
  pinMode(13, OUTPUT); 

  // RTC setup
  clock.begin();
  clock.fillByYMD(2013,1,19);//Jan 19,2013
  clock.fillByHMS(15,28,30);//15:28 30"
  clock.fillDayOfWeek(SAT);//Saturday
  clock.setTime();//write time to the RTC chip

  Serial.println("Ready...");  
}

 const byte numChars = 32;
 char receivedChars[numChars];   // an array to store the received data
 boolean newData = false;


void loop() { 
//  analogWrite(LED_PIN, brg);
//  brg += delta;
//  if (brg <= 0 ) {brg = 0; delta = -delta; }
//  if (brg >= 255) {brg = 255; delta = -delta; }
//  delay(30);

  printTime();


 if(Serial.available() > 0)  
  {
    readCommandFromBT();
    if (newData == true) {
        Serial.print("Command: ");
        Serial.println(receivedChars);
        newData = false;
    }

    
//    Incoming_value = Serial.read();      //Read the incoming data and store it into variable Incoming_value
//    Serial.print(Incoming_value);        //Print Value of Incoming_value in Serial monitor
//    Serial.print("\n");        //New line 
//    if(Incoming_value == '1'){
//      digitalWrite(13, HIGH);  //If value is 1 then LED turns ON
//      analogWrite(LED_PIN, 128);
//    }
//    else if(Incoming_value == '0'){
//      digitalWrite(13, LOW);   
//      analogWrite(LED_PIN, 0);
//    }
  }     
}



void readCommandFromBT(){
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
   
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void printTime()
{
  clock.getTime();
  Serial.print(clock.hour, DEC);
  Serial.print(":");
  Serial.print(clock.minute, DEC);
  Serial.print(":");
  Serial.print(clock.second, DEC);
  Serial.print("  ");
  Serial.print(clock.month, DEC);
  Serial.print("/");
  Serial.print(clock.dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(clock.year+2000, DEC);
  Serial.print(" ");
  switch (clock.dayOfWeek)// Friendly printout the weekday
  {
    case MON:
      Serial.print("MON");
      break;
    case TUE:
      Serial.print("TUE");
      break;
    case WED:
      Serial.print("WED");
      break;
    case THU:
      Serial.print("THU");
      break;
    case FRI:
      Serial.print("FRI");
      break;
    case SAT:
      Serial.print("SAT");
      break;
    case SUN:
      Serial.print("SUN");
      break;
  }
  Serial.println(" ");
}
