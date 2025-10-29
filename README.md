# CloudMouse Boilerplate

**Event-driven dual-core firmware for CloudMouse hardware**

A complete, ready-to-use starting point for building CloudMouse applications. Fork this repo, customize it, and create your own interactive projects with display, encoder, LEDs, WiFi, and more.

## ✨ Features

- **Dual-core architecture** - UI rendering on Core 1, networking and logic on Core 0
- **Event-driven design** - Thread-safe communication via FreeRTOS queues
- **Complete hardware abstraction** - Easy-to-use classes for all CloudMouse components
- **WiFi setup flow** - Captive portal with QR code for easy configuration
- **NTP time sync** - Automatic time synchronization
- **Preferences storage** - Persistent configuration management
- **LED animations** - Smooth pulsing, loading states, and color control
- **Encoder handling** - Rotation, click, long-press detection with hardware PCNT
- **Display rendering** - High-performance graphics with LovyanGFX
- **Multi-platform support** - Works with both Arduino IDE and PlatformIO

## 🏗️ Architecture

CloudMouse Boilerplate uses a dual-core architecture to maximize performance:

### Core 0 (Main/Networking)
- WiFi management and connection handling
- Bluetooth communication service
- HTTP/WebSocket communication
- Business logic and state machine
- Event coordination
- NTP time synchronization

### Core 1 (UI)
- Display rendering (LovyanGFX)
- Encoder input reading
- User interface updates
- Real-time graphics

### Event System
Communication between cores happens via FreeRTOS queues:
- `uiToMainQueue` - Events from UI task to Main task
- `mainToUIQueue` - Events from Main task to UI task

This ensures thread-safe operation without race conditions.

## 📦 Required Libraries

### For Arduino IDE
Install these via Arduino Library Manager:

**Core Libraries**
- **LovyanGFX** v1.2.7+ - Display driver for ILI9488
- **Adafruit NeoPixel** v1.12.0+ - WS2812B LED control
- **RotaryEncoderPCNT** v1.1.0+ - Hardware encoder support (vickash/RotaryEncoderPCNT)

**Networking & Communication**
- **ArduinoJson** v7.0.0+ - JSON parsing
- **AsyncTCP** v3.3.2+ - Async TCP for ESP32
- **ESPAsyncWebServer** v3.6.0+ - Async web server
- **ESP32 BLE Keyboard** v0.3.2+ - BLE Keyboard abstraction for Arduino (included in this repo see below!)

**Utilities**
- **QRCode** v0.0.1 - QR code generation for WiFi setup (ricmoo/QRCode)

#### 💡 BLE Keyboard Support

The SDK includes a modified version of ESP32-BLE-Keyboard library 
compatible with ESP32 core v3.x. This is temporary until the 
upstream PR is merged.

