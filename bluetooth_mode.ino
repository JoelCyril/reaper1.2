#include <BleKeyboard.h>
#include <BleMouse.h>
#include "globals.h"
#include "Bluetooth.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include "esp_bt.h"
#include "esp_bt_main.h"

BleKeyboard bleKeyboard("Reaper One", "Espressif", 100);
BleMouse bleMouse("Reaper One", "Espressif", 100);
BLEScan* pBLEScan = nullptr;  
BLEAdvertisedDevice* selectedDevice = nullptr;
bool bluetoothModeRunning = false;
bool isKeyboardMode = false;
bool isMouseMode = false;
int selectBLEOption = 0;
int totalBLEOption = 3;
bool deviceConnected = false;
bool bleMemoryReleased = false;  

enum BLEMODE {
    BLEMENU,
    DEVICE_LIST,  
    KEYBOARD,
    MOUSE
};

BLEMODE currentBLEMode = BLEMENU;
std::vector<BLEAdvertisedDevice> foundDevices;
int selectedDeviceIndex = 0;

void InitBluetooth() {
  if (bleMemoryReleased) {
    Serial.println("BLE memory was released for WiFi use; restart the device to use Bluetooth again.");
    return;
  }
  if (bluetoothStarted) return;  

  WiFi.mode(WIFI_OFF);  

  BLEDevice::init("Reaper One");
  bluetoothStarted = true;
  Serial.println("BLE initialized.");
}

void releaseBleMemoryForWifi() {
  if (bleMemoryReleased) return;
  if (!bluetoothStarted) return;

  Serial.print("Free heap before BLE release: ");
  Serial.println(ESP.getFreeHeap());

  bleKeyboard.end();
  bleMouse.end();
  BLEDevice::deinit(false);
  esp_bluedroid_disable();
  esp_bluedroid_deinit();
  esp_bt_controller_disable();
  unsigned long start = millis();
  while (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED && millis() - start < 2000) {
    delay(20);
  }
  if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED) {
    esp_bt_controller_deinit();
  }
  esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);

  bluetoothStarted = false;
  bleMemoryReleased = true;

  Serial.print("Free heap after BLE release: ");
  Serial.println(ESP.getFreeHeap());
}

void handleBLEMode() {
    if (bleMemoryReleased) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 16, "BLE released its");
        u8g2.drawStr(0, 28, "RAM for WiFi use.");
        u8g2.drawStr(0, 44, "Restart to reuse.");
        u8g2.sendBuffer();
        return;
    }
    switch (currentBLEMode) {
        case BLEMENU:
            displayBLEMenu();
            break;
        case DEVICE_LIST:
            displayDeviceList();
            break;
        case KEYBOARD:
            handleBLEKeyboardInput();
            break;
        case MOUSE:
            handleMouseInput();
            break;
        default:
            break;
    }
}

// Confirms whatever is highlighted in the top-level BLE menu
void selectBLEMenuOption() {
    switch (selectBLEOption) {
        case 0: // Scan & Connect
            scanForDevices();  // leaves currentBLEMode on DEVICE_LIST or BLEMENU
            break;
        case 1: // Keyboard
            bleMouse.end();
            bleKeyboard.begin();
            Serial.println("Keyboard advertising as Reaper One");
            currentRow = 0;
            currentCol = 0;
            currentBLEMode = KEYBOARD;
            break;
        case 2: // Mouse
            bleKeyboard.end();
            bleMouse.begin();
            Serial.println("Mouse advertising as Reaper One");
            currentBLEMode = MOUSE;
            break;
        default:
            break;
    }
}

