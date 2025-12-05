# Quil: The AI Companion Robot

Quil is an interactive, voice-activated desktop companion robot built on the ESP32 platform. It uses OpenAI APIs for voice interactions, drives expressive animations on a 128x64 OLED display, and accepts touch input via TTP223 capacitive sensors.

![Quil](https://github.com/sddion/quil.site/blob/main/public/android-chrome-512x512.png)

## Features

*   **Voice Interaction**: Talk to Quil and get responses from OpenAI's GPT models.
*   **Expressive Animations**: A variety of animations to express different emotions and states.
*   **Touch Gestures**: Control Quil with simple touch gestures like tapping and swiping.
*   **WiFi Connectivity**: Connects to your WiFi network to access online services.
*   **Over-the-Air (OTA) Updates**: Update the firmware wirelessly.
*   **Web-Based Configuration**: Easy setup and configuration through a web portal.

## Getting Started

### Prerequisites

*   **Hardware**:
    *   ESP32 Development Board
    *   SSD1306 OLED Display (128x64)
    *   TTP223 Touch Sensor
    *   INMP441 I2S Microphone
    *   MAX98357A I2S Speaker
*   **Software**:
    *   Visual Studio Code with the PlatformIO IDE extension.
    *   Git

### Installation

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/your-username/quil.git
    cd quil
    ```

2.  **Build and Upload**:
    *   Open the project in Visual Studio Code.
    *   Use the PlatformIO extension to build and upload the firmware to your ESP32.

For more detailed instructions, please see the [Quick Start Guide](docs/QUICKSTART.md).

## Usage

1.  **Power On**: Connect your ESP32 to a power source.
2.  **WiFi Setup**: On the first boot, Quil will create a WiFi access point for you to configure your network credentials.
3.  **Interact**: Once connected, you can interact with Quil using voice commands and touch gestures.

## Contributing

Contributions are welcome! Please read our [Contributing Guidelines](CONTRIBUTING.md) for more information on how to get involved.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
