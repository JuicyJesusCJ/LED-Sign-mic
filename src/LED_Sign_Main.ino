/*
 * LED Sign with Microphone Control
 * ESP32-C3 Main Firmware
 * 
 * Features:
 * - 100 WS2812B RGB LED control
 * - MAX4466 microphone audio processing
 * - WiFi management with captive portal
 * - Web interface for configuration
 * - Sound-reactive LED patterns
 * - Button controls
 */

#include <WiFi.h>
#include <WiFiAP.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <FastLED.h>
#include <DNSServer.h>

// Hardware Pin Definitions
#define LED_DATA_PIN     2
#define MIC_ANALOG_PIN   3
#define MODE_BUTTON_PIN  4
#define POWER_BUTTON_PIN 5
#define STATUS_LED_PIN   8

// LED Configuration
#define NUM_LEDS         100
#define LED_TYPE         WS2812B
#define COLOR_ORDER      GRB
#define MAX_BRIGHTNESS   255
#define FRAMES_PER_SECOND 60

// Audio Configuration
#define SAMPLE_RATE      8000
#define SAMPLES          256
#define NOISE_THRESHOLD  30

// Network Configuration
#define AP_SSID          "LED-Sign-Setup"
#define AP_PASSWORD      "configure123"
#define DNS_PORT         53
#define WEB_PORT         80
#define WEBSOCKET_PORT   81

// Global Objects
CRGB leds[NUM_LEDS];
WebServer webServer(WEB_PORT);
WebSocketsServer webSocket(WEBSOCKET_PORT);
DNSServer dnsServer;
Preferences preferences;

// System State Variables
struct SystemConfig {
  // LED Settings
  uint8_t brightness = 80;
  uint8_t speed = 50;
  uint8_t mode = 0;
  CRGB currentColor = CRGB::Red;
  bool autobrightness = false;
  
  // Audio Settings
  uint8_t sensitivity = 70;
  uint8_t noiseGate = 30;
  bool autoGain = true;
  uint8_t smoothing = 5;
  
  // System Settings
  char deviceName[32] = "LED-Sign";
  char timezone[16] = "UTC-5";
  uint16_t sleepTimer = 0;
  uint8_t buttonMode = 0;
  uint16_t wifiTimeout = 30;
  
  // WiFi Settings
  char ssid[64] = "";
  char password[64] = "";
} config;

// System State
bool systemOn = true;
bool configMode = false;
unsigned long lastModePress = 0;
unsigned long lastPowerPress = 0;
unsigned long lastUpdate = 0;
uint8_t currentMode = 0;
uint16_t audioLevel = 0;
uint16_t audioSamples[SAMPLES];
uint16_t sampleIndex = 0;

// LED Modes
enum LEDModes {
  MODE_SOLID_COLOR = 0,
  MODE_RAINBOW_STATIC,
  MODE_RAINBOW_CYCLE,
  MODE_COLOR_WIPE,
  MODE_THEATER_CHASE,
  MODE_BREATHING,
  MODE_FIRE_EFFECT,
  MODE_VU_METER,
  MODE_SPECTRUM_ANALYZER,
  MODE_BEAT_FLASH,
  MODE_SOUND_COLORS,
  MODE_AUDIO_RIPPLE,
  MODE_COUNT
};

const char* modeNames[] = {
  "Solid Color", "Rainbow Static", "Rainbow Cycle", "Color Wipe",
  "Theater Chase", "Breathing", "Fire Effect", "VU Meter",
  "Spectrum Analyzer", "Beat Flash", "Sound Colors", "Audio Ripple"
};

void setup() {
  Serial.begin(115200);
  Serial.println("LED Sign Starting...");
  
  // Initialize hardware
  initializePins();
  initializeLEDs();
  initializeAudio();
  
  // Load configuration
  loadConfiguration();
  
  // Initialize WiFi
  initializeWiFi();
  
  // Initialize web server
  initializeWebServer();
  initializeWebSocket();
  
  // Initialize file system
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }
  
  Serial.println("LED Sign Ready!");
  
  // Initial LED test
  testLEDs();
}

