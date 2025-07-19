# LED Sign with Microphone Control - Project Specification

## Project Overview

An intelligent LED sign system controlled by ESP32-C3 microcontroller that responds to audio input and provides web-based configuration. The system features a 100-LED WS2812 RGB strip that can display various patterns, react to sound, and be controlled remotely via a web interface.

## Hardware Components

### Core Components

| Component | Model/Type | Quantity | Purpose |
|-----------|------------|----------|---------|
| Microcontroller | ESP32-C3 | 1 | Main processing unit with WiFi capability |
| LED Strip | WS2812B RGB | 100 LEDs | Visual display output |
| Microphone | MAX4466 Electret Microphone Amplifier | 1 | Audio input for sound-reactive patterns |
| On/Off Button | Momentary Push Button | 1 | System power control |
| Mode Button | Momentary Push Button | 1 | Cycle through display modes |

### Power Requirements

- **ESP32-C3**: 3.3V, ~240mA (peak)
- **WS2812B Strip (100 LEDs)**: 5V, ~6A (max brightness, all white)
- **MAX4466**: 2.4V to 5.5V, ~4mA
- **Recommended Power Supply**: 5V, 8A minimum

### Pin Assignments (ESP32-C3)

| Pin | Component | Function |
|-----|-----------|----------|
| GPIO2 | WS2812B LED Strip | Data Out (DIN) |
| GPIO3 | MAX4466 Microphone | Analog Input (OUT) |
| GPIO4 | Mode Button | Digital Input (Pull-up) |
| GPIO5 | On/Off Button | Digital Input (Pull-up) |
| GPIO8 | Status LED | System Status Indicator |
| 5V | Power Rail | LED Strip Power |
| 3.3V | Power Rail | ESP32-C3, MAX4466 Power |
| GND | Ground | Common Ground |

## Software Architecture

### Core Firmware Features

#### 1. LED Control System
- **Library**: FastLED or NeoPixelBus
- **Modes**:
  - Static colors
  - Rainbow patterns
  - Sound-reactive visualizations
  - Custom patterns
  - Text scrolling
  - Breathing effects
  - Strobe patterns

#### 2. Audio Processing
- **Sampling Rate**: 8-10 kHz
- **Features**:
  - Volume level detection
  - Basic frequency analysis
  - Beat detection
  - Noise filtering
  - Configurable sensitivity

#### 3. Button Interface
- **On/Off Button**: 
  - Short press: Toggle system on/off
  - Long press (3s): Factory reset
- **Mode Button**:
  - Short press: Next mode
  - Long press (2s): Enter configuration mode

#### 4. WiFi Management
- **Modes**:
  - Station mode (connect to existing network)
  - Access Point mode (for initial setup)
  - Smart Config support
- **Auto-reconnection**: Automatic reconnection on WiFi loss
- **Fallback**: AP mode if connection fails for 30 seconds

### Web Server Functionality

#### 1. WiFi Setup Portal
- **Captive Portal**: Automatic redirection when in AP mode
- **Network Scanner**: Display available WiFi networks
- **Credential Storage**: Secure storage in EEPROM/Flash
- **Connection Testing**: Verify connectivity before saving

#### 2. Control Interface
- **Real-time Control**: WebSocket-based communication
- **Features**:
  - LED pattern selection
  - Color picker
  - Brightness control
  - Speed adjustment
  - Microphone sensitivity
  - Mode scheduling
  - Factory reset option

#### 3. Configuration Management
- **Settings Storage**: Non-volatile storage (Preferences library)
- **Backup/Restore**: Configuration export/import
- **OTA Updates**: Over-the-air firmware updates

## Web Interface Specifications

### Pages Structure

#### 1. Home Dashboard (`/`)
- Current mode display
- Quick controls (on/off, brightness)
- System status indicators
- WiFi signal strength
- Uptime information

#### 2. LED Control (`/control`)
- Mode selection dropdown
- Color picker (HSV/RGB)
- Brightness slider (0-100%)
- Speed control (0-100%)
- Pattern preview

#### 3. Audio Settings (`/audio`)
- Microphone sensitivity slider
- Audio visualization type selection
- Frequency response settings
- Auto-gain control toggle
- Real-time audio level meter

#### 4. WiFi Configuration (`/wifi`)
- Network scanner with signal strength
- Connection form (SSID/Password)
- Current connection status
- AP mode toggle
- Network diagnostics

#### 5. System Settings (`/settings`)
- Device name configuration
- Time zone settings
- Auto-sleep timer
- Button behavior customization
- Factory reset option
- Firmware update interface

### API Endpoints

#### REST API
```
GET    /api/status          - System status
POST   /api/power          - Power on/off
GET    /api/modes          - Available modes list
POST   /api/mode           - Set current mode
GET    /api/config         - Current configuration
POST   /api/config         - Update configuration
GET    /api/wifi/scan      - Scan WiFi networks
POST   /api/wifi/connect   - Connect to WiFi
GET    /api/audio/level    - Current audio level
POST   /api/reset          - Factory reset
```

