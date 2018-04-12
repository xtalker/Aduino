// Sonoff wifi relay control.
//
//  -Set Wifi info via config mode access point if it can't connect. Access point name:
//       "Sonoff_AP" go to setup page at 192.168.4.1 (if your device doesn't present it)
//  -Sonoff button (short press) provides local relay control, led shows relay status
//  -Sonoff button (long press, > 10 secs) forces config mode access point setup (after reboot), 
//        led flashes to let you know it has changed to config mode
//  -Serves a simple "mobile friendly" web page to control the relay (ip address must be 
//        determined from router, name on my router: "ESP_<last 6 digits of MAC>")
//  -Optionally tracks relay state with a state variable in the ISY994i home control system
//        (attempts this only if "ISY variable Index" is set on the config page)
//
// With thanks to: http://randomnerdtutorials.com/reprogram-sonoff-smart-switch-with-web-server/
// 7/3/17, Bob S.

#include <FS.h>                   //this needs to be first, or it all crashes and burns...
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//#include <WiFiClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson

HTTPClient http;
ESP8266WebServer server(80);

// Setup for saving extra config params from access point setup
// Define your default values here, if there are different values in config.json, they are overwritten.
char isy_var_index[6];
char device_name[34] = "My Sonoff Relay #1";
char isy_url[50];  // Should look like: "http://<user>:<passwd>@<ISY_IP_Address>" (with your credentials and IP)

// Flag for saving data
bool shouldSaveConfig = false;

// Callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Init vars
String webPage = "";
//String isy_url = "http://admin:admin@192.168.0.10";
String deviceName = "Sonoff Relay";
String value = "";
int gpio13Led = 13;
int gpio12Relay = 12;
int gpio0Button  = 0;
int relayState = 0;
int buttonCnt = 0;
int IsyVarIndex = 1;

WiFiManager wifiManager;

