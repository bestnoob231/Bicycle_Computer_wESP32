// /==================================================================================================\
// ||                                                                                                ||
// ||         ____  _                 _         ____                            _                    ||
// ||        | __ )(_) ___ _   _  ___| | ___   / ___|___  _ __ ___  _ __  _   _| |_ ___ _ __         ||
// ||        |  _ \| |/ __| | | |/ __| |/ _ \ | |   / _ \| '_ ` _ \| '_ \| | | | __/ _ \ '__|        ||
// ||        | |_) | | (__| |_| | (__| |  __/ | |__| (_) | | | | | | |_) | |_| | ||  __/ |           ||
// ||        |____/|_|\___|\__, |\___|_|\___|  \____\___/|_| |_| |_| .__/ \__,_|\__\___|_|           ||
// ||                      |___/                                   |_|                               ||
// ||                                     _______ ____  ____ _________                               ||
// ||                          __      __/ / ____/ ___||  _ \___ /___ \                              ||
// ||                          \ \ /\ / / /|  _| \___ \| |_) ||_ \ __) |                             ||
// ||                           \ V  V / / | |___ ___) |  __/___) / __/                              ||
// ||                            \_/\_/_/  |_____|____/|_|  |____/_____|                             ||
// ||                                                                                                ||
// ||         _ _   _           _       ___               _                     _    ____  _____ _   ||
// ||    __ _(_) |_| |__  _   _| |__   / / |__   ___  ___| |_ _ __   ___   ___ | |__|___ \|___ // |  ||
// ||   / _` | | __| '_ \| | | | '_ \ / /| '_ \ / _ \/ __| __| '_ \ / _ \ / _ \| '_ \ __) | |_ \| |  ||
// ||  | (_| | | |_| | | | |_| | |_) / / | |_) |  __/\__ \ |_| | | | (_) | (_) | |_) / __/ ___) | |  ||
// ||   \__, |_|\__|_| |_|\__,_|_.__/_/  |_.__/ \___||___/\__|_| |_|\___/ \___/|_.__/_____|____/|_|  ||
// ||   |___/                                                                                        ||
// ||                                                                                                ||
// \==================================================================================================/

//    to Do
//  Tekerlek çevresini mm cinsinden kullanıcının girmesini sağla
//  (ekranı yap)
//
//  ESP32 sleep olayını hallet. (hangi durumda uyanacağını kararlaştır)
//  (Tuşa basıldığında, manyetik alan sensörü tetiklendiğinde uyanması vb.)
//
//  Multicore öğren. Nerede kullanılabilir düşün.

// #################   INCLUDES    #################

#include "button/button.h"
#include "menu/menu.h"
#include "screen/screen.h"
#include <images/images.h>

#include <EEPROM.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// #################################################

// #################  DEFINITIONS  #################

bool debug = false; //  Default "false". If it's true, there will be a lot of information on Serial Monitor

