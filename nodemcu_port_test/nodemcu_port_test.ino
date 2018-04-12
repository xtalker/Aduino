
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15


void setup() {
  
  pinMode(D0,  INPUT_PULLUP);
  pinMode(D1,  INPUT_PULLUP);
  pinMode(D2,  INPUT_PULLUP);
  pinMode(D3,  INPUT_PULLUP);
  pinMode(D4,  INPUT_PULLUP);
  pinMode(D5,  INPUT_PULLUP);
  pinMode(D6,  INPUT_PULLUP);
  pinMode(D7,  INPUT_PULLUP);
  pinMode(D8,  INPUT_PULLUP);

//  pinMode(D0,  OUTPUT);
//  pinMode(D1,  OUTPUT);
//  pinMode(D2,  OUTPUT);
//  pinMode(D3,  OUTPUT);
//  pinMode(D4,  OUTPUT);
//  pinMode(D5,  OUTPUT);
//  pinMode(D6,  OUTPUT);
//  pinMode(D7,  OUTPUT);
//  pinMode(D8,  OUTPUT);

  Serial.begin(115200);
  Serial.println("\nNodeMCU Test");

}

void loop() {

  digitalRead(D0) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D1) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D2) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D3) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D4) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D5) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D6) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D7) ? Serial.print("H ") : Serial.print("L ");
  digitalRead(D8) ? Serial.print("H ") : Serial.print("L ");
  Serial.println("!");     

//  Serial.println("HIGH");     
//  digitalWrite(D0,  HIGH);
//  digitalWrite(D1,  HIGH);
//  digitalWrite(D2,  HIGH);
//  digitalWrite(D3,  HIGH);
//  digitalWrite(D4,  HIGH);
//  digitalWrite(D5,  HIGH);
//  digitalWrite(D6,  HIGH);
//  digitalWrite(D7,  HIGH);
//  digitalWrite(D8,  HIGH);
//
//  delay(1000);
//
//  Serial.println("LOW");     
//  digitalWrite(D0,  LOW);
//  digitalWrite(D1,  LOW);
//  digitalWrite(D2,  LOW);
//  digitalWrite(D3,  LOW);
//  digitalWrite(D4,  LOW);
//  digitalWrite(D5,  LOW);
//  digitalWrite(D6,  LOW);
//  digitalWrite(D7,  LOW);
//  digitalWrite(D8,  LOW);
//
  delay(1000);

}
