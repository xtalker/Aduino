  // Setup the ESP's file system to save config data
  eeprom_setup();

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_isy_x10_dev("x10Dev", "ISY X10 Device", isy_x10_dev, 4);
  WiFiManagerParameter custom_isy_url("isyUrl", "ISY URL", isy_url, 50);

  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);

  // Add custom setup parameters here
  wifiManager.addParameter(&custom_isy_x10_dev);
  wifiManager.addParameter(&custom_isy_url);

