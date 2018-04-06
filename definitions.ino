#include <stdlib.h>
#include <math.h> 

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>

#include <ArduinoJson.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <PubSubClient.h>

/*
 * Parametre
 *
 */
#define ONE_WIRE_BUS 5
#define ONE_WIRE_POWER 4
 
#define WIFI_INFO_PIN 2

#define BOOTING_PIN 15

#define RGB_PIN_R 16
#define RGB_PIN_G 14
#define RGB_PIN_B 12

#define WHITE_LED_PIN 13

/*
 * Global strings
 */
 const char* _NEW_LINE = "\n";

/*
 * Služby a globálne premenne
 */

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

enum ESPWifiState { WIFI_CONNECTED, WIFI_DISCONNECTED };
typedef enum ESPWifiState ESPWifiState_t;

ESPWifiState_t wifiState = WIFI_DISCONNECTED;

AsyncWebServer server(80);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char ssid[32] = "";
char password[32] = "";

const char * hostName = "esp-osvetlenie";
const char* mqttServer = "YOUR_MQTT_BROKER";

//flag to use from web update to reboot the ESP
bool shouldReboot = false;

int CURR_R = 0;
int CURR_G = 0;
int CURR_B = 0;
int CURR_W = 0;

int temperatureInterval = 60;

ADC_MODE(ADC_VCC);
