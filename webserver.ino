const char* text_plain = "text/plain";
const char* text_html = "text/html";
const char* text_json = "text/json";

/*
 * Webserver metody
 */

void setupServer() {
  server.addHandler(new SPIFFSEditor());

  server.on("/setWhite", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!request->hasParam("value", 1)) {
      request->send(400, text_html, ("<h4>Bad request</h4><p>Value of<code>value</code> is missing</p>"));
      return;
    }
    int value = request->getParam("value", 1)->value().toInt();
    analogWrite(WHITE_LED_PIN, value);
    CURR_W = value;
  });

  server.on("/ledSetColor", HTTP_POST, [](AsyncWebServerRequest *request){
    if (!request->hasParam("red", 1)) {
      request->send(400, text_html, ("<h4>Bad request</h4><p>Value of<code>red</code> is missing</p>"));
      return;
    }
    if (!request->hasParam("green", 1)) {
      request->send(400, text_html, ("<h4>Bad request</h4><p>Value of<code>green</code> is missing</p>"));
      return;
    }
    if (!request->hasParam("blue", 1)) {
      request->send(400, text_html, ("<h4>Bad request</h4><p>Value of<code>blue</code> is missing</p>"));
      return;
    }
    int r = request->getParam("red", 1)->value().toInt();
    int g = request->getParam("green", 1)->value().toInt();
    int b = request->getParam("blue", 1)->value().toInt();
    rgbClearAnimation();
    setRGB(r,g,b);
    CURR_R = r;
    CURR_B = b;
    CURR_G = g;
    request->send(200, text_plain, "OK");
  });

  server.on("/ledSetAnimation2", HTTP_POST, [](AsyncWebServerRequest *request){
    int n = request->params();
    if (n % 3 != 0) {
      request->send(400, text_html, ("<h4>Bad request</h4><p>Wrong numbers of parameters</p>"));
      return;
    }
    int j = 0;
    rgbCreateAnimationBuffer(n/3);
    for (uint8_t i=0; i<request->params(); i+=3){
      const char *hex = request->getParam(i)->value().c_str();
      uint32_t number = (uint32_t)strtol(hex, NULL, 16);
      rgbAnimationSetValue(j, (number>>16)&0xFF, (number>>8)&0xFF, number&0xFF,
        request->getParam(i+1)->value().toInt(),
        request->getParam(i+2)->value().toInt());
      j++;
    }
    request->send(200, text_plain, "OK");
  });

  server.on("/getLedValues", HTTP_GET, [](AsyncWebServerRequest *request){
    String message = "{";
    message+= "\"r\": "+String(CURR_R)+",";
    message+= "\"g\": "+String(CURR_G)+",";
    message+= "\"b\": "+String(CURR_B)+",";
    message+= "\"w\": "+String(CURR_W);
    message+= "}"; 
    request->send(200, text_json, message);
  });

  server.on("/getLedAnimationValues", HTTP_GET, [](AsyncWebServerRequest *request){
    String message = "[";
    if (animacia != NULL) {
      for (int i=0; i<rgbAnimationMaxItems; i++) {
        message+= "{";
        message+= "\"r\": "+String(animacia[i].red)+",";
        message+= "\"g\": "+String(animacia[i].green)+",";
        message+= "\"b\": "+String(animacia[i].blue)+",";
        message+= "\"trvanie\": "+String(animacia[i].trvanie)+",";
        message+= "\"prechod\": "+String(animacia[i].prechod);
        message+= "}";
        if ((i+1) < rgbAnimationMaxItems) {
          message+= ",";
        }
      }
    }
    message+= "]";
    request->send(200, text_json, message);
  });

  server.on("/getPower", HTTP_GET, [](AsyncWebServerRequest *request){
    float vdd = ESP.getVcc() / 1000.0;
    char tempBuff[10];
    dtostrf(vdd, 5, 2, tempBuff);
    request->send(200, text_plain, tempBuff);
  });

  server.on("/getTemp", HTTP_GET, [](AsyncWebServerRequest *request){
    float temp;
    do {
      DS18B20.requestTemperatures(); 
      temp = DS18B20.getTempCByIndex(0);
    } while (temp == 85.0 || temp == (-127.0));
    char tempBuff[10];
    dtostrf(temp, 5, 2, tempBuff);
    request->send(200, text_plain, tempBuff);
  });

  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    shouldReboot = true;
    request->send(200, text_plain, "OK");
  });

  // Simple Firmware Update Form
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, text_html, ("<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>"));
  });
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, text_plain, shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.onNotFound([](AsyncWebServerRequest *request){
    String message = ("File Not Found\n\n");
    message += "URI..........: ";
    message += request->url().c_str();
    message += "\nMethod.....: ";
    if(request->method() == HTTP_GET)
      message +=("GET");
    else if(request->method() == HTTP_POST)
      message += ("POST");
    else if(request->method() == HTTP_DELETE)
      message += ("DELETE");
    else if(request->method() == HTTP_PUT)
      message += ("PUT");
    else if(request->method() == HTTP_PATCH)
      message += ("PATCH");
    else if(request->method() == HTTP_HEAD)
      message += ("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      message += ("OPTIONS");
    else
      message += ("UNKNOWN");
      
    message += "\nArguments..: ";
    message += request->params();
    message += _NEW_LINE;
    for (uint8_t i=0; i<request->params(); i++){
      AsyncWebParameter* p = request->getParam(i);
      message += " " + String(p->name().c_str()) + ": " + String(p->value().c_str()) + _NEW_LINE;
    }
    message += _NEW_LINE;
    message += ("FreeHeap.....: ") + String(ESP.getFreeHeap()) + _NEW_LINE;
    message += ("ChipID.......: ") + String(ESP.getChipId()) + _NEW_LINE;
    message += ("FlashChipId..: ") + String(ESP.getFlashChipId()) + _NEW_LINE;
    message += ("FlashChipSize: ") + String(ESP.getFlashChipSize()) + (" bytes\n");
    message += ("getCycleCount: ") + String(ESP.getCycleCount()) + (" Cycles\n");
    message += ("Milliseconds.: ") + String(millis()) + (" Milliseconds\n");

    request->send(404, text_plain, message);
  });
}

