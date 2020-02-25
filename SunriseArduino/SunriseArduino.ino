#include <Wire.h>
#include "DS1307.h"

DS1307 clock;

int LED_PIN = 3;
int brg;
#define BRG_MAX 128

#define MODE_OFF 0
#define MODE_ON  1
#define MODE_SUNRISE 2
#define MODE_SWITCHING_ON 3
#define MODE_SWITCHING_OFF 4
int mode = MODE_OFF;

struct SUNRISE_TIME {
  uint8_t hour;
  uint8_t minute;
} SunriseTime;

void setup() {
  // PWM setup, see https://etechnophiles.com/change-frequency-pwm-pins-arduino-uno/
  TCCR2B = TCCR2B & B11111000 | B00000001; // D3 & D11 PWM frequency 31372.55 Hz
  pinMode(LED_PIN, OUTPUT);

  // bluetooth setup
  Serial.begin(9600);
  pinMode(13, OUTPUT);

  Serial.println("Ready...");
}


const byte numChars = 32;
char command[numChars];   // command from mobile phone


void loop() {

  // is it time to Sunrise?
  if (mode == MODE_OFF) {
    clock.getTime();
    if (SunriseTime.hour == clock.hour && SunriseTime.minute == clock.minute) {
      Serial.print("Sunrise time");
      mode = MODE_SUNRISE;
    }
  }

  // read command from Bluetooth
  if (readCommandFromBT()) {
    Serial.print("Command: ");
    Serial.println(command);
    executeCommand();
  }

  // adjust brightness
  adjustBrightness();

  //  analogWrite(LED_PIN, brg);
  //  brg += delta;
  //  if (brg <= 0 ) {brg = 0; delta = -delta; }
  //  if (brg >= 255) {brg = 255; delta = -delta; }
  delay(50);
}

void adjustBrightness()
{
  if (MODE_SUNRISE == mode) {
    if (brg < BRG_MAX) analogWrite(LED_PIN, ++brg);
    if (brg == BRG_MAX) {
      mode = MODE_ON;
    }
  } else if (mode == MODE_SWITCHING_ON) {
    if (brg < BRG_MAX) analogWrite(LED_PIN, ++brg);
    if (brg == BRG_MAX) {
      mode = MODE_ON;
    }
  } else if (mode == MODE_SWITCHING_OFF) {
    if (brg > 0) analogWrite(LED_PIN, --brg);
    if (brg == 0) {
      mode = MODE_OFF;
    }
  }
  Serial.print("Brg=");
  Serial.println(brg);
}

void executeCommand()
{
  if (strcmp(command, "0") == 0) {
    Serial.println("MODE_SWITCHING_OFF");
    mode = MODE_SWITCHING_OFF;
  } else if (strcmp(command, "1") == 0) {
    Serial.println("MODE_SWITCHING_ON");
    mode = MODE_SWITCHING_ON;
  } else if (command[0] == 'S') {
    // "sHH:mm:ss|HH:mm"
    if (strlen(command) != 18) {
      Serial.println("Wrong command length");
      return;
    }
    if (command[3] != ':' || command[6] != ':' || command[9] != '|' || command[12] != ':') {
      Serial.println("Wrong command delimiter");
      return;
    }
    clock.hour = atoi(command + 1);
    clock.minute = atoi(command + 4);
    clock.second = atoi(command + 7);
    clock.setTime(); //write time to the RTC chip
    SunriseTime.hour = atoi(command + 10);
    SunriseTime.minute = atoi(command + 13);

    //    clock.getTime();
    //    char strBuf[50];
    //    sprintf(strBuf, "%d:%d:%d", x);
    //    Serial.println(strBuf);

    //    int h = atoi(command+1);
    //    int m = atoi(command+4);
    //    int s = atoi(command+7);
    //    Serial.print("h=");
    //    Serial.println(h);
    //    Serial.print("m=");
    //    Serial.println(m);
    //    Serial.print("s=");
    //    Serial.println(s);
    printTime();
  }
}


bool readCommandFromBT() {
  static byte n = 0;
  char endMarker = '\n';
  char c;

  while (Serial.available() > 0) {
    if ((c = Serial.read()) == endMarker) {
      command[n] = '\0'; // terminate the string
      n = 0;
      return true;
    }
    command[n++] = c;
    if (n == numChars) n--;
  }
  return false;
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
