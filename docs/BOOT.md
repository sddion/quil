🚀 QUIL Boot Sequence
Phase 1: Hardware Initialization (Lines 29-42)
Serial Communication starts at 115200 baud for debugging
I2C Bus initializes (for OLED display communication)
OLED Display initializes - if this fails, the device halts with error message
Config Storage opens (loads persistent settings from flash memory)
Diagnostics system initializes (monitors heap memory and uptime)
State Machine initializes - sets robot to STATE_IDLE and display to MODE_TIME_DATE
Gesture & Touch systems initialize
Display contrast loads from saved settings (default: 128)
Phase 2: WiFi Setup (Lines 50-68)
Two Different Paths:

A) First Boot (No Saved Credentials):

Device starts in Access Point (AP) mode
Creates WiFi network: "QUIL SETUP" with password "quil1234"
AP IP address: typically 192.168.4.1
User can connect to configure WiFi credentials
B) Subsequent Boots (Has Saved Credentials):

Loads saved WiFi SSID and password from flash
Attempts to connect to saved network (20-second timeout)
If connection fails, stays in station mode and retries in background (doesn't switch to AP)
If successful, gets IP address and checks internet connectivity
Phase 3: Service Initialization (Lines 72-77)
HTTP Web Server starts (always runs, even if WiFi isn't connected yet)
OTA Update Manager initializes (for firmware updates over WiFi)
Voice Manager initializes (I2S microphone and speaker)
Wake Word Detection initializes
LLM Bridge initializes (Serial communication for AI commands)
Phase 4: Boot Animation (Lines 78-87)
Animation Manager initializes
Plays ANIM_BOOT animation on OLED display
Animation runs at 50ms per frame (20 FPS)
Blocks until animation completes - this is the only one-shot animation
Clears display after animation
Phase 5: Mode Initialization (Lines 89-96)
Time Mode initializes (NTP client, battery monitor, weather)
Chat Mode initializes
Theme Preview Mode initializes
WiFi Info Mode initializes
Forces initial render of Time mode to prevent black screen
Phase 6: Ready State (Lines 98-99)
Serial prints:

Quil ready
[HTTP] Web interface available at device IP when connected
🔁 Main Loop Execution
After boot, the 
loop()
 function runs continuously:

WiFi Reconnection Task - automatically reconnects if connection drops (every 10 seconds)
State Machine Update - manages mode transitions
Diagnostics Update - monitors system health
OTA Handler - processes firmware update requests
HTTP Server - handles web interface requests
Touch Sensor Polling - detects touch events
Gesture Detection - interprets taps/double-taps
Wake Word Detection - listens for voice activation
Voice Processing - captures audio and sends to LLM bridge
Bridge Response Handling - receives AI responses
Animation Updates - if any animation is playing
Mode Rendering - updates current display mode (Time/Chat/Theme/WiFi Info)
The loop runs every 10ms (100 FPS), providing responsive touch and display updates.

📝 Key Behaviors on First Boot
Display shows boot animation first
If no WiFi configured: Creates AP "QUIL SETUP" / "quil1234"
User connects to AP and visits http://192.168.4.1 to configure WiFi
After WiFi is saved, device remembers credentials in flash memory
On subsequent boots, automatically connects to saved network
Default display mode is TIME_DATE showing clock and battery status
Double-tap anywhere cycles through display modes
Web interface available at device's IP address for configuration