// TFT screen object and variables
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
const char *SSID = "K_Andac_2";
const char *PASSWORD = "kut769up";
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// TFT screen object and variables
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
TFT_eSPI tft = TFT_eSPI();
unsigned short txtFont = 2;           // Selecting font. For more information search for TFT_eSPI Library
unsigned short rotation = 2;          // Set screen rotation. (normal: 0, upside down: 2)
unsigned long lastShowGearChange = 0; // For tracking of when was last change of state
bool draw_gear_icon = false;          // default "false". For tracking whether the gear icon is drawn or not
bool darkTheme = true;                // dark = true, white = false. For rendering screen theme
uint16_t backgroundColor = 0x0000;    // Default "0x0000" (black). Background color hex value
uint16_t textColor = 0xFFFF;          // Default "0xFFFF" (white). Text rendering color hex value
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// Other pin definitions
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
#define HallEffectSensor 15 // ESP32 pin 21 aka. GPIO15
#define buttonLeft 12       // ESP32 pin 18 aka. GPIO12
#define buttonMid 13        // ESP32 pin 20 aka. GPIO13
#define buttonRight 14      // ESP32 pin 17 aka. GPIO14
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// Needs for menu operations
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
enum MenuState
{
  TIME,           // Main menu
  AVG,            //
  SETTINGS,       // - - - - - - - - - - - - - - - -
  THEME,          // Settings submenu
  THEME_DARK,     // Option for changing theme. Not a menu
  THEME_WHITE,    // Option for changing theme. Not a menu
  CIRC,           // Settings submenu
  CHANGE_CIRC,    // For input the perimeter of wheel. Not a menu
  UPDATE,         // Settings submenu
  TRY_UPT,        // Try start connection for update. Not a menu
  ERR_UPT,        // If there is any error, this screen will shown. Not a menu
  RESET,          // Settings submenu
  TRY_RST,        // Confirmation for reset all values. Not a menu
  BACK,           // For quiting from settings
  MENU_ENUM_COUNT // Using for knowing end of the enum
};
MenuState currentMenu = TIME; // Default "TIME". For keeping the current menu state
bool enter_settings = false;  // in settings = true, not in settings = false. To select menu cycle; main menu or settings
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// Variables to keep track of button presses
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
#define DEBOUNCE_TIME 200                 // Default 200 (ms). If button ghosting occure, increase value. If you want click faster in a row, decrease value.
unsigned long lastButtonPressTime = 0;    // For tracking of when was last press
volatile bool buttonLeftPressed = false;  // Default false. Left button flag
volatile bool buttonRightPressed = false; // Default false. Right button flag
volatile bool buttonMidPressed = false;   // Default false. Middle button flag
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// #################################################

// ################# INITIALIZIONS #################

// Variables for calculating and tracking time
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
unsigned long seconds = 0;
unsigned long minutes = 0;
unsigned long hours = 0;
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// All string variables
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
// Strings variables for printing to screen
String speedStr;
String maxSpeedStr;
String avgSpeedStr;
String totalDistanceStr;
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// EEPROM variables
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
#define EEPROM_SIZE 64                    // EEPROM size that will used
const int eeMaxSpeedAddress = 0;          // 0-3
const int eeAvgSpeedAddress = 4;          // 4-7
const int eeDistAddress = 8;              // 8-11
const int eeDistHundredAddress = 12;      // 12-15
const int eeElapsedTotalTimeAddress = 16; // 16-19
const int eeDarkThemeAddress = 20;        // 19
const int eeCircumferenceAddress = 24;    // 24-27
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

// Misc
//. . . . . . . . . . . . . . . . . . . . . . . . . . .
unsigned int waitUntil = 1600;       // "calculate()" function will be triggered if designated time has passed
volatile unsigned short counter = 0; // Counter for storing how many passes did magnet on wheel

float fStartTime = 0.0;
float fElapsedTime = 0.0;

float fSpeed = 0.0;
float fMaxSpeed = 0.0;
float fAvgSpeed = 0.0;

float fTakenDistanceCm = 0.0;
float fTotalDistanceKm = 0.0;
int distanceHundredKm = 0;
float fElapsedTotalTime = 0.0;

int currentCircumferenceMm = 2078; // Bicycle's wheel perimeer (in mm) that magnet attached.
//. . . . . . . . . . . . . . . . . . . . . . . . . . .

void menuDynamic(int spd_x, int spd_y, int max_x, int max_y, int time_x, int time_y, int dist_x, int dist_y, int avg_x, int avg_y);
void menuStatic(int spd_x, int spd_y, int max_x, int max_y, int time_x, int time_y, int dist_x, int dist_y, int avg_x, int avg_y);
void settingsStatic(String text_1, String text_2, String text_3, String text_4, byte textSize);
void settingsDynamic(int selected);
void tryConnection(const char *ssid, const char *password);
void resetAll();

// #################################################

