#ifndef CONFIG_H
#define CONFIG_H

// Project Information
#define PROJECT_NAME "LED Sign with Microphone Control"
#define PROJECT_VERSION "1.0.0"
#define BUILD_DATE __DATE__
#define BUILD_TIME __TIME__

// Hardware Pin Definitions
#ifndef LED_DATA_PIN
#define LED_DATA_PIN     2      // WS2812B LED strip data pin
#endif

#ifndef MIC_ANALOG_PIN
#define MIC_ANALOG_PIN   3      // MAX4466 microphone analog output
#endif

#ifndef MODE_BUTTON_PIN
#define MODE_BUTTON_PIN  4      // Mode change button
#endif

#ifndef POWER_BUTTON_PIN
#define POWER_BUTTON_PIN 5      // Power on/off button
#endif

#ifndef STATUS_LED_PIN
#define STATUS_LED_PIN   8      // Built-in status LED
#endif

// LED Configuration
#ifndef NUM_LEDS
#define NUM_LEDS         100    // Number of LEDs in the strip
#endif

#define LED_TYPE         WS2812B
#define COLOR_ORDER      GRB
#define MAX_BRIGHTNESS   255
#define DEFAULT_BRIGHTNESS 80
#define FRAMES_PER_SECOND 60
#define LED_VOLTAGE      5      // LED strip voltage
#define LED_MAX_CURRENT  6000   // Maximum current in mA (6A)

// Audio Configuration
#define SAMPLE_RATE      8000   // Audio sampling rate in Hz
#define SAMPLES          256    // Number of audio samples to store
#define ADC_RESOLUTION   12     // 12-bit ADC resolution (0-4095)
#define NOISE_THRESHOLD  30     // Default noise gate threshold
#define DEFAULT_SENSITIVITY 70  // Default microphone sensitivity (0-100%)
#define AUDIO_SMOOTHING  5      // Audio level smoothing factor

// Network Configuration
#define AP_SSID          "LED-Sign-Setup"
#define AP_PASSWORD      "configure123"
#define AP_CHANNEL       6
#define AP_MAX_CLIENTS   4
#define DNS_PORT         53
#define WEB_PORT         80
#define WEBSOCKET_PORT   81
#define WIFI_TIMEOUT     30000  // WiFi connection timeout in ms
#define RECONNECT_DELAY  5000   // WiFi reconnection delay in ms

// Web Server Configuration
#define MAX_CLIENTS      4      // Maximum concurrent web clients
#define WEBSOCKET_PING_INTERVAL 30000  // WebSocket ping interval in ms
#define API_RATE_LIMIT   100    // API requests per minute per IP

// Button Configuration
#define BUTTON_DEBOUNCE_TIME 50    // Button debounce time in ms
#define LONG_PRESS_TIME      2000  // Long press duration for mode button in ms
#define FACTORY_RESET_TIME   3000  // Long press duration for factory reset in ms

// System Configuration
#define DEVICE_NAME      "LED-Sign"
#define HOSTNAME         "led-sign"
#define TIMEZONE         "UTC-5"
#define NTP_SERVER       "pool.ntp.org"
#define NTP_OFFSET       -18000  // UTC-5 offset in seconds

// Memory Configuration
#define CONFIG_PARTITION_SIZE 0x1000  // 4KB for configuration storage
#define SPIFFS_SIZE      0x200000     // 2MB for web files
#define MIN_FREE_HEAP    32768        // Minimum free heap (32KB)

// LED Pattern Modes
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
    MODE_COUNT  // Keep this last for counting modes
};

// System States
enum SystemState {
    STATE_NORMAL = 0,
    STATE_CONFIG,
    STATE_UPDATE,
    STATE_ERROR
};

// Default Configuration Values
#define DEFAULT_SPEED           50
#define DEFAULT_COLOR_R         255
#define DEFAULT_COLOR_G         0
#define DEFAULT_COLOR_B         0
#define DEFAULT_MODE            MODE_RAINBOW_CYCLE
#define DEFAULT_SLEEP_TIMER     0
#define DEFAULT_AUTO_BRIGHTNESS false

// Audio Processing Configuration
#define BEAT_DETECT_THRESHOLD   500   // Beat detection threshold
#define BEAT_COOLDOWN_TIME      300   // Minimum time between beats in ms
#define FREQUENCY_BINS          8     // Number of frequency analysis bins
#define SPECTRUM_DECAY_RATE     50    // Spectrum analyzer decay rate

