#include <ESP8266WiFi.h>
#include "DHT.h"
#include <TM1637Display.h>
#include "sensor-settings.h"

// DHT-22 temp/humidity sensor on pin 2
DHT dht(2, DHT22);

#define dispClockPin 4
#define dispDioPin 5
TM1637Display dd(dispClockPin, dispDioPin);

float h;
float t;

WiFiClient client;

void setup() {
  read();  
  
  dd.setBrightness(0);
  dd.setColon(false);
  dd.showNumberDec(0, false, 4, 0);

  dht.begin();
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
  delay(2000);

  read();  
  transmit(h, t);
  if (DEEP_SLEEP) {
    Serial.println("Feeling sleepy");
    ESP.deepSleep(SLEEP_SECS * 1000000, WAKE_RF_DEFAULT);
  }
  Serial.println("Delaying...");
  delay(SLEEP_SECS * 1000);
  Serial.println("...Done. Starting again");
  dd.setBrightness(0);
}

void read() {
  h = readHumidity();
  t = readTemperature();

  dd.setBrightness(8);
  dd.showNumberDec(t, false, 4, 0);
}

void transmit(float h, float t) {
  
  if (connectToHost()) {
    
    client.print("{\"humidity\": "); client.print(h);
    client.print(", \"temperature\": "); client.print(t);
    client.print(", \"device\": \""); client.print(device); client.print("\"");
    client.println("}");
    
    /*
    HTTP POST doesn't seem to work right now. Server won't parse the body
    client.println("POST /event HTTP/1.0");
    client.println("Content-Type: application/json");
    client.println();
    client.print("{\"type\": \"temperature\"");
    client.print(",\"location\": \""); client.print(location); client.print("\"");
    client.print(",\"value\": "); client.print(t); client.print("}");
    client.println();
    */
    client.flush();
    client.stop();
    Serial.println("Temperature and humidity sent");    
  }
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


int connectToHost() {
  Serial.print("connecting to ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return false;
  }

  Serial.println("Connected");
  return true;
}