void IRAM_ATTR hallInterrupt()
{
  counter++;
}

void IRAM_ATTR buttonInterrupt()
{
  unsigned long currentTime = millis();
  if ((currentTime - lastButtonPressTime) > DEBOUNCE_TIME)
  {
    if (digitalRead(buttonLeft) == LOW)
    {
      buttonLeftPressed = true;
    }
    else if (digitalRead(buttonRight) == LOW)
    {
      buttonRightPressed = true;
    }
    else if (digitalRead(buttonMid) == LOW)
    {
      buttonMidPressed = true;
    }
    lastButtonPressTime = currentTime;
  }
}

bool shouldBypassMenu(MenuState menu)
{
  switch (menu)
  {
  case THEME_DARK:
  case THEME_WHITE:
  case CHANGE_CIRC:
  case TRY_UPT:
  case ERR_UPT:
  case TRY_RST:
    return true;
  default:
    return false;
  }
}

void nextMenu()
{
  do
  {
    currentMenu = static_cast<MenuState>((currentMenu + 1) % MENU_ENUM_COUNT);
  } while (
      (enter_settings && (currentMenu <= SETTINGS || shouldBypassMenu(currentMenu))) || (!enter_settings && currentMenu > SETTINGS));
}

void previousMenu()
{
  do
  {
    currentMenu = static_cast<MenuState>((currentMenu - 1 + MENU_ENUM_COUNT) % MENU_ENUM_COUNT);
  } while (
      (enter_settings && (currentMenu <= SETTINGS || shouldBypassMenu(currentMenu))) || (!enter_settings && currentMenu > SETTINGS));
}

void changeTheme()
{
  if (darkTheme)
  {
    textColor = 0XFFFF;
    backgroundColor = 0X0000;
  }
  else
  {
    textColor = 0X0000;
    backgroundColor = 0XFFFF;
  }
}

void drawImage(const uint8_t img[], unsigned &img_w, unsigned &img_h, bool &is_drew, unsigned img_size = 4)
{
  unsigned long now = millis();

  if (now - lastShowGearChange > 500)
  {

    tft.setTextSize(img_size);
    tft.setTextDatum(BL_DATUM);

    switch (is_drew)
    {
    case 0:
      tft.drawBitmap(
          tft.width() - (img_w + 5), tft.height() - (img_h + 5),
          img, gearW, gearH,
          textColor);

      is_drew = true;
      break;
    case 1:
      tft.drawBitmap(
          tft.width() - (img_w + 5), tft.height() - (img_h + 5),
          img, gearW, gearH,
          backgroundColor);

      is_drew = false;
      break;
    }

    lastShowGearChange = millis();
  }
}

void displayMenuStatic(MenuState state)
{
  tft.fillScreen(backgroundColor); // Clear the screen

  switch (state)
  {
  case TIME:
    menuStatic(5, 0, 70, 0, 50, 50, 5, 115, 70, 115);
    break;
  case AVG:
    menuStatic(5, 0, 70, 0, 70, 115, 5, 115, 53, 50);
    break;
  case SETTINGS:
    settingsStatic("ENTER", "SETTINGS", "", "", 2);
    break;
  case THEME:
    settingsStatic("CHANGE", "THEME", "", "", 2);
    break;
  case THEME_DARK:
    settingsStatic("", "DARK", "WHITE", "", 2);
    break;
  case THEME_WHITE:
    settingsStatic("", "DARK", "WHITE", "", 2);
    break;
  case CIRC:
    settingsStatic("CHANGE", "WHEEL", "DIAMETER", "", 2);
    break;
  case CHANGE_CIRC:
    settingsStatic("", String(currentCircumferenceMm), "", "", 2);
    break;
  case RESET:
    settingsStatic("RESET", "ALL", "", "", 2);
    break;
  case TRY_RST:
    settingsStatic("", "CONFIRM", "RESET", "", 2);
    break;
  case UPDATE:
    settingsStatic("TRY", "CONNECT", "", "", 2);
    break;
  case TRY_UPT:
    settingsStatic("TRYING", "TO", "CONNECT", "", 2);
    break;
  case ERR_UPT:
    settingsStatic("ERROR", "OCCURED!", "", "", 2);
    break;
  case BACK:
    settingsStatic("BACK", "", "", "", 2);
    break;
  }
}

