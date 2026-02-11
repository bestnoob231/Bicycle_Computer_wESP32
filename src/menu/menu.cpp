#include "menu.h"

Menu::Menu()
{
}

Menu::~Menu()
{
}

int Menu::lenght(String *arr)
{
    if (arr->isEmpty())
    {
        return -1;
    }

    return (sizeof(arr) / sizeof(String));
}

bool Menu::menuDown(String *arr)
{
    if (arr->isEmpty())
    {
        return false;
    }

    String temp = arr[0];

    for (int i = 0; i < lenght(arr) - 1; i++)
    {
        arr[i] = arr[i + 1];
    }

    arr[lenght(arr) - 1] = temp;

    return true;
}

bool Menu::menuUp(String *arr)

{
    if (arr->isEmpty())
    {
        return false;
    }

    String temp = arr[lenght(arr) - 1];

    for (int i = lenght(arr) - 1; i > 0; i--)
    {
        arr[i] = arr[i - 1];
    }

    arr[0] = temp;

    return true;
}
