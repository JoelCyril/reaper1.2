Reaper One is an ESP32 hardware experimentation project with a button-driven OLED menu. It combines infrared, NFC/RFID, Sub-GHz, Bluetooth, Wi-Fi, and a small game mode in one firmware build. The repository also contains a separate ESP32 Marauder-based firmware sketch.

## Project layout

- `Main_menu/` - Main Reaper One firmware.
  - IR signal receive, save, and replay
  - PN532 NFC/RFID reading, saving( writing, and limited emulation features in code but need different module)
  - CC1101/MD1101 Sub-GHz signal capture and saved-signal handling
  - BLE keyboard, BLE mouse, and nearby-device scanning
  - Wi-Fi/Marauder lab and testing modes
  - Simple endless-runner game
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
