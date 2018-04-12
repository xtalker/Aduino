const byte interruptPin = 0;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;

int gpio0Button  = 0;
int gpio13Led = 13;
int relayState = 0;
int buttonCnt = 0;
   
void setup() {
 
  pinMode(gpio0Button, INPUT);
  pinMode(gpio13Led, OUTPUT);
  
  Serial.begin(115200);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, RISING);
}
 
void handleInterrupt() {
  interruptCounter++;
  if (relayState == 1) {
    digitalWrite(gpio13Led, LOW);
//    Serial.println("setRelay: On");
    relayState = 0;
  } else {
    digitalWrite(gpio13Led, HIGH);
//    Serial.println("setRelay: Off");
    relayState = 1;
  } 
}

 
void loop() {
 
  if(interruptCounter>0){
 
      interruptCounter--;
      numberOfInterrupts++;
 
      Serial.print("An interrupt has occurred. Total: ");
      Serial.print(numberOfInterrupts);
      Serial.print(" - ");
      Serial.println(relayState);
  }

    // Detect Sonoff button presses, short and long (~10 secs)
  if (digitalRead(gpio0Button) == LOW) {
    buttonCnt += 1;
    delay(100);
  } else {
    if (buttonCnt >= 40) {
      Serial.printf("Long button press (Cnt: %d) Enter setup mode\n",buttonCnt);
      // Blink led to indicate mode change
      for (int t=0; t<20; t++) {
        digitalWrite(gpio13Led, LOW);
        delay(100);
        digitalWrite(gpio13Led, HIGH);
        delay(100);
      }    
      Serial.printf("Resetting wifi, please cycle power!");
    }
    buttonCnt = 0;
  }

}

