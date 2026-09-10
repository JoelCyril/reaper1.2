#include <IRremote.h>
#include "globals.h"
#include "IRprototype.h"


int totalIROption = 3;  
int selectedIROption = 0;  
int IRcurrentCol = 0;

enum IRMODE { 
  IRMENU, 
  IR_RECEIVE, 
  IR_UNIVERSAL, 
  IR_SAVED 
};

IRMODE currentIRMode = IRMENU;

#define IR_SEND_PIN 15

void InitIR() {
 
  IrReceiver.begin(13, ENABLE_LED_FEEDBACK);  
  IrSender.begin(IR_SEND_PIN);      
}

void handleIRMode() {
  switch (currentIRMode) {
    case IR_RECEIVE:
      currentIRMode = IR_RECEIVE;
      receiveIRSignal();  
      break;
    case IR_UNIVERSAL:
      currentIRMode = IR_UNIVERSAL;
      universalRemote();
      break;
    case IR_SAVED:
      currentIRMode = IR_SAVED;
      savedFilesIR();
      break;
    default:
      break;
  }
}

void HandlebuttonIR(){
  static unsigned long lastButtonPress = 0; 
  if (readButton(BUTTON_UP) == LOW && millis() - lastButtonPress > 200) {
    u8g2.clearBuffer();
    navigateIRMenu(-1);  
    lastButtonPress = millis();
    u8g2.sendBuffer();  
  }
  
  
  if (readButton(BUTTON_DOWN) == LOW && millis() - lastButtonPress > 200) {
    u8g2.clearBuffer();
    navigateIRMenu(1);  
    lastButtonPress = millis();
    u8g2.sendBuffer();  
  }
  
  
  if (readButton(BUTTON_LEFT) == LOW && millis() - lastButtonPress > 200) {
    u8g2.clearBuffer();
    navigateIRLeftRight(-1); 
    lastButtonPress = millis();
    u8g2.sendBuffer(); 
  }
  
  
  if (readButton(BUTTON_RIGHT) == LOW && millis() - lastButtonPress > 200) {
    u8g2.clearBuffer();
    navigateIRLeftRight(1);  
    lastButtonPress = millis();
    u8g2.sendBuffer();  
  }
  
  
  if (readButton(BUTTON_SELECT) == LOW && millis() - lastButtonPress > 200) {
    u8g2.clearBuffer();
    currentIRMode = static_cast<IRMODE>(selectedIROption + 1);
    handleIRMode();  
    lastButtonPress = millis();
    u8g2.sendBuffer();  
  }
  
  
  if (readButton(BUTTON_BACK) == LOW && millis() - lastButtonPress > 200) {
    u8g2.clearBuffer();
    exitIRMode();  
    lastButtonPress = millis();
    u8g2.sendBuffer(); 
  }
}

void navigateIRMenu(int direction) {
  selectedIROption = static_cast<IRMODE>((selectedIROption + direction + totalIROption) % totalIROption);
  displayIRMenu(); 

void navigateIRLeftRight(int direction) {
  IRcurrentCol = (IRcurrentCol + direction + maxCols) % maxCols;
  showIRKeyboard();  
}


void exitIRMode() {
  currentMode = MENU;;  
}


void displayIRMenu() {
  u8g2.clearBuffer(); 
  u8g2.setCursor(0, 0);  
  String menuOptions[] = {"Receive IR Signal", "Universal Remote", "Saved Files"};
  totalIROption = sizeof(menuOptions) / sizeof(menuOptions[0]);  
  for (int i = 0; i < totalIROption; i++) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    if (i == selectedIROption) {
      u8g2.drawBox(0, i * 10, 128, 10);
      u8g2.setDrawColor(0);  
    } else {
      u8g2.setDrawColor(1);
    }
    u8g2.setCursor(0, (i + 1) * 10);
    u8g2.print(menuOptions[i]);
  }
  u8g2.sendBuffer();
}

