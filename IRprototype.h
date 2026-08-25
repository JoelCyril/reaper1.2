#ifndef IRPROTOTYPE_H
#define IRPROTOTYPE_H

// Function prototypes
void InitIR();
void handleIRMode();
void navigateIRMenu(int direction);
void navigateIRLeftRight(int direction);
void displayIRMenu();
void receiveIRSignal();
void saveIRSignal(String name, uint32_t signalData);
void savedFilesIR();
void emulateSavedSignal(String fileName);
void universalRemote();
void showIRKeyboard();
void handleIRKeyboardInput();
void HandlebuttonIR();

#endif // IRPROTOTYPE_H
