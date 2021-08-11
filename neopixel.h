#include <Adafruit_NeoPixel.h>
#include "colors.h"

#define NEOPIXEL 11
#define NUMPIXELS 45

struct Color {int red, green, blue; };

int currBrightness = 255;
int minBrightness = 10;
int nightBrightness = 3;
int previousBrightness = currBrightness;
Color currColor = { 255, 255, 255 };

// set up the neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL, NEO_GRBW + NEO_KHZ800);

void startNeoPixel() {
    pixels.begin();
    pixels.setBrightness(255);
    pixels.fill(pixels.Color(5, 5, 5));
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

void setLedColor(char *colorString)
{
    char red[2] = {colorString[1], colorString[2]};
    char green[2] = {colorString[3], colorString[4]};
    char blue[2] = {colorString[5], colorString[6]};
    Color newColor = {
        StrToHex(red),
        StrToHex(green),
        StrToHex(blue)};

    setLedColor(newColor);
}

void setLedBrightness(int brightness)
{
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
    setLedBrightness(map(atoi(brightnessString), 0, 100, 0, 255));
}

void turnOnLamp() {
    pixels.fill(pixels.Color(255, 255, 255, 255));
    pixels.show();
}