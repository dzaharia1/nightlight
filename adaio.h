#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "wificonfig.h"
#include <WiFi101.h>
#include "neopixel.h"

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define WINC_CS 8
#define WINC_IRQ 7
#define WINC_RST 4
#define WINC_EN 2

// operating modes
#define MODE_NORMAL 0
#define MODE_NIGHTLIGHT 1
#define MODE_CHILL 2
#define MODE_PARTY 3

// set up the wifi
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
int status = WL_IDLE_STATUS;
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish motionSensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/motionsensor");
Adafruit_MQTT_Publish photocellStream = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Publish colorFeedPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/color-setting");
Adafruit_MQTT_Publish brightnessPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/brightness");
Adafruit_MQTT_Subscribe colorFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/color-setting");
Adafruit_MQTT_Subscribe brightnessFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/brightness");
Adafruit_MQTT_Subscribe colorTrigger = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/color-trigger");
Adafruit_MQTT_Subscribe modeFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/night-mode");

void connectionSetup() {
    WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
    if (WiFi.status() == WL_NO_SHIELD)
    {
        Serial.println("No WINC1500");
        while (true)
            ;
    }
    Serial.println("wifi ok");

    mqtt.subscribe(&colorFeed);
    mqtt.subscribe(&brightnessFeed);
    mqtt.subscribe(&colorTrigger);
    mqtt.subscribe(&modeFeed);
}

void mqttPublish(Adafruit_MQTT_Publish stream, char *value)
{
    if (!stream.publish(value))
    {
        Serial.println("Failed");
    }
    else
    {
        Serial.print("Published ");
        Serial.print(value);
        Serial.print(" to ");
        Serial.println(stream.getTopic());
    }
}

void checkMode(int timeout)
{
    Adafruit_MQTT_Subscribe *subscription;
    while (subscription = mqtt.readSubscription(timeout))
    {
        if (subscription == &colorFeed)
        {
            if (mode != MODE_NIGHTLIGHT)
            {
                mode = MODE_NORMAL;
            }
            setLedColor((char *)colorFeed.lastread);
        }
        if (subscription == &brightnessFeed)
        {
            if (mode == MODE_NIGHTLIGHT)
            {
                nightBrightness = map(atoi((char *)brightnessFeed.lastread), 0, 100, 0, 255);
            }
            else if (mode == MODE_CHILL || mode == MODE_PARTY)
            {
                currBrightness = map(atoi((char *)brightnessFeed.lastread), 0, 100, 0, 255);
            }
            else
            {
                setLedBrightness((char *)brightnessFeed.lastread);
            }
        }
        if (subscription == &colorTrigger && parseColor((char *)colorTrigger.lastread) > 0)
        {
            mqttPublish(colorFeedPublish, parseColor((char *)colorTrigger.lastread));
        }
        if (subscription == &modeFeed)
        {
            mode = atoi((char *)modeFeed.lastread);
            if (mode == MODE_NORMAL)
            {
                setLedColor(currColor);
            }
            Serial.println(mode);
        }
    }
}

void MQTT_connect()
{
    int8_t ret;

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        uint8_t timeout = 10;
        while (timeout && (WiFi.status() != WL_CONNECTED))
        {
            timeout--;
            delay(1000);
        }
    }

    // Stop if already connected.
    if (mqtt.connected())
    {
        return;
    }

    Serial.print("Connecting to MQTT... ");

    while ((ret = mqtt.connect()) != 0)
    { // connect will return 0 for connected
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 5 seconds...");
        mqtt.disconnect();
        delay(5000); // wait 5 seconds
    }

    Serial.println("MQTT Connected!");
}
