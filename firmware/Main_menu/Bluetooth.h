#ifndef BLE_MODE_H
#define BLE_MODE_H

#include <BleKeyboard.h>
#include <BleMouse.h>

// Function Prototypes
void InitBluetooth();
void handleBLEMode();
void HandlebuttonBLE();
void exitBLEMode();
void navigateBLEMenu(int direction);
void selectBLEMenuOption();
void displayBLEMenu();
void scanForDevices();
void displayDeviceList();
void connectToDevice();
void handleBLEKeyboardInput();
void displayKeyboardLayout();
void processKeyboardInput();
void handleMouseInput();
void stopBluetoothMode();

extern bool bluetoothModeRunning;
extern bool isKeyboardMode;
extern bool isMouseMode;
extern int selectBLEOption;
extern int totalBLEOption;

extern BleKeyboard bleKeyboard;
extern BleMouse bleMouse;

extern bool bluetoothStarted;
extern bool bleMemoryReleased;
void releaseBleMemoryForWifi();

#endif 
