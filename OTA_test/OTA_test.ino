#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>


#define NODE_LED    2   // D4 (active LOW)  
#define LED         4   // D2

const long node_led_interval = 50;
unsigned long node_led_prev_millis = 0;  // Last time NODE_LED was updated
String webPage = "";
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  WiFiManager wifiManager;
  wifiManager.autoConnect("esp8266-setup");

  pinMode(NODE_LED,  OUTPUT);
  pinMode(LED,       OUTPUT);
  digitalWrite(LED,       LOW);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Wifi-Doorbell");

  ArduinoOTA.onStart([]() {
    Serial.println("Start OTA");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("End OTA");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\n", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  webPage += "<h1 style=\"text-align:center;font-size:500%;\">Love Shack Doorbell</h1>";
  webPage += "<a href=\"on\"><button style=\"height:200px;width:250px;font-size:600%;\">On</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";
  webPage += "<a href=\"off\"><button style=\"height:200px;width:250px;font-size:600%;\">Off</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";

  // HTTP Server commands
  server.on("/", [](){
    server.send(200, "text/html", webPage);
    //Serial.println("Server: root!");
  });
 server.on("/on", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Web triggered: On!");     
    digitalWrite(LED,       HIGH);
  });
  server.on("/off", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Web triggered: Off!");     
    digitalWrite(LED,       LOW);
  });

  server.begin();
  Serial.println("Doorbell HTTP server started");  

}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  unsigned long currentMillis = millis();

  // Blink the node led to indicate this loop is running
  if (currentMillis - node_led_prev_millis >= node_led_interval) {
    node_led_prev_millis = currentMillis;
    digitalWrite(NODE_LED, !digitalRead(NODE_LED));
  }
}
