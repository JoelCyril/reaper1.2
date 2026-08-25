#ifndef GLOBALS_H
#define GLOBALS_H

#include "FS.h"
#include "SD.h"
#include "Wire.h"
#include <U8g2lib.h>

extern U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;

extern void exitMode();
extern const int BUTTON_SELECT;   
extern const int BUTTON_BACK;  
extern const int BUTTON_UP;
extern const int BUTTON_DOWN;
extern const int BUTTON_LEFT;
extern const int BUTTON_RIGHT;
extern const int ANALOG_BUTTONS_PIN;
int readButton(int buttonId);

extern String currentName;
extern const char* keyboard[4][10];
extern int currentRow;
extern int currentCol;
extern const int maxRows;
extern const int maxCols;
extern bool useSD;
bool sdAvailable();
void showUniversalKeyboard();
bool handleUniversalKeyboardInput();

enum Mode { MENU, IR_MODE, GAME_MODE, SUBGHZ_MODE, NFC_MODE, BLUETOOTH_MODE, MARAUDER_MODE };
extern Mode currentMode;

#endif
