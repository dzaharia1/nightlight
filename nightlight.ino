#include <SPI.h>
#include "wificonfig.h"
#include "adaio.h"

// setup variables
#define PHOTOCELL       10
#define PIRSENSOR       12

bool motionDetected = false;
int nightBrightness = 3;

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
  
  checkMode(500);

  if (mode == MODE_NIGHTLIGHT) {
    nightFadeOut();
    if (digitalRead(PIRSENSOR) && analogRead(PHOTOCELL) < 160) {
      nightFadeIn();
    }
  } else if (mode == MODE_CHILL) {
    party(65536/2);
  } else if (mode == MODE_PARTY) {
    party(256);
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

void party(int timing) {
    int currMode = mode;
    
    for (int i = 0; i < timing; i++) {
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
