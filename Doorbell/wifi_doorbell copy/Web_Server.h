
  // Setup web page and server functions
  webPage += "<h1 style=\"text-align:center;font-size:500%;\">Love Shack Doorbell</h1>";
  webPage += "<p align=\"center\">";
  webPage += "<a href=\"front\"><button style=\"height:200px;width:250px;font-size:600%;\">Front</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";
  webPage += "<a href=\"back\"><button style=\"height:200px;width:250px;font-size:600%\">Back</button></a>";
  webPage += "</p><br>";
  webPage += "<p align=\"center\">";
  webPage += "<a href=\"low\"><button style=\"height:200px;width:200px;font-size:500%\">Low</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";
  webPage += "<a href=\"med\"><button style=\"height:200px;width:200px;font-size:500%\">Med</button></a>";
  webPage += "&nbsp;&nbsp;&nbsp;&nbsp;";
  webPage += "<a href=\"hi\"><button style=\"height:200px;width:200px;font-size:500%\">Hi </button></a>";
  webPage += "</p><br><br>";
  webPage += "<h2 style=\"text-align:center;font-size:300%;\">Settings</h2>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">ISY X10 Device:    " + String(isy_x10_dev) + "</h3>";
  webPage += "<h3 style=\"text-align:left;font-size:100%;\">ISY URL: " + String(isy_url) + "</h3>";
  webPage += "<p style=\"padding-bottom:3cm;\"></p>";
  webPage += "<h4>Compiled: " + String(compile_info) + "</h4>";

  // HTTP Server commands
  server.on("/", [](){
    server.send(200, "text/html", webPage);
    //Serial.println("Server: root!");
  });

  server.on("/front", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Web triggered: Front Door!");     
    int len = sizeof (tune_1) / sizeof (tune_1[0]);
    activate("front", tune_1, len, isy_x10_dev, isy_url);

  });

  server.on("/back", [](){
    server.send(200, "text/html", webPage);
    Serial.println("Web triggered: Back Door!");     
    int len = sizeof (tune_1) / sizeof (tune_1[0]);
    activate("back", tune_1, len, isy_x10_dev, isy_url);
  });

  server.on("/low", [](){
    server.send(200, "text/html", webPage);
    int note[] {1,0,0,0};
    chime( note );
  });

  server.on("/med", [](){
    server.send(200, "text/html", webPage);
    int note[] {0,1,0,0};
    chime( note );
  });

  server.on("/hi", [](){
    server.send(200, "text/html", webPage);
    int note[] {0,0,1,0};
    chime( note );
  });

