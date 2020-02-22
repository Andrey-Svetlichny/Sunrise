int LED_PIN = 5;
int brg;
int delta = 10;

void setup() {

  // PWM setup, see https://etechnophiles.com/change-frequency-pwm-pins-arduino-uno/
//  TCCR0B = TCCR0B & B11111000 | B00000010; // PWM frequency of 7812.50 Hz
  TCCR0B = TCCR0B & B11111000 | B00000001; // PWM frequency of 62500.00 Hz
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 128);

  // bluetooth setup
  Serial.begin(9600);
  pinMode(13, OUTPUT); 

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
