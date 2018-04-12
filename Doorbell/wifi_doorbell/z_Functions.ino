
// Activate a doorbell, play tune, toggle X10 on ISY
void activate(String door, int tune[][4], int len, char isy_x10[4], char isy_url[50]) {

  //Serial.printf("activate: %i, %s, %s, ", len, isy_x10, isy_url);
  //Serial.println(door);
  
  if (door == "front") {

    // Set the pseudo X10 ISY device
    if (strlen(isy_x10) != 0 and strlen(isy_url) != 0) {
      setISYx10(isy_x10, 1);
    }

    // Play the tune on the doorbell
    for (int row=0; row <= len - 1; row++) {
      chime(tune[row]);      
      Serial.println(".");     
    }

    // Clear the pseudo X10 ISY device
    if (strlen(isy_x10) != 0 and strlen(isy_url) != 0) {
      setISYx10(isy_x10, 0);
    }
  }

  if (door == "back") {

    // Set the pseudo X10 ISY device
    if (strlen(isy_x10) != 0 and strlen(isy_url) != 0) {
      setISYx10(isy_x10, 1);
    }

    // Play the tune on the doorbell
    for (int row=0; row <= len - 1; row++) {
      chime(tune[row]);      
      Serial.println(".");     
    }

    // Clear the pseudo X10 ISY device
    if (strlen(isy_x10) != 0 and strlen(isy_url) != 0) {
      setISYx10(isy_x10, 0);
    }
  }

 if (door == "") {
    // Play the tune on the doorbell
    for (int row=0; row <= len - 1; row++) {
      chime(tune[row]);      
      Serial.println(".");     
    }
 }
}

// Activate solenoids to strike chimes in the tune row
void chime(int row[]) {
  int wait_time;
  if (row[LEN] > 0) { 
    wait_time = row[LEN];
  } else {
    wait_time = TEMPO;
  }
  
  for (int note=LOW; note <= HI; note++) {

    if ((note == LOW) && (row[note] == 1)) {
      Serial.print("L");  
      digitalWrite(CHIME_LOW, HIGH);
    }

    if ((note == MED) && (row[note] == 1)) {
      Serial.print("M");  
      digitalWrite(CHIME_MED, HIGH);
    }

    if ((note == HI) && (row[note] == 1)) {
      Serial.print("H");  
      digitalWrite(CHIME_HI, HIGH);
    }
  }
  
  delay(HIT_DELAY);

  digitalWrite(CHIME_LOW, LOW);
  digitalWrite(CHIME_MED, LOW);
  digitalWrite(CHIME_HI, LOW);

  delay(wait_time);
}

// Callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Callback notifying us we are in config mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  digitalWrite(LED, HIGH);
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

// Set a ISY994i X10 device
void setISYx10(char* x10Device, int state){
  String url;

  if (state == 0) {
    // Set to off
    url = String(isy_url) + "/rest/X10/" + String(x10Device) + "/11";
  } else {
    // Set to on
    url = String(isy_url) + "/rest/X10/" + String(x10Device) + "/3";
  }

//  Serial.print("setISYx10 URL: ");
//  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) { 
    Serial.printf("setISYx10: Set ISY X10 device %s to %d OK\n",x10Device,state);
  } else {
    Serial.printf("ERROR: setISYx10: httpCode: %d\n", httpCode);
  }
  http.end();   //Close connection  
}

// Setup the ESP's eeprom file system to save config data
void eeprom_setup() {
  
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
        Serial.println();
        
        if (json.success()) {
          //Serial.println("\nparsed json");
          
          strcpy(isy_x10_dev,   json["isy_x10_dev"]);
          strcpy(isy_url,         json["isy_url"]);
          
        } else {
          Serial.println("ERROR: Failed to load json config");
        }
      }
    }
  } else {
    Serial.println("ERROR: Failed to mount FS");
  }
}

// Save config vars to the ESP's eeprom file system
void eeprom_save() {
  Serial.println("Saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["isy_x10_dev"]   = isy_x10_dev;
  json["isy_url"]       = isy_url;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("ERROR: Failed to open config file for writing");
  }

  Serial.println("Saved:");
  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}

