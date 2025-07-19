# 🚀 LED Sign Setup Guide

## Table of Contents
- [Prerequisites](#prerequisites)
- [Software Installation](#software-installation)
- [Firmware Compilation & Upload](#firmware-compilation--upload)
- [Initial Setup](#initial-setup)
- [Web Interface Configuration](#web-interface-configuration)
- [Troubleshooting](#troubleshooting)
- [Advanced Configuration](#advanced-configuration)

## Prerequisites

### Hardware Requirements
- ✅ Completed hardware assembly (see [HARDWARE_GUIDE.md](HARDWARE_GUIDE.md))
- ✅ ESP32-C3 connected to computer via USB
- ✅ All components properly wired and tested
- ✅ 5V power supply connected and tested

### Software Requirements
- **Operating System**: Windows 10+, macOS 10.14+, or Linux (Ubuntu 18.04+)
- **USB Drivers**: CP2102 or CH340 drivers (usually auto-installed)
- **Development Environment**: Choose one:
  - PlatformIO (recommended)
  - Arduino IDE 2.0+
- **Web Browser**: Chrome, Firefox, Safari, or Edge (latest versions)

## Software Installation

### Option 1: PlatformIO (Recommended)

1. **Install Visual Studio Code**
   ```bash
   # Download from: https://code.visualstudio.com/
   ```

2. **Install PlatformIO Extension**
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "PlatformIO IDE"
   - Click Install

3. **Install ESP32 Platform**
   ```bash
   # PlatformIO will auto-install when you open the project
   ```

### Option 2: Arduino IDE

1. **Download Arduino IDE 2.0+**
   ```bash
   # Download from: https://www.arduino.cc/en/software
   ```

2. **Install ESP32 Board Package**
   - File → Preferences
   - Add to Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager
   - Search "ESP32" and install "esp32 by Espressif Systems"

3. **Install Required Libraries**
   - Tools → Manage Libraries
   - Install these libraries:
     ```
     FastLED by Daniel Garcia
     ArduinoJson by Benoit Blanchon
     WebSockets by Markus Sattler
     NTPClient by Fabrice Weinberg
     ```

## Firmware Compilation & Upload

### Using PlatformIO

1. **Clone/Download Project**
   ```bash
   git clone <repository-url>
   cd LED-Sign-mic
   ```

2. **Open Project in VS Code**
   ```bash
   code .
   ```

3. **Select Environment**
   - Click PlatformIO icon in sidebar
   - Select `esp32-c3-devkitm-1` environment

4. **Compile Firmware**
   ```bash
   # In PlatformIO terminal
   pio run
   ```

5. **Upload Firmware**
   ```bash
   # Make sure ESP32-C3 is connected via USB
   pio run --target upload
   ```

6. **Upload File System (SPIFFS)**
   ```bash
   # Upload web interface files
   pio run --target uploadfs
   ```

### Using Arduino IDE

1. **Configure Board Settings**
   - Tools → Board → ESP32 Arduino → ESP32C3 Dev Module
   - Tools → Port → (Select your ESP32 port)
   - Tools → Upload Speed → 921600
   - Tools → CPU Frequency → 160MHz (WiFi/BT)
   - Tools → Flash Size → 4MB (32Mb)
   - Tools → Partition Scheme → Huge APP (3MB No OTA/1MB SPIFFS)

2. **Open Main Sketch**
   - File → Open → `src/LED_Sign_Main.ino`

3. **Install Additional Files**
   - Copy `include/config.h` to sketch folder
   - Copy `src/WebServer.ino` to sketch folder

4. **Compile and Upload**
   - Sketch → Verify/Compile
   - Sketch → Upload

5. **Upload SPIFFS Data**
   - Install ESP32 Sketch Data Upload plugin
   - Tools → ESP32 Sketch Data Upload

## Initial Setup

### First Boot Sequence

1. **Power On Device**
   - Connect power supply
   - ESP32 should boot and run LED test sequence:
     - Red → Green → Blue → Off

2. **Monitor Serial Output**
   ```bash
   # PlatformIO
   pio device monitor

   # Arduino IDE
   Tools → Serial Monitor (115200 baud)
   ```

   Expected output:
   ```
   LED Sign Starting...
   WiFi: Attempting connection...
   WiFi: No saved credentials, starting AP mode
   AP Mode: LED-Sign-Setup
   IP Address: 192.168.4.1
   Web server started
   WebSocket server started
   LED Sign Ready!
   ```

3. **Check LED Status Indicator**
   - **Solid ON**: Startup/Error
   - **Blinking slowly**: AP mode active
   - **OFF**: Connected to WiFi
   - **Fast blinking**: Configuration mode

### WiFi Configuration

1. **Connect to Setup Network**
   - Look for WiFi network: `LED-Sign-Setup`
   - Password: `configure123`
   - Connect your device (phone/computer)

2. **Access Setup Portal**
   - Your browser should automatically redirect
   - If not, navigate to: `http://192.168.4.1`

3. **Scan and Connect**
   - Click "Scan for Networks"
   - Select your WiFi network
   - Enter password
   - Click "Connect to WiFi"

4. **Note New IP Address**
   - Setup page will show new IP address
   - Example: `http://192.168.1.100`
   - Bookmark this address for future access

## Web Interface Configuration

### Accessing the Interface

1. **Connect to Main Interface**
   ```
   http://[device-ip-address]
   Example: http://192.168.1.100
   ```

2. **Navigate Interface**
   - **Dashboard**: Status and quick controls
   - **Control**: LED patterns and colors
   - **Audio**: Microphone settings
   - **WiFi**: Network configuration
   - **Settings**: System configuration

### Initial Configuration Steps

#### 1. Device Settings
- Go to Settings → General Settings
- Set device name (e.g., "Living Room LED Sign")
- Set timezone for your location
- Configure auto-sleep timer if desired

#### 2. Audio Calibration
- Go to Audio → Calibration Tools
- Click "Microphone Test" - should show audio response
- Adjust MAX4466 potentiometer for optimal sensitivity
- Run "Noise Floor Detection" for automatic calibration

#### 3. LED Configuration
- Go to Control → LED Patterns
- Test different modes to verify LED operation
- Adjust brightness for your environment
- Set preferred default colors

#### 4. Button Configuration
- Go to Settings → Button Configuration
- Test button functionality
- Customize button actions if desired

### Testing All Functions

```
Test Checklist:
□ Power button toggles LED on/off
□ Mode button cycles through patterns
□ Web interface loads correctly
□ All LED patterns work smoothly
□ Audio-reactive modes respond to sound
□ WiFi connection is stable
□ WebSocket real-time updates work
□ Settings save and persist after reboot
```

## Troubleshooting

### Common Issues

#### 1. Device Won't Boot
**Symptoms**: No serial output, LEDs don't light up
**Solutions**:
- Check power supply voltage (should be 5V)
- Verify USB connection
- Try different USB cable
- Check for short circuits

#### 2. WiFi Connection Fails
**Symptoms**: Can't connect to setup network or internet
**Solutions**:
- Reset WiFi credentials: Hold power button for 3 seconds
- Check WiFi password spelling
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Move closer to router

#### 3. LEDs Not Working
**Symptoms**: No LED response or random colors
**Solutions**:
- Check LED strip power connections
- Verify data line connection (GPIO2)
- Test level shifter circuit
- Check for loose connections

#### 4. No Audio Response
**Symptoms**: Audio-reactive modes don't respond
**Solutions**:
- Verify MAX4466 connections
- Adjust gain potentiometer
- Check microphone placement
- Test with louder sounds

#### 5. Web Interface Not Loading
**Symptoms**: Browser can't reach device
**Solutions**:
- Check device IP address
- Verify WiFi connection
- Try different browser
- Clear browser cache

### Debug Commands

#### Serial Monitor Commands
```
Available commands (type in serial monitor):
- status: Show system status
- restart: Restart device
- factory: Factory reset
- wifi: Show WiFi info
- audio: Show audio levels
- test: Run LED test sequence
```

#### Network Diagnostics
```bash
# Ping device
ping 192.168.1.100

# Check if web server is running
curl http://192.168.1.100/api/status

# Test WebSocket connection
# Use browser developer tools → Network → WS
```

### Recovery Procedures

#### Factory Reset
1. **Hardware Method**:
   - Hold power button for 5+ seconds during boot
   - Device will flash red LEDs and restart

2. **Software Method**:
   - Serial monitor command: `factory`
   - Web interface: Settings → Factory Reset

#### Firmware Recovery
If device becomes unresponsive:
1. Enter download mode:
   - Hold GPIO9 (boot button) while powering on
   - Release after 2 seconds

2. Re-upload firmware using PlatformIO or Arduino IDE

## Advanced Configuration

### Custom LED Patterns

1. **Create Custom Pattern**
   ```cpp
   // Add to LED_Sign_Main.ino
   void customPattern() {
       // Your custom LED code here
       for (int i = 0; i < NUM_LEDS; i++) {
           leds[i] = CHSV(i * 10, 255, 255);
       }
   }
   ```

2. **Add to Mode List**
   ```cpp
   // Add case to updateLEDs() switch statement
   case MODE_CUSTOM:
       customPattern();
       break;
   ```

### API Integration

#### REST API Examples
```bash
# Get system status
curl http://192.168.1.100/api/status

# Set power on
curl -X POST http://192.168.1.100/api/power -d "power=true"

# Change mode
curl -X POST http://192.168.1.100/api/mode -d "mode=2"

# Get configuration
curl http://192.168.1.100/api/config
```

#### WebSocket Integration
```javascript
// Connect to WebSocket
const ws = new WebSocket('ws://192.168.1.100:81');

// Send command
ws.send(JSON.stringify({
    command: 'set_brightness',
    value: 75
}));

// Receive updates
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Status update:', data);
};
```

### OTA Updates

#### Enable OTA in Code
```cpp
// Uncomment in config.h
#define ENABLE_OTA_UPDATES 1
```

#### Upload via OTA
```bash
# PlatformIO
pio run --target upload --upload-port 192.168.1.100
```

### Performance Optimization

#### Memory Usage
- Monitor free heap in web interface
- Reduce NUM_LEDS if memory issues occur
- Disable unused features in config.h

#### Power Optimization
- Reduce LED brightness for lower power consumption
- Use sleep timer for automatic power-off
- Consider external power supply for high LED counts

---

## Quick Start Checklist

```
□ Hardware assembled and tested
□ Software environment installed
□ Firmware compiled and uploaded
□ SPIFFS file system uploaded
□ Device boots with LED test sequence
□ WiFi configured and connected
□ Web interface accessible
□ All functions tested and working
□ Device configured for your environment
```

## Support

### Getting Help
- Check troubleshooting section above
- Review hardware connections
- Monitor serial output for error messages
- Test with minimal configuration first

### Useful Resources
- ESP32-C3 Datasheet: [Official Documentation]
- FastLED Library: [FastLED Documentation]
- WS2812B LED Strip: [Adafruit Guide]
- MAX4466 Microphone: [Breakout Board Guide]

**Congratulations!** Your LED sign should now be fully operational. Enjoy experimenting with different patterns and audio-reactive effects! 