#include <SPI.h>
#include "wificonfig.h"
#include "feeds.h"

// setup variables
#define PHOTOCELL       9
#define PIRSENSOR       12

// operating modes
#define MODE_NORMAL     "Normal"
#define MODE_NIGHTLIGHT "Night light"
#define MODE_CHILL      "Chill"
#define MODE_PARTY      "Party"
#define MODE_LAMP       "Blast"

bool motionDetected = false;
int dayBrightness;
char *mode = MODE_NORMAL;

void setup()
{
  Serial.begin(115200);
  Serial.println("Startup");
  pinMode(PIRSENSOR, INPUT);
  pinMode(PHOTOCELL, INPUT);

  connectionSetup();
  startNeoPixel();
}

void loop() {
  MQTT_connect();
  checkMode();
  Serial.print("Mode is ");
  Serial.println(mode);

  if (!strcmp(mode, MODE_CHILL)) {
    party(65536/2);
  } else if (!strcmp(mode, MODE_PARTY)) {
    party(256);
  } else if (!strcmp(mode, MODE_LAMP)) {
    turnOnLamp();
  }
}

void checkMode() {
  checkMode(500);
}

void checkMode(int timeout) {
  mqtt.ping();
  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(timeout)) {
    
    if (subscription == &switchFeed) {
      Serial.print("Switch to ");
      Serial.println((char *)switchFeed.lastread);
      if (!strcmp((char *)switchFeed.lastread, "OFF")) {
        Serial.println("Turning off");
        setLedBrightness(0);
      }
      
    } else if (subscription == &colorFeed) {
      Serial.print("Set color to ");
      Serial.println((char *)colorFeed.lastread);
      mode = MODE_NORMAL;
      setLedColor((char *)colorFeed.lastread);
      
    } else if (subscription == &brightnessFeed) {
      Serial.print("Set brightness to ");
      Serial.println((char *)brightnessFeed.lastread);
      if (mode == MODE_NIGHTLIGHT) {
        nightBrightness = atoi((char *)brightnessFeed.lastread);
      } else if (!strcmp(mode, MODE_CHILL) || !strcmp(mode, MODE_PARTY)) {
        currBrightness = atoi((char *)brightnessFeed.lastread);
      } else {
        setLedBrightness((char *)brightnessFeed.lastread);
      }
    }
    
    else if (subscription == &modeFeed) {
      Serial.print("Set mode to ");
      Serial.println((char *)modeFeed.lastread);

      mode = (char *)modeFeed.lastread;

      if (!strcmp(mode, MODE_NORMAL)) {
        setLedColor(currColor);
      } else if (!strcmp(mode, MODE_NIGHTLIGHT)) {
        dayBrightness = currBrightness;
        setLedBrightness(0);
      } else if (!strcmp(mode, MODE_LAMP)) {
        turnOnLamp();
      }
    }
  }

  MQTT_connect();
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
    checkMode(200);
    if (mode != MODE_NIGHTLIGHT) { return; }
  }

  checkMode(10000);
  if (mode != MODE_NIGHTLIGHT) { return; }

  while (digitalRead(PIRSENSOR)) {
    Serial.println("Still sensing motion");
    checkMode(3000);
    if (mode != MODE_NIGHTLIGHT) { return; }
  }

  nightFadeOut(true);
}

void nightFadeOut(bool watchMotion) {
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
    checkMode(200);
    if (mode != MODE_NIGHTLIGHT) { return; }

    if (digitalRead(PIRSENSOR) && watchMotion) {
      Serial.println("oop faded out too soon!");
      nightFadeIn();
      return;
    }
  }
}

void party(int timing) {
  for (int i = 0; i < timing; i++) {
    pixels.fill(pixels.ColorHSV(i * (65536 / timing), 255, currBrightness));
    pixels.show();
    if (i % 128 == 0) {
      char *currMode = mode;
      checkMode(50);
      if (currMode != mode) { return; }
    }
    delay(10);
  }
}