void HandlebuttonBLE() {
  static unsigned long lastButtonPress = 0;

  if (bleMemoryReleased) {
    if (readButton(BUTTON_BACK) == LOW && millis() - lastButtonPress > 200) {
      currentMode = MENU;
      lastButtonPress = millis();
    }
    return;
  }

  if (currentBLEMode == KEYBOARD || currentBLEMode == MOUSE) return;

  if (millis() - lastButtonPress < 200) return;

  switch (currentBLEMode) {
    case BLEMENU:
      if (readButton(BUTTON_UP) == LOW) {
        navigateBLEMenu(-1);
        lastButtonPress = millis();
      } else if (readButton(BUTTON_DOWN) == LOW) {
        navigateBLEMenu(1);
        lastButtonPress = millis();
      } else if (readButton(BUTTON_SELECT) == LOW) {
        selectBLEMenuOption();
        lastButtonPress = millis();
      } else if (readButton(BUTTON_BACK) == LOW) {
        exitBLEMode();
        lastButtonPress = millis();
      }
      break;

    case DEVICE_LIST:
      if (readButton(BUTTON_UP) == LOW && !foundDevices.empty()) {
        selectedDeviceIndex = (selectedDeviceIndex - 1 + foundDevices.size()) % foundDevices.size();
        lastButtonPress = millis();
      } else if (readButton(BUTTON_DOWN) == LOW && !foundDevices.empty()) {
        selectedDeviceIndex = (selectedDeviceIndex + 1) % foundDevices.size();
        lastButtonPress = millis();
      } else if (readButton(BUTTON_SELECT) == LOW) {
        connectToDevice();
        lastButtonPress = millis();
      } else if (readButton(BUTTON_BACK) == LOW) {
        currentBLEMode = BLEMENU;
        lastButtonPress = millis();
      }
      break;

    default:
      break;
  }
}

void exitBLEMode() {

    bleKeyboard.end();
    bleMouse.end();
    currentMode = MENU;
}

void navigateBLEMenu(int direction) {
    selectBLEOption = (selectBLEOption + direction + totalBLEOption) % totalBLEOption;
    displayBLEMenu();
}

void displayDeviceList() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);

    if (foundDevices.empty()) {
        u8g2.drawStr(0, 20, "No devices found");
        u8g2.sendBuffer();
        return;
    }

    for (int i = 0; i < (int)foundDevices.size(); i++) {
        if (i == selectedDeviceIndex) {
            u8g2.drawBox(0, i * 10, 128, 10);
            u8g2.setDrawColor(0);
        } else {
            u8g2.setDrawColor(1);
        }
        u8g2.setCursor(0, (i + 1) * 10);
        u8g2.print(foundDevices[i].getName().c_str());
    }
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
}

void displayBLEMenu() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    String menuOptions[] = {"Scan & Connect", "Keyboard", "Mouse"};
    totalBLEOption = sizeof(menuOptions) / sizeof(menuOptions[0]);

    for (int i = 0; i < totalBLEOption; i++) {
        if (i == selectBLEOption) {
            u8g2.drawBox(0, i * 10, 128, 10);
            u8g2.setDrawColor(0);
        } else {
            u8g2.setDrawColor(1);
        }
        u8g2.setCursor(0, (i + 1) * 10);
        u8g2.print(menuOptions[i]);
    }
    u8g2.setDrawColor(1);
    u8g2.sendBuffer();
}

void scanForDevices() {
    Serial.println("Scanning for BLE devices...");

    foundDevices.clear();

    if (!pBLEScan) {
        pBLEScan = BLEDevice::getScan();
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(100);
        pBLEScan->setWindow(99);
    }

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 20, "Scanning...");
    u8g2.sendBuffer();

    BLEScanResults* scanResults = pBLEScan->start(3, false);  // 3 second scan
    int deviceCount = scanResults->getCount();

    Serial.print("Devices found: ");
    Serial.println(deviceCount);

    for (int i = 0; i < deviceCount; i++) {
        BLEAdvertisedDevice device = scanResults->getDevice(i);
        if (device.getName().length() > 0) {
            Serial.print("Device ");
            Serial.print(i);
            Serial.print(": ");
            Serial.println(device.getName().c_str());
            foundDevices.push_back(device);
        }
    }

    pBLEScan->clearResults();  // free scan memory

    if (foundDevices.empty()) {
        Serial.println("No named devices found.");
        u8g2.clearBuffer();
        u8g2.drawStr(0, 20, "No devices found");
        u8g2.sendBuffer();
        delay(1000);
        currentBLEMode = BLEMENU;
        return;
    }

    selectedDeviceIndex = 0;
    currentBLEMode = DEVICE_LIST;
}

void connectToDevice() {
    if (foundDevices.empty() || selectedDeviceIndex >= (int)foundDevices.size()) return;
    selectedDevice = &foundDevices[selectedDeviceIndex];
    deviceConnected = true;
    Serial.print("Selected device: ");
    Serial.println(selectedDevice->getName().c_str());
    currentBLEMode = BLEMENU;
    displayBLEMenu();
}

