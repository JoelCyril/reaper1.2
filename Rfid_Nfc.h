#ifndef RFID_MODULE_H
#define RFID_MODULE_H

#include <Adafruit_PN532.h>
// Function Prototypes
void showKeyboard();
void displayTagDetails(uint8_t *uid, uint8_t uidLength);
void Initnfc();
void handleNFCMode();
void saveTag(uint8_t* uid, uint8_t uidLength, String currentName);
void writeTag(uint8_t* uid, uint8_t uidLength);
void displayRFIDMenu();
bool waitForButton(int buttonPin, unsigned long timeout = 5000);
void partialEmulate(uint8_t *tagData, uint8_t tagLength);
void showRfidKeyboard();
void handleRfidKeyboardInput();
void savedFilesRfid();
void emulateSavedTag(String fileName);
void navigateRfidLeftRight(int direction);
void navigateRfidMenu(int direction);
void receiveRfidSignal();
void HandlebuttonNFC();
void selectNFCOption();

#define SDA_PIN 21
#define SCL_PIN 22
#define PN532_IRQ 25
#define PN532_RESET 33


#endif 
#pragma once
