#include "wificonfig.h"
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFi101.h>
#include <Adafruit_NeoPixel.h>

// setup variables
#define PHOTOCELL       9
#define NEOPIXEL        11
#define PIRSENSOR       12
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define WINC_CS         8
#define WINC_IRQ        7
#define WINC_RST        4
#define WINC_EN         2

// color definitions
#define RED             "#ff0000"
#define LEMONADE        "#ff0018"
#define PEACH           "#ff3700"
#define GREEN           "#00ff00"
#define TEAL            "#00ff93"
#define CYAN            "#00ffff"
#define BLUE            "#0000ff"
#define PURPLE          "#9600ff"
#define MAGENTA         "#ff00e8"
#define WHITE           "#ffffff"
#define YELLOW          "#ffb900"
#define COOLWHITE       "#eeeeee"
#define WARMWHITE       "#ff7e2b"
#define NUMPIXELS       45

// operating modes
#define MODE_NORMAL     0
#define MODE_NIGHTLIGHT 1
#define MODE_CHILL      2
#define MODE_PARTY      3

// set up the neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL, NEO_GRBW + NEO_KHZ800);

// set up the wifi
char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASS;
int status = WL_IDLE_STATUS;
WiFiClient client;

// set up the MQTT connection
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish motionSensor = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/motionsensor");
Adafruit_MQTT_Publish photocellStream = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/photocell");
Adafruit_MQTT_Publish colorFeedPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/color-setting");
Adafruit_MQTT_Publish brightnessPublish = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/brightness");
Adafruit_MQTT_Subscribe colorFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/color-setting");
Adafruit_MQTT_Subscribe brightnessFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/brightness");
Adafruit_MQTT_Subscribe colorTrigger = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/color-trigger");
Adafruit_MQTT_Subscribe modeFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/night-mode");

struct Color {
  int red, green, blue;
};

int currBrightness = 255;
int minBrightness = 10;
int nightBrightness = 15;
int previousBrightness = currBrightness;
int readTimeout = 500;
int mode = MODE_NORMAL;
bool motionDetected = false;
char startingColor[] = "ffffff";
Color currColor = { 255, 255, 255 };

void setup()
{
  Serial.begin(115200);
  pinMode(PIRSENSOR, INPUT);
  pinMode(PHOTOCELL, INPUT);

  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("No WINC1500");
    while (true);
  }
  Serial.println("wifi ok");

  mqtt.subscribe(&colorFeed);
  mqtt.subscribe(&brightnessFeed);
  mqtt.subscribe(&colorTrigger);
  mqtt.subscribe(&modeFeed);

  pixels.begin();
  pixels.setBrightness(255);
  // setLedColor(WARMWHITE);
  pixels.fill(pixels.Color(0, 0, 0, 255));
  pixels.show();
}

int counter = 0;

void loop() {
  MQTT_connect();
  
  checkMode(500);

  if (mode == MODE_NIGHTLIGHT) {
    setLedBrightness(0);
    if (digitalRead(PIRSENSOR) && analogRead(PHOTOCELL) < 160) {
      nightFadeIn();
    }
  } else if (mode == MODE_CHILL) {
    party(65536/2);
  } else if (mode == MODE_PARTY) {
    party(256);
  }
}

void checkMode (int timeout) {
  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(timeout)){
    if (subscription == &colorFeed) {
      if (mode != MODE_NIGHTLIGHT) { mode = MODE_NORMAL; }
      setLedColor((char *)colorFeed.lastread);
    }
    if (subscription == &brightnessFeed) {
      if (mode == MODE_NIGHTLIGHT) {
        nightBrightness = map(atoi((char *)brightnessFeed.lastread), 0, 100, 0, 255);
      }
      else if (mode == MODE_CHILL || mode == MODE_PARTY) {
        currBrightness = map(atoi((char *)brightnessFeed.lastread), 0, 100, 0, 255);
      }
      else {
        setLedBrightness((char *)brightnessFeed.lastread);
      }
    }
    if (subscription == &colorTrigger && parseColor((char *)colorTrigger.lastread) > 0) {
      mqttPublish(colorFeedPublish, parseColor((char *)colorTrigger.lastread));
    }
    if (subscription == &modeFeed) {
      mode = atoi((char *)modeFeed.lastread);
      if (mode == MODE_NORMAL) {
        setLedColor(currColor);
      }
      Serial.println(mode);
    }
  }
}

void party(int timing) {
  int currMode = mode;
  for (int i = 0; i < timing; i ++) {
    pixels.fill(pixels.ColorHSV(i * (65536 / timing), 255, currBrightness));
    pixels.show();
    Serial.println(i);
    if (i % 128 == 0) {
      checkMode(50);
      if (currMode != mode) {
        return;
      }
    }
    delay(10);
  }
}

void setLedColor(char * colorString) {
  char red[2] = { colorString[1], colorString[2] };
  char green[2] = { colorString[3], colorString[4] };
  char blue[2] = { colorString[5], colorString[6] };
  Color newColor = {
    StrToHex(red),
    StrToHex(green),
    StrToHex(blue)
  };

  setLedColor(newColor);
}

