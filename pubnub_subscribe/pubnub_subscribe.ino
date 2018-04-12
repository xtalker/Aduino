/*
 *  Pubnub subscribe test
*/

#include <ESP8266WiFi.h>
#include "string.h"
#include "iotconnector.h"

const char* ssid     = "6fusion";
const char* password = "W4C1TG00D";

char pubkey[] = "pub-c-83fee7a8-67d4-4461-a3e0-0e1f7234d7ef";
char subkey[] = "sub-c-a7169fa6-7365-11e5-8f6d-0619f8945a4f";
char channel[] = "my_channel";
char uuid[] = "Arduino";

iotbridge arduino;

void do_something(String value) {
  Serial.println("in the callback");
  Serial.println(value);
}

void setup() {  
  Serial.begin(115200);
  delay(10);
 
  // We start by connecting to a WiFi network 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  arduino.init(pubkey, subkey, uuid);       
}

void loop() {      
  String returnmessage;
  delay(5000);

  //Subscribe 
  returnmessage = arduino.connect(channel); 
        
  //callback function of sorts, to work with the received message
  do_something(returnmessage); 
  
  Serial.println('<');
}
                                      

