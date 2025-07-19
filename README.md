# 🌈 LED Sign with Microphone Control

A smart LED sign project featuring ESP32-C3 microcontroller, 100 WS2812B RGB LEDs, MAX4466 microphone, and a comprehensive web interface for control and configuration.

## ✨ Features

### Hardware
- **ESP32-C3** microcontroller with WiFi capability
- **100 WS2812B RGB LEDs** for vibrant color displays
- **MAX4466 microphone** for audio-reactive patterns
- **Physical buttons** for power and mode control
- **Web-based configuration** with captive portal

### LED Patterns
- 🎨 Solid colors and rainbow effects
- 🔄 Animated patterns (breathing, theater chase, color wipe)
- 🔥 Fire simulation and special effects
- 📊 Audio-reactive modes (VU meter, spectrum analyzer)
- ⚡ Beat detection and sound-responsive colors
- 〰️ Audio ripple effects

### Web Interface
- 📱 **Responsive design** - works on phone, tablet, and desktop
- 🏠 **Dashboard** - real-time status and quick controls
- 🎯 **Control Panel** - pattern selection and color picker
- 🔊 **Audio Settings** - microphone calibration and sensitivity
- 📶 **WiFi Management** - network scanning and connection
- ⚙️ **System Settings** - device configuration and updates

### Smart Features
- 🌐 **WiFi Setup Portal** - easy initial configuration
- 🔌 **WebSocket Communication** - real-time updates
- 💾 **Configuration Backup/Restore** - save your settings
- 🔄 **OTA Updates** - wireless firmware updates
- 🎛️ **REST API** - programmatic control
- 🔒 **Security Options** - optional password protection

## 📋 Quick Start

### Hardware Requirements
- ESP32-C3 DevKit
- WS2812B LED strip (100 LEDs)
- MAX4466 microphone module
- 2 push buttons
- 5V 8A power supply
- Basic electronic components (resistors, capacitors)

### Software Setup
1. **Install PlatformIO** or Arduino IDE
2. **Clone this repository**
3. **Upload firmware** to ESP32-C3
4. **Upload web files** to SPIFFS
5. **Connect to WiFi** via setup portal

## 📖 Documentation

- 📋 **[Project Specification](PROJECT_SPEC.md)** - Complete technical specifications
- 🔧 **[Hardware Guide](HARDWARE_GUIDE.md)** - Wiring diagrams and assembly instructions  
- 🚀 **[Setup Guide](SETUP_GUIDE.md)** - Software installation and configuration

## 🎮 Usage

### First Time Setup
1. Power on the device
2. Connect to `LED-Sign-Setup` WiFi network (password: `configure123`)
3. Follow the web setup wizard to connect to your WiFi
4. Access the main interface at the device's IP address

### Controls
- **Power Button**: Short press = toggle on/off, Long press = factory reset
- **Mode Button**: Short press = next pattern, Long press = configuration mode
- **Web Interface**: Full control over all settings and patterns

### Audio Modes
- Adjust microphone sensitivity in the web interface
- Try different audio-reactive patterns like VU meter and spectrum analyzer
- Calibrate beat detection for your music style

## 🔧 Customization

### Adding Custom Patterns
```cpp
void customPattern() {
    // Your custom LED code here
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(i * 10, 255, 255);
    }
}
```

### API Control
```bash
# Set brightness
curl -X POST http://device-ip/api/config -d '{"led":{"brightness":75}}'

# Change mode
curl -X POST http://device-ip/api/mode -d 'mode=2'
```

### WebSocket Integration
```javascript
const ws = new WebSocket('ws://device-ip:81');
ws.send(JSON.stringify({command: 'set_power', value: true}));
```

## 🔌 Pin Configuration

| Pin | Component | Function |
|-----|-----------|----------|
| GPIO2 | WS2812B LED Strip | Data Out |
| GPIO3 | MAX4466 Microphone | Analog Input |
| GPIO4 | Mode Button | Digital Input |
| GPIO5 | Power Button | Digital Input |
| GPIO8 | Status LED | Status Indicator |

## 📊 System Architecture

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   ESP32-C3  │────│  Web Server │────│ Browser UI  │
│             │    │ WebSockets  │    │ Real-time   │
└─────┬───────┘    └─────────────┘    └─────────────┘
      │
      ├── WS2812B LED Strip (100 LEDs)
      ├── MAX4466 Microphone
      └── Physical Buttons
```

## 🛠️ Development

### Build Environments
- **Development**: Full debugging enabled
- **Release**: Optimized for performance
- **Minimal**: Reduced features for memory constraints
- **OTA**: Over-the-air update capability

### Libraries Used
- FastLED - LED control
- ArduinoJson - JSON processing
- WebSockets - Real-time communication
- WiFi & Web Server - Network connectivity

## 📈 Performance

- **LED Refresh Rate**: 60 FPS
- **Audio Sampling**: 8kHz
- **WebSocket Updates**: 10 Hz
- **Memory Usage**: ~200KB
- **Power Consumption**: 0.8W (ESP32) + up to 30W (LEDs)

## 🔒 Security Features

- Optional web interface password protection
- WiFi credential encryption
- Rate limiting on API endpoints
- Input validation and sanitization

## 🚨 Safety

- Overcurrent protection with fuse
- Power supply safety margins
- Thermal considerations
- EMI reduction techniques

## 📄 License

This project is open source. Feel free to modify and distribute according to your needs.

## 🤝 Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## 📞 Support

- Check the [Setup Guide](SETUP_GUIDE.md) for troubleshooting
- Review [Hardware Guide](HARDWARE_GUIDE.md) for wiring issues
- Monitor serial output for debug information

---

**Built with ❤️ for the maker community**

*Transform any space with responsive LED art that dances to your music!*