void displayMenuDynamic(MenuState state)
{
  switch (state)
  {
  case TIME:
    menuDynamic(5, 13, 126, 13, 10, 70, 5, 130, 125, 130);
    break;
  case AVG:
    menuDynamic(5, 13, 126, 13, 70, 140, 5, 130, 96, 70);
    break;
  case THEME:
  case CIRC:
  case CHANGE_CIRC:
  case RESET:
  case TRY_RST:
  case BACK:
  case UPDATE:
    drawImage(gear_icon, gearW, gearH, draw_gear_icon, 4);
    break;
  case TRY_UPT:
    ArduinoOTA.handle();
    drawImage(gear_icon, gearW, gearH, draw_gear_icon, 4);
    break;
  case THEME_DARK:
  case THEME_WHITE:
    drawImage(gear_icon, gearW, gearH, draw_gear_icon, 4);
    settingsDynamic(state == THEME_DARK ? 1 : 2);
    break;
  }
}

void handleLeftButton()
{
  switch (currentMenu)
  {
  case THEME_WHITE:
    darkTheme = true;
    changeTheme();
    currentMenu = THEME_DARK;
    break;
  case CHANGE_CIRC:
    currentCircumferenceMm--;
    break;
  case THEME_DARK:
    darkTheme = false;
    changeTheme();
    currentMenu = THEME_WHITE;
    break;
  case TRY_RST:
    currentMenu = RESET;
    break;
  case TRY_UPT:
    currentMenu = UPDATE;
    WiFi.disconnect();
    break;
  default:
    previousMenu();
    break;
  }

  displayMenuStatic(currentMenu);
}

void handleRightButton()
{
  switch (currentMenu)
  {
  case THEME_DARK:
    darkTheme = false;
    changeTheme();
    currentMenu = THEME_WHITE;
    break;
  case CHANGE_CIRC:
    currentCircumferenceMm++;
    break;
  case THEME_WHITE:
    darkTheme = true;
    changeTheme();
    currentMenu = THEME_DARK;
    break;
  case TRY_RST:
    currentMenu = RESET;
    break;
  case TRY_UPT:
    currentMenu = UPDATE;
    WiFi.disconnect();
    break;
  default:
    nextMenu();
    break;
  }

  displayMenuStatic(currentMenu);
}

void handleMiddleButton()
{

  switch (currentMenu)
  {
  case SETTINGS:
    enter_settings = true;
    nextMenu();
    break;
  case THEME:
    currentMenu = darkTheme ? THEME_DARK : THEME_WHITE;
    break;
  case THEME_DARK:
  case THEME_WHITE:
    currentMenu = THEME;
    break;
  case CIRC:
    currentMenu = CHANGE_CIRC;
    break;
  case CHANGE_CIRC:
    currentMenu = CIRC;
    break;
  case RESET:
    currentMenu = TRY_RST;
    break;
  case TRY_RST:
    resetAll();
    enter_settings = false;
    currentMenu = TIME;
    break;
  case UPDATE:
    currentMenu = TRY_UPT;
    tryConnection(SSID, PASSWORD);
    break;
  case TRY_UPT:
    break;
  case ERR_UPT:
    enter_settings = false;
    currentMenu = TIME;
    break;
  case BACK:
    enter_settings = false;
    currentMenu = TIME;
    break;
  }

  displayMenuStatic(currentMenu);
}

