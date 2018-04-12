// Drive doorbell solenoids, provides switch inputs, URL and web page control of chimes
// Initially used with a LiLon NodeMCU 1.0 (based on ESP-12E module)
// Bob S. 12/7/17
//
// Chime sounds: https://www.1800doorbell.com/resources/door-chimes/doorbell-door-chime-sounds/
//
//  Wifi Setup
//  If no wifi connection or connection is lost, it goes into config mode and will time out, 
//  reboot and try again to reconnect (like after a power or wifi fail).
//  The config access point (should open automatically, or go to 192.168.4.1) can be started at 
//  any time by pressing and holding (for ~5 secs) the blue button. Note: this will erase wifi 
//  connection info!
//
//  Blue Button LED:  Off: normal operation, On: in config mode
// 
//  Available endpoints: Doorbells: /front, /back, Individual Chimes: /low, /med, /hi
//
//  OTA Programming supported! (set Tools-Port: "Wifi-Doorbell at <ip address>")
//
//  ISY URL: http://admin:admin@192.168.0.10

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>           //https://github.com/esp8266/Arduino/tree/master/doc/ota_updates

// Local project files (see tabs)
#include "Vars_Constants.h"
#include "Tunes.h"

// Capture compile time info
const char compile_info[] = __DATE__ ", " __TIME__ " File:" __FILE__;

WiFiManager wifiManager;

void setup() {

  delay(3000);
  Serial.begin(115200);
  Serial.println("\nLove Shack Wifi Doorbell");

  #include "OTA.h"
  #include "Init_IO.h"

  // Clean FS, for testing, make sure to run this then connect if any var names change, or it crashes!
  //Serial.println("Formatting FS...");
  //SPIFFS.format();
  //wifiManager.resetSettings();

  #include "ESP_Data.h"

  wifiManager.setConfigPortalTimeout(60);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.autoConnect("Wifi_Doorbell_Config");
    
  if (WiFi.status() == WL_CONNECTED) {

    digitalWrite(LED, LOW);
    Serial.println("Wifi Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Read updated setup parameters from ESP's file system
    strcpy(isy_x10_dev, custom_isy_x10_dev.getValue());
    strcpy(isy_url, custom_isy_url.getValue());
  
    // Save the custom parameters to FS
    if (shouldSaveConfig) {
      eeprom_save();
    }

    Serial.printf("ISY X10 Dev: \t%s\n",isy_x10_dev);
    Serial.printf("ISY URL: \t\t%s\n",isy_url);

    #include "Web_Server.h"
    server.begin();
    Serial.println("Doorbell HTTP server started");  
  } else {
    Serial.println("Wifi NOT Connected, resetting!");
    ESP.reset();
    delay(5000);  
  }

  // Play chimes to let you know it's up
  int len = sizeof (startup) / sizeof (startup[0]);
  activate("", startup, len, "", "");

}

void loop(void) {

  ArduinoOTA.handle();
  server.handleClient();

  unsigned long currentMillis = millis();

  // Check config button
//  if ( digitalRead(CTRL_BUTTON) == LOW ) {
//    WiFi.disconnect();
//  }

  // Detect a long button presses (~10 secs)
  if (digitalRead(CTRL_BUTTON) == LOW) {
    buttonCnt += 1;
  } else {
    
    if ( buttonCnt >= 150000 ) {
      Serial.printf("Long button press (Cnt: %d) Resetting Wifi, enter config mode\n",buttonCnt);
      // Blink led to indicate mode change
      for (int t=0; t<10; t++) {
        digitalWrite(LED, LOW);
        delay(100);
        digitalWrite(LED, HIGH);
        delay(100);
      }     
      WiFi.disconnect();
    }
    buttonCnt = 0;
  }


  // Reset if not connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Resetting!");
    ESP.reset();
    delay(5000);  
  } else {
    digitalWrite(LED, LOW);    
  }
  
  // Check front doorbell button
  if (digitalRead(FRONT_DOOR) == LOW) {
    Serial.println("Front Door!"); 
    int len = sizeof (tune_1) / sizeof (tune_1[0]);
    activate("front", tune_1, len, isy_x10_dev, isy_url);
  }

  // Check back doorbell button
  if (digitalRead(BACK_DOOR) == LOW) {
    Serial.println("Back Door!");     
    int len = sizeof (tune_1) / sizeof (tune_1[0]);
    activate("back", tune_1, len, isy_x10_dev, isy_url);
  }

  // Blink the node led to indicate this loop is running
  if (currentMillis - node_led_prev_millis >= node_led_interval) {
    node_led_prev_millis = currentMillis;
    digitalWrite(NODE_LED, !digitalRead(NODE_LED));
  }
  
}



