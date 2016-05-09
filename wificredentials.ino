/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  EEPROM.get(0, ssid);
  EEPROM.get(0+sizeof(ssid), password);
  EEPROM.get(0+sizeof(ssid)+sizeof(password), host);
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(host), device_name);
  char ok[2+1];
  EEPROM.get(0+sizeof(ssid)+sizeof(password)+sizeof(host)+sizeof(device_name), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    resetCredentials();
  }
  Serial.println("Loaded configuration from flash memory:");
  Serial.println(String("Wifi SSID: ") + ssid);
  Serial.println(String("Device name: ") + device_name);
  Serial.println(String("Server address: ") + host);
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {
  EEPROM.begin(512);
  EEPROM.put(0, ssid);
  EEPROM.put(0+sizeof(ssid), password);
  EEPROM.put(0+sizeof(ssid)+sizeof(password), host);
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(host), device_name);
  char ok[2+1] = "OK";
  EEPROM.put(0+sizeof(ssid)+sizeof(password)+sizeof(host)+sizeof(device_name), ok);
  EEPROM.commit();
  EEPROM.end();
}

int hasWifiCredentials() {
  return strlen(ssid) > 0;  
}

void resetCredentials() {
  ssid[0] = 0;
  password[0] = 0;
  host[0] = 0;
  device_name[0] = 0;
}

