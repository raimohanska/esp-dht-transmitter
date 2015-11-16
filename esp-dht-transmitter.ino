#include <ESP8266WiFi.h>
#include "DHT.h"
#include "sensor-settings.h"
#include "esp-dht-transmitter.h"

WiFiClient client;

void setup() {  
  for (int i=0; i < sensor_count; i++) {
    dht[i].begin();  
  }
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

  for (int i = 0; i < sensor_count; i++) {
    temp_hum th = read_values(i);
    transmit(i, th);
  }
  client.stop();

  if (DEEP_SLEEP) {
    Serial.println("Feeling sleepy");
    ESP.deepSleep(SLEEP_SECS * 1000000, WAKE_RF_DEFAULT);
  }
  Serial.println("Delaying...");
  delay(SLEEP_SECS * 1000);
  Serial.println("...Done. Starting again");
}

temp_hum read_values(int sensor_index) {
  temp_hum result;  
  result.hum = readHumidity(sensor_index);
  result.temp = readTemperature(sensor_index);
  return result;
}

void transmit(int sensor_index, temp_hum th) {
  if (connectToHost()) {
    client.print("{\"humidity\": "); client.print(th.hum);
    client.print(", \"temperature\": "); client.print(th.temp);
    client.print(", \"sensor\": \""); client.print(sensor_index); client.print("\"");
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
    Serial.print("Temperature and humidity from sensor "); Serial.print(sensor_index); Serial.println(" sent");    
  }
}

float readHumidity(int sensor_index) {
  float h = dht[sensor_index].readHumidity();
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  return h;
}

float readTemperature(int sensor_index) {
  float h = dht[sensor_index].readTemperature(0);
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
