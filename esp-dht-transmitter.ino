#include <ESP8266WiFi.h>
#include "DHT.h"
#include "sensor-settings.h"

// DHT-22 temp/humidity sensor on pin 2
DHT dht(2, DHT22);

const char* device   = "esp-dht";

WiFiClient client;

void setup() {
  //dht.begin();
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
}

void loop() {
  measureAndTransmit();
  if (DEEP_SLEEP) {
    Serial.println("Feeling sleepy");
    ESP.deepSleep(SLEEP_SECS * 1000000, WAKE_RF_DEFAULT);
  }
  delay(SLEEP_SECS * 1000);
}

void measureAndTransmit() {
  float h = readHumidity();
  float t = readTemperature();

  connectToHost();

  client.print("{\"humidity\": "); client.print(h);
  client.print(", \"temperature\": "); client.print(t);
  client.print(", \"location\": \""); client.print(location); client.print("\"");
  client.print(", \"device\": \""); client.print(device); client.print("\"");
  client.println("}");
  client.flush();
  client.stop();
  Serial.println("Temperature and humidity sent");
}

float readHumidity() {
  float h = dht.readHumidity();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  return h;
}

float readTemperature() {
  float h = dht.readTemperature(0);
  Serial.print("Temperature: ");
  Serial.print(h);
  Serial.println("C");
  return h;
}


void connectToHost() {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }

  Serial.println("Connected");
}
