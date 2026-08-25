<img width="3000" height="4000" alt="20260819_214306" src="https://github.com/user-attachments/assets/e3397bba-4173-456c-8255-a001d5151d7c" />Reaper One is an ESP32 hardware experimentation project with a button-driven OLED menu. It combines infrared, NFC/RFID, Sub-GHz, Bluetooth, Wi-Fi, and a small game mode in one firmware build. The repository also contains a separate ESP32 Marauder-based firmware sketch.

## Project layout

- `Main_menu/` - Main Reaper One firmware.
  - IR signal receive, save, and replay
  - PN532 NFC/RFID reading, saving( writing, and limited emulation features in code but need different module)
  - CC1101/MD1101 Sub-GHz signal capture and saved-signal handling
  - BLE keyboard, BLE mouse, and nearby-device scanning
  - Wi-Fi/Marauder lab and testing modes
  - Simple endless-runner game
 
### Required

| Part | Quantity | Notes |
| --- | ---: | --- |
| [ESP32 DevKitC](https://www.espressif.com/en/products/devkits/esp32-devkitc) | 1 | Must provide the GPIO pins listed above; an ESP32-WROOM style board is suitable. |
| [SH1106 128x64 I2C OLED](https://www.waveshare.com/1.3inch-oled-module.htm) | 1 | Main user interface display. |
| [Momentary push buttons](https://www.sparkfun.com/tactile-switch-buttons-12mm-square.html) | 2 | Select and Back buttons on GPIO 27 and GPIO 26. |
| [Resistor assortment](https://www.sparkfun.com/resistor-kit.html) | 1 | Used to build the resistor-ladder network for the Up, Down, Left, and Right buttons on GPIO 34. |
| [MicroSD SPI card module](https://www.adafruit.com/product/254) | 1 | Connected to the SPI bus with chip-select on GPIO 5. |
| [MicroSD card](https://www.adafruit.com/product/2693) | 1 | Used for saved IR, RFID, and Sub-GHz files. |
| [USB cable and 5 V power source](https://www.adafruit.com/product/592) | 1 | Used to program and power the ESP32. |

### Optional feature modules

| Part | Quantity | Function |
| --- | ---: | --- |
| [CC1101 Sub-GHz module](https://www.elechouse.com/product/cc1101-wireless-module/) | 1 | Sub-GHz receive and saved-signal functions. |
| [PN532 NFC module](https://www.adafruit.com/product/364) | 1 | NFC/RFID functions; configure the module for I2C. |
| [IR receiver module](https://www.adafruit.com/product/157) | 1 | IR signal receive functions. |
| [IR LED](https://www.adafruit.com/product/388) and [driver transistor](https://www.adafruit.com/product/756) | 1 each | IR signal transmission and replay. |

The BLE, Wi-Fi, and game modes use the ESP32 and do not require an additional module in the current `Main_menu` implementation. Modules described here are not included with the repository.

The BLE, Wi-Fi, and game modes use the ESP32 and do not require an additional module in the current `Main_menu` implementation. Modules described here are not included with the repository.

## Main Menu hardware

The current `Main_menu` code is configured for an ESP32 with:

- 128x64 SH1106 I2C OLED
- SD card on SPI chip-select GPIO 5
- CC1101 or MD1101 Sub-GHz module
- PN532 NFC module in I2C mode
- IR receiver and IR LED/transmitter
- Two digital buttons and a resistor-ladder analog button input

### Default pins

| Function | GPIO |
| --- | ---: |
| Select button | 27 |
| Back button | 26 |
| Analog Up/Down/Left/Right buttons | 34 |
| PN532 SDA / SCL | 21 / 22 |
| PN532 IRQ / reset | 25 / 33 |
| IR receiver | 13 |
| IR transmitter | 15 |
| CC1101 CS | 2 |
| CC1101 MOSI / MISO / SCK | 23 / 19 / 18 |
| CC1101 GDO0 | 4 |
| SD CS | 5 |


## Main Menu controls

- Analog buttons navigate Up, Down, Left, and Right.
- Select enters a menu item or confirms an action.
- Back exits the current mode.
- In BLE keyboard and mouse modes, the navigation buttons act as the input controls.

Saved files are stored on the SD card in these directories:

- `/ir`
- `/rfid`
- `/subghz`

## Installing the Arduino IDE build

1. Install Arduino IDE 2.x.
2. Add the Espressif ESP32 board package through **Boards Manager**.
3. Install the libraries used by `Main_menu`:
   - U8g2
   - RadioLib
   - IRremote
   - Adafruit PN532
   - ESP32 BLE Keyboard
   - ESP32 BLE Mouse
4. Connect the required modules and buttons using the pin table above.
5. In Arduino IDE, open the sketch folder `Main_menu` and select the matching ESP32 board and serial port.
6. Compile and upload the sketch.
7. Open the Serial Monitor at `115200` baud.

Arduino compiles the `.ino` files in the selected sketch folder together. Open `Main_menu` as the sketch folder rather than opening only `Main_menu.ino` as an isolated file.

### Reaper One

- **IR** - Receive infrared signals, save them to SD, and replay supported signals.
- **NFC/RFID** - Read tags and manage saved tag data with a PN532.
- **Sub-GHz** - Capture and manage OOK/NRZ signals through a CC1101-compatible module. The default frequency is `433.92 MHz`.
- **Bluetooth** - Scan for BLE devices and advertise as a BLE keyboard or mouse using the name `Reaper One`.
- **Marauder** - Includes Wi-Fi testing modes such as target scanning, beacon testing, deauthentication testing, and an informational `MARAUDER_LAB` access point.
- **Game** - A small button-controlled runner game.

### AI declaration 
- AI has done the main optimization for the code and has helped a lot in debugging

### Photos
- final wiring 
<img width="3000" height="4000" alt="20260819_214306" src="https://github.com/user-attachments/assets/e1bc60b7-47b4-4b13-a0fd-95d961567572" />
<img width="3000" height="4000" alt="20260819_214321" src="https://github.com/user-attachments/assets/a1ca11cf-5d51-49e6-8b46-70caec23beaf" />
- final item
<img width="4000" height="3000" alt="20260819_214224" src="https://github.com/user-attachments/assets/cbaf0958-b6be-46be-9ad3-8734e22f5393" />
<img width="4000" height="3000" alt="20260819_214231" src="https://github.com/user-attachments/assets/bf4e43d2-83f0-436c-9e1e-8669563540e3" />