#### WebSocket Events
```
status_update    - Real-time system status
audio_data       - Real-time audio visualization data
config_changed   - Configuration update notification
```

## System Modes

### 1. Static Modes
- **Solid Color**: Single color across all LEDs
- **Rainbow Static**: Static rainbow pattern
- **Custom Pattern**: User-defined color sequences

### 2. Animated Modes
- **Rainbow Cycle**: Moving rainbow effect
- **Color Wipe**: Sequential LED activation
- **Theater Chase**: Moving dot patterns
- **Breathing**: Fade in/out effects
- **Fire Effect**: Simulated fire animation

### 3. Sound-Reactive Modes
- **VU Meter**: Volume-based bar graph
- **Spectrum Analyzer**: Frequency-based visualization
- **Beat Flash**: Flash on detected beats
- **Sound Colors**: Color change based on frequency
- **Audio Ripple**: Wave-like effects from center

### 4. Special Modes
- **Text Scroll**: Scrolling text display
- **Clock Mode**: Time display (requires NTP)
- **Notification**: Alert patterns
- **Sleep Mode**: Dim or off during set hours

## Configuration Parameters

### LED Settings
```json
{
  "brightness": 80,           // 0-100%
  "speed": 50,               // 0-100%
  "mode": "rainbow_cycle",   // Current mode
  "color": {
    "r": 255,
    "g": 0,
    "b": 0
  },
  "auto_brightness": false   // Ambient light adjustment
}
```

### Audio Settings
```json
{
  "sensitivity": 70,         // Microphone sensitivity 0-100%
  "noise_gate": 30,         // Noise threshold 0-100%
  "auto_gain": true,        // Automatic gain control
  "sample_rate": 8000,      // Sampling frequency (Hz)
  "smoothing": 5            // Audio smoothing factor
}
```

### System Settings
```json
{
  "device_name": "LED-Sign",
  "timezone": "UTC-5",
  "sleep_timer": 0,         // Auto-sleep in minutes (0=disabled)
  "button_mode": "normal",  // Button behavior mode
  "wifi_timeout": 30        // WiFi connection timeout (seconds)
}
```

## Implementation Timeline

### Phase 1: Basic Hardware Setup (Week 1)
- [ ] ESP32-C3 board setup and testing
- [ ] LED strip connection and basic control
- [ ] Button interface implementation
- [ ] Power supply design and testing

### Phase 2: Core Software (Week 2)
- [ ] LED control library integration
- [ ] Basic pattern implementation
- [ ] Button handling and mode switching
- [ ] WiFi connection management

### Phase 3: Audio Integration (Week 3)
- [ ] MAX4466 microphone setup
- [ ] Audio sampling and processing
- [ ] Sound-reactive pattern development
- [ ] Audio calibration and testing

### Phase 4: Web Interface (Week 4)
- [ ] Basic web server implementation
- [ ] WiFi setup portal
- [ ] Control interface development
- [ ] Real-time communication (WebSocket)

### Phase 5: Polish & Features (Week 5)
- [ ] Advanced patterns and effects
- [ ] Configuration persistence
- [ ] OTA update capability
- [ ] User testing and refinement

## Security Considerations

### WiFi Security
- WPA2/WPA3 support only
- Credential encryption in storage
- Secure AP mode with temporary passwords
- Rate limiting on login attempts

### Web Interface Security
- Input validation on all forms
- CSRF protection
- Secure WebSocket connections
- Access control for admin functions

## Testing Requirements

### Unit Tests
- LED pattern rendering
- Audio processing accuracy
- Button response timing
- WiFi connection reliability

### Integration Tests
- End-to-end web interface testing
- Real-time audio visualization
- Multi-user web access
- Power management testing

### Performance Tests
- LED refresh rate (>30 FPS)
- Audio latency (<50ms)
- Web interface responsiveness
- Memory usage optimization

## Documentation Deliverables

1. **User Manual**: Setup and operation guide
2. **API Documentation**: Complete REST and WebSocket API reference
3. **Hardware Guide**: Wiring diagrams and assembly instructions
4. **Developer Guide**: Code architecture and extension guidelines
5. **Troubleshooting Guide**: Common issues and solutions

## Future Enhancements

### Hardware Additions
- Temperature sensor for thermal protection
- Ambient light sensor for auto-brightness
- External storage (SD card) for patterns
- Multiple microphone support for directional audio

### Software Features
- Mobile app companion
- Music streaming integration
- Social media connectivity
- Advanced scheduling system
- Machine learning for pattern generation

---

**Project Version**: 1.0  
**Last Updated**: [Current Date]  
**Author**: [Your Name]  
**License**: [Your Choice] 