// The setup function runs once when you press reset or power the board
void setup() {
  delay(3000);
  Serial.begin(115200);     // Note Serial monitor doesn't work after Sonoff reset, not sure why.
  Serial.println("START!");

  // Clean FS, for testing, make sure to run this then connect if var names change!
  //Serial.println("Formatting FS...");
  //SPIFFS.format();

  // Read configuration from FS json
  //Serial.println("Mounting FS...");

  // Setup the ESP's file system to save config data
  if (SPIFFS.begin()) {
    Serial.println("Mounted file system");

    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      //Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");

      if (configFile) {
        //Serial.println("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        Serial.println("Config file:");
        json.printTo(Serial);
        
        if (json.success()) {
          //Serial.println("\nparsed json");
          strcpy(isy_var_index, json["isy_var_index"]);
          strcpy(device_name, json["device_name"]);
          strcpy(isy_url, json["isy_url"]);
        } else {
          Serial.println("ERROR: Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("ERROR: Failed to mount FS");
  }

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_isy_var_index("varIndex", "ISY variable Index", isy_var_index, 6);
  WiFiManagerParameter custom_device_name("devName", "device name", device_name, 32);
  WiFiManagerParameter custom_isy_url("isyUrl", "ISY URL", isy_url, 50);
  
  // preparing GPIOs
  pinMode(gpio0Button, INPUT);
  pinMode(gpio13Led, OUTPUT);
  pinMode(gpio12Relay, OUTPUT);
  digitalWrite(gpio13Led, LOW);

  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Add custom setup parameters here
  wifiManager.addParameter(&custom_isy_var_index);
  wifiManager.addParameter(&custom_device_name);
  wifiManager.addParameter(&custom_isy_url);
  
  // Use Wifimanager to connect with saved ssid/passwd.  If none or no connection, start an access point. 
  wifiManager.autoConnect("Sonoff_AP");

  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Read updated setup parameters from ESP's file system
  strcpy(isy_var_index, custom_isy_var_index.getValue());
  strcpy(device_name, custom_device_name.getValue());
  strcpy(isy_url, custom_isy_url.getValue());

  // Save the custom parameters to FS
  if (shouldSaveConfig) {
    Serial.println("Saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["isy_var_index"] = isy_var_index;
    json["device_name"] = device_name;
    json["isy_url"] = isy_url;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("ERROR: Failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
  }

  Serial.print("ISY Var: ");
  Serial.println(isy_var_index);
  Serial.print("Device Name: " );
  Serial.println(device_name);
  Serial.print("ISY URL: " );
  Serial.println(isy_url);
  String deviceNameStr(device_name);

  if (isy_var_index == "") {
    IsyVarIndex = 0;
  } else {
    IsyVarIndex = atoi(isy_var_index);
  }

  webPage += "<h1 style=\"text-align:center;font-size:500%;\">" + deviceNameStr + "</h1>";
  webPage += "<p align=\"center\">";
  webPage += "<a href=\"on\"><button style=\"height:200px;width:200px;font-size:600%;\">ON</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";
  webPage += "<a href=\"off\"><button style=\"height:200px;width:200px;font-size:600%\">OFF</button></a>";
  webPage += "</p><br><br>";
  webPage += "<h2 style=\"text-align:center;font-size:300%;\">Settings</h2>";
  webPage += "<h2 style=\"text-align:left;font-size:200%;\">ISY State Variable: " + String(IsyVarIndex) + "</h2>";
  webPage += "<h2 style=\"text-align:left;font-size:200%;\">ISY URL: " + String(isy_url) + "</h2>";

  // HTTP Server commands
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  
  server.on("/on", [](){
    server.send(200, "text/html", webPage);
    setRelay(1);
  });
  
  server.on("/off", [](){
    server.send(200, "text/html", webPage);
    setRelay(0);
  });

  server.begin();
  Serial.println("HTTP server started");  

  // Get the value of a state var on the ISY994i home controller
  if (IsyVarIndex != 0) {
    String url = String(isy_url) + "/rest/vars/get/2/" + String(IsyVarIndex);
    http.begin(url);
    int httpCode = http.GET();
  
    if (httpCode == 200) { 
      String payload = http.getString();
      
      // Get the integer between the <val> and </val> tags of the xml response
      int vStart = payload.indexOf("<val>");
      int vStop  = payload.indexOf("</val>");
      //printf("start: %d, stop: %d\n",vStart,vStop);
      value = payload.substring(vStart + 5, vStop);
      Serial.print("Initial ISY var value: ");
      Serial.println(value);
    } else {
      Serial.printf("ERROR: httpCode: %d\n", httpCode);
    }
    http.end();   //Close connection
  
    // Set intial relay state the same as the ISY var
    if (value.toInt() == 0) {
      setRelay(LOW);
    } else {
      setRelay(HIGH);
    }
  }
}

void loop(void){
  server.handleClient();

  // Detect Sonoff button presses, short and long (~10 secs)
  if (digitalRead(gpio0Button) == LOW) {
    buttonCnt += 1;
    delay(100);
  } else {
    
    if ((buttonCnt > 0) && (buttonCnt < 80)) {
      //Serial.printf("Short press, Cnt: %d\n",buttonCnt);
      // Short press: Toggle Sonoff relay
      if (relayState == 0) {
        setRelay(1);
      } else {
        setRelay(0);
      }
    }
    
    if (buttonCnt >= 80) {
      Serial.printf("Long button press (Cnt: %d) Enter setup mode\n",buttonCnt);
      // Blink led to indicate mode change
      for (int t=0; t<20; t++) {
        digitalWrite(gpio13Led, LOW);
        delay(100);
        digitalWrite(gpio13Led, HIGH);
        delay(100);
      }
      
      Serial.printf("Resetting wifi, please cycle power!");
      wifiManager.resetSettings();
      delay(2000);
      ESP.reset();   // Note: This won't work until board has been power cycled once, after that, ok
      delay(5000);
    }
    buttonCnt = 0;
  }
} 

///////////// Functions /////////////

// Set a ISY994i state var
void setISYVar(int var, int val){

  String url = String(isy_url) + "/rest/vars/set/2/" + String(var) + "/" + String(val);
  //Serial.println("setISYVar: ");
  //Serial.println(url);
  
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) { 
    Serial.printf("Set ISY state var %d to %d OK\n",var,val);
  } else {
    Serial.printf("ERROR: setISYVar: httpCode: %d\n", httpCode);
  }
  http.end();   //Close connection  
}

// Set the relay and led
void setRelay(int state) {
  if (state == 1) {
    digitalWrite(gpio13Led, LOW);
    digitalWrite(gpio12Relay, HIGH);
    relayState = 1;
    Serial.println("On");

    if (IsyVarIndex != 0) {
      // Set the assoc var on the ISY
      setISYVar(IsyVarIndex,1);
    }
  } else {
    digitalWrite(gpio13Led, HIGH);
    digitalWrite(gpio12Relay, LOW);
    relayState = 0;
    Serial.println("Off");
    
    if (IsyVarIndex != 0) {
      // Clear the assoc var on the ISY
      setISYVar(IsyVarIndex,0);
    }
  } 
  delay(1000);
}

