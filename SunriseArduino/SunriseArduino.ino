#include <Wire.h>
#include "DS1307.h"

DS1307 clock;

#define LED_PIN 3
#define LED_STATUS 13
#define BRG_MAX 128
int brg;

#define MODE_OFF 0
#define MODE_ON  1
#define MODE_SUNRISE 2
#define MODE_SWITCHING_ON 3
#define MODE_SWITCHING_OFF 4
int mode = MODE_OFF;

#define COMMAND_BUF_SIZE 16
char command[COMMAND_BUF_SIZE];   // command from mobile phone

struct {
  uint8_t hour;
  uint8_t minute;
} SunriseTime = {25, 0};

bool syncComplete = false;

void setup() {
  // PWM setup, see https://etechnophiles.com/change-frequency-pwm-pins-arduino-uno/
  TCCR2B = TCCR2B & B11111000 | B00000001; // D3 & D11 PWM frequency 31372.55 Hz
  pinMode(LED_PIN, OUTPUT);

  // status led setup
  pinMode(LED_STATUS, OUTPUT);

  // RTC setup
  clock.begin();

  // bluetooth setup
  Serial.begin(9600);

  Serial.println("Ready...");
}

void loop() {
  // is it time to Sunrise?
  if (mode == MODE_OFF) {
    clock.getTime();

printStatus();
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

  // show status
  showStatus();

  delay(25);
}

void printStatus(){
  Serial.print("Mode= ");
  Serial.print(mode);
  Serial.print(" Time= ");
  Serial.print(clock.hour, DEC);
  Serial.print(":");
  Serial.print(clock.minute, DEC);
  Serial.print(":");
  Serial.print(clock.second, DEC);
  Serial.print(" SunriseTime= ");
  Serial.print(SunriseTime.hour, DEC);
  Serial.print(":");
  Serial.print(SunriseTime.minute, DEC);  
  Serial.println(" ");
    
}

void showStatus() {
  static int status = LOW;

  if (!syncComplete) status = (status == LOW ? HIGH : LOW);
  else if (BRG_MAX == brg)  status = HIGH;
  else if (0 == brg) status = LOW;
  
  digitalWrite(LED_STATUS, status);
}

void adjustBrightness()
{
  static int delay = 0;
  if (delay > 0) {
    delay--;
    return;
  }

  if (MODE_SUNRISE == mode && brg < BRG_MAX) {
    analogWrite(LED_PIN, ++brg);
    delay = 450;
  }
  else if (MODE_SWITCHING_ON == mode && brg < BRG_MAX) analogWrite(LED_PIN, ++brg);
  else if (MODE_SWITCHING_OFF == mode && brg > 0) analogWrite(LED_PIN, --brg);

  if (BRG_MAX == brg) {
    mode = MODE_ON;
  }
  if (0 == brg) {
    mode = MODE_OFF;
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
  } else if (command[0] == 'S') { // sync    
    // "sHH:mm:ss|HH:mm" current_time|sunrise_time
    if (strlen(command) != 15) {
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
//    SunriseTime.hour = 3;
    SunriseTime.minute = atoi(command + 13);
    syncComplete = true;
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
    if (n == COMMAND_BUF_SIZE) n--;
  }
  return false;
}