void setLedColor(Color newColor) {
  Color startingColor = calibrateColorBrightness(currColor, previousBrightness);
  Color targetColor = calibrateColorBrightness(newColor, currBrightness);

  int numSteps = 20;
  int redIncrement = (targetColor.red - startingColor.red) / numSteps;
  int greenIncrement = (targetColor.green - startingColor.green) / numSteps;
  int blueIncrement = (targetColor.blue - startingColor.blue) / numSteps;
  int delayTime = 20;

  for (int i = 0; i < numSteps; i++)
  {
    pixels.fill(pixels.Color(
        startingColor.red + (i * redIncrement),
        startingColor.green + (i * greenIncrement),
        startingColor.blue + (i * blueIncrement)));
    pixels.show();
    delay(delayTime);
  }

  pixels.fill(pixels.Color(
    targetColor.red,
    targetColor.green,
    targetColor.blue
  ));
  pixels.show();

  currColor = newColor;
}

Color calibrateColorBrightness(Color originalColor, int brightness) {
  Color retColor;

  if (originalColor.red >= originalColor.blue && originalColor.red >= originalColor.green) {
    retColor.red = brightness;
    retColor.green = map(
      originalColor.green,
      0, originalColor.red,
      0, brightness
    );
    retColor.blue = map(
      originalColor.blue,
      0, originalColor.red,
      0, brightness
    );
  } else if (originalColor.green >= originalColor.red && originalColor.green >= originalColor.blue) {
    retColor.green = brightness;
    retColor.red = map(
      originalColor.red,
      0, originalColor.green,
      0, brightness
    );
    retColor.blue = map(
      originalColor.blue,
      0,
      originalColor.green,
      0, brightness
    );
  } else if (originalColor.blue >= originalColor.red && originalColor.blue >= originalColor.green) {
    retColor.blue = brightness;
    retColor.red = map(
      originalColor.red,
      0, originalColor.blue,
      0, brightness
    );
    retColor.green = map(
      originalColor.green,
      0, originalColor.blue,
      0, brightness
    );
  }

  return retColor;
}

void setLedBrightness(char * brightnessString) {
  setLedBrightness(map(atoi(brightnessString), 0, 100, 0, 255));
}

void setLedBrightness(int brightness) {
  previousBrightness = currBrightness;
  currBrightness = brightness;
  if (brightness == 0)
  {
    currBrightness = 0;
  }
  else if (currBrightness < minBrightness)
  {
    currBrightness = minBrightness;
  }
  setLedColor(currColor);
  previousBrightness = currBrightness;
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
  } else if (!colorNameString.compareTo("yellow") || !colorNameString.compareTo("Yellow")){
    return (char *)YELLOW;
  } else if (!colorNameString.compareTo("lemonade") || !colorNameString.compareTo("Lemonade")) {
    return (char *)LEMONADE;
  } else if (!colorNameString.compareTo("green") || !colorNameString.compareTo("Green")) {
    return (char *)GREEN;
  } else if (!colorNameString.compareTo("magenta") || !colorNameString.compareTo("Magenta")) {
    return (char *)MAGENTA;
  } else if (!colorNameString.compareTo("purple") || !colorNameString.compareTo("Purple")) {
    return (char *)PURPLE;
  } else if (!colorNameString.compareTo("teal") || !colorNameString.compareTo("Teal")) {
    return (char *)TEAL;
  } else if (!colorNameString.compareTo("cyan") || !colorNameString.compareTo("Cyan")) {
    return (char *)CYAN;
  } else if (!colorNameString.compareTo( "Warm White") || !colorNameString.compareTo( "warm white") || !colorNameString.compareTo( "Warm white") || !colorNameString.compareTo( "warm White")) {
    return (char *)WARMWHITE;
  } else if (!colorNameString.compareTo( "Cool White") || !colorNameString.compareTo( "cool white") || !colorNameString.compareTo( "Cool white") || !colorNameString.compareTo( "cool White")) {
    return (char *)COOLWHITE;
  }
  return colorName;
}

void nightFadeIn() {
  Serial.println("Fading in");
  while (currBrightness < nightBrightness) {
    currBrightness ++;
    Color calibratedColor = calibrateColorBrightness(currColor, currBrightness);
    pixels.fill(pixels.Color(
      calibratedColor.red,
      calibratedColor.green,
      calibratedColor.blue
    ));
    pixels.show();
    delay(200);
  }

  delay(10000);

  while (digitalRead(PIRSENSOR)) {
    Serial.println("Still sensing motion");
    delay(3000);
  }

  nightFadeOut();
}

void nightFadeOut() {
  Serial.println("fading out");
  while (currBrightness > 0) {
    currBrightness --;
    Color calibratedColor = calibrateColorBrightness(currColor, currBrightness);
    pixels.fill(pixels.Color(
      calibratedColor.red,
      calibratedColor.green,
      calibratedColor.blue
    ));
    pixels.show();
    Serial.print("Fade progress is ");
    Serial.println(currBrightness);
    delay(200);

    if (digitalRead(PIRSENSOR)) {
      Serial.println("oop faded out too soon!");
      nightFadeIn();
      break;
    }
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
    Serial.print("Published ");
    Serial.print(value);
    Serial.print(" to ");
    Serial.println(stream.getTopic());
  }
}
