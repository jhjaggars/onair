#include "Arduino.h"
#include "secret.h"
#include "pulser.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define MQTT_SERVER "192.168.4.53"
#define MQTT_SERVERPORT 1883 // use 8883 for SSL
#define LED D5
#define VERSION "0.2.1"

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(128, 32, &Wire, -1);
Pulser pulser(LED);

#define _print(x)      \
  ({                   \
    Serial.print(x);   \
    display.print(x);  \
    display.display(); \
  })

#define _println(x)     \
  ({                    \
    Serial.println(x);  \
    display.println(x); \
    display.display();  \
  })

void setup_wifi();
void sub_handler(char *topic, byte *payload, unsigned int length);
void reconnect();

void setup_wifi()
{
  delay(10);

  _print(F("Connecting to "));
  _println(WLAN_SSID);

  WiFi.mode(WIFI_STA);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    _print(F("."));
  }
  _println();

  _println(F("WiFi Connected"));
  _print(F("IP: "));
  _println(WiFi.localIP());
}

void sub_handler(char *topic, byte *payload, unsigned int length)
{
  display.clearDisplay();

  display.setCursor(0, 0);
  payload[length] = '\0';

  pulser.on = payload[0] == '*' ? true : false;

  _println((char *)payload);
}

void setup()
{

  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed!"));
    for (;;)
      ;
  }

  pinMode(LED, OUTPUT);

  // display.display();
  // delay(2000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  setup_wifi();

  _print("Current Version: ");
  _println(VERSION);

  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  t_httpUpdate_return ret = ESPhttpUpdate.update(espClient, "http://192.168.4.53:8080/", VERSION);
  Serial.println(ret);

  client.setServer(MQTT_SERVER, MQTT_SERVERPORT);
  client.setCallback(sub_handler);
}

void reconnect()
{
  String clientId = "ESP8266Client-";
  clientId += String(random(0xffff), HEX);

  while (!client.connected())
  {
    _println(F("Attempting MQTT connection..."));
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_KEY))
    {
      _println("Connected!");
      client.subscribe("/test/in");
    }
    else
    {
      Serial.println("failed");
      Serial.println(client.state());
      _println(F(" try again in 5 seconds"));
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }

  client.loop();
  pulser.pulse();
}