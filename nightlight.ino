#include <SPI.h>
#include "wificonfig.h"
#include "adaio.h"

// setup variables
#define PHOTOCELL       9
#define PIRSENSOR       12

// operating modes
#define MODE_NORMAL     0
#define MODE_NIGHTLIGHT 1
#define MODE_CHILL      2
#define MODE_PARTY      3
#define MODE_LAMP       4

bool motionDetected = false;
int dayBrightness;
int mode = MODE_NORMAL;

void setup()
{
  Serial.begin(115200);
  pinMode(PIRSENSOR, INPUT);
  pinMode(PHOTOCELL, INPUT);

  connectionSetup();
  startNeoPixel();
}

void loop() {
  MQTT_connect();
  checkMode();

  if (mode == MODE_NIGHTLIGHT && digitalRead(PIRSENSOR) && analogRead(PHOTOCELL) < 160) {
    nightFadeIn();
  } else if (mode == MODE_CHILL) {
    party(65536/2);
  } else if (mode == MODE_PARTY) {
    party(256);
  }
}

void checkMode() {
  checkMode(500);
}

void checkMode(int timeout) {
  mqtt.ping();

  Adafruit_MQTT_Subscribe *subscription;
  while (subscription = mqtt.readSubscription(timeout)) {
    if (subscription == &colorFeed) {
      if (mode != MODE_NIGHTLIGHT) {
        mode = MODE_NORMAL;
      }
      setLedColor((char *)colorFeed.lastread);
    } else if (subscription == &brightnessFeed) {
      if (mode == MODE_NIGHTLIGHT) {
        nightBrightness = map(atoi((char *)brightnessFeed.lastread), 0, 100, 0, 255);
      } else if (mode == MODE_CHILL || mode == MODE_PARTY) {
        currBrightness = map(atoi((char *)brightnessFeed.lastread), 0, 100, 0, 255);
      } else {
        setLedBrightness((char *)brightnessFeed.lastread);
      }
    } else if (subscription == &colorTrigger && parseColor((char *)colorTrigger.lastread) > 0) {
      mqttPublish(colorFeedPublish, parseColor((char *)colorTrigger.lastread));
    } else if (subscription == &modeFeed) {
      if (atoi((char *)modeFeed.lastread) != MODE_NIGHTLIGHT && mode == MODE_NIGHTLIGHT) {
        setLedBrightness(dayBrightness);
      }

      mode = atoi((char *)modeFeed.lastread);

      if (mode == MODE_NORMAL) {
        setLedColor(currColor);
      } else if (mode == MODE_NIGHTLIGHT) {
        dayBrightness = currBrightness;
        setLedBrightness(0);
      } else if (mode == MODE_LAMP) {
        turnOnLamp();
      }
      Serial.println(mode);
    }
  }
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
  }

  checkMode(10000);

  while (digitalRead(PIRSENSOR)) {
    Serial.println("Still sensing motion");
    checkMode(3000);
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

    if (digitalRead(PIRSENSOR) && watchMotion) {
      Serial.println("oop faded out too soon!");
      nightFadeIn();
      break;
    }
  }
}

void party(int timing) {
  for (int i = 0; i < timing; i++) {
    pixels.fill(pixels.ColorHSV(i * (65536 / timing), 255, currBrightness));
    pixels.show();
    if (i % 128 == 0) {
      int currMode = mode;
      checkMode(50);
      if (currMode != mode) {
          return;
      }
    }
    delay(10);
  }
}
