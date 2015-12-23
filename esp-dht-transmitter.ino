#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include "sensor-settings.h"
#include "esp-dht-transmitter.h"
#define WIFI_RETRY_SECS 60
#define ONE_SECOND 1000000

int magic = 847236345;
int storageVersion = 2 + magic;
#define BUFFER_SIZE 200 * SENSOR_COUNT
StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

temp_hum measurements[SENSOR_COUNT];
app_state state;

WiFiClient client;

void setup() {  
  EEPROM.begin(128);
  for (int i=0; i < SENSOR_COUNT; i++) {
    dht[i].begin();  
  }
  Serial.begin(115200);
  Serial.println();
}



void loop() {
  state = readState();  
  int firstTime = (state.version != storageVersion);
  state.version = storageVersion;
  if (firstTime) {
    state.shouldSend = false;
    state.wifiFailures = 0;
    state.connectionFailures = 0;
    Serial.println("Storage version mismatch -> first time");
  } else if (state.shouldSend) {
    Serial.println("Sending previous values");
  } else {
    Serial.print("Statistics: wifiFailures=");
    Serial.print(state.wifiFailures);
    Serial.print(" connectionFailures=");
    Serial.println(state.connectionFailures);
  }

  if (state.shouldSend) {
    // Sending previous values
    JsonArray& output = jsonBuffer.createArray();
    for (int i = 0; i < SENSOR_COUNT; i++) {
      temp_hum prev = readFromStorage(i);
      if (isValid(prev)) {
        encodeJson(i, prev, output);        
      } else {
        Serial.print("Skipping sensor "); Serial.println(i);
        Serial.print("Skipped values "); printValues(prev);
      }
      prev.age++;
      writeToStorage(i, prev);
    }
    transmit(output);
    state.shouldSend = false;
    
  } else {
    Serial.println("Warming up...");
    delay(1000);
    // Make new measurements
    for (int i = 0; i < SENSOR_COUNT; i++) {
      Serial.print("Sensor "); Serial.println(i);
      temp_hum prev = readFromStorage(i);
      Serial.print("Previous "); printValues(prev);
      measurements[i] = read_values(i);
      temp_hum th = measurements[i];
      Serial.print("Measured "); printValues(th);
      if (firstTime || (isValid(th) && (!isValid(prev) || abs(th.temp - prev.temp) > TEMP_SEND_THRESHOLD || abs(th.hum - prev.hum) > HUM_SEND_THRESHOLD))) {
        Serial.println("Sendable");
        state.shouldSend = true;
      } else {
        prev.age++;
        writeToStorage(i, prev);
      }
    }   
  }

  if (state.shouldSend) {
    // found something to send -> store all valid new values for sending
    for (int i = 0; i < SENSOR_COUNT; i++) {
        temp_hum th = measurements[i];
        if (isValid(th) || firstTime) {
          Serial.print("Storing sensor "); Serial.println(i);
          writeToStorage(i, th);          
        }
    }
  }

  writeState(state);
  
  client.stop();
  if (state.shouldSend) {
    Serial.println("Short sleep");
    ESP.deepSleep(ONE_SECOND, WAKE_RF_DEFAULT);
  } else {
    Serial.println("Long sleep");
    ESP.deepSleep(SLEEP_SECS * ONE_SECOND, WAKE_RF_DISABLED);
  }    
  Serial.println("Delaying...");
  delay(SLEEP_SECS * 1000);
  Serial.println("...Done. Starting again");
}

int isValid(temp_hum values) {
  return !isnan(values.hum) && !isnan(values.temp) && values.age <= MAX_SKIP;
}

temp_hum read_values(int sensor_index) {
  temp_hum result;  
  result.age = 0;
  result.hum = readHumidity(sensor_index);
  result.temp = readTemperature(sensor_index);
  return result;
}

int encodeJson(int sensor_index, temp_hum th, JsonArray& output) {
  encodeJson(sensor_index, "temperature", th.temp, output);
  encodeJson(sensor_index, "humidity", th.hum, output);
}

int encodeJson(int sensor_index, char* tyep, float value, JsonArray& output) {
  JsonObject& root = output.createNestedObject();
  root["type"] = tyep;
  root["device"] = device;
  root["sensor"] = sensor_index;
  root["value"] = value;
}

int transmit(JsonArray& output) {
   if (!connectToWifi()) {
    Serial.print("Wifi connection failed. Trying to restore connectivity in "); Serial.print(WIFI_RETRY_SECS); Serial.println(" seconds");
    ESP.deepSleep(WIFI_RETRY_SECS * ONE_SECOND, WAKE_RF_DEFAULT);
    state.wifiFailures++;
    delay(60000);
  }
  if (connectToHost()) {
    int len = output.measureLength();
    client.println("POST /event HTTP/1.0");
    client.println("Content-Type: application/json");
    
    client.print("Content-Length: "); client.println(len);
    
    client.println();
    output.printTo(client);
    output.printTo(Serial);
    
    client.flush();
    Serial.print("Values sent in "); Serial.print(len); Serial.println(" bytes");
    return true;
  } else {
    state.connectionFailures++;
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
  Serial.print("C, age: ");
  Serial.println(values.age);
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

int memoryPos(int index) {
  temp_hum value;
  app_state state;
  return sizeof(state) + index * sizeof(value);
}

void writeToStorage(int index, temp_hum value)
{
   int ee = memoryPos(index);
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
   EEPROM.commit();
}

temp_hum readFromStorage(int index)
{
   temp_hum value;
   int ee = memoryPos(index);
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return value;
}

void writeState(app_state value)
{
   int ee = 0;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
   EEPROM.commit();
}

app_state readState()
{
   int ee = 0;
   app_state value;
   byte* p = (byte*)(void*)&value;
   for (int i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return value;
}

int connectToWifi() {  
  int timeoutMs = 20000;
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(ssid, password);

    int elapsed = 0;
    int d = 100;
    while (WiFi.status() != WL_CONNECTED) {
      elapsed += d;
      if (elapsed >= timeoutMs) {
        Serial.println();
        return false;
      }
      delay(d);
      Serial.print(".");
    }
  
    Serial.println();
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());      
    return true;
  }
}
