#ifndef SUBGHZ_MODE_H
#define SUBGHZ_MODE_H

#include <Arduino.h>

// Function Prototype
void displayStartingMenu();
void handleSubGHzKeyboardInput();
void showSubGHzKeyboard();
void savedFilesSubGHz();
void saveSubGhzSignal(String name);
void emulateSubGhzSignal();
void emulateSavedSubGhz(String fileName);
void transmitSubGhzRaw(const int32_t* raw, size_t len);
void saveOrEmulateSubGhz();
void receiveSubGhzSignal();
void displaySubGHzMenu();
void HandleselectSubGHzOption(int option);
void navigateSubGHzLeftRight(int direction);
void navigateSubGHzMenu(int direction);
void InitSubGHz();
void configureCC1101();

#define CC1101_CS 2 
#define CC1101_MOSI 23
#define CC1101_MISO 19
#define CC1101_SCK 18
#define CC1101_GDO0 4
#define SUBGHZ_MAX_SAMPLES 600
extern int32_t subGhzRawBuffer[SUBGHZ_MAX_SAMPLES];
extern size_t subGhzRawCount;
extern bool subGhzHasCapture;
extern bool subGhzRadioReady;

extern int selectSubGHzOption;  
extern int totalSubGHzOptions; 

#endif