// Performance Configuration
#define TASK_STACK_SIZE         4096  // Stack size for FreeRTOS tasks
#define AUDIO_TASK_PRIORITY     2     // Audio processing task priority
#define LED_TASK_PRIORITY       1     // LED update task priority
#define WEB_TASK_PRIORITY       1     // Web server task priority

// Debugging Configuration
#ifdef DEBUG
    #define DEBUG_SERIAL_SPEED  115200
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTF(x, y)  Serial.printf(x, y)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(x, y)
#endif

// Error Codes
#define ERROR_NONE              0
#define ERROR_WIFI_FAILED       1
#define ERROR_SPIFFS_FAILED     2
#define ERROR_LED_INIT_FAILED   3
#define ERROR_AUDIO_INIT_FAILED 4
#define ERROR_CONFIG_LOAD_FAILED 5
#define ERROR_OUT_OF_MEMORY     6

// Feature Flags (can be disabled to save memory)
#ifndef MINIMAL_BUILD
    #define ENABLE_OTA_UPDATES      1
    #define ENABLE_NTP_TIME         1
    #define ENABLE_CAPTIVE_PORTAL   1
    #define ENABLE_BEAT_DETECTION   1
    #define ENABLE_SPECTRUM_ANALYZER 1
    #define ENABLE_WEB_AUTHENTICATION 0  // Disabled by default for easier setup
#else
    #define ENABLE_OTA_UPDATES      0
    #define ENABLE_NTP_TIME         0
    #define ENABLE_CAPTIVE_PORTAL   0
    #define ENABLE_BEAT_DETECTION   0
    #define ENABLE_SPECTRUM_ANALYZER 0
    #define ENABLE_WEB_AUTHENTICATION 0
#endif

// File Paths
#define CONFIG_FILE_PATH        "/config.json"
#define PRESETS_FILE_PATH       "/presets.json"
#define WIFI_CONFIG_PATH        "/wifi.json"

// API Endpoints
#define API_BASE_PATH           "/api"
#define API_STATUS_PATH         "/api/status"
#define API_POWER_PATH          "/api/power"
#define API_MODE_PATH           "/api/mode"
#define API_CONFIG_PATH         "/api/config"
#define API_WIFI_SCAN_PATH      "/api/wifi/scan"
#define API_WIFI_CONNECT_PATH   "/api/wifi/connect"
#define API_AUDIO_LEVEL_PATH    "/api/audio/level"
#define API_RESET_PATH          "/api/reset"

// WebSocket Message Types
#define WS_TYPE_STATUS_UPDATE   "status_update"
#define WS_TYPE_AUDIO_DATA      "audio_data"
#define WS_TYPE_CONFIG_CHANGED  "config_changed"
#define WS_TYPE_COMMAND_RESULT  "command_result"
#define WS_TYPE_WELCOME         "welcome"

// Validation Limits
#define MAX_DEVICE_NAME_LENGTH  32
#define MAX_SSID_LENGTH         32
#define MAX_PASSWORD_LENGTH     64
#define MAX_TIMEZONE_LENGTH     16
#define MIN_BRIGHTNESS          0
#define MAX_BRIGHTNESS          100
#define MIN_SPEED               0
#define MAX_SPEED               100
#define MIN_SENSITIVITY         0
#define MAX_SENSITIVITY         100

// Timing Constants
#define MAIN_LOOP_DELAY         10    // Main loop delay in ms
#define STATUS_UPDATE_INTERVAL  100   // Status update interval in ms
#define AUDIO_UPDATE_INTERVAL   33    // Audio update interval in ms (~30 FPS)
#define LED_UPDATE_INTERVAL     16    // LED update interval in ms (~60 FPS)
#define WIFI_CHECK_INTERVAL     30000 // WiFi status check interval in ms

// Color Definitions (RGB)
#define COLOR_RED               0xFF0000
#define COLOR_GREEN             0x00FF00
#define COLOR_BLUE              0x0000FF
#define COLOR_WHITE             0xFFFFFF
#define COLOR_BLACK             0x000000
#define COLOR_YELLOW            0xFFFF00
#define COLOR_CYAN              0x00FFFF
#define COLOR_MAGENTA           0xFF00FF
#define COLOR_ORANGE            0xFF8000
#define COLOR_PURPLE            0x8000FF

// Macro Utilities
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))
#define CONSTRAIN(x, a, b)      ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define MAP(x, in_min, in_max, out_min, out_max) \
    ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

#endif // CONFIG_H 