void changeMenu()
{
  if (buttonLeftPressed)
  {
    buttonLeftPressed = false;
    handleLeftButton();
  }
  else if (buttonRightPressed)
  {
    buttonRightPressed = false;
    handleRightButton();
  }
  else if (buttonMidPressed)
  {
    buttonMidPressed = false;
    handleMiddleButton();
  }
}

void calculate()
{
  if (counter > 0 && counter <= 20)
  {

    fElapsedTime = millis() - fStartTime;
    fElapsedTotalTime += fElapsedTime;
    fTakenDistanceCm = currentCircumferenceMm / 10 * float(counter);
    fTotalDistanceKm += (fTakenDistanceCm / 100000.0);
    fSpeed = (fTakenDistanceCm / float(fElapsedTime)) * 36;
    fMaxSpeed = max(fMaxSpeed, fSpeed);
    fElapsedTotalTime += fElapsedTime;
    fAvgSpeed = fTotalDistanceKm / (fElapsedTotalTime / 3600000.0);
  }
  else
  {
    fSpeed = 0.0; // Set speed value zero if there is no sensor input
  }

  counter = 0;
  fStartTime = millis();
}

void eepromRead()
{
  float dataFloat;
  int dataInt;

  uint8_t themeTemp;

  EEPROM.get(eeMaxSpeedAddress, dataFloat);
  if ((int)dataFloat != 0 && !isnan(dataFloat))
    fMaxSpeed = dataFloat;

  EEPROM.get(eeAvgSpeedAddress, dataFloat);
  if ((int)dataFloat != 0 && !isnan(dataFloat))
    fAvgSpeed = dataFloat;

  EEPROM.get(eeDistAddress, dataFloat);
  if ((int)dataFloat != 0 && !isnan(dataFloat))
    fTotalDistanceKm = dataFloat;

  EEPROM.get(eeDistHundredAddress, dataInt);
  if ((int)dataFloat != 0 && !isnan(dataInt))
    distanceHundredKm = dataInt;

  EEPROM.get(eeElapsedTotalTimeAddress, dataFloat);
  if ((int)dataFloat != 0 && !isnan(dataFloat))
    fElapsedTotalTime = dataFloat;

  EEPROM.get(eeDarkThemeAddress, themeTemp);
  if (themeTemp != 0xFF)
    darkTheme = themeTemp;

  EEPROM.get(eeCircumferenceAddress, dataInt);
  if (dataInt != 0 && !isnan(dataInt))
    currentCircumferenceMm = dataInt;
}

void eepromWrite()
{
  float dataFloat;
  int dataInt;
  uint8_t themeTemp;

  EEPROM.get(eeMaxSpeedAddress, dataFloat);
  if (dataFloat != fMaxSpeed)
    EEPROM.put(eeMaxSpeedAddress, fMaxSpeed);

  EEPROM.get(eeAvgSpeedAddress, dataFloat);
  if (dataFloat != fAvgSpeed)
    EEPROM.put(eeAvgSpeedAddress, fAvgSpeed);

  EEPROM.get(eeDistAddress, dataFloat);
  if (dataFloat != fTotalDistanceKm)
    EEPROM.put(eeDistAddress, fTotalDistanceKm);

  EEPROM.get(eeDistHundredAddress, dataInt);
  if (dataInt != distanceHundredKm)
    EEPROM.put(eeDistHundredAddress, distanceHundredKm);

  EEPROM.get(eeElapsedTotalTimeAddress, dataFloat);
  if (dataFloat != fElapsedTotalTime)
    EEPROM.put(eeElapsedTotalTimeAddress, fElapsedTotalTime);

  EEPROM.get(eeDarkThemeAddress, themeTemp);
  if (themeTemp != darkTheme)
    EEPROM.put(eeDarkThemeAddress, darkTheme);

  Serial.print("EEPROMWrite (before): ");
  Serial.println(currentCircumferenceMm);

  EEPROM.get(eeCircumferenceAddress, dataInt);

  Serial.print("EEPROMWrite (data): ");
  Serial.println(dataInt);
  if (dataInt != currentCircumferenceMm)
    EEPROM.put(eeCircumferenceAddress, currentCircumferenceMm);

  Serial.print("EEPROMWrite (data after): ");
  Serial.println(EEPROM.get(eeCircumferenceAddress, dataInt));

  EEPROM.commit();
}

