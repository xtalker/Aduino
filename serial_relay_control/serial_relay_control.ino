// Arduino program to advance a slide projector then trigger a camera shutter release
// first used with Kodak carousel projector and a Nikon D90 DSLR camera.  Overall
// timing and control handled by PC via serial port link to arduino.
// Jan 30, 2016 - Bob S.

int MAX_CMD_LENGTH = 10;
char cmd[10];
int cmdIndex;
char incomingByte;
unsigned char relayPin[5] = {4,5,6,7,13};
int focus   = 4;
int shutter = 5;
int slide   = 6;
int on_delay = 1000;

void setup() {
  
  //Setup Serial Port with baud rate of 9600
  Serial.begin(9600);
  Serial.println("Slide Scan Control: 'SLIDE' to trigger projector, 'PHOTO' to trigger camera shutter");
  Serial.println();
  
  int i;
  for(i = 0; i < 5; i++) {
    pinMode(relayPin[i],OUTPUT);
  }
  
  //indicate start
  cmdIndex = 0;
}
 
void loop() {
    
  if (incomingByte=Serial.available()>0) {
    
    char byteIn = Serial.read();
    cmd[cmdIndex] = byteIn;
    Serial.print(byteIn);

    // Mac='\n', PC='\r'
    if(byteIn=='\r' || byteIn=='\n') {
      
      //command finished
      cmd[cmdIndex] = '\0';
      //Serial.println("<n>");
      cmdIndex = 0;
      
      if(strcmp(cmd, "SLIDE")  == 0){
        
        //Serial.println("Command received: SLIDE");
        digitalWrite(slide,HIGH);
        delay(250);
        digitalWrite(slide,LOW);
        Serial.println("OK");
        
      }else if (strcmp(cmd, "PHOTO")  == 0) {
        
        //Serial.println("Command received: PHOTO");
        // Camera focus must be on before shutter can be triggered
        digitalWrite(focus,HIGH);
        delay(on_delay);
        digitalWrite(shutter,HIGH);
        delay(250);
        digitalWrite(shutter,LOW);
        digitalWrite(focus,LOW);
        Serial.println("OK");
        
      }else{
        //Serial.println("Command received: unknown!");
        Serial.println("ERROR");
     }
      
    }else{
      if(cmdIndex++ >= MAX_CMD_LENGTH){
        cmdIndex = 0;
      }
    }
  }
}