void receiveIRSignal() {
  while (true) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("Receiving IR Signal...");
    u8g2.sendBuffer();

    if (IrReceiver.decode()) {
      uint32_t capturedCode = IrReceiver.decodedIRData.decodedRawData;
      Serial.println("IR Signal received: " + String(capturedCode, HEX));
      IrReceiver.resume();
      
      u8g2.clearBuffer();
      u8g2.setCursor(0, 0);
      u8g2.print("Save or Emulate?");
      u8g2.sendBuffer();

      bool decisionMade = false;
      while (!decisionMade) {
        if (readButton(BUTTON_SELECT) == LOW) {
          if (!sdAvailable()) {
            u8g2.clearBuffer();
            u8g2.drawStr(0, 20, "SD unavailable");
            u8g2.sendBuffer();
            delay(1000);
            decisionMade = true;
            break;
          }
          u8g2.clearBuffer();
          u8g2.setCursor(0, 0);
          u8g2.print("Enter name:");
          u8g2.sendBuffer();
          handleIRKeyboardInput();
          if (currentName.length() > 0) {
            saveIRSignal(currentName, capturedCode);
          }
          decisionMade = true;
        }
        if (readButton(BUTTON_BACK) == LOW) {
          u8g2.clearBuffer();
          u8g2.setCursor(0, 0);
          u8g2.print("Emulating...");
          IrSender.sendNEC(capturedCode, 32);
          u8g2.sendBuffer();
          delay(1000);
          decisionMade = true;
        }
      }
    } else {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 0);
      u8g2.print("No IR Signal");
      u8g2.sendBuffer();
      delay(1000);
    }


    if (readButton(BUTTON_BACK) == LOW) {
      return;
    }
  }
}

// Save IR Signal
void saveIRSignal(String name, uint32_t signalData) {
  if (!sdAvailable()) {
    Serial.println("SD card unavailable; IR signal was not saved.");
    return;
  }
  File file = SD.open("/ir/" + name + ".txt", FILE_WRITE);
  if (file) {
    file.println(String(signalData, HEX));
    file.close();
  } else {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("Error saving file");
    u8g2.sendBuffer();
    delay(1000);  // Show the error for a while
  }
  displayStartingMenu();
}

// Saved Files IR
void savedFilesIR() {
  if (!sdAvailable()) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "SD unavailable");
    u8g2.sendBuffer();
    delay(1000);
    currentMode = MENU;
    return;
  }
  File root = SD.open("/ir");  
  u8g2.clearBuffer();
  u8g2.setCursor(0, 0);
  String files[20];  
  int fileCount = 0;
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      break; 
    }
    if (entry.isDirectory()) {
      continue;  
    }
    files[fileCount] = entry.name();  
    fileCount++;
    if (fileCount >= 20) {
      break;
    }
    entry.close();
  }
  if (fileCount == 0) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("No saved files");
    u8g2.sendBuffer();
    delay(1500);
    displayStartingMenu();
    return;
  }

  int currentFile = 0;  
  while (true) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("File: ");
    u8g2.print(files[currentFile]);
    u8g2.sendBuffer();
    if (readButton(BUTTON_UP) == LOW) {
      currentFile = (currentFile - 1 + fileCount) % fileCount;  
      delay(200);
    }
    if (readButton(BUTTON_DOWN) == LOW) {
      currentFile = (currentFile + 1) % fileCount; 
      delay(200);
    }
    if (readButton(BUTTON_SELECT) == LOW) {
      emulateSavedSignal(files[currentFile]);
      return;  
    }
    if (readButton(BUTTON_BACK) == LOW) {
      displayStartingMenu();  
      return;
    }
    delay(100);  
  }
}

void emulateSavedSignal(String fileName) {
  String path = fileName.startsWith("/") ? fileName : ("/ir/" + fileName);
  File file = SD.open(path);
  if (file) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("Emulating: ");
    u8g2.print(fileName);
    u8g2.sendBuffer();
    String signalData = "";
    while (file.available()) {
      signalData += (char)file.read();
    }
    file.close();
    uint32_t signal = strtoul(signalData.c_str(), NULL, 16);  
    IrSender.sendNEC(signal, 32);
    delay(1000);  
  } else {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("Error reading file");
    u8g2.sendBuffer();
    delay(1000); 
  }
}


void universalRemote() {
  String signals[] = {"TV", "DVD", "AC", "Fan"};
  int selectedSignal = 0;
  while (true) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    for (int i = 0; i < 4; i++) {
      if (i == selectedSignal) {
        u8g2.setDrawColor(1);
      } else {
        u8g2.setDrawColor(0);
      }
      u8g2.print(signals[i]);
      u8g2.setCursor(0, (i + 1) * 10);
    }
    u8g2.sendBuffer();
    if (readButton(BUTTON_UP) == LOW) {
      selectedSignal = (selectedSignal - 1 + 4) % 4;
      delay(200);
    }
    if (readButton(BUTTON_DOWN) == LOW) {
      selectedSignal = (selectedSignal + 1) % 4;
      delay(200);
    }
    if (readButton(BUTTON_SELECT) == LOW) {
      if (selectedSignal == 0) {
        IrSender.sendNEC(0x20DF10EF, 32);  
      }
      break;
    }
    if (readButton(BUTTON_BACK) == LOW) {
      displayStartingMenu();
      break;
    }
  }
}

void showIRKeyboard() {
  showUniversalKeyboard();
}

void handleIRKeyboardInput() {
  handleUniversalKeyboardInput();
}