void loop() {
  // Handle web server and websockets
  webServer.handleClient();
  webSocket.loop();
  dnsServer.processNextRequest();
  
  // Handle button inputs
  handleButtons();
  
  // Process audio input
  processAudio();
  
  // Update LED patterns
  updateLEDs();
  
  // Send periodic updates
  sendWebSocketUpdates();
  
  // Small delay for stability
  delay(10);
}

void initializePins() {
  pinMode(MODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(MIC_ANALOG_PIN, INPUT);
  
  digitalWrite(STATUS_LED_PIN, HIGH); // Status LED on during startup
}

void initializeLEDs() {
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(map(config.brightness, 0, 100, 0, MAX_BRIGHTNESS));
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 6000); // Safety limit
  FastLED.clear();
  FastLED.show();
}

void initializeAudio() {
  // Initialize audio sampling
  analogReadResolution(12); // 12-bit ADC resolution
  for (int i = 0; i < SAMPLES; i++) {
    audioSamples[i] = 0;
  }
}

void loadConfiguration() {
  preferences.begin("ledsign", false);
  
  // Load settings with defaults
  config.brightness = preferences.getUChar("brightness", 80);
  config.speed = preferences.getUChar("speed", 50);
  config.mode = preferences.getUChar("mode", 0);
  config.sensitivity = preferences.getUChar("sensitivity", 70);
  config.noiseGate = preferences.getUChar("noiseGate", 30);
  config.autoGain = preferences.getBool("autoGain", true);
  config.sleepTimer = preferences.getUShort("sleepTimer", 0);
  
  preferences.getString("deviceName", config.deviceName, sizeof(config.deviceName));
  preferences.getString("ssid", config.ssid, sizeof(config.ssid));
  preferences.getString("password", config.password, sizeof(config.password));
  
  preferences.end();
  
  currentMode = config.mode;
}

void saveConfiguration() {
  preferences.begin("ledsign", false);
  
  preferences.putUChar("brightness", config.brightness);
  preferences.putUChar("speed", config.speed);
  preferences.putUChar("mode", config.mode);
  preferences.putUChar("sensitivity", config.sensitivity);
  preferences.putUChar("noiseGate", config.noiseGate);
  preferences.putBool("autoGain", config.autoGain);
  preferences.putUShort("sleepTimer", config.sleepTimer);
  
  preferences.putString("deviceName", config.deviceName);
  preferences.putString("ssid", config.ssid);
  preferences.putString("password", config.password);
  
  preferences.end();
}

void initializeWiFi() {
  // Try to connect to saved WiFi first
  if (strlen(config.ssid) > 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);
    
    Serial.print("Connecting to WiFi");
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < (config.wifiTimeout * 1000)) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.print("Connected! IP: ");
      Serial.println(WiFi.localIP());
      digitalWrite(STATUS_LED_PIN, LOW); // Status LED off when connected
      return;
    }
  }
  
  // If connection failed, start AP mode
  startAPMode();
}

