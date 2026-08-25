//libraries
#include <Adafruit_PN532.h>
#include "globals.h"
#include "Rfid_Nfc.h"

int selectRfidOption = 0;
int totalRfidOption = 2;
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET); 
bool pn532_initialized = false;
const int maxRfidOptions = 2; 
int menuIndex = 0;
uint8_t savedUID[10];
uint8_t savedUIDLength = 0;


enum NFCMODE { 
  NFC_MENU, 
  NFC_RECEIVE,  
  NFC_SAVED 
};

NFCMODE currentNFCMode = NFC_MENU;

void Initnfc() {
  Wire.begin(SDA_PIN, SCL_PIN);
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 not found! Check wiring / that it's jumpered to I2C mode.");
    pn532_initialized = false;
    return;
  }
  Serial.print("Found PN532, firmware v");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);

  nfc.SAMConfig();
  pn532_initialized = true;

  currentNFCMode = NFC_MENU;
}



int parseUID(String uidData, uint8_t *uid, int maxLength) {
  int index = 0;
  while (index < (int)uidData.length() && length < maxLength) {
    if (uidData[index] == ' ') {
      index++;
      continue;
    }
    if (index + 1 >= (int)uidData.length()) break; 
    char hexByte[3] = { uidData[index], uidData[index+1], '\0' };
    uid[length] = strtol(hexByte, NULL, 16);
    length++;
    index += 2;
  }
  return length;
}

void handleNFCMode() {
  Serial.print("Current NFC Mode: ");
  Serial.println(currentNFCMode);

  if (currentNFCMode == NFC_MENU) {
    displayRFIDMenu();
    HandlebuttonNFC();  
    return;
  }

  switch (currentNFCMode) {
    case NFC_RECEIVE:
      Serial.println("Receiving RFID Signal...");
      receiveRfidSignal();
      break;
    case NFC_SAVED:
      Serial.println("Showing Saved Files...");
      savedFilesRfid();
      break;
    default:
      Serial.println("Unknown State");
      break;
  }
}



void HandlebuttonNFC() {
    if (currentNFCMode != NFC_MENU) return;

    static unsigned long lastButtonPress = 0;
    unsigned long currentMillis = millis();
    if (currentMillis - lastButtonPress < 200) return;

    if (readButton(BUTTON_SELECT) == LOW) {  
        delay(50); 
        if (readButton(BUTTON_SELECT) == LOW) {  
            Serial.println("SELECT Button Pressed!");
            selectNFCOption();
            lastButtonPress = millis();
        }
    }

    if (readButton(BUTTON_UP) == LOW) {  
        delay(50);
        if (readButton(BUTTON_UP) == LOW) {
            Serial.println("UP Button Pressed!");
            navigateRfidMenu(-1);
            lastButtonPress = millis();
        }
    }

    if (readButton(BUTTON_DOWN) == LOW) {  
        delay(50);
        if (readButton(BUTTON_DOWN) == LOW) {
            Serial.println("DOWN Button Pressed!");
            navigateRfidMenu(1);
            lastButtonPress = millis();
        }
    }

    if (readButton(BUTTON_LEFT) == LOW) {  
        delay(50);
        if (readButton(BUTTON_LEFT) == LOW) {
            Serial.println("LEFT Button Pressed!");
            navigateRfidLeftRight(-1);
            lastButtonPress = millis();
        }
    }

    if (readButton(BUTTON_RIGHT) == LOW) {  
        delay(50);
        if (readButton(BUTTON_RIGHT) == LOW) {
            Serial.println("RIGHT Button Pressed!");
            navigateRfidLeftRight(1);
            lastButtonPress = millis();
        }
    }

    if (readButton(BUTTON_BACK) == LOW) {  
        delay(50);
        if (readButton(BUTTON_BACK) == LOW) {
            Serial.println("BACK Button Pressed!");
            exitNFCMode();
            lastButtonPress = millis();
        }
    }
}





bool returnToNFCMenu = false;  

void selectNFCOption() {
    while (true) {  
        if (readButton(BUTTON_BACK) == LOW) {  
            return; 
        }

        switch (selectRfidOption) {
            case 0: 
                returnToNFCMenu = false;
                currentNFCMode = NFC_RECEIVE;
                receiveRfidSignal();
                if (returnToNFCMenu) return;
                break;
            case 1:                
                returnToNFCMenu = false;
                currentNFCMode = NFC_SAVED;
                savedFilesRfid();
                return; 
            default: 
                Serial.println("Unknown Option");
                break;
        }
    }
}

