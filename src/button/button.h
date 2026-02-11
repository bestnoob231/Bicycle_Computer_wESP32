#ifndef BUTTON_H
#define BUTTON_H
#include <Arduino.h>

class ButtonClass
{
public:
    ButtonClass();

private:
    unsigned int DEBOUNCE_TIME;        // Default 200 (ms). If button ghosting occure, increase value. If you want click faster in a row, decrease value.
    unsigned long lastButtonPressTime; // For tracking of when was last press
    volatile bool buttonLeftPressed;   // Default false. Left button flag
    volatile bool buttonRightPressed;  // Default false. Right button flag
    volatile bool buttonMidPressed;    // Default false. Middle button flag
};

#endif