void resetAll()
{
  fMaxSpeed = 0;
  fElapsedTotalTime = 0;
  fAvgSpeed = 0;
  fTotalDistanceKm = 0;
  distanceHundredKm = 0;
  fElapsedTotalTime = 0;
  currentCircumferenceMm = 2078;
  darkTheme = true;

  eepromWrite();

  esp_restart();
}

void drawFloat(float value, int digits, int x, int y, int datum = TL_DATUM)
{
  int padding = tft.textWidth("99.9", txtFont);
  tft.setTextPadding(padding);
  tft.setTextDatum(datum);
  tft.drawFloat(value, digits, x, y, txtFont);
}

void drawInt(int value, int x, int y, int datum = TR_DATUM, const char *paddingText = "999")
{
  int padding = tft.textWidth(paddingText, txtFont);
  tft.setTextPadding(padding);
  tft.setTextDatum(datum);
  tft.drawNumber(value, x, y, txtFont);
}

void drawTime(int x, int y)
{
  unsigned long totalSeconds = fElapsedTotalTime / 1000;
  unsigned long hrs = totalSeconds / 3600;
  unsigned long mins = (totalSeconds % 3600) / 60;
  unsigned long secs = totalSeconds % 60;

  char timeStr[9];
  sprintf(timeStr, "%02lu:%02lu:%02lu", hrs, mins, secs);

  int padding = tft.textWidth("99.99.99", txtFont);
  tft.setTextPadding(padding);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(timeStr, x, y, txtFont);
}

void menuDynamic(int spd_x, int spd_y, int max_x, int max_y, int time_x, int time_y, int dist_x, int dist_y, int avg_x, int avg_y)
{
  tft.setTextColor(textColor, backgroundColor);
  tft.setTextSize(2);
  tft.setTextDatum(TL_DATUM);

  drawFloat(fSpeed, 1, spd_x, spd_y);

  if (fTotalDistanceKm >= 100.0)
  {
    distanceHundredKm += static_cast<int>(fTotalDistanceKm / 100.0);
    fTotalDistanceKm = fmod(fTotalDistanceKm, 100.0);
  }

  if (distanceHundredKm > 0)
  {
    tft.setTextSize(1);
    drawInt(distanceHundredKm, (tft.width() / 2) - 2, dist_y - 14);
    tft.setTextSize(2);
  }

  drawFloat(fTotalDistanceKm, 1, dist_x, dist_y);
  drawFloat(fMaxSpeed, 1, max_x, max_y, TR_DATUM);
  drawFloat(fAvgSpeed, 1, avg_x, avg_y, TR_DATUM);

  // Menüye göre saat yazı boyutu
  tft.setTextSize((currentMenu == TIME) ? 2 : 1);

  drawTime(time_x, time_y);
}

void menuStatic(int spd_x, int spd_y, int max_x, int max_y, int time_x, int time_y, int dist_x, int dist_y, int avg_x, int avg_y)
{

  tft.setTextDatum(TL_DATUM);

  // Write static text to screen
  //-----------------------------------------------------
  tft.setTextSize(1);

  tft.drawFastHLine(0, 45, tft.width(), TFT_ORANGE);
  tft.drawFastVLine(64, 0, 45, TFT_ORANGE);
  tft.drawFastHLine(0, 115, tft.width(), TFT_ORANGE);
  tft.drawFastVLine(64, 115, 45, TFT_ORANGE);

  tft.setTextColor(TFT_MAGENTA);

  tft.drawString("TIME", time_x, time_y, txtFont);
  tft.drawString("SPD", spd_x, spd_y, txtFont);
  tft.drawString("DIST", dist_x, dist_y, txtFont);
  tft.drawString("MAX", max_x, max_y, txtFont);
  tft.drawString("AVG", avg_x, avg_y, txtFont);

  //-----------------------------------------------------
  //-----------------------------------------------------
}

