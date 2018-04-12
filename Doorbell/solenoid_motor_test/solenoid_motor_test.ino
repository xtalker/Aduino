// This tests using a generic motor shield to drive solenoids from an old Rittenhouse 520 doorbell
// Motor shield is well documented here:
// http://playground.arduino.cc/Main/AdafruitMotorShield
// (I removed anything not required to drive a solenoid)
//
// Chime sounds: https://www.1800doorbell.com/resources/door-chimes/doorbell-door-chime-sounds/
//
// 7/16/17,  Bob S.


// Arduino pins for the shift register
#define MOTORLATCH 12
#define MOTORCLK 4
#define MOTORENABLE 7
#define MOTORDATA 8

// 8-bit bus after the 74HC595 shift register 
// (not Arduino pins)
// These are used to set the direction of the bridge driver.
#define CHIME_A 2
#define CHIME_B 1
#define CHIME_C 5

void setup()
{
  Serial.begin(115200);
  Serial.println("Simple Motor Shield sketch");
}

void loop() {
  // Suppose there is a relay, or light or solenoid
  // connected to M3_A and GND.
  // The output is a push-pull output (half bridge), 
  // so it can also be used to drive something low.

  chime(CHIME_A, 1000);
  chime(CHIME_B, 1000);
  chime(CHIME_C, 1000);

}

// Activate a solenoid to strike a chime
void chime(int note, int wait_time)
{
  motor_output(note, HIGH);
  delay(20);
  motor_output(note, LOW);

  delay(wait_time);
}


// ---------------------------------
// motor_output
//
// The function motor_ouput uses the motor driver to
// drive normal outputs like lights, relays, solenoids, 
// DC motors (but not in reverse).
//
// The high_low variable should be set 'HIGH' 
// to drive lights, etc.
// It can be set 'LOW', to switch it off
//
//
void motor_output (int output, int high_low)
{
  int motorPWM;

  // Set the direction with the shift register 
  // on the MotorShield
  
  shiftWrite(output, high_low);

}


// ---------------------------------
// shiftWrite
//
// The parameters are just like digitalWrite().
//
// The output is the pin 0...7 (the pin behind 
// the shift register).
// The second parameter is HIGH or LOW.
//
// There is no initialization function.
// Initialization is automatically done at the first
// time it is used.
//
void shiftWrite(int output, int high_low)
{
  static int latch_copy;
  static int shift_register_initialized = false;

  // Do the initialization on the fly, 
  // at the first time it is used.
  if (!shift_register_initialized)
  {
    // Set pins for shift register to output
    pinMode(MOTORLATCH, OUTPUT);
    pinMode(MOTORENABLE, OUTPUT);
    pinMode(MOTORDATA, OUTPUT);
    pinMode(MOTORCLK, OUTPUT);

    // Set pins for shift register to default value (low);
    digitalWrite(MOTORDATA, LOW);
    digitalWrite(MOTORLATCH, LOW);
    digitalWrite(MOTORCLK, LOW);
    // Enable the shift register, set Enable pin Low.
    digitalWrite(MOTORENABLE, LOW);

    // start with all outputs (of the shift register) low
    latch_copy = 0;

    shift_register_initialized = true;
  }

  // The defines HIGH and LOW are 1 and 0.
  // So this is valid.
  bitWrite(latch_copy, output, high_low);

  // Use the default Arduino 'shiftOut()' function to
  // shift the bits with the MOTORCLK as clock pulse.
  // The 74HC595 shiftregister wants the MSB first.
  // After that, generate a latch pulse with MOTORLATCH.
  shiftOut(MOTORDATA, MOTORCLK, MSBFIRST, latch_copy);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, HIGH);
  delayMicroseconds(5);    // For safety, not really needed.
  digitalWrite(MOTORLATCH, LOW);
}