void startAPMode() {
  Serial.println("Starting Access Point mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  
  configMode = true;
  
  // Blink status LED to indicate AP mode
  for (int i = 0; i < 6; i++) {
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    delay(200);
  }
}

void handleButtons() {
  // Handle Mode Button
  if (digitalRead(MODE_BUTTON_PIN) == LOW) {
    if (millis() - lastModePress > 50) { // Debounce
      unsigned long pressStart = millis();
      while (digitalRead(MODE_BUTTON_PIN) == LOW) {
        delay(10);
      }
      unsigned long pressDuration = millis() - pressStart;
      
      if (pressDuration > 2000) {
        // Long press - enter config mode
        toggleConfigMode();
      } else {
        // Short press - next mode
        nextMode();
      }
      lastModePress = millis();
    }
  }
  
  // Handle Power Button
  if (digitalRead(POWER_BUTTON_PIN) == LOW) {
    if (millis() - lastPowerPress > 50) { // Debounce
      unsigned long pressStart = millis();
      while (digitalRead(POWER_BUTTON_PIN) == LOW) {
        delay(10);
      }
      unsigned long pressDuration = millis() - pressStart;
      
      if (pressDuration > 3000) {
        // Long press - factory reset
        factoryReset();
      } else {
        // Short press - toggle power
        togglePower();
      }
      lastPowerPress = millis();
    }
  }
}

void nextMode() {
  currentMode = (currentMode + 1) % MODE_COUNT;
  config.mode = currentMode;
  saveConfiguration();
  
  Serial.print("Mode changed to: ");
  Serial.println(modeNames[currentMode]);
  
  // Flash LEDs to indicate mode change
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  delay(100);
  FastLED.clear();
  FastLED.show();
}

void togglePower() {
  systemOn = !systemOn;
  Serial.print("System ");
  Serial.println(systemOn ? "ON" : "OFF");
  
  if (!systemOn) {
    FastLED.clear();
    FastLED.show();
  }
}

void toggleConfigMode() {
  configMode = !configMode;
  Serial.print("Config mode ");
  Serial.println(configMode ? "ON" : "OFF");
  
  if (configMode) {
    startAPMode();
  }
}

void factoryReset() {
  Serial.println("Factory Reset!");
  preferences.begin("ledsign", false);
  preferences.clear();
  preferences.end();
  
  // Flash LEDs red to indicate reset
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(1000);
  
  ESP.restart();
}

void processAudio() {
  // Read analog value from microphone
  uint16_t sample = analogRead(MIC_ANALOG_PIN);
  
  // Apply noise gate
  if (sample < map(config.noiseGate, 0, 100, 0, 4095)) {
    sample = 0;
  }
  
  // Store sample
  audioSamples[sampleIndex] = sample;
  sampleIndex = (sampleIndex + 1) % SAMPLES;
  
  // Calculate audio level (RMS)
  unsigned long sum = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sum += audioSamples[i] * audioSamples[i];
  }
  audioLevel = sqrt(sum / SAMPLES);
  
  // Apply sensitivity
  audioLevel = map(audioLevel, 0, 4095, 0, map(config.sensitivity, 0, 100, 0, 4095));
  audioLevel = constrain(audioLevel, 0, 4095);
}

void updateLEDs() {
  if (!systemOn) {
    return;
  }
  
  switch (currentMode) {
    case MODE_SOLID_COLOR:
      solidColor();
      break;
    case MODE_RAINBOW_STATIC:
      rainbowStatic();
      break;
    case MODE_RAINBOW_CYCLE:
      rainbowCycle();
      break;
    case MODE_COLOR_WIPE:
      colorWipe();
      break;
    case MODE_THEATER_CHASE:
      theaterChase();
      break;
    case MODE_BREATHING:
      breathing();
      break;
    case MODE_FIRE_EFFECT:
      fireEffect();
      break;
    case MODE_VU_METER:
      vuMeter();
      break;
    case MODE_SPECTRUM_ANALYZER:
      spectrumAnalyzer();
      break;
    case MODE_BEAT_FLASH:
      beatFlash();
      break;
    case MODE_SOUND_COLORS:
      soundColors();
      break;
    case MODE_AUDIO_RIPPLE:
      audioRipple();
      break;
  }
  
  FastLED.setBrightness(map(config.brightness, 0, 100, 0, MAX_BRIGHTNESS));
  FastLED.show();
}

// LED Pattern Functions
void solidColor() {
  fill_solid(leds, NUM_LEDS, config.currentColor);
}

void rainbowStatic() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(map(i, 0, NUM_LEDS-1, 0, 255), 255, 255);
  }
}

void rainbowCycle() {
  static uint8_t hue = 0;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((hue + (i * 255 / NUM_LEDS)) & 255, 255, 255);
  }
  hue += map(config.speed, 0, 100, 1, 10);
}

void colorWipe() {
  static uint8_t pos = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > map(config.speed, 0, 100, 100, 10)) {
    FastLED.clear();
    for (int i = 0; i <= pos; i++) {
      leds[i] = config.currentColor;
    }
    pos++;
    if (pos >= NUM_LEDS) pos = 0;
    lastUpdate = millis();
  }
}

void theaterChase() {
  static uint8_t offset = 0;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > map(config.speed, 0, 100, 200, 20)) {
    FastLED.clear();
    for (int i = offset; i < NUM_LEDS; i += 3) {
      leds[i] = config.currentColor;
    }
    offset = (offset + 1) % 3;
    lastUpdate = millis();
  }
}

