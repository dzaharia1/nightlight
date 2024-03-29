#include <Adafruit_NeoPixel.h>
#include "colors.h"

#define NEOPIXEL 11
#define NUMPIXELS 84
#define MAXBRIGHTNESS 255

struct Color {int red, green, blue; };

int currBrightness = MAXBRIGHTNESS;
int minBrightness = 10;
int nightBrightness = 3;
int previousBrightness = currBrightness;
Color currColor = { MAXBRIGHTNESS / 10, MAXBRIGHTNESS / 10, MAXBRIGHTNESS / 10 };

// set up the neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL, NEO_GRBW + NEO_KHZ800);

void startNeoPixel() {
    pixels.begin();
    pixels.setBrightness(200);
    pixels.fill(pixels.Color(50, 50, 50));
    pixels.show();
}

int StrToHex(char str[]) {
    return (int)strtol(str, 0, 16);
}

Color calibrateColorBrightness(Color originalColor, int brightness) {
    Color retColor;

    if (originalColor.red >= originalColor.blue && originalColor.red >= originalColor.green) {
        retColor.red = brightness;
        retColor.green = map(
            originalColor.green,
            0, originalColor.red,
            0, brightness);
        retColor.blue = map(
            originalColor.blue,
            0, originalColor.red,
            0, brightness);
    } else if (originalColor.green >= originalColor.red && originalColor.green >= originalColor.blue) {
        retColor.green = brightness;
        retColor.red = map(
            originalColor.red,
            0, originalColor.green,
            0, brightness);
        retColor.blue = map(
            originalColor.blue,
            0,
            originalColor.green,
            0, brightness);
    } else if (originalColor.blue >= originalColor.red && originalColor.blue >= originalColor.green) {
        retColor.blue = brightness;
        retColor.red = map(
            originalColor.red,
            0, originalColor.blue,
            0, brightness);
        retColor.green = map(
            originalColor.green,
            0, originalColor.blue,
            0, brightness);
    }

    return retColor;
}

void setLedColor(Color newColor)
{
    Color startingColor = calibrateColorBrightness(currColor, previousBrightness);
    Color targetColor = calibrateColorBrightness(newColor, currBrightness);

    int numSteps = 20;
    int redIncrement = (targetColor.red - startingColor.red) / numSteps;
    int greenIncrement = (targetColor.green - startingColor.green) / numSteps;
    int blueIncrement = (targetColor.blue - startingColor.blue) / numSteps;
    int delayTime = 20;

    for (int i = 0; i < numSteps; i++) {
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
        targetColor.blue));
    pixels.show();

    currColor = newColor;
}

int toInt(String str) {
  int val = 0;
  for (int i = 0; i < str.length(); i ++) {
    val = val * 10 + str[i] - '0';
  }
  return val;
}

void setLedColor(char *colorString)
{
    Serial.println(colorString);
    String newColors[3];
    int characterIndex = 0;
    int colorIndex = 0;
    int i = 0;

    for (int j = 0; j < 3; j ++){
      newColors[j] = "";
    }

    for (char *ptr = colorString; *ptr != '\0'; ptr++) {
        if (*ptr != ',' && characterIndex <= 2) {
          newColors[colorIndex].concat(*ptr);
          characterIndex++;
        } else if (*ptr == ',') {
          characterIndex = 0;
          colorIndex++;
        }
    }

    Color newColor = {
        toInt(newColors[0]),
        toInt(newColors[1]),
        toInt(newColors[2])
    };
    setLedColor(newColor);
}

void setLedBrightness(int brightness)
{
    Serial.println(brightness);
    previousBrightness = currBrightness;
    currBrightness = brightness;
    if (brightness == 0) {
        currBrightness = 0;
    }
    else if (currBrightness < minBrightness) {
        currBrightness = minBrightness;
    }
    setLedColor(currColor);
    previousBrightness = currBrightness;
}

void setLedBrightness(char *brightnessString)
{
    setLedBrightness(atoi(brightnessString));
}

void turnOnLamp() {
    pixels.fill(pixels.Color(255, 255, 255, 255));
    pixels.show();
}
