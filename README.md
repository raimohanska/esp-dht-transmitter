## esp-dht-transmitter

Software for sending temperature/humidity values over WiFi. Runs on 
ESP8266. Built using Arduino IDE. Expects DHT-22 humidity sensor to be
connected on GPIO2.

Before opening this in Arduino IDE, create a file `sensor-settings.h` with 
your sensor settings, including Wifi settings. Like this:

```
const char* ssid     = "<my wifi ssid>";
const char* password = "<my wifi password>";

const char* host = "<server ip>";
const int   port = <server port>;

const char* location = "livingroom";

#define DEEP_SLEEP 0   // set to 1 to enable deep sleep
#define SLEEP_SECS 60  // interval between sending
```

I use my `sensor-server` project as the server that accepts the connections
from my sensors. You may experiment simply with `nc -k -l 5000` as well.