void breathing() {
  static uint8_t brightness = 0;
  static int8_t direction = 1;
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > map(config.speed, 0, 100, 50, 5)) {
    brightness += direction * 5;
    if (brightness >= 255 || brightness <= 0) {
      direction = -direction;
    }
    
    fill_solid(leds, NUM_LEDS, config.currentColor);
    FastLED.setBrightness(brightness);
    lastUpdate = millis();
  }
}

void fireEffect() {
  static byte heat[NUM_LEDS];
  
  // Cool down every cell a little
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((55 * 10) / NUM_LEDS) + 2));
  }
  
  // Heat from each cell drifts 'up' and diffuses a little
  for (int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  
  // Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < 120) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  
  // Map from heat cells to LED colors
  for (int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor(heat[j]);
    leds[j] = color;
  }
}

void vuMeter() {
  FastLED.clear();
  
  int numLEDs = map(audioLevel, 0, 4095, 0, NUM_LEDS);
  
  for (int i = 0; i < numLEDs; i++) {
    if (i < NUM_LEDS / 3) {
      leds[i] = CRGB::Green;
    } else if (i < (NUM_LEDS * 2) / 3) {
      leds[i] = CRGB::Yellow;
    } else {
      leds[i] = CRGB::Red;
    }
  }
}

void spectrumAnalyzer() {
  // Simplified spectrum analyzer using audio level
  FastLED.clear();
  
  int bands = 8;
  int ledsPerBand = NUM_LEDS / bands;
  
  for (int band = 0; band < bands; band++) {
    int height = map(audioLevel + random8(0, 100), 0, 4095, 0, ledsPerBand);
    uint8_t hue = map(band, 0, bands-1, 0, 255);
    
    for (int i = 0; i < height; i++) {
      int ledIndex = band * ledsPerBand + i;
      if (ledIndex < NUM_LEDS) {
        leds[ledIndex] = CHSV(hue, 255, 255);
      }
    }
  }
}

void beatFlash() {
  static uint16_t lastAudioLevel = 0;
  static unsigned long lastBeat = 0;
  
  // Simple beat detection
  if (audioLevel > lastAudioLevel + 500 && millis() - lastBeat > 300) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    lastBeat = millis();
  } else {
    fadeToBlackBy(leds, NUM_LEDS, 50);
  }
  
  lastAudioLevel = audioLevel;
}

void soundColors() {
  uint8_t hue = map(audioLevel, 0, 4095, 0, 255);
  uint8_t brightness = map(audioLevel, 0, 4095, 0, 255);
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, brightness));
}

void audioRipple() {
  static int center = NUM_LEDS / 2;
  static uint8_t wave = 0;
  
  if (audioLevel > 100) {
    wave = 255;
  }
  
  if (wave > 0) {
    fadeToBlackBy(leds, NUM_LEDS, 10);
    
    int size = map(255 - wave, 0, 255, 0, NUM_LEDS / 2);
    uint8_t brightness = wave;
    
    for (int i = max(0, center - size); i <= min(NUM_LEDS - 1, center + size); i++) {
      leds[i] = CHSV(map(audioLevel, 0, 4095, 0, 255), 255, brightness);
    }
    
    wave = max(0, wave - 10);
  }
}

void testLEDs() {
  // Quick LED test on startup
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(300);
  
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(300);
  
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(300);
  
  FastLED.clear();
  FastLED.show();
}

void sendWebSocketUpdates() {
  static unsigned long lastUpdate = 0;
  
  if (millis() - lastUpdate > 100) { // Send updates every 100ms
    StaticJsonDocument<512> doc;
    doc["type"] = "status_update";
    doc["power"] = systemOn;
    doc["mode"] = currentMode;
    doc["mode_name"] = modeNames[currentMode];
    doc["brightness"] = config.brightness;
    doc["audio_level"] = audioLevel;
    doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
    doc["uptime"] = millis() / 1000;
    
    String message;
    serializeJson(doc, message);
    webSocket.broadcastTXT(message);
    
    lastUpdate = millis();
  }
} 