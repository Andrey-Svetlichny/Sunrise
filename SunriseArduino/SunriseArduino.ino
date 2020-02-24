#include <Wire.h>
#include "DS1307.h"

DS1307 clock;

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
  setTime(11, 50, 0);

  Serial.println("Ready...");
}

//write time to the RTC chip
void setTime(int h, int m, int s) {
  clock.fillByHMS(h, m, s);
  clock.setTime();
}

const byte numChars = 32;
char command[numChars];   // an array to store the received data
boolean newCommand = false;


void loop() {
  //  analogWrite(LED_PIN, brg);
  //  brg += delta;
  //  if (brg <= 0 ) {brg = 0; delta = -delta; }
  //  if (brg >= 255) {brg = 255; delta = -delta; }
  //  delay(30);



  if (Serial.available() > 0)
  {
    readCommandFromBT();
    if (newCommand == true) {
      Serial.print("Command: ");
      Serial.println(command);
      newCommand = false;
      printTime();
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



void readCommandFromBT() {
  static byte n = 0;
  char endMarker = '\n';
  char c;

  while (Serial.available() > 0) {
    if ((c = Serial.read()) == endMarker) {
      command[n] = '\0'; // terminate the string
      n = 0;
      newCommand = true;
      break;
    }
    command[n++] = c;
    if (n == numChars) n--;
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
  Serial.println(" ");
}
