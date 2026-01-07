#ifndef CONFIG_H
#define CONFIG_H

// ========================
// VULNERABLE CONFIGURATION
// ========================
// WARNING: Credentials hardcoded - FOR DEMONSTRATION ONLY

// Wi-Fi Credentials (HARDCODED - VULNERABLE!)
#define WIFI_SSID "Bang"
#define WIFI_PASSWORD "12345678"

// API Configuration (HARDCODED - VULNERABLE!)
#define API_URL "http://192.168.137.1:5000/api/sensor/data"
#define API_KEY "sk_live_51234567890abcdef"

// MQTT Configuration (DISABLED IN THIS VERSION)
#define MQTT_BROKER "192.168.137.1"
#define MQTT_PORT 1883
#define MQTT_USER "esp32_client"
#define MQTT_PASSWORD "mqtt_password"
#define MQTT_TOPIC "home/sensor/esp32_001"

// Device Configuration
#define DEVICE_ID "esp32_001"
#define DEVICE_NAME "Living Room Sensor"

// Pin Definitions for ESP32-S3
#define DHT_PIN 4
#define DHT_TYPE DHT11

// LDR Pin - IMPORTANT: GPIO1 is strapping pin
// If boot issues occur, try GPIO10 or GPIO11
#define LDR_PIN 1           // GPIO1 (ADC1_CH0)

// I2C Pins for OLED
#define I2C_SDA 14          // Custom I2C SDA
#define I2C_SCL 13          // Custom I2C SCL

// LED Pin Definitions
// WARNING: Check your board schematic for conflicts
// ESP32-S3-DevKitC-1 has RGB LED at GPIO38 (WS2812)
#define LED_STATUS_PIN 2    // Status LED (Blue)
#define LED_WIFI_PIN 5      // WiFi indicator (Green)
#define LED_SENSOR_PIN 6    // Sensor activity (Yellow)
#define LED_ALERT_PIN 7     // Alert/Warning (Red)
#define LED_CONTROL_PIN 8   // Remote control (White)

// Timing Configuration
#define SENSOR_INTERVAL 10000       // 10 seconds
#define WIFI_TIMEOUT 20000          // 20 seconds
#define MQTT_RECONNECT_DELAY 5000   // 5 seconds (not used)

// Sensor Validation Thresholds (SECURITY FEATURE)
#define TEMP_MIN -40.0
#define TEMP_MAX 80.0
#define HUMIDITY_MIN 0.0
#define HUMIDITY_MAX 100.0
#define LIGHT_MIN 0.0
#define LIGHT_MAX 100.0

// LED Control Settings
#define LED_BLINK_FAST 200      // Fast blink (ms)
#define LED_BLINK_SLOW 1000     // Slow blink (ms)
#define LED_BRIGHTNESS_PWM 128  // PWM brightness (0-255)

// LED States Enum
enum LEDState {
    LED_OFF = 0,
    LED_ON = 1,
    LED_BLINK_FAST_STATE = 2,
    LED_BLINK_SLOW_STATE = 3,
    LED_FADE = 4
};

// Debug Settings
#define DEBUG_SERIAL true
#define DEBUG_SENSORS true

// Feature Flags
#define ENABLE_MQTT false       // MQTT disabled for HTTP-only mode
#define ENABLE_OTA false        // OTA updates disabled
#define ENABLE_WIFI_SCAN true   // WiFi scanning enabled

#endif