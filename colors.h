// color definitions
#define RED         "#ff0000"
#define LEMONADE    "#ff0018"
#define CORAL       "#ff1f08"
#define PEACH       "#ff2800"
#define GREEN       "#00ff00"
#define LIME        "#77ff07"
#define TEAL        "#00ff93"
#define CYAN        "#00ffff"
#define BLUE        "#0000ff"
#define PURPLE      "#9600ff"
#define MAGENTA     "#ff00e8"
#define WHITE       "#ffffff"
#define YELLOW      "#ffb900"
#define COOLWHITE   "#ffffff"
#define WARMWHITE   "#ff7e2b"

char *parseColor(char *colorName) {
    String colorNameString = colorName;

    if (!colorNameString.compareTo("blue") ||
               !colorNameString.compareTo("Blue")) {
        return (char *)BLUE;
    } else if (!colorNameString.compareTo("red") ||
               !colorNameString.compareTo("Red")) {
        return (char *)RED;
    } else if (!colorNameString.compareTo("Peach") ||
               !colorNameString.compareTo("peach")) {
        return (char *)PEACH;
    } else if (!colorNameString.compareTo("coral") ||
               !colorNameString.compareTo("Coral")) {
        return (char *)CORAL;
    } else if (!colorNameString.compareTo("yellow") ||
               !colorNameString.compareTo("Yellow")) {
        return (char *)YELLOW;
    } else if (!colorNameString.compareTo("lemonade") ||
               !colorNameString.compareTo("Lemonade")) {
        return (char *)LEMONADE;
    } else if (!colorNameString.compareTo("lime") ||
               !colorNameString.compareTo("Lime")) {
        return (char *)LIME;
    } else if (!colorNameString.compareTo("green") ||
               !colorNameString.compareTo("Green")) {
        return (char *)GREEN;
    } else if (!colorNameString.compareTo("magenta") ||
               !colorNameString.compareTo("Magenta")) {
        return (char *)MAGENTA;
    } else if (!colorNameString.compareTo("purple") ||
               !colorNameString.compareTo("Purple")) {
        return (char *)PURPLE;
    } else if (!colorNameString.compareTo("teal") ||
               !colorNameString.compareTo("Teal")) {
        return (char *)TEAL;
    } else if (!colorNameString.compareTo("cyan") ||
               !colorNameString.compareTo("Cyan")) {
        return (char *)CYAN;
    } else if (!colorNameString.compareTo("Warm White") ||
               !colorNameString.compareTo("warm white") ||
               !colorNameString.compareTo("Warm white") ||
               !colorNameString.compareTo("warm White")) {
        return (char *)WARMWHITE;
    } else if (!colorNameString.compareTo("Cool White") ||
               !colorNameString.compareTo("cool white") ||
               !colorNameString.compareTo("Cool white") ||
               !colorNameString.compareTo("cool White")) {
        return (char *)COOLWHITE;
    }

    return COOLWHITE;
}