void navigateRfidMenu(int direction) {
    selectRfidOption += direction;
    if (selectRfidOption < 0) selectRfidOption = maxRfidOptions - 1;
    if (selectRfidOption >= maxRfidOptions) selectRfidOption = 0;

    Serial.print("Current selection: ");
    Serial.println(selectRfidOption);
    
    displayRFIDMenu();
}


void navigateRfidLeftRight(int direction) {

  currentCol = (currentCol + direction + maxCols) % maxCols;
  showRfidKeyboard(); 


void exitNFCMode() {
  currentMode = MENU; 
}

void exitRfidMode() {
  currentNFCMode = NFC_MENU;
  currentMode = MENU;
}


void displayRFIDMenu() {
  Serial.println("Displaying NFC Menu..."); 
  u8g2.clearBuffer();
  u8g2.setCursor(0, 0);

  String menuOptions[] = {"Receive Rfid Signal", "Saved Files Rfid"};
  totalRfidOption = sizeof(menuOptions) / sizeof(menuOptions[0]);

  for (int i = 0; i < totalRfidOption; i++) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    if (i == selectRfidOption) {
      u8g2.drawBox(0, i * 10, 128, 10);
      u8g2.setDrawColor(0);
    } else {
      u8g2.setDrawColor(1);
    }
    u8g2.setCursor(2, (i + 1) * 10);
    u8g2.print(menuOptions[i]);
  }
  u8g2.sendBuffer();
}



// =========================== RFID RECEIVE FUNCTION ================================

void receiveRfidSignal() {
  if (!pn532_initialized) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.print("NFC reader not");
    u8g2.setCursor(0, 24);
    u8g2.print("found at boot!");
    u8g2.setCursor(0, 40);
    u8g2.print("Check wiring/I2C");
    u8g2.sendBuffer();
    delay(2000);
    currentNFCMode = NFC_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  Serial.println("Waiting for RFID tag...");
  
  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  u8g2.print("Waiting for Tag...");
  u8g2.setCursor(0, 24);
  u8g2.print("(Back to cancel)");
  u8g2.sendBuffer();
  
  uint8_t uid[7];  // Stores the UID of the tag (max size is 7 bytes)
  uint8_t uidLength = 0;
  uint8_t tagData[16];
  uint8_t tagLength = 16; 

  bool tagFound = false;
  while (!tagFound) {
    if (readButton(BUTTON_BACK) == LOW) {
      exitRfidMode();
      delay(200);
      return;
    }
    tagFound = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200);
  }

  if (tagFound) {
    Serial.print("Tag detected: ");
    String uidString = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        Serial.print(uid[i], HEX);
        Serial.print(" ");
        uidString += String(uid[i], HEX) + " ";
    }
    Serial.println();

    u8g2.clearBuffer();
    u8g2.setCursor(0, 10);
    u8g2.print("Tag Detected!");
    u8g2.setCursor(0, 25);
    u8g2.print("UID: " + uidString);
    u8g2.sendBuffer();
    delay(2000);  

    int menuIndex = 0;
    while (true) {  
      u8g2.clearBuffer();
      u8g2.setCursor(0, 10);
      u8g2.print("Select Option:");
      u8g2.setCursor(0, 25);
      u8g2.print(menuIndex == 0 ? "> Save Tag" : "  Save Tag");
      u8g2.setCursor(0, 35);
      u8g2.print(menuIndex == 1 ? "> Write Tag" : "  Write Tag");
      u8g2.setCursor(0, 45);
      u8g2.print(menuIndex == 2 ? "> Emulate" : "  Emulate");
      u8g2.sendBuffer();

      if (readButton(BUTTON_UP) == LOW) {
          menuIndex = (menuIndex == 0) ? 2 : menuIndex - 1;  
          delay(200);
      }
      if (readButton(BUTTON_DOWN) == LOW) {
          menuIndex = (menuIndex == 2) ? 0 : menuIndex + 1;
          delay(200);
      }
      if (readButton(BUTTON_SELECT) == LOW) {
          u8g2.clearBuffer();
          u8g2.setCursor(0, 10);
          u8g2.print("Processing...");
          u8g2.sendBuffer();
          delay(500);

          switch (menuIndex) {
              case 0: 
                  if (!sdAvailable()) {
                    Serial.println("SD card unavailable; RFID tag was not saved.");
                    break;
                  }
                  handleRfidKeyboardInput(); 
                  saveTag(uid, uidLength, currentName); 
                  break;
              case 1: 
                  writeTag(uid, uidLength); 
                  break;
              case 2: 
                  partialEmulate(tagData, tagLength); 
                  break;
          }
      }

      if (readButton(BUTTON_BACK) == LOW) {
          Serial.println("Back Button Pressed! Returning to NFC Menu.");
          exitRfidMode();
          delay(100);  
          return; 
      }
    }
  }
}