void handleBLEKeyboardInput() {
  static unsigned long lastButtonPress = 0;

  if (!bleKeyboard.isConnected()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 20, "Waiting for PC");
    u8g2.drawStr(0, 32, "to pair...");
    u8g2.sendBuffer();
    if (readButton(BUTTON_BACK) == LOW && millis() - lastButtonPress > 200) {
      stopBluetoothMode();
      lastButtonPress = millis();
    }
    return;
  }

  displayKeyboardLayout();

  if (millis() - lastButtonPress < 150) return;

  if (readButton(BUTTON_UP) == LOW) {
    currentRow = max(currentRow - 1, 0);
    lastButtonPress = millis();
  } else if (readButton(BUTTON_DOWN) == LOW) {
    currentRow = min(currentRow + 1, maxRows - 1);
    lastButtonPress = millis();
  } else if (readButton(BUTTON_LEFT) == LOW) {
    currentCol = max(currentCol - 1, 0);
    lastButtonPress = millis();
  } else if (readButton(BUTTON_RIGHT) == LOW) {
    currentCol = min(currentCol + 1, maxCols - 1);
    lastButtonPress = millis();
  } else if (readButton(BUTTON_SELECT) == LOW) {
    processKeyboardInput();
    lastButtonPress = millis();
  } else if (readButton(BUTTON_BACK) == LOW) {
    stopBluetoothMode();
    lastButtonPress = millis();
  }
}

void displayKeyboardLayout() {
  showUniversalKeyboard();
}

void processKeyboardInput() {
  if (!bleKeyboard.isConnected()) return;

  const char* selectedKey = keyboard[currentRow][currentCol];
  if (selectedKey[0] == '\0') return;  

  Serial.print("Sending key: ");
  Serial.println(selectedKey);

  if (strcmp(selectedKey, "^") == 0) {
    bleKeyboard.press(KEY_RETURN);
  } else if (strcmp(selectedKey, "<") == 0) {
    bleKeyboard.press(KEY_BACKSPACE);
  } else if (strcmp(selectedKey, "_") == 0) {
    bleKeyboard.press(' ');
  } else if (strcmp(selectedKey, "OK") == 0) {
    bleKeyboard.press(KEY_ESC);
  } else {
    bleKeyboard.press(selectedKey[0]);
  }

  delay(50);
  bleKeyboard.releaseAll();
}

void handleMouseInput() {
  static unsigned long lastButtonPress = 0;

  if (!bleMouse.isConnected()) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 20, "Waiting for PC");
    u8g2.drawStr(0, 32, "to pair...");
    u8g2.sendBuffer();
    if (readButton(BUTTON_BACK) == LOW && millis() - lastButtonPress > 200) {
      stopBluetoothMode();
      lastButtonPress = millis();
    }
    return;
  }

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 20, "Mouse Mode");
  u8g2.drawStr(0, 32, "D-pad = move");
  u8g2.drawStr(0, 44, "Select = click");
  u8g2.sendBuffer();

  const int step = 10;
  if (readButton(BUTTON_UP) == LOW) bleMouse.move(0, -step);
  if (readButton(BUTTON_DOWN) == LOW) bleMouse.move(0, step);
  if (readButton(BUTTON_LEFT) == LOW) bleMouse.move(-step, 0);
  if (readButton(BUTTON_RIGHT) == LOW) bleMouse.move(step, 0);

  if (readButton(BUTTON_SELECT) == LOW && millis() - lastButtonPress > 200) {
    bleMouse.click(MOUSE_LEFT);
    lastButtonPress = millis();
  }
  if (readButton(BUTTON_BACK) == LOW && millis() - lastButtonPress > 200) {
    stopBluetoothMode();
    lastButtonPress = millis();
  }

  delay(15); 
}

void stopBluetoothMode() {
  bluetoothModeRunning = false;
  isKeyboardMode = false;
  isMouseMode = false;
  deviceConnected = false;

  u8g2.clearBuffer();
  u8g2.setCursor(0, 0);
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 20, "Bluetooth Stopped");
  u8g2.sendBuffer();
  delay(600);

  bleKeyboard.end();
  bleMouse.end();
  currentBLEMode = BLEMENU;
  selectBLEOption = 0;
}
