
#include <RadioLib.h>
#include <SPI.h>
#include "globals.h"
#include "SubGHz.h"


enum SUBMODE { 
  SUB_MENU, 
  SUB_RECEIVE,  
  SUB_SAVED 
};

SUBMODE currentSUBMode = SUB_MENU;

int selectSubGHzOption = 0;
int totalSubGHzOptions = 2;
int32_t subGhzRawBuffer[SUBGHZ_MAX_SAMPLES];
size_t subGhzRawCount = 0;
bool subGhzHasCapture = false;
bool subGhzRadioReady = false;


// --- RADIOLIB SETUP --
#define RADIOLIB_PIN_CS   CC1101_CS
#define RADIOLIB_PIN_GDO0 CC1101_GDO0
#define RADIOLIB_PIN_GDO2 -1   

CC1101 radio = CC1101(new Module(RADIOLIB_PIN_CS, RADIOLIB_PIN_GDO0, RADIOLIB_PIN_GDO2));

void resetCC1101() {
    pinMode(CC1101_CS, OUTPUT);
    digitalWrite(CC1101_CS, LOW);
    delay(10);
    digitalWrite(CC1101_CS, HIGH);
    delay(10);
}

void configureCC1101() {
  if (!subGhzRadioReady) {
    Serial.println("CC1101 is not ready; skipping radio configuration");
    return;
  }

  int16_t state = radio.setOOK(true);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Failed to enable OOK mode, code: ");
    Serial.println(state);
  }

  radio.setEncoding(RADIOLIB_ENCODING_NRZ);
  radio.setDataShaping(RADIOLIB_SHAPING_NONE);
  radio.setRxBandwidth(812.0);
  radio.setPromiscuousMode(true);
  radio.setCrcFiltering(false);
}

// Initialize the Sub‑GHz (CC1101/M1101D) module using RadioLib.
void InitSubGHz() {
  Serial.println("Initializing MD1101 (CC1101) with RadioLib...");

  SPI.end();
  SPI.begin(CC1101_SCK, CC1101_MISO, CC1101_MOSI, CC1101_CS);
  pinMode(CC1101_CS, OUTPUT);
  digitalWrite(CC1101_CS, HIGH);
  pinMode(CC1101_GDO0, INPUT);

  subGhzRadioReady = false;
  
  // Begin initialization of the radio.
  int state = radio.begin();
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("MD1101 module detected successfully with RadioLib!");
    subGhzRadioReady = true;
    Serial.print("CC1101 version: 0x");
    Serial.println(radio.getChipVersion(), HEX);
  } else {
    Serial.print("Error initializing MD1101 with RadioLib. Error code: ");
    Serial.println(state);
    Serial.println("Check MD1101D power, ground, CS, SCK, MISO and MOSI wiring.");
    return; // Exit if initialization failed
  }

  if (useSD && !SD.exists("/subghz")) {
    SD.mkdir("/subghz");
  }
  
  // Set the operating frequency
  state = radio.setFrequency(433.92);  // Set the frequency to 433.92 MHz
  if(state == RADIOLIB_ERR_NONE){
      Serial.println("Frequency set to 433.92 MHz");
  } else {
      Serial.print("Error setting frequency, code: ");
      Serial.println(state);
  }

  configureCC1101();
}


// --- MENU AND MODE FUNCTIONS ---
void HanldeSubGHzbutton() {
  unsigned long currentMillis = millis();
  static unsigned long lastButtonPress = 0; // For debouncing
  
  if (readButton(BUTTON_UP) == LOW && currentMillis - lastButtonPress > 200) {
    navigateSubGHzMenu(-1);  // Navigate up
    lastButtonPress = currentMillis;
  }
  
  if (readButton(BUTTON_DOWN) == LOW && currentMillis - lastButtonPress > 200) {
    navigateSubGHzMenu(1);  // Navigate down
    lastButtonPress = currentMillis;
  }
  
  if (readButton(BUTTON_LEFT) == LOW && currentMillis - lastButtonPress > 200) {
    navigateSubGHzLeftRight(-1);  // Navigate left
    lastButtonPress = currentMillis;
  }
  
  if (readButton(BUTTON_RIGHT) == LOW && currentMillis - lastButtonPress > 200) {
    navigateSubGHzLeftRight(1);  // Navigate right
    lastButtonPress = currentMillis;
  }
  
  if (readButton(BUTTON_SELECT) == LOW && currentMillis - lastButtonPress > 200) {
    HandleselectSubGHzOption(selectSubGHzOption);  // Process the selected option
    lastButtonPress = currentMillis;
  }
  
  if (readButton(BUTTON_BACK) == LOW && currentMillis - lastButtonPress > 200) {
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    radio.standby();
    displayStartingMenu();
    lastButtonPress = currentMillis;
  }
}