void settingsDynamic(int selected)
{
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);

  int step = 20;

  for (int i = 0; i < selected; i++)
  {
    step += 30;
  }

  tft.drawRect(3, step, tft.width() - 6, 30, TFT_ORANGE);
}

void settingsStatic(String text_1, String text_2, String text_3, String text_4, byte textSize)
{

  tft.setTextSize(textSize);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(textColor, backgroundColor);

  if (!text_1.isEmpty())
  {
    tft.drawString(text_1, tft.width() / 2, 35, txtFont);
  }

  if (!text_2.isEmpty())
  {
    tft.drawString(text_2, tft.width() / 2, 65, txtFont);
  }

  if (!text_3.isEmpty())
  {
    tft.drawString(text_3, tft.width() / 2, 95, txtFont);
  }

  if (!text_4.isEmpty())
  {
    tft.drawString(text_4, tft.width() / 2, 125, txtFont);
  }

  draw_gear_icon = false;
  drawImage(gear_icon, gearW, gearH, draw_gear_icon, 4);

  lastShowGearChange = millis();
}

void tryConnection(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Exiting...");
    delay(1500);
    currentMenu = ERR_UPT;
    return;
  }

  ArduinoOTA
      .onStart([]()
               {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
    Serial.printf("Error[%u]: ", error);
    switch (error)
    {
    case OTA_AUTH_ERROR:
      Serial.println("Auth Failed");
      break;
    case OTA_BEGIN_ERROR:
      Serial.println("Begin Failed");
      break;
    case OTA_CONNECT_ERROR:
      Serial.println("Connect Failed");
      break;
    case OTA_RECEIVE_ERROR:
      Serial.println("Receive Failed");
      break;
    case OTA_END_ERROR:
      Serial.println("End Failed");
      break;
    default:
      break;
    currentMenu = ERR_UPT;
    return;
    } });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void fakeTrigger()
{
  if (counter <= 10)
  {
    int randomValue = random(0, 4); // 0 dahil, 6 hariç => 0 ile 5 arasında
    counter += randomValue;
  }
}

void setup()
{

  Serial.begin(115200); // Initialize Serial connection
  Serial.println("Starting booting sequence...");
  EEPROM.begin(EEPROM_SIZE);

  pinMode(HallEffectSensor, INPUT_PULLUP); // Set pin mode for hall effect sensor
  pinMode(buttonRight, INPUT_PULLUP);      // Set pin mode for right button
  pinMode(buttonLeft, INPUT_PULLUP);       // Set pin mode for left button
  pinMode(buttonMid, INPUT_PULLUP);        // Set pin mode for middle button
  delay(100);

  eepromRead();

  tft.init();
  tft.fillScreen(backgroundColor); // Clear the screen
  tft.setRotation(rotation);
  Serial.println("TFT Screen Initialized!"); // Write "TFT Screen Initialized!"
  delay(50);

  attachInterrupt(HallEffectSensor, hallInterrupt, FALLING);
  attachInterrupt(buttonLeft, buttonInterrupt, FALLING);
  attachInterrupt(buttonMid, buttonInterrupt, FALLING);
  attachInterrupt(buttonRight, buttonInterrupt, FALLING);

  displayMenuStatic(currentMenu);
  displayMenuDynamic(currentMenu);
}

void loop()
{
  if (debug)
  {
    fakeTrigger();
  }

  if (millis() - fStartTime >= waitUntil)
  {
    calculate();
  }

  eepromWrite();

  changeMenu();

  changeTheme();

  displayMenuDynamic(currentMenu);

  delay(150);
}