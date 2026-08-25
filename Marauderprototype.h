#ifndef MARAUDER_MODE_H
#define MARAUDER_MODE_H

extern bool marauderModeStarted; 
extern bool targetFound;     

void InitMarauder();
void displaymarauderMenu();
void handleMarauderMode();
void handleMarauderInput();
void selectedMarauderOption();
void exitMarauderMode();
void scanAndSetTarget();
void sendBeaconAttack(uint8_t* targetBSSID);
void createBeaconPacket(uint8_t* packet, const char* ssid);
void deauthAttack(uint8_t* bssid);
void printBSSID(uint8_t* bssid);
void enableMonitorMode();
void startLabAccessPoint();
void handleLabAccessPoint();
void stopLabAccessPoint();

#endif
