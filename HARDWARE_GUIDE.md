# 🔧 Hardware Setup Guide - LED Sign with Microphone Control

## Table of Contents
- [Required Components](#required-components)
- [Wiring Diagram](#wiring-diagram)
- [Step-by-Step Assembly](#step-by-step-assembly)
- [Power Supply Design](#power-supply-design)
- [Enclosure Recommendations](#enclosure-recommendations)
- [Testing & Troubleshooting](#testing--troubleshooting)
- [Safety Considerations](#safety-considerations)

## Required Components

### Core Electronics

| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| **ESP32-C3 DevKit** | ESP32-C3-DevKitM-1 or compatible | 1 | Main microcontroller |
| **LED Strip** | WS2812B RGB, 5V, 100 LEDs | 1 | 60 LEDs/meter recommended |
| **Microphone** | MAX4466 Electret Microphone Amplifier | 1 | Adjustable gain |
| **Push Buttons** | 6mm tactile switches | 2 | Momentary, normally open |
| **Power Supply** | 5V, 8A switching adapter | 1 | LED strip requires high current |
| **Resistors** | 470Ω, 1/4W | 2 | Pull-up resistors for buttons |
| **Capacitors** | 1000µF, 16V electrolytic | 1 | Power supply smoothing |
| **Capacitors** | 100nF ceramic | 3 | Decoupling capacitors |

### Connectivity & Protection

| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| **Level Shifter** | 74HCT125 or SN74LV1T34 | 1 | 3.3V to 5V logic conversion |
| **Fuse** | 10A fast-blow | 1 | Power supply protection |
| **Terminal Blocks** | 5mm pitch, 2-way | 3 | Power and LED connections |
| **Jumper Wires** | Male-to-female, 20cm | 10 | Prototyping connections |
| **Breadboard** | Half-size or PCB | 1 | For prototyping |

### Optional Components

| Component | Purpose | Notes |
|-----------|---------|-------|
| **Status LED** | 5mm LED + 220Ω resistor | System status indication |
| **Heat Sink** | Small aluminum heatsink | For ESP32 cooling under load |
| **Ferrite Beads** | EMI suppression | Reduce electromagnetic interference |

## Wiring Diagram

```
                    ESP32-C3 DevKit
                   ┌─────────────────┐
                   │                 │
                   │              3V3├─── VCC (MAX4466)
                   │                 │
                   │             GND├─── GND (Common)
                   │                 │
     LED Data ─────┤GPIO2            │
                   │                 │
   Microphone ─────┤GPIO3            │
                   │                 │
   Mode Button ────┤GPIO4            │
                   │                 │
   Power Button ───┤GPIO5            │
                   │                 │
   Status LED ─────┤GPIO8            │
                   │                 │
                   │             5V ├─── 5V Power Rail
                   │                 │
                   └─────────────────┘

Power Supply (5V, 8A)
         │
         ├── ESP32-C3 (via USB or VIN)
         │
         └── LED Strip (WS2812B)
             ┌─────────────────────────────┐
             │ LED 1 │ LED 2 │ ... │ LED 100 │
             └─────────────────────────────┘
                │       │             │
               5V      GND          Data
                │       │             │
                └───────┼─────────────┘
                        │
                       GND (Common)

MAX4466 Microphone
┌─────────────────┐
│     MAX4466     │
│                 │
│ VCC ────────────┼── 3.3V
│ GND ────────────┼── GND
│ OUT ────────────┼── GPIO3
│                 │
└─────────────────┘

Buttons (x2)
┌─────────────┐
│   Button    │
│             │
│  ┌─── GPIO4/5
│  │    │
│  │   ┌┴┐ 470Ω
│  └───┤ ├──── 3.3V
│      └┬┘
│       │
│      GND
└─────────────┘
```

## Step-by-Step Assembly

### Step 1: Prepare the Workspace
1. **Anti-static precautions**: Use an anti-static wrist strap or touch a grounded surface
2. **Organize components**: Lay out all components and verify quantities
3. **Tools needed**: Soldering iron, solder, wire strippers, multimeter, screwdrivers

### Step 2: ESP32-C3 Setup
1. **Power test**: Connect ESP32-C3 to computer via USB
2. **Verify functionality**: Upload a simple blink sketch
3. **Check pin assignments**: Ensure all pins are accessible

### Step 3: Power Distribution
```
Power Supply Wiring:
┌─────────────┐
│ 5V 8A PSU   │
│             │
│ +5V ────────┼── Terminal Block (+)
│ GND ────────┼── Terminal Block (-)
│             │
└─────────────┘

Add 10A fuse in series with +5V line
Add 1000µF capacitor across +5V and GND near ESP32
```

1. **Install fuse**: Place 10A fuse in series with +5V line
2. **Terminal blocks**: Install terminal blocks for power distribution
3. **Smoothing capacitor**: Add 1000µF capacitor across power rails
4. **Power connections**:
   - Connect ESP32 VIN to +5V (or use USB for programming)
   - Connect ESP32 GND to common ground
   - Connect LED strip +5V and GND to power rails

### Step 4: LED Strip Connection
```
LED Strip Connections:
WS2812B Strip
┌─────────────────────────────────┐
│ RED   │ WHITE │ GREEN │ ... ... │
│ +5V   │ GND   │ DIN   │         │
└───┬───────┬─────┬───────────────┘
    │       │     │
    │       │     └── GPIO2 (via level shifter)
    │       └──────── GND (Common)
    └──────────────── +5V Power Rail
```

1. **Level shifter setup** (Important for reliable operation):
   ```
   74HCT125 Level Shifter:
   ┌─────────────┐
   │  74HCT125   │
   │             │
   │ VCC ────────┼── +5V
   │ GND ────────┼── GND
   │ A1  ────────┼── GPIO2 (ESP32)
   │ Y1  ────────┼── DIN (LED Strip)
   │ OE  ────────┼── GND (Always enabled)
   │             │
   └─────────────┘
   ```

2. **LED strip preparation**:
   - Cut LED strip to desired length if needed
   - Solder wires to: +5V (red), GND (white), Data In (green)
   - Use 18-20 AWG wire for power, 22-24 AWG for data

3. **Connections**:
   - Connect LED +5V to power rail
   - Connect LED GND to common ground
   - Connect LED Data In to level shifter output
   - Connect level shifter input to ESP32 GPIO2

### Step 5: Microphone Setup
```
MAX4466 Connections:
┌─────────────────┐
│     MAX4466     │    Components on module:
│   ┌─────────┐   │    - Electret microphone
│   │   MIC   │   │    - Amplifier circuit
│   └─────────┘   │    - Gain adjustment pot
│                 │
│ VCC ────────────┼── 3.3V (ESP32)
│ GND ────────────┼── GND (Common)
│ OUT ────────────┼── GPIO3 (ESP32)
│                 │
└─────────────────┘
```

1. **Power connections**:
   - VCC to ESP32 3.3V pin
   - GND to common ground

2. **Signal connection**:
   - OUT to ESP32 GPIO3 (analog input)
   - Add 100nF ceramic capacitor between OUT and GND (noise filtering)

3. **Gain adjustment**:
   - Turn potentiometer on MAX4466 to adjust sensitivity
   - Start with mid-position, adjust after software testing

### Step 6: Button Connections
```
Button Wiring (Both buttons use same pattern):
┌─────────────┐
│   Button    │
│             │
│      ┌──────┼── GPIO4 (Mode) or GPIO5 (Power)
│      │      │
│     ┌┴┐     │
│ 470Ω│ │     │    Pull-up resistor
│     └┬┘     │
│      │      │
│     3.3V    │
│             │
│     GND ────┼── When button pressed
│             │
└─────────────┘
```

1. **Mode button** (GPIO4):
   - One terminal to GPIO4
   - Other terminal to GND
   - 470Ω pull-up resistor from GPIO4 to 3.3V

2. **Power button** (GPIO5):
   - One terminal to GPIO5
   - Other terminal to GND
   - 470Ω pull-up resistor from GPIO5 to 3.3V

3. **Debouncing**:
   - Add 100nF ceramic capacitor across each button (optional)
   - Software debouncing is implemented in firmware

### Step 7: Status LED (Optional)
```
Status LED Connection:
┌─────────────┐
│ Status LED  │
│             │
│ Anode ──────┼── GPIO8 (ESP32)
│             │
│ Cathode ────┼── 220Ω ──── GND
│             │
└─────────────┘
```

1. **LED connection**:
   - Anode (long leg) to GPIO8
   - Cathode (short leg) to 220Ω resistor
   - Other end of resistor to GND

## Power Supply Design

### Power Requirements Analysis
```
Component Power Consumption:
┌─────────────────┬─────────┬─────────┬─────────────┐
│ Component       │ Voltage │ Current │ Power       │
├─────────────────┼─────────┼─────────┼─────────────┤
│ ESP32-C3        │ 3.3V    │ 240mA   │ 0.8W        │
│ MAX4466         │ 3.3V    │ 4mA     │ 0.01W       │
│ LED Strip (max) │ 5V      │ 6000mA  │ 30W         │
│ Status LED      │ 3.3V    │ 20mA    │ 0.07W       │
├─────────────────┼─────────┼─────────┼─────────────┤
│ Total Maximum   │ 5V      │ 6264mA  │ 31.3W       │
└─────────────────┴─────────┴─────────┴─────────────┘

Recommended PSU: 5V, 8A (40W) for safety margin
```

### Power Supply Recommendations
1. **Switching power supply**: More efficient than linear regulators
2. **Safety certifications**: UL, CE, FCC listed
3. **Overcurrent protection**: Built-in fuse or current limiting
4. **Ripple specification**: <100mV peak-to-peak at full load

### Recommended Models
- **Mean Well LRS-50-5**: 5V, 10A, 50W
- **ANVISION APS050-050**: 5V, 10A, 50W
- **Generic 5V 8A**: Ensure proper certifications

## Enclosure Recommendations

### Enclosure Requirements
- **Size**: Minimum 200mm x 150mm x 50mm
- **Material**: ABS plastic or aluminum
- **Ventilation**: Passive cooling vents or small fan
- **Access**: Removable panels for maintenance
- **Mounting**: LED strip mounting channels or brackets

### Suggested Enclosures
1. **Hammond 1591XXFLBK**: ABS plastic, flanged lid
2. **BUD Industries CU-3006-B**: Aluminum, removable cover
3. **Custom 3D printed**: Design files can be created

### Mounting Considerations
```
Enclosure Layout:
┌─────────────────────────────────────┐
│                                     │
│  [ESP32]  [Power Supply]            │
│                                     │
│  [MAX4466]    [Terminal Blocks]     │
│                                     │
│  [Buttons on front panel]           │
│                                     │
│  [LED Strip exit point]             │
│                                     │
└─────────────────────────────────────┘

Ventilation holes on sides and bottom
Status LED on front panel
```

## Testing & Troubleshooting

### Initial Power Test
1. **Voltage verification**:
   ```
   Measurement Points:
   - Power supply output: Should read 5.0V ±0.1V
   - ESP32 3.3V pin: Should read 3.3V ±0.1V
   - LED strip power: Should read 5.0V ±0.1V
   ```

2. **Current draw test**:
   - ESP32 only: ~240mA
   - With all LEDs white at 50% brightness: ~3A
   - Maximum (all white, 100%): ~6A

### LED Strip Testing
```
Test Sequence:
1. Single LED test: First LED should light up red
2. Strip test: All LEDs should display same color
3. Pattern test: Rainbow pattern should flow smoothly
4. No flickering or random colors
```

### Audio Testing
```
Microphone Test:
1. Connect oscilloscope to GPIO3
2. Clap hands - should see voltage spike
3. Adjust MAX4466 gain potentiometer
4. Typical output: 1.6V ±0.5V with audio input
```

### Common Issues & Solutions

| Problem | Symptom | Solution |
|---------|---------|----------|
| **No LED response** | LEDs don't light up | Check power supply, data line, level shifter |
| **Random LED colors** | Flickering, wrong colors | Add decoupling capacitors, check ground connections |
| **ESP32 won't boot** | No serial output | Check power supply voltage, USB connection |
| **No audio response** | Microphone not working | Verify MAX4466 connections, adjust gain |
| **Buttons not working** | No response to presses | Check pull-up resistors, button connections |
| **WiFi connection fails** | Can't connect to network | Check antenna, power supply stability |

### Debug Tools
1. **Multimeter**: Voltage and continuity testing
2. **Oscilloscope**: Signal analysis (optional)
3. **Logic analyzer**: Digital signal debugging (optional)
4. **USB-to-Serial adapter**: Programming and debugging

## Safety Considerations

### Electrical Safety
- **Ground fault protection**: Use GFCI outlets when possible
- **Proper fusing**: 10A fuse protects against overcurrent
- **Wire gauge**: Use appropriate wire gauge for current load
- **Insulation**: Ensure all connections are properly insulated

### Fire Safety
- **Thermal protection**: Monitor ESP32 temperature under load
- **Ventilation**: Ensure adequate airflow around power supply
- **Component ratings**: Don't exceed manufacturer specifications
- **Smoke detectors**: Install near project if permanent installation

### EMI Considerations
- **Ferrite beads**: Add to power and data lines if needed
- **Shielded enclosure**: Metal enclosure reduces EMI emissions
- **Proper grounding**: Connect metal enclosure to earth ground

### Component Protection
- **ESD protection**: Use anti-static precautions during assembly
- **Overvoltage protection**: Don't exceed component voltage ratings
- **Thermal management**: Provide adequate cooling for power components

---

## Quick Reference Pinout

```
ESP32-C3 Pinout for LED Sign:
┌─────────────────┐
│     ESP32-C3    │
│                 │
│ GPIO0  ──  USB  │  (Programming)
│ GPIO1  ──  USB  │  (Programming)
│ GPIO2  ──  LED  │  (WS2812B Data)
│ GPIO3  ──  MIC  │  (MAX4466 Analog)
│ GPIO4  ──  BTN  │  (Mode Button)
│ GPIO5  ──  BTN  │  (Power Button)
│ GPIO6  ──  ---  │  (Reserved)
│ GPIO7  ──  ---  │  (Reserved)
│ GPIO8  ──  LED  │  (Status LED)
│ GPIO9  ──  ---  │  (Boot Button)
│ GPIO10 ──  ---  │  (Available)
│                 │
│ 3V3    ──  PWR  │  (3.3V Output)
│ GND    ──  PWR  │  (Ground)
│ 5V     ──  PWR  │  (5V Input/Output)
│                 │
└─────────────────┘
```

This completes the hardware setup guide. Follow these instructions carefully and test each component individually before final assembly. Always prioritize safety and double-check all connections before applying power. 