Original library: [https://github.com/T-vK/ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard)

### For PlatformIO
All dependencies are automatically managed via `platformio.ini` - no manual installation needed! 🎉

## 🚀 Quick Start

### Arduino IDE (Recommended for Development)

#### Prerequisites
- Arduino IDE 2.x or newer
- ESP32 board support installed (see [Getting Started Guide](https://docs.cloudmouse.co/getting-started))
- CloudMouse device

#### Installation

1. **Clone the repository**
```bash
git clone https://github.com/cloudmouse-co/boilerplate.git
cd boilerplate
```

2. **Install required libraries**
   - Open Arduino IDE
   - Go to Sketch → Include Library → Manage Libraries
   - Install each library listed above

3. **Configure Arduino IDE**
   - Board: `ESP32S3 Dev Module`
   - USB CDC On Boot: `Enabled`
   - Flash Size: `16MB`
   - PSRAM: `OPI PSRAM`
   - Partition Scheme: `Default 4MB with spiffs`

4. **Open and Upload**
   - Open `cloudmouse-boilerplate-platformio.ino` in Arduino IDE
   - Connect CloudMouse via USB-C
   - Click Upload
   - If upload fails, hold BOOT button while connecting USB cable, then clicking Upload

5. **Test**
   - Display shows "Hello CloudMouse!"
   - LED ring pulses gently
   - Rotate encoder to see direction detection
   - Press encoder button to see click detection and LED flash

---

### PlatformIO (Optional - For Advanced Users)

This project fully supports PlatformIO for users who prefer command-line workflows, CI/CD integration, or advanced debugging features.

#### Why PlatformIO?
- ✅ Automatic dependency management
- ✅ Built-in debugging support
- ✅ CI/CD integration ready
- ✅ Multi-environment builds
- ✅ Professional development workflow

#### Setup

**Important: Source Code Switching**

The project maintains a single codebase that works with both Arduino IDE and PlatformIO. The `src/main.cpp` file is kept in sync but needs to be toggled:

**To use PlatformIO:**
1. Open `src/main.cpp` in your editor
2. **Uncomment the entire file** (remove `/*` at line 1 and `*/` at the end)
3. Save and build with PlatformIO

**To switch back to Arduino IDE:**
1. Open `src/main.cpp` in your editor
2. **Re-comment the entire file** (add `/*` at line 1 and `*/` at the end)
3. Save and build with Arduino IDE

> 💡 **Pro tip**: Most editors support block comment toggling with `Ctrl+/` (Windows/Linux) or `Cmd+/` (Mac). Select all (`Ctrl+A`) then toggle comments!

#### Build Commands

```bash
# First time - install dependencies
pio lib install

# Build the firmware
pio run

# Upload to device
pio run --target upload

# Serial monitor
pio device monitor

# All-in-one: build + upload + monitor
pio run -t upload -t monitor
```

#### Configuration

The `platformio.ini` is pre-configured with:
- ESP32-S3 board with PSRAM support (`board_build.arduino.memory_type = qio_opi`)
- All required library dependencies
- Optimized build flags
- USB CDC enabled for serial communication
- Custom include paths for library structure

#### Important Notes

⚠️ **Don't forget to toggle comments in `src/main.cpp`** when switching between platforms!

✅ **Hardware PCNT Support**: The project includes a custom `RotaryEncoderPCNT.h` wrapper in `lib/hardware/` that automatically adapts to both ESP-IDF 4.4 (PlatformIO) and ESP-IDF 5.x (Arduino IDE), ensuring flawless encoder performance on both platforms.

✅ **True Single Codebase**: No file copying, no syncing - just toggle comments and you're ready to go!

---

## 🔧 Advanced: ESP-IDF Native

While not officially supported, CloudMouse hardware is compatible with ESP-IDF native development. 
The hardware is fully open, and the community is welcome to create ESP-IDF ports.

If you develop an ESP-IDF version, consider contributing it back! 🚀

---

## 📁 Project Structure

```
cloudmouse-boilerplate-platformio/
├── cloudmouse-boilerplate-platformio.ino   # Main sketch (Arduino IDE)
├── platformio.ini                          # PlatformIO configuration
├── src/
│   └── main.cpp                            # PlatformIO entry (toggle comments)
│
├── lib/                                    # Core libraries
│   ├── core/                               # Event system
│   │   ├── Core.h / Core.cpp               # Main coordinator (Core 0)
│   │   ├── EventBus.h / EventBus.cpp       # FreeRTOS queues
│   │   └── Events.h                        # Event definitions
│   │
│   ├── hardware/                           # Hardware abstraction
│   │   ├── DisplayManager.h/cpp            # ILI9488 display
│   │   ├── LEDManager.h/cpp                # WS2812B LEDs
│   │   ├── EncoderManager.h/cpp            # Rotary encoder
│   │   ├── RotaryEncoderPCNT.h             # Cross-platform PCNT wrapper
│   │   ├── SimpleBuzzer.h                  # Piezo buzzer
│   │   └── LGFX_ILI9488.h                  # Display config
│   │
│   ├── network/
|   |   ├── BluetoothManager.h/cpp          # Bluetooth management
│   │   ├── WiFiManager.h/cpp               # WiFi management
│   │   ├── WebServerManager.h/cpp          # Web server
│   │
│   ├── utils/                              # Utilities
│   │   ├── NTPManager.h/cpp                # Time sync
│   │   ├── QRCodeManager.h/cpp             # QR codes
│   │   ├── JsonHelper.h                    # JSON utils
│   │   ├── AsyncHttpClient.h               # HTTP client
│   │   └── DeviceID.h                      # Device ID
│   │
│   ├── prefs/                              # Storage
│   │   └── PreferencesManager.h            # Preferences
│   │
│   └── config/                             # Configuration
│       └── DeviceConfig.h                  # Device settings
│
├── assets/                                 # Fonts & graphics
│   ├── OpenSans*.h                         # Fonts
│   ├── LeagueSpartan_50.h
│   └── logo.h                              # Logo
│
└── README.md                               # This file
```

## 💡 What the Boilerplate Does

When you upload the firmware, CloudMouse will:

1. **Boot sequence** - Initialize hardware, start dual-core tasks, LED animation
2. **WiFi setup** (if needed) - Create AP, show QR code, provide web config
3. **Normal operation** - Display UI, handle encoder/buttons, LED animations
4. **Time sync** - Automatic NTP synchronization

## 🔧 Customization

### Adding Your Own Logic

#### 1. Add Events
Edit `lib/core/Events.h`:
```cpp
enum class EventType {
  MY_CUSTOM_EVENT,
  // ...
};
```

#### 2. Handle Events
Edit `lib/core/Core.cpp`:
```cpp
void Core::processEvents() {
  Event event;
  while (EventBus::instance().receiveFromUI(event, 0)) {
    switch (event.type) {
      case EventType::MY_CUSTOM_EVENT:
        // Your code here
        break;
    }
  }
}
```

#### 3. Update Display
Edit `lib/hardware/DisplayManager.cpp`:
```cpp
void DisplayManager::renderMyScreen() {
  sprite.fillSprite(TFT_BLACK);
  sprite.drawString("Hello!", 240, 160);
  pushSprite();
}
```

#### 4. Trigger Events
From anywhere:
```cpp
Event event(EventType::MY_CUSTOM_EVENT);
event.value = 42;
EventBus::instance().sendToMain(event);
```

## 🎯 Next Steps

1. **Examples** - [cloudmouse.co/docs/example-code](https://cloudmouse.co//docs/example-code)
2. **Documentation** - [cloudmouse.co/docs](https://cloudmouse.co/docs)
3. **Community** - [Discord](https://discord.gg/cloudmouse)
4. **Build!** - Fork and create something awesome! 🚀

## 🐛 Troubleshooting

### Upload fails
- Hold BOOT button while plugging-in, then upload
- Try different USB cable
- Check port selection

### Display blank
- Verify PSRAM setting (Arduino: "OPI PSRAM", PlatformIO: `board_build.arduino.memory_type = qio_opi`)
- Check library versions
- Re-upload firmware

### Encoder not responding
- Verify pins: CLK=16, DT=18, SW=17
- Check wiring
- Hardware PCNT is auto-configured for both platforms

### Platform switching issues
- **Arduino IDE → PlatformIO**: Uncomment `src/main.cpp`
- **PlatformIO → Arduino IDE**: Re-comment `src/main.cpp`
- Always save after toggling!

### PlatformIO build errors
- Ensure `src/main.cpp` is uncommented
- Run `pio lib install`
- Try `pio run --target clean`

### Arduino IDE compilation errors  
- Ensure `src/main.cpp` is commented out
- Install all required libraries
- Check board configuration

For more help: [docs.cloudmouse.co/troubleshooting](https://docs.cloudmouse.co/troubleshooting)

## 📄 License

MIT License - Free to use, modify, and distribute. See [LICENSE](LICENSE) for details.

## 🌟 Community

- **Discord**: [discord.gg/cloudmouse](https://discord.gg/cloudmouse)
- **Forum**: [forum.cloudmouse.co](https://forum.cloudmouse.co)
- **GitHub**: [github.com/cloudmouse-co](https://github.com/cloudmouse-co)
- **Website**: [cloudmouse.co](https://cloudmouse.co)
- **Docs**: [docs.cloudmouse.co](https://docs.cloudmouse.co)

## 💖 Credits

Created by the CloudMouse Team.

Special thanks to contributors and the maker community.

**Technical Achievement**: This boilerplate features a custom cross-platform `RotaryEncoderPCNT` wrapper that seamlessly bridges ESP-IDF 4.4 and 5.x APIs, enabling true multi-platform compatibility between Arduino IDE and PlatformIO.

---

**Ready to build?** Fork this repo and start creating! 🚀

*Built with ❤️ for makers, by makers.*