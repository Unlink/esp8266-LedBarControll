/*
 * Wifi Eventy
 */
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo) {
  Serial.printf(("Got IP: %s\r\n"), ipInfo.ip.toString().c_str());
  wifiState = WIFI_CONNECTED;
}

void onSTADisconnected(WiFiEventStationModeDisconnected event_info) {
  Serial.printf(("Disconnected from SSID: %s\n"), event_info.ssid.c_str());
  Serial.printf(("Reason: %d\n"), event_info.reason);
  wifiState = WIFI_DISCONNECTED;

  
}

/**
 * Esp Setup
 */
void setup() {
  pinMode(BOOTING_PIN, OUTPUT);
  digitalWrite(BOOTING_PIN, HIGH);
  
  Serial.begin(115200);
  SPIFFS.begin();
  loadConfig();

  analogWriteRange(255);
  analogWriteFreq(250);

  pinMode(WIFI_INFO_PIN, OUTPUT);
  digitalWrite(WIFI_INFO_PIN, HIGH);

  pinMode(RGB_PIN_R, OUTPUT);
  digitalWrite(RGB_PIN_R, LOW);
  pinMode(RGB_PIN_G, OUTPUT);
  digitalWrite(RGB_PIN_G, LOW);
  pinMode(RGB_PIN_B, OUTPUT);
  digitalWrite(RGB_PIN_B, LOW);
  pinMode(WHITE_LED_PIN, OUTPUT);
  digitalWrite(WHITE_LED_PIN, LOW);

  pinMode(ONE_WIRE_POWER, OUTPUT);
  digitalWrite(ONE_WIRE_POWER, HIGH);
  
  static WiFiEventHandler e1, e2;

  WiFi.hostname(hostName);
  WiFi.mode(WIFI_STA);
  e1 = WiFi.onStationModeGotIP(onSTAGotIP);
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  setupServer();
  server.begin();

  MDNS.begin(hostName);
  MDNS.addService("http", "tcp", 80);

  mqttClient.setServer(mqttServer, 1883);
  
  digitalWrite(BOOTING_PIN, LOW);
}

void loop() {
  if(shouldReboot){
    Serial.println(("Rebooting..."));
    delay(100);
    ESP.restart();
  }
  
  static unsigned long previousMillis = 0;
  static unsigned long previousMillisTemp = 0;
  unsigned long currentMillis = millis();
  int ledOnTime = 500;
  int ledOfTime = 500;
  /*if (wifiState == WIFI_CONNECTED) {
    ledOnTime = 5;
    ledOfTime = 1000;
  }*/
  if (wifiState != WIFI_CONNECTED) {
    if (digitalRead(WIFI_INFO_PIN) == LOW && currentMillis - previousMillis > ledOnTime) {
      digitalWrite(WIFI_INFO_PIN, HIGH);
      previousMillis = currentMillis;
    }
    else if (digitalRead(WIFI_INFO_PIN) == HIGH && currentMillis - previousMillis > ledOfTime) {
      digitalWrite(WIFI_INFO_PIN, LOW);
      previousMillis = currentMillis;
    }
  }

  if (currentMillis - previousMillisTemp > temperatureInterval*1000) {
    if (mqttReconnect()) {
      previousMillisTemp = currentMillis;
      static char msg[100];
      float temp;
      do {
        DS18B20.requestTemperatures(); 
        temp = DS18B20.getTempCByIndex(0);
        yield();
      } while (temp == 85.0 || temp == (-127.0));
      sprintf(msg, "{\"client\": \"%s\", \"sensor\": \"tempLivingroom\", \"value\": %d.%d}", hostName, (int)temp, (int) ((temp - (int)temp)*100));
      mqttClient.publish("temp/publish", msg); 
    }
  }
  
  rgbAnimate();
}
