#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include "sensor-settings.h"
#include "esp-dht-transmitter.h"

WiFiClient client;


void setup() {  
  EEPROM.begin(128);
  for (int i=0; i < sensor_count; i++) {
    dht[i].begin();  
  }
  Serial.begin(115200);
  Serial.println();
}

void connectToWifi() {  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
  
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());      
  }
}

void loop() {
  delay(1000);
  int anythingToSend = false;
  for (int i = 0; i < sensor_count; i++) {
    temp_hum prev = readFromStorage(i);
    Serial.print("Previous "); printValues(prev);
    if (prev.send && isValid(prev)) {
      int success = transmit(i, prev);     
      prev.send = false;
      writeToStorage(i, prev); // reset send flag
    } else {
      temp_hum th = read_values(i);
      Serial.print("Measured "); printValues(th);
      if (!isValid(prev) || abs(th.temp - prev.temp) > TEMP_SEND_THRESHOLD || abs(th.hum - prev.hum) > HUM_SEND_THRESHOLD) {
        Serial.println("Marking for send");
        th.send = true;
        anythingToSend = true;
        writeToStorage(i, th);
      }
    }
  }
  
  client.stop();
  if (DEEP_SLEEP) {
    if (anythingToSend) {
      Serial.println("Short sleep");
      ESP.deepSleep(1000000, WAKE_RF_DEFAULT);
    } else {
      Serial.println("Long sleep");
      ESP.deepSleep(SLEEP_SECS * 1000000, WAKE_RF_DISABLED);
    }    
  }
  Serial.println("Delaying...");
  delay(SLEEP_SECS * 1000);
  Serial.println("...Done. Starting again");
}

int isValid(temp_hum values) {
  return !isnan(values.hum) && !isnan(values.temp);
}

temp_hum read_values(int sensor_index) {
  temp_hum result;  
  result.send = false;
  result.hum = readHumidity(sensor_index);
  result.temp = readTemperature(sensor_index);
  return result;
}

void transmit(int sensor_index, temp_hum th) {
  connectToWifi();
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
    return true;
  }
  return false;
}

float readHumidity(int sensor_index) {
  return dht[sensor_index].readHumidity();  
}

float printValues(temp_hum values) {
  Serial.print("Humidity: ");
  Serial.print(values.hum);
  Serial.print("% ,Temperature: ");
  Serial.print(values.temp);
  Serial.println("C");
}

float readTemperature(int sensor_index) {
  return dht[sensor_index].readTemperature(0);
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

void writeToStorage(int index, temp_hum value)
{
   int ee = index * sizeof(value);
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
   EEPROM.commit();
}

temp_hum readFromStorage(int index)
{
   temp_hum value;
   int ee = index * sizeof(value);
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return value;
}
