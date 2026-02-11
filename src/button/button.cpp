#include "button.h"

ButtonClass::ButtonClass()
    : DEBOUNCE_TIME(200),
      lastButtonPressTime(0),
      buttonLeftPressed(false),
      buttonRightPressed(false),
      buttonMidPressed(false)
{
}