void navigateSubGHzMenu(int direction) {
  selectSubGHzOption = (selectSubGHzOption + direction + totalSubGHzOptions) % totalSubGHzOptions;
  displaySubGHzMenu();  // Update the display with the new selection.
}


void navigateSubGHzLeftRight(int direction) {
  // Horizontal navigation for keyboard input.
  currentCol = (currentCol + direction + maxCols) % maxCols;
  showSubGHzKeyboard();  // Update the on-screen keyboard.
}

void HandleselectSubGHzOption(int option) {
  Serial.print("Selected Option: ");
  Serial.println(option);  // Debugging print
  
  switch (option) {
    case 0:  // SUB_RECEIVE
      Serial.println("Receiving SubGHz Signal...");
      currentSUBMode = SUB_RECEIVE;
      receiveSubGhzSignal();
      break;
      
    case 1:  // SUB_SAVED
      Serial.println("Opening Saved SubGHz Signals...");
      currentSUBMode = SUB_SAVED;
      savedFilesSubGHz();
      break;
      
    default:
      Serial.println("Unknown Option Selected");
      break;
  }
}

void displaySubGHzMenu() {
  u8g2.clearBuffer();
  u8g2.setCursor(0, 0);

  String menuOptions[] = {"Receive SubGHz Signal", "Saved SubGHz Signals"};
  totalSubGHzOptions = sizeof(menuOptions) / sizeof(menuOptions[0]);

  for (int i = 0; i < totalSubGHzOptions; i++) {
    u8g2.setFont(u8g2_font_ncenB08_tr);
    
    if (i == selectSubGHzOption) { 
      // Highlight selected option
      u8g2.drawBox(0, i * 10, 128, 10);
      u8g2.setDrawColor(0);
    } else {
      u8g2.setDrawColor(1);
    }

    u8g2.setCursor(2, (i + 1) * 10);
    u8g2.print(menuOptions[i]);

    u8g2.setDrawColor(1);  // Reset draw color after highlighting
  }

  u8g2.sendBuffer();
}


volatile int32_t isrRawBuffer[SUBGHZ_MAX_SAMPLES];
volatile size_t isrRawCount = 0;
volatile uint32_t isrLastEdgeUs = 0;
volatile bool isrCapturing = false;

void IRAM_ATTR onSubGhzEdge() {
  if (!isrCapturing) return;
  uint32_t now = micros();
  uint32_t dt = now - isrLastEdgeUs;
  isrLastEdgeUs = now;
  if (isrRawCount < SUBGHZ_MAX_SAMPLES) {
    bool newLevel = digitalRead(CC1101_GDO0);
    bool prevLevel = !newLevel;  // CHANGE interrupt: level just flipped FROM this
    isrRawBuffer[isrRawCount++] = prevLevel ? (int32_t)dt : -(int32_t)dt;
  }
}

