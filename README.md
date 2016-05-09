## esp-dht-transmitter

Software for sending temperature/humidity values over WiFi. Runs on 
ESP8266. Built using Arduino IDE. Supports multiple DHT-22 sensors connected to
different GPIO pins.

Requires [DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library), [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
and of course the [ESP8266 Arduino IDE](https://github.com/esp8266/Arduino) extension.

Before opening this in Arduino IDE, create a file `settings.h` with 
your sensor, WiFi and server settings. Like this:

```
#define SENSOR_COUNT 2
DHT dht[] = { DHT(2, DHT22), DHT(4, DHT22) };

#define SLEEP_SECS 300 // 300=5min, 1800=30min
#define MAX_SEND_INTERVAL_SECS 7200 // Max send interval

#define TEMP_SEND_THRESHOLD 1.0
#define HUM_SEND_THRESHOLD 1.0

#define ENABLE_PIN 5 // Use pin _ as enabling pin for sensor
#define ENABLE_LEVEL LOW // Set pin _ to ___ when enabled
```

I use my `sensor-server` project as the server that accepts the connections
from my sensors. You may experiment simply with `nc -k -l 5000` as well.

Data is sent to the server using HTTP POST, as a JSON array of measurements.

When the device starts for the first time, it creates a Wifi access point "RAIMO SENSOR SETUP".
Joining this network should give you a pop-up web page that allows you to set your
Wifi credentials, server address and device name.
