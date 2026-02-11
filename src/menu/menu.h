#ifndef MENU_H
#define MENU_H
#include <Arduino.h>

class Menu
{
public:
    Menu();

    ~Menu();

    int lenght(String *arr);
    bool menuDown(String *arr);
    bool menuUp(String *arr);
};

#endif