# Quil Firmware Architecture

This document provides a high-level overview of the Quil firmware architecture. The firmware is designed to be modular and extensible, making it easy to add new features and functionality.

## System Diagram

The following diagram illustrates the high-level architecture of the Quil firmware:

```mermaid
graph TD
    subgraph User Interaction
        A[Touch Sensor] --> B{Gesture Manager};
        C[Microphone] --> D{Voice Manager};
    end

    subgraph Core System
        B --> E{State Machine};
        D --> E;
        E --> F[Display Modes];
    end

    subgraph Hardware Abstraction Layer (HAL)
        G[HAL I2C] --> A;
        H[HAL I2S] --> C;
        I[HAL Display] --> J[OLED Display];
    end

    subgraph Application Modules
        K[WiFi Manager] --> L{NTP Client};
        M[Cloud Sync] --> N{LLM Bridge};
    end

    subgraph Main Loop
        O[main.cpp] --> E;
        O --> B;
        O --> D;
    end

    F --> I;
```

## Directory Structure

The firmware is organized into the following directories:

```
firmware/
├── include/              # Global headers
│   ├── pins.h           # Pin assignments for ESP32/ESP8266
│   ├── config.h         # System-wide constants and configuration
│   └── types.h          # Core enumerations and type definitions
├── src/
│   ├── hal/             # Hardware Abstraction Layer
│   ├── core/            # Core system logic
│   ├── modules/         # High-level application 
│   ├── themes/          # Themes for the device
│   ├── modes/           # Different display modes for the device
│   └── main.cpp         # Main application entry point
└── lib/                 # Third-party libraries (managed by PlatformIO)
```

## Layers

### Hardware Abstraction Layer (HAL)

The HAL provides a consistent interface to the underlying hardware, regardless of the specific components used. This makes it easier to port the firmware to new hardware platforms.

-   `hal_i2c.*`: Wrapper for the I2C communication protocol.
-   `hal_display.*`: Driver for the SSD1306 OLED display.
-   `hal_mpr121.*`: Driver for the MPR121 touch sensor.
-   `hal_i2s.*`: Driver for the I2S audio interface (used for the microphone and speaker on ESP32).

### Core Layer

The Core layer contains the fundamental logic that drives the application.

-   `state_machine.*`: Manages the current state and mode of the device. This includes transitions between different display modes and robot states.
-   `diagnostics.*`: Provides functions for monitoring system health, such as heap memory usage and uptime.

### Application Modules

Modules are self-contained components that provide specific features and functionality.

-   `config_store.*`: Manages persistent storage of configuration settings, such as WiFi credentials, using the Non-Volatile Storage (NVS) library.
-   `wifi_manager.*`: Handles WiFi connectivity, including connecting to an existing network (STA mode) or creating an access point (AP mode) for initial setup.
-   `ntp_client.*`: Synchronizes the device's internal clock with a Network Time Protocol (NTP) server.
-   `gesture_manager.*`: Decodes touch inputs from the MPR121 sensor into gestures (e.g., tap, swipe).
-   `voice_manager.*`: Manages audio input and output, including wake word detection and voice streaming.
-   `llm_bridge.*`: Facilitates communication with a backend service for Large Language Model (LLM) processing.

### Display Modes

Modes define what is shown on the display at any given time. Each mode is responsible for rendering its own UI and handling relevant inputs.

-   `mode_time.*`: Displays the current time and date.
-   `mode_wifi_info.*`: Shows WiFi connection status, IP address, and signal strength (RSSI).
-   `mode_chat.*`: Provides a user interface for interacting with the LLM.
-   `mode_music.*`: Controls music playback.
-   `mode_theme_preview.*`: Allows the user to preview and select different UI themes.

## Data Flow

1.  **Input**: User interactions are captured by the `gesture_manager` (for touch) and `voice_manager` (for audio).
2.  **State Machine**: The `state_machine` processes these inputs and determines if a state change is required (e.g., switching display modes).
3.  **Mode Logic**: The active display mode executes its logic, which may involve interacting with various modules (e.g., `ntp_client` for the time, `llm_bridge` for chat).
4.  **Rendering**: The mode then renders its output to the display via the `hal_display` driver.

## Coding Conventions

To maintain code quality and consistency, please adhere to the following rules:

-   **Function Size**: Keep functions concise and focused.
-   **File Size**: Limit file length to under 200 lines to improve readability and maintainability.
-   **Pin Definitions**: Do not hardcode pin numbers directly in the code. Use the definitions provided in `firmware/include/pins.h`.
-   **Non-Blocking Code**: Avoid using `delay()` or other blocking functions. Use `millis()` for timing and non-blocking alternatives whenever possible.
-   **Platform-Specific Code**: Isolate platform-specific code (e.g., for ESP32 vs. ESP8266) using `#ifdef` blocks.
