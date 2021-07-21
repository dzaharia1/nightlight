#include "wificonfig.h"
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi101.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define PHOTOCELL       A5
#define neoPixel        11
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define WINC_CS         8
#define WINC_IRQ        7
#define WINC_RST        4
#define WINC_EN         2

#define RED             "#ff0000"
#define GREEN           "#00ff00"
#define TEAL            "#1aff61"
#define BLUE            "#0000ff"
#define PURPLE          "#9600ff"
#define MAGENTA         "#ff00e8"
#define WHITE           "#ffffff"
#define COOLWHITE       "#ffffff"
#define WARMWHITE       "#ffb257"
#define NUMPIXELS       45

// set up the neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, neoPixel, NEO_GRBW + NEO_KHZ800);

// set up the wifi
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
int status = WL_IDLE_STATUS;
WiFiClient client;

// set up the display
#define DISPLAY_CS 9
#define DISPLAY_DC 10
Adafruit_ILI9341 tft = Adafruit_ILI9341(DISPLAY_CS, DISPLAY_DC);

// set up the MQTT connection
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish motionSensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/motionsensor");
Adafruit_MQTT_Publish photocellStream = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Publish colorSettingPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/color-setting");
Adafruit_MQTT_Subscribe colorSetting = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/color-setting");
Adafruit_MQTT_Subscribe brightnessSetting = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/brightness");
Adafruit_MQTT_Subscribe colorTrigger = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/color-trigger");

int currBrightness = 10;
char startingColor[] = "ffffff";
int currColor;

struct color {
  char * colorString;
  int red, green, blue;
};

void setup()
{
  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);

//  while (!Serial);
  Serial.begin(115200);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("No WINC1500");
    while (true);
  }
  Serial.println("wifi ok");
  mqtt.subscribe(&colorSetting);
  mqtt.subscribe(&brightnessSetting);
  mqtt.subscribe(&colorTrigger);

  pixels.begin();
  pixels.setBrightness(currBrightness);
  currColor = StrToHex(startingColor);
  for (int i = 0; i < NUMPIXELS; i ++) {
    // pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    pixels.setPixelColor(i, currColor);
  }
  pixels.show();

  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
}

uint32_t x = 0;

void loop() {
  MQTT_connect();

  Adafruit_MQTT_Subscribe *subscription;
  while((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &colorSetting) {
      setLedColor((char *)colorSetting.lastread);
    }
    if (subscription == &brightnessSetting) {
      setLedBrightness((char *)brightnessSetting.lastread);
    }
    if (subscription == &colorTrigger) {
      if (parseColor((char *)colorTrigger.lastread) > 0) {
        mqttPublish(colorSettingPublish, parseColor((char *)colorTrigger.lastread));
      }
    }
  }

  delay (10);
}

void setLedColor(char * color) {
  char colorString[6];
  for (int i = 0; i < 6; i ++) {
    colorString[i] = color[i + 1];
  }

  currColor = StrToHex(colorString);

  for (int i = 0; i < NUMPIXELS; i ++) {
    pixels.setPixelColor(i, currColor);
  }
  pixels.show();
}

void setLedBrightness(char * brightnessString) {
  int brightness = atoi(brightnessString);
  // if (i < 10) { i *= 10; }
  brightness = map(brightness, 0, 100, 0, 255);
  int numSteps = 50;
  int interval = brightness - currBrightness;
  double increment = interval / numSteps;

  for (int i = 0; i < numSteps; i ++) {
    pixels.setBrightness(currBrightness + (increment * i));
    pixels.show();
    Serial.println(currBrightness + (increment * i));
    delay(10);
  }

  currBrightness = brightness;
  pixels.setBrightness(currBrightness);
}

int StrToHex(char str[]) {
  return (int) strtol(str, 0, 16);
}

char * parseColor(char * colorName) {
  String colorNameString = colorName;

  if (!colorNameString.compareTo("blue") || !colorNameString.compareTo("Blue")) {
      return (char *)BLUE;
  } else if (!colorNameString.compareTo("red") || !colorNameString.compareTo("Red")) {
      return (char *)RED;
  } else if (!colorNameString.compareTo("green") || !colorNameString.compareTo("Green")) {
      return (char *)GREEN;
  } else if (!colorNameString.compareTo("magenta") || !colorNameString.compareTo("Magenta")) {
      return (char *)MAGENTA;
  } else if (!colorNameString.compareTo("purple") || !colorNameString.compareTo("Purple")) {
      return (char *)PURPLE;
  } else if (!colorNameString.compareTo("teal") || !colorNameString.compareTo("Teal")) {
      return (char *)TEAL;
  } else if (!colorNameString.compareTo( "Warm White") || !colorNameString.compareTo( "warm white") || !colorNameString.compareTo( "Warm white") || !colorNameString.compareTo( "warm White")) {
      return (char *)WARMWHITE;
  } else if (!colorNameString.compareTo( "Cool White") || !colorNameString.compareTo( "cool white") || !colorNameString.compareTo( "Cool white") || !colorNameString.compareTo( "cool White")) {
      return (char *)COOLWHITE;
  }
  return colorName;
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
       delay(5000);  // wait 5 seconds
  }
 
Serial.println("MQTT Connected!");
}

void mqttPublish(Adafruit_MQTT_Publish stream, char * value) {
  if (!stream.publish(value))
  {
    Serial.println("Failed");
  }
  else
  {
    Serial.println("OK");
  }
}
