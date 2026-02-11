#ifndef SCREEN_H
#define SCREEN_H
#include <Arduino.h>

class Screen
{
public:
    Screen();

    ~Screen();

    void displayStatic();
    void displayDynamic();
};

#endif