void receiveSubGhzSignal() {
  Serial.println("Listening for a raw Sub-GHz signal...");

  if (!subGhzRadioReady) {
    Serial.println("Cannot receive: MD1101D initialization failed");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Radio not found!");
    u8g2.sendBuffer();
    delay(1500);
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  u8g2.print("Listening...");
  u8g2.setCursor(0, 25);
  u8g2.print("(Back to cancel)");
  u8g2.sendBuffer();

  radio.standby();
  configureCC1101();
  int16_t state = radio.receiveDirectAsync();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("receiveDirectAsync() failed, code: ");
    Serial.println(state);
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Radio error!");
    u8g2.sendBuffer();
    delay(1500);
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  isrRawCount = 0;
  isrLastEdgeUs = micros();
  isrCapturing = true;
  pinMode(CC1101_GDO0, INPUT);
  attachInterrupt(digitalPinToInterrupt(CC1101_GDO0), onSubGhzEdge, CHANGE);

  const uint32_t captureWindowMs = 8000;  // time to press the remote button
  uint32_t start = millis();
  while (millis() - start < captureWindowMs && isrRawCount < SUBGHZ_MAX_SAMPLES) {
    if (readButton(BUTTON_BACK) == LOW) break;
    delay(5);
  }

  isrCapturing = false;
  detachInterrupt(digitalPinToInterrupt(CC1101_GDO0));
  radio.standby();

  // Copy out of the volatile ISR buffer into the staged capture.
  subGhzRawCount = isrRawCount;
  for (size_t i = 0; i < subGhzRawCount; i++) {
    subGhzRawBuffer[i] = isrRawBuffer[i];
  }
  // A handful of edges is almost always noise, not a real transmission.
  subGhzHasCapture = subGhzRawCount > 8;

  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  if (subGhzHasCapture) {
    u8g2.print("Captured ");
    u8g2.print((int)subGhzRawCount);
    u8g2.print(" edges");
  } else {
    u8g2.print("Nothing captured");
  }
  u8g2.sendBuffer();
  delay(1500);

  if (subGhzHasCapture) {
    saveOrEmulateSubGhz();
  } else {
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
  }
}

// Presents an option to either save or emulate the just-captured signal.
void saveOrEmulateSubGhz() {
  enum CaptureAction { CAPTURE_SAVE, CAPTURE_TRANSMIT, CAPTURE_CANCEL };
  CaptureAction action = CAPTURE_SAVE;

  while (readButton(BUTTON_SELECT) == LOW || readButton(BUTTON_BACK) == LOW) {
    delay(10);
  }

  while (true) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Capture ready");
    u8g2.drawStr(0, 25, action == CAPTURE_SAVE ? "> Save" : "  Save");
    u8g2.drawStr(0, 38, action == CAPTURE_TRANSMIT ? "> Transmit" : "  Transmit");
    u8g2.drawStr(0, 51, action == CAPTURE_CANCEL ? "> Cancel" : "  Cancel");
    u8g2.sendBuffer();

    if (readButton(BUTTON_UP) == LOW) {
      action = static_cast<CaptureAction>((action + 2) % 3);
      delay(180);
    } else if (readButton(BUTTON_DOWN) == LOW) {
      action = static_cast<CaptureAction>((action + 1) % 3);
      delay(180);
    } else if (readButton(BUTTON_SELECT) == LOW) {
      delay(200);
      break;
    } else if (readButton(BUTTON_BACK) == LOW) {
      action = CAPTURE_CANCEL;
      break;
    }
  }

  if (action == CAPTURE_TRANSMIT) {
    emulateSubGhzSignal();
    return;
  }
  if (action == CAPTURE_CANCEL) {
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  if (!sdAvailable()) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "SD unavailable");
    u8g2.sendBuffer();
    delay(1000);
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  u8g2.print("Enter name");
  u8g2.sendBuffer();
  if (handleUniversalKeyboardInput()) {
    saveSubGhzSignal(currentName);
  } else {
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
  }
}

void transmitSubGhzRaw(const int32_t* raw, size_t len) {
  if (raw == nullptr || len == 0) {
    Serial.println("transmitSubGhzRaw: nothing to send");
    return;
  }

  if (!subGhzRadioReady) {
    Serial.println("Cannot transmit: MD1101D initialization failed");
    return;
  }

  u8g2.clearBuffer();
  u8g2.setCursor(0, 10);
  u8g2.print("Transmitting...");
  u8g2.sendBuffer();

  radio.standby();
  configureCC1101();
  int16_t state = radio.transmitDirectAsync();
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("transmitDirectAsync() failed, code: ");
    Serial.println(state);
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Radio error!");
    u8g2.sendBuffer();
    delay(1500);
    return;
  }

  pinMode(CC1101_GDO0, OUTPUT);
  for (size_t i = 0; i < len; i++) {
    int32_t v = raw[i];
    bool level = v > 0;
    uint32_t us = level ? (uint32_t)v : (uint32_t)(-v);
    digitalWrite(CC1101_GDO0, level ? HIGH : LOW);
    // delayMicroseconds() isn't reliable for very long spans; chunk it.
    while (us > 16000) {
      delayMicroseconds(16000);
      us -= 16000;
    }
    delayMicroseconds(us);
  }
  digitalWrite(CC1101_GDO0, LOW);
  radio.standby();
  pinMode(CC1101_GDO0, INPUT);

  u8g2.clearBuffer();
  u8g2.drawStr(0, 10, "Sent!");
  u8g2.sendBuffer();
  delay(800);
}

// Emulates (transmits) whatever is currently staged in subGhzRawBuffer.
void emulateSubGhzSignal() {
  if (!subGhzHasCapture || subGhzRawCount == 0) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Nothing to send");
    u8g2.sendBuffer();
    delay(1000);
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }
  transmitSubGhzRaw(subGhzRawBuffer, subGhzRawCount);
  currentSUBMode = SUB_MENU;
  currentMode = MENU;
  displayStartingMenu();
}

// Saves the currently staged raw capture as comma-separated signed
// microsecond durations under /subghz/<name>.txt.
void saveSubGhzSignal(String name) {
  if (!subGhzHasCapture || subGhzRawCount == 0) {
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  if (!sdAvailable()) {
    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "SD card unavailable");
    u8g2.sendBuffer();
    delay(1000);
    currentSUBMode = SUB_MENU;
    currentMode = MENU;
    displayStartingMenu();
    return;
  }

  File file = SD.open("/subghz/" + name + ".txt", FILE_WRITE);
  if (file) {
    for (size_t i = 0; i < subGhzRawCount; i++) {
      file.print(subGhzRawBuffer[i]);
      if (i + 1 < subGhzRawCount) file.print(",");
    }
    file.println();
    file.close();
    Serial.println("Saved Sub-GHz raw capture: " + name);
  } else {
    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);
    u8g2.print("Error saving file");
    u8g2.sendBuffer();
    delay(1000);
  }
  currentSUBMode = SUB_MENU;
  currentMode = MENU;
  displayStartingMenu();
}

void savedFilesSubGHz() {
    if (!sdAvailable()) {
      u8g2.clearBuffer();
      u8g2.drawStr(0, 10, "SD card unavailable");
      u8g2.sendBuffer();
      delay(1000);
      currentSUBMode = SUB_MENU;
      currentMode = MENU;
      displayStartingMenu();
      return;
    }

    File root = SD.open("/subghz");  // Only this mode's saved captures
    if (!root) {
      currentSUBMode = SUB_MENU;
      currentMode = MENU;
      displayStartingMenu();
      return;
    }

    u8g2.clearBuffer();
    u8g2.setCursor(0, 0);

    String files[20];  // Array to hold file names
    int fileCount = 0;

    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory() && fileCount < 20) {
          files[fileCount] = entry.name();
          fileCount++;
        }
        entry.close();
        if (fileCount >= 20) break;
    }
    root.close();

    if (fileCount == 0) {
      u8g2.clearBuffer();
      u8g2.setCursor(0, 0);
      u8g2.print("No saved files");
      u8g2.sendBuffer();
      delay(1500);
      currentSUBMode = SUB_MENU;
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
        emulateSavedSubGhz(files[currentFile]);
        displayStartingMenu();
        return;
      }
      if (readButton(BUTTON_BACK) == LOW) {
        currentSUBMode = SUB_MENU;
        currentMode = MENU;
        displayStartingMenu();
        return;
      }
      delay(100);
    }
}

// Loads a saved raw capture back into subGhzRawBuffer and replays it.
void emulateSavedSubGhz(String fileName) {
    String path = fileName.startsWith("/") ? fileName : ("/subghz/" + fileName);
    File file = SD.open(path);
    if (!file) {
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Error reading file");
        u8g2.sendBuffer();
        delay(1000);
        currentSUBMode = SUB_MENU;
        currentMode = MENU;
        return;
    }
    String data = "";
    while (file.available()) data += (char)file.read();
    file.close();

    size_t count = 0;
    int start = 0;
    while (start < (int)data.length() && count < SUBGHZ_MAX_SAMPLES) {
        int comma = data.indexOf(',', start);
        String token = (comma == -1) ? data.substring(start) : data.substring(start, comma);
        token.trim();
        if (token.length() > 0) {
            subGhzRawBuffer[count++] = token.toInt();
        }
        if (comma == -1) break;
        start = comma + 1;
    }
    subGhzRawCount = count;

    if (subGhzRawCount == 0) {
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Empty/bad file");
        u8g2.sendBuffer();
        delay(1000);
        currentSUBMode = SUB_MENU;
        currentMode = MENU;
        return;
    }
    transmitSubGhzRaw(subGhzRawBuffer, subGhzRawCount);
}

void showSubGHzKeyboard() {
  showUniversalKeyboard();
}

void handleSubGHzKeyboardInput() {
  handleUniversalKeyboardInput();
}
