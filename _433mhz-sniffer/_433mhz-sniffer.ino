/*
 RF_Sniffer
 
 Hacked from http://code.google.com/p/rc-switch/
 by @justy to provide a handy RF code sniffer

 hacked further by pico to supply power to unit from data pins (lazy!)
 just make sure your recv unit has max draw < 40mA... (typically rated 10-20mA)

 this is set up for a 4 pin recv unit GND DATA DATA VCC
 plug GND into D2, DATA into D3 and D4, and VCC into D5
*/

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

#define VCC_PIN 5 // source 5V up to 40mA from this pin
#define GND_PIN 2 // sink up to 40mA on this pin
#define DATA_PIN 3 // external int 1 on Uno
#define LED 13 // Onboard led

void setup() {

   pinMode(LED, OUTPUT);

   pinMode(DATA_PIN, INPUT);
   // just leave D4 tristated
 
   pinMode(GND_PIN, OUTPUT);
   digitalWrite(GND_PIN, LOW); 

   pinMode(VCC_PIN, OUTPUT);
   digitalWrite(VCC_PIN, HIGH);

   Serial.begin(9600);
   mySwitch.enableReceive(1);  // Receiver on interrupt 1 => that is pin D3
  Serial.println("rf_sniffer started");
}

static unsigned long count = 0;

void loop() {
    
  if (mySwitch.available()) {
     digitalWrite(LED, HIGH); 
    int value = mySwitch.getReceivedValue();
   
    if (value == 0) {
       Serial.print("Unknown encoding");
    } 
    else {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
    }
   
      mySwitch.resetAvailable();    
      count = 0;
      digitalWrite(LED, LOW); 
    }
  else {
      if (++count == 0) Serial.println("no activity");
  }
}
