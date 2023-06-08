#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "wificonfig.h"
#include <WiFi101.h>
#include "neopixel.h"

#define WINC_CS 8
#define WINC_IRQ 7
#define WINC_RST 4
#define WINC_EN 2

// set up the wifi
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
int status = WL_IDLE_STATUS;
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, HA_SERVER, HA_SERVERPORT, HA_USERNAME, HA_PASSWORD);
Adafruit_MQTT_Publish colorFeedPublish = Adafruit_MQTT_Publish(&mqtt, "box-light/rgb");
Adafruit_MQTT_Publish brightnessPublish = Adafruit_MQTT_Publish(&mqtt, "box-light/brightness");
Adafruit_MQTT_Publish switchPublish = Adafruit_MQTT_Publish(&mqtt, "box-light/status");
Adafruit_MQTT_Subscribe colorFeed = Adafruit_MQTT_Subscribe(&mqtt, "box-light/rgb/set");
Adafruit_MQTT_Subscribe brightnessFeed = Adafruit_MQTT_Subscribe(&mqtt, "box-light/brightness/set");
Adafruit_MQTT_Subscribe switchFeed = Adafruit_MQTT_Subscribe(&mqtt, "box-light/switch");
Adafruit_MQTT_Subscribe modeFeed = Adafruit_MQTT_Subscribe(&mqtt, "box-light/effect/set");

void mqttPublish(Adafruit_MQTT_Publish stream, char *value) {
    if (!stream.publish(value)) {
        Serial.println("Failed");
    } else {
        Serial.print("Published ");
        Serial.println(value);
    }
}

void MQTT_connect() {
    int8_t ret;

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        uint8_t timeout = 10;
        while (timeout && (WiFi.status() != WL_CONNECTED)) {
            timeout--;
            delay(1000);
        }
    }

    // Stop if already connected.
    if (mqtt.connected()) {
        return;
    }

    Serial.print("Connecting to MQTT... ");

    while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000); // wait 5 seconds
    }

    Serial.println("MQTT Connected!");
}

void connectionSetup() {
    WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("No WINC1500");
        while (true);
    }
    Serial.println("wifi ok");

    mqtt.subscribe(&colorFeed);
    mqtt.subscribe(&brightnessFeed);
    mqtt.subscribe(&modeFeed);
    mqtt.subscribe(&switchFeed);
    Serial.println("Connecting...");
    MQTT_connect();
}