//============================================= Write Tag ===============================================

void writeTag(uint8_t *uid, uint8_t uidLength) {
    if (!pn532_initialized) {
      Serial.println("PN532 is not initialized.");
      return;
    }

    if (uid == nullptr || uidLength == 0) {
        Serial.println("No tag data available!");
        u8g2.clearBuffer();
        u8g2.setCursor(10, 20);
        u8g2.print("No tag data!");
        u8g2.sendBuffer();
        delay(1000);
        return;
    }

    Serial.println("Bring a new tag closer...");

    u8g2.clearBuffer();
    u8g2.setCursor(10, 20);
    u8g2.print("Bring new tag...");
    u8g2.sendBuffer();
    delay(2000);

    Serial.println("Writing a test block to the detected MIFARE Classic tag...");

    u8g2.clearBuffer();
    u8g2.setCursor(10, 20);
    u8g2.print("Writing...");
    u8g2.sendBuffer();

    uint8_t testData[16] = {
      'M', 'A', 'R', 'A', 'U', 'D', 'E', 'R',
      '_', 'T', 'E', 'S', 'T', 0x00, 0x00, 0x00
    };

    uint8_t keyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    bool authenticated = nfc.mifareclassic_AuthenticateBlock(
      uid, uidLength, 4, 0, keyA);
    if (authenticated && nfc.mifareclassic_WriteDataBlock(4, testData)) {
        Serial.println("Write successful!");
        
        u8g2.clearBuffer();
        u8g2.setCursor(10, 20);
        u8g2.print("Write Done!");
        u8g2.sendBuffer();
        delay(2000);
    } else {
        Serial.println("Write failed!");
        
        u8g2.clearBuffer();
        u8g2.setCursor(10, 20);
        u8g2.print("Write Failed!");
        u8g2.sendBuffer();
        delay(2000);
    }
}

//================================================Save Tag================================================

void saveTag(uint8_t* uid, uint8_t uidLength, String currentName) {
  if (!sdAvailable()) {
    Serial.println("SD card unavailable; tag was not saved.");
    return;
  }

    File file = SD.open("/rfid/" + currentName + ".txt", FILE_WRITE);
    if (file) {
        for (uint8_t i = 0; i < uidLength; i++) {
            file.print(uid[i], HEX);
            file.print(" ");
        }
        file.println();
        file.close();
        Serial.println("RFID tag saved successfully.");
    } else {
        Serial.println("Error opening file for writing.");
    }
}

void partialEmulate(uint8_t *tagData, uint8_t tagLength) {
    (void)tagData;
    (void)tagLength;
    Serial.println("PN532 tag emulation is not supported by this implementation.");
    u8g2.clearBuffer();
    u8g2.setCursor(0, 30);
    u8g2.print("Emulation unsupported");
    u8g2.sendBuffer();
    delay(1500);
}


//=====================================================open Savde files======================================
void savedFilesRfid() {
  if (!sdAvailable()) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("SD card unavailable");
    u8g2.sendBuffer();
    delay(1000);
    currentNFCMode = NFC_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  File root = SD.open("/rfid");  
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
      entry.close();
      continue; 
    }

    files[fileCount] = entry.name();
    fileCount++;

    if (fileCount >= 20) { 
      entry.close();
      break;
    }

    entry.close();
  }
  root.close();

  if (fileCount == 0) {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("No saved files");
    u8g2.sendBuffer();
    delay(1500);
    currentNFCMode = NFC_MENU;
    currentMode = MENU;
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
      emulateSavedTag(files[currentFile]);
      return;
    }

    if (readButton(BUTTON_BACK) == LOW) {
      currentNFCMode = NFC_MENU;
      currentMode = MENU;
      displayStartingMenu();  
      return;
    }

    delay(100); 
  }
}

//================================================emulate saved tag===========================================
void emulateSavedTag(String fileName) {
  (void)fileName;
  partialEmulate(nullptr, 0);
  currentNFCMode = NFC_MENU;
  currentMode = MENU;
  displayStartingMenu();
}
//=============================================Keyboard===============================================
void showRfidKeyboard() {
  showUniversalKeyboard();
}

void handleRfidKeyboardInput() {
  handleUniversalKeyboardInput();
}