// Drive doorbell solenoids with a TIP120 transistor
// Chime sounds: https://www.1800doorbell.com/resources/door-chimes/doorbell-door-chime-sounds/
//
// 8/1/17,  Bob S.

// Arduino outputs
#define CHIME_A 14
#define CHIME_B 13
#define CHIME_C 12

void setup()
{
  // initialize digital outputs.
  pinMode(CHIME_A, OUTPUT);
  pinMode(CHIME_B, OUTPUT);
  pinMode(CHIME_C, OUTPUT);
  
  digitalWrite(CHIME_A, LOW);
  digitalWrite(CHIME_B, LOW);
  digitalWrite(CHIME_C, LOW);

  Serial.begin(115200);
  Serial.println("Simple Doorbell Solenoid sketch");
}

void loop() {

//  chime(CHIME_A, 2000);
  chime(CHIME_B, 2000);
//  chime(CHIME_C, 2000);

}

// Activate a solenoid to strike a chime
void chime(int note, int wait_time)
{
  Serial.println("chime: high");
  digitalWrite(note, HIGH);
  delay(20);
  Serial.println("chime: low");
  digitalWrite(note, LOW);

  delay(wait_time);
}


