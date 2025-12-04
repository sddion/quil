# Quil Firmware - Quick Start Guide

Welcome to the Quil Firmware Quick Start Guide! This guide will walk you through setting up your development environment, building and uploading the firmware, and configuring your device.

## Prerequisites

Before you begin, make sure you have the following hardware and software:

### Hardware

*   **ESP32 Development Board**: A compatible ESP32 board.
*   **OLED Display**: An SSD1306-based I2C OLED display.
*   **Touch Sensor**: One TTP223 touch sensor.
*   **Microphone**: An INMP441 I2S microphone.
*   **Speaker**: A MAX98357A I2S speaker.
-   **USB Cable**: For connecting your ESP32 to your computer.

### Software

*   **Visual Studio Code**: A free and open-source code editor.
*   **PlatformIO IDE**: An extension for VS Code for embedded development.
*   **Git**: A version control system for cloning the repository.

## 🚀 Step 1: Build and Upload

1.  **Clone the Repository**:
    ```bash
    git clone https://github.com/sddion/quil.git
    cd quil
    ```

2.  **Install PlatformIO**:
    If you don't have PlatformIO installed, you can install it from the command line:
    ```bash
    pip install platformio
    ```
    Alternatively, you can install the PlatformIO IDE extension directly from within VS Code.

3.  **Build the Firmware**:
    This command compiles the code and builds the firmware binary for the ESP32 platform.
    ```bash
    pio run -e esp32
    ```

4.  **Upload to ESP32**:
    Connect your ESP32 board to your computer via USB. Then, run the following command to upload the firmware:
    ```bash
    pio run -e esp32 -t upload
    ```

5.  **Monitor Serial Output**:
    To view logs and debugging information from the device, use the serial monitor:
    ```bash
    pio device monitor -b 115200
    ```

## 📡 Step 2: WiFi Setup

### First Boot

1.  **Access Point Mode**: On its first boot, the device will create a WiFi access point named **QUIL_SETUP** with the password `quil1234`.
2.  **Connect to the AP**: Use your phone or laptop to connect to the `QUIL_SETUP` network.
3.  **Open the Configuration Portal**: Open a web browser and navigate to `http://192.168.4.1`.
4.  **Enter Your Credentials**: Select your home WiFi network from the list, enter the password, and click "Save".
5.  **Restart**: The device will save the credentials and restart, automatically connecting to your WiFi network.

### Resetting WiFi

If you need to change the WiFi configuration, you can either re-flash the firmware or clear the NVS (Non-Volatile Storage) where the credentials are saved.

## 🎮 Step 3: Using the Device

### Mode Switching

You can cycle through the different display modes by **double-tapping** the TTP223 touch sensor. The available modes are:

1.  **TIME_DATE**: Shows the current time and date, synchronized via NTP.
2.  **CHAT**: Enables voice interaction with the LLM.
3.  **THEME_PREVIEW**: Allows you to select from different UI themes.
4.  **WIFI_INFO**: Displays network status, including IP address and RSSI.

## 🔧 Over-the-Air (OTA) Updates

Once the device is connected to your WiFi network, you can update the firmware wirelessly.

*   **Via PlatformIO**:
    ```bash
    pio run -e esp32 -t upload --upload-port quil.local
    ```
*   **Via Arduino IDE**:
    In the Arduino IDE, you should see a network port named `quil`. Select it and upload as usual.

## 📝 Code Structure Overview

For a detailed explanation of the architecture, please see the [ARCHITECTURE.md](ARCHITECTURE.md) file.

```
firmware/
├── include/       # Global configuration (pins, constants, types)
├── src/
│   ├── hal/       # Hardware Abstraction Layer (drivers)
│   ├── core/      # Core logic (state machine, diagnostics)
│   ├── modules/   # Application features (WiFi, OTA, etc.)
│   ├── modes/     # Display modes
│   └── main.cpp   # Main application entry point
```

## 🎯 Adding a New Mode

1.  Create your new mode files: `modes/mode_yourname.h` and `modes/mode_yourname.cpp`.
2.  Add a new entry to the `DisplayMode_t` enum in `firmware/include/types.h`.
3.  Implement the `init`, `update`, and `render` functions for your mode.
4.  Include your new mode's header in `main.cpp` and add it to the main switch statement.
5.  Increment the mode count in the `state_cycle_mode()` function.

## 🔌 Pin Reference (ESP32)

| Peripheral    | Pins                               | Notes          |
|---------------|------------------------------------|----------------|
| Display/Touch | 21 (SDA), 22 (SCL)                 | I2C shared bus |
| Microphone    | 25 (WS), 26 (BCLK), 27 (DOUT)      | I2S input      |
| Speaker       | 32 (WS), 33 (BCLK), 19 (DIN)       | I2S output     |
| LED           | 4                                  | Optional RGB   |

## 🐛 Troubleshooting

**Display not working?**
*   **Check I2C Address**: The default address for the SSD1306 is `0x3C`. Verify this in your display's documentation.
*   **Verify Pins**: Ensure that the SDA and SCL pins are correctly defined in `firmware/include/pins.h`.

**WiFi won't connect?**
*   **Check Credentials**: Double-check your SSID and password in the web portal.
*   **2.4GHz Only**: The ESP32 does not support 5GHz networks. Make sure you are connecting to a 2.4GHz band.

**Touch not responding?**
*   **Check Connections**: Ensure the TTP223 sensor is connected to the correct GPIO pin (34 on ESP32).
*   **Verify Power**: Make sure the sensors are receiving 3.3V.

**OTA not appearing?**
*   **Same Network**: Ensure your computer and the device are on the same local network.
*   **mDNS**: Try pinging the device at `quil.local`. If it doesn't respond, there may be an mDNS issue on your network.
*   **Firewall**: A firewall on your computer might be blocking the OTA port (usually 3232).

## 📞 Next Steps

1.  Upload the firmware to your device.
2.  Complete the WiFi setup process.
3.  Test the mode switching functionality.
4.  (Future) Add your OpenAI API keys to enable the chat feature.
5.  Customize the existing themes or create your own!
