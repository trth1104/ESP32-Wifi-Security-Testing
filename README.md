# ESP32 WiFi Deauther with OLED Display

## Introduction

The ESP32 WiFi Deauther is a WiFi security testing tool using the ESP32 microcontroller combined with an SH1106 128x64 OLED display. The device allows you to perform WiFi network analysis and penetration testing tasks through a menu interface controlled by 4 buttons.

## Features

### 1. **WiFi Scan** 
Scan and display a list of nearby WiFi networks with detailed information:
- Network name (SSID)
- Signal strength (RSSI)
- Operating channel
- Access Point MAC address (BSSID)

### 2. **Client Scan**
Scan and detect client devices connected to WiFi networks (under development).

### 3. **Beacon Spam**
Create multiple fake Access Points to test WiFi device handling capabilities (under development).

### 4. **Deauth Attack**
Send deauthentication packets to disconnect clients from an Access Point:
- Select target AP from the scanned list
- Broadcast attack to all clients of the AP
- Display attack status on screen

## Hardware Requirements

- **ESP32** (any ESP32 development board)
- **OLED Display SH1106** 128x64, I2C interface (address 0x3C)
- **4 push buttons** for menu control:
  - `UP` (GPIO 3): Move up
  - `DOWN` (GPIO 2): Move down  
  - `SELECT` (GPIO 1): Select/Confirm
  - `BACK` (GPIO 6): Go back

### Wiring Diagram

| Component | ESP32 GPIO |
|-----------|------------|
| OLED SDA | GPIO 4     |
| OLED SCL | GPIO 5     |
| Button UP | GPIO 3    |
| Button DOWN | GPIO 2  |
| Button SELECT | GPIO 1 |
| Button BACK | GPIO 6 |

## Required Libraries

```cpp
- Wire (I2C)
- Adafruit_GFX
- Adafruit_SH110X
- WiFi
- esp_wifi
```

## Installation

1. Install Arduino IDE and configure for ESP32
2. Install required libraries through Library Manager
3. Open file `src/esp32.ino`
4. Select the correct ESP32 board and COM port
5. Upload code to ESP32

## Usage

1. After startup, the OLED display will show the main menu
2. Use **UP/DOWN** buttons to navigate the menu
3. Press **SELECT** to choose a function
4. In sub-functions, press **BACK** to return to main menu
5. For Deauth Attack feature:
   - Select "Deauth Clients" from menu
   - Choose target AP from the list
   - Device will start sending deauth frames
   - Press **BACK** to stop the attack

## Warning

⚠️ **IMPORTANT NOTICE**: This tool is for educational purposes and security testing on networks you own or have explicit permission to test. Using this tool on unauthorized networks is **illegal** and may violate cybersecurity laws.

## License

This project is for educational purposes only. Users are fully responsible for how they use this tool.