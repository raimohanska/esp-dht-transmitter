## esp-dht-transmitter

Software for sending temperature/humidity values over WiFi. Runs on 
ESP8266. Built using Arduino IDE. Supports multiple DHT-22 sensors connected to
different GPIO pins.

Requires [DHT-sensor-library](https://github.com/adafruit/DHT-sensor-library), [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
and of course the [ESP8266 Arduino IDE](https://github.com/esp8266/Arduino) extension.

Before opening this in Arduino IDE, create a file `sensor-settings.h` with 
your sensor settings, including Wifi settings. Like this:

```
const char* ssid     = "<my wifi ssid>";
const char* password = "<my wifi password>";

const char* host = "<server ip>";
const int   port = <server port>;

const char* device = "my-esp-transmitter";

#define SENSOR_COUNT 2
DHT dht[] = { DHT(2, DHT22), DHT(4, DHT22) };

#define SLEEP_SECS 1800 // 300=5min, 1800=30min
#define MAX_SEND_INTERVAL_SECS 7200 // Max send interval

#define TEMP_SEND_THRESHOLD 1.0
#define HUM_SEND_THRESHOLD 1.0
#define ENABLE_PIN 5 // Use pin x as GND for sensor. Sensor is only enabled when this pin is pulled low by the software.
```

I use my `sensor-server` project as the server that accepts the connections
from my sensors. You may experiment simply with `nc -k -l 5000` as well.

Data is sent to the server using HTTP POST, as a JSON array of measurements.
