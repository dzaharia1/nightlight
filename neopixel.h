#include <Adafruit_NeoPixel.h>

#define NEOPIXEL 9

// color definitions
#define RED "#ff0000"
#define LEMONADE "#ff0018"
#define PEACH "#ff3700"
#define GREEN "#00ff00"
#define TEAL "#00ff93"
#define CYAN "#00ffff"
#define BLUE "#0000ff"
#define PURPLE "#9600ff"
#define MAGENTA "#ff00e8"
#define WHITE "#ffffff"
#define YELLOW "#ffb900"
#define COOLWHITE "#eeeeee"
#define WARMWHITE "#ff7e2b"
#define NUMPIXELS 45

#define MODE_NORMAL 0
#define MODE_NIGHTLIGHT 1
#define MODE_CHILL 2
#define MODE_PARTY 3

struct Color {int red,green,blue;};

int currBrightness = 255;
int minBrightness = 10;
int nightBrightness = 15;
int previousBrightness = currBrightness;
int mode = MODE_NORMAL;
Color currColor = { 255, 255, 255 };

// set up the neopixel
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL, NEO_GRBW + NEO_KHZ800);

void startNeoPixel() {
    pixels.begin();
    pixels.setBrightness(255);
    // setLedColor(WARMWHITE);
    pixels.fill(pixels.Color(0, 0, 0, 255));
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
    }
    else if (originalColor.green >= originalColor.red && originalColor.green >= originalColor.blue) {
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
    }
    else if (originalColor.blue >= originalColor.red && originalColor.blue >= originalColor.green) {
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

char *parseColor(char *colorName)
{
    String colorNameString = colorName;

    if (!colorNameString.compareTo("blue") || !colorNameString.compareTo("Blue")) {
        return (char *)BLUE;
    }
    else if (!colorNameString.compareTo("red") || !colorNameString.compareTo("Red")) {
        return (char *)RED;
    }
    else if (!colorNameString.compareTo("yellow") || !colorNameString.compareTo("Yellow")) {
        return (char *)YELLOW;
    }
    else if (!colorNameString.compareTo("lemonade") || !colorNameString.compareTo("Lemonade")) {
        return (char *)LEMONADE;
    }
    else if (!colorNameString.compareTo("green") || !colorNameString.compareTo("Green")) {
        return (char *)GREEN;
    }
    else if (!colorNameString.compareTo("magenta") || !colorNameString.compareTo("Magenta")) {
        return (char *)MAGENTA;
    }
    else if (!colorNameString.compareTo("purple") || !colorNameString.compareTo("Purple")) {
        return (char *)PURPLE;
    }
    else if (!colorNameString.compareTo("teal") || !colorNameString.compareTo("Teal")) {
        return (char *)TEAL;
    }
    else if (!colorNameString.compareTo("cyan") || !colorNameString.compareTo("Cyan")) {
        return (char *)CYAN;
    }
    else if (!colorNameString.compareTo("Warm White") || !colorNameString.compareTo("warm white") || !colorNameString.compareTo("Warm white") || !colorNameString.compareTo("warm White")) {
        return (char *)WARMWHITE;
    }
    else if (!colorNameString.compareTo("Cool White") || !colorNameString.compareTo("cool white") || !colorNameString.compareTo("Cool white") || !colorNameString.compareTo("cool White")) {
        return (char *)COOLWHITE;
    }
    return colorName;
}
