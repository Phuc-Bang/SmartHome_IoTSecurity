#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <esp_wifi.h>  // For WiFi configuration
#include "config.h"

// ========================
// GLOBAL OBJECTS
// ========================
DHT dht(DHT_PIN, DHT_TYPE);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);  // Defined but not used in HTTP-only mode
HTTPClient http;

// ========================
// OLED DISPLAY ICONS (8x8 bitmaps)
// ========================
// Temperature icon (thermometer)
const unsigned char PROGMEM icon_temp[] = {
    0x1C, 0x22, 0x2A, 0x2A, 0x2A, 0x3E, 0x3E, 0x1C
};

// Humidity icon (water drop)
const unsigned char PROGMEM icon_humidity[] = {
    0x08, 0x14, 0x14, 0x22, 0x41, 0x41, 0x41, 0x3E
};

// Light icon (sun)
const unsigned char PROGMEM icon_light[] = {
    0x10, 0x54, 0x38, 0xFE, 0x38, 0x54, 0x10, 0x00
};

// WiFi icon
const unsigned char PROGMEM icon_wifi[] = {
    0x00, 0x3C, 0x42, 0x18, 0x24, 0x00, 0x18, 0x00
};

// Heart icon (for status)
const unsigned char PROGMEM icon_heart[] = {
    0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00
};

// Clock icon
const unsigned char PROGMEM icon_clock[] = {
    0x3C, 0x42, 0x91, 0x91, 0x8D, 0x81, 0x42, 0x3C
};

// Alert icon
const unsigned char PROGMEM icon_alert[] = {
    0x18, 0x18, 0x3C, 0x3C, 0x7E, 0x7E, 0x00, 0x7E
};

// Server/System icon
const unsigned char PROGMEM icon_server[] = {
    0xFF, 0x81, 0xBD, 0x81, 0xFF, 0x81, 0xBD, 0x81
};

// ========================
// OLED DISPLAY VARIABLES
// ========================
enum DisplayScreen {
    SCREEN_MAIN = 0,
    SCREEN_STATS,
    SCREEN_WIFI,
    SCREEN_ALERT,
    SCREEN_COUNT
};

DisplayScreen currentScreen = SCREEN_MAIN;
unsigned long lastScreenChange = 0;
unsigned long lastAnimationFrame = 0;
int animationFrame = 0;
bool screenNeedsUpdate = true;

#define SCREEN_CHANGE_INTERVAL 5000   // 5 seconds between screens
#define ANIMATION_INTERVAL 100        // 100ms per animation frame
#define PROGRESS_BAR_WIDTH 60
#define PROGRESS_BAR_HEIGHT 6

// ========================
// SENSOR DATA STRUCTURE
// ========================
struct SensorData {
    float temperature;
    float humidity;
    float light;
    unsigned long timestamp;
};

SensorData currentData;

// ========================
// LED CONTROL VARIABLES
// ========================
struct LEDControl {
    int pin;
    LEDState state;
    unsigned long lastToggle;
    bool currentState;
    int brightness;
};

LEDControl leds[] = {
    {LED_STATUS_PIN, LED_OFF, 0, false, 255},      // Status LED
    {LED_WIFI_PIN, LED_OFF, 0, false, 255},        // WiFi LED
    {LED_SENSOR_PIN, LED_OFF, 0, false, 255},      // Sensor LED
    {LED_ALERT_PIN, LED_OFF, 0, false, 255},       // Alert LED
    {LED_CONTROL_PIN, LED_OFF, 0, false, 255}      // Control LED
};

const int NUM_LEDS = sizeof(leds) / sizeof(leds[0]);

// ========================
// FUNCTION DECLARATIONS
// ========================
void setupWiFi();
void setupSensors();
void setupDisplay();
void setupLEDs();
void readSensors();
void displayData();
void sendDataHTTP();
void printSystemInfo();
void checkSensorHealth();
void scanWiFiNetworks();
void updateLEDs();
void setLED(int ledIndex, LEDState state);
void setLEDBrightness(int ledIndex, int brightness);
void handleLEDEffects();

// OLED Display Functions
void updateDisplay();
void displayMainScreen();
void displayStatsScreen();
void displayWiFiScreen();
void displayAlertScreen();
void drawProgressBar(int x, int y, int width, int height, int percentage);
void drawHeader(const char* title);
void drawFooter();
void displayStartupAnimation();

// ========================
// SETUP
// ========================
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("ESP32-S3 Smart Home - VULNERABLE VERSION");
    Serial.println("Board: ESP32-S3 DevKit-C-1");
    Serial.println("Device ID: " DEVICE_ID);
    Serial.println("Mode: HTTP Only (MQTT Disabled)");
    Serial.println("========================================\n");
    
    // Initialize I2C (will be done by setupDisplay)
    Serial.printf("[I2C] Pins configured: SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
    
    // Setup components
    setupDisplay();
    
    // Show startup animation
    displayStartupAnimation();
    
    // Scan WiFi networks before connecting
    scanWiFiNetworks();
    
    setupWiFi();
    setupSensors();
    setupLEDs();
    
    // MQTT is disabled in this version
    Serial.println("\n[MQTT] ⚠️  DISABLED - HTTP only mode");
    
    // Print system information for debugging
    printSystemInfo();
    
    Serial.println("\n[INIT] ✅ System ready!");
    Serial.println("[WARNING] ⚠️  Running in VULNERABLE mode!");
    Serial.println("[WARNING] ⚠️  Credentials are HARDCODED!");
    Serial.println("[WARNING] ⚠️  HTTP plaintext transmission!");
}

// ========================
// MAIN LOOP
// ========================
void loop() {
    static unsigned long lastRead = 0;
    static unsigned long lastHealthCheck = 0;
    
    // Check sensor health periodically (every 30 seconds)
    if (millis() - lastHealthCheck >= 30000) {
        lastHealthCheck = millis();
        checkSensorHealth();
    }
    
    // Read sensors periodically
    if (millis() - lastRead >= SENSOR_INTERVAL) {
        lastRead = millis();
        
        Serial.println("\n========== Sensor Reading ==========");
        
        // Blink sensor LED during reading
        setLED(2, LED_BLINK_FAST_STATE);
        
        readSensors();
        displayData();
        
        // Send data via HTTP (VULNERABLE - Plaintext)
        sendDataHTTP();
        
        // Turn off sensor LED after reading
        setLED(2, LED_OFF);
        
        Serial.println("====================================\n");
    }
    
    // Update LED effects
    updateLEDs();
    
    // Update OLED display (for animations and screen cycling)
    updateDisplay();
    
    delay(50);  // Faster loop for smoother animations
}

// ========================
// WIFI NETWORK SCANNER
// ========================
void scanWiFiNetworks() {
    Serial.println("\n[WiFi] 🔍 Scanning for networks...");
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        Serial.println("[WiFi] No networks found");
    } else {
        Serial.printf("[WiFi] Found %d networks:\n", n);
        
        bool targetFound = false;
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            int32_t rssi = WiFi.RSSI(i);
            wifi_auth_mode_t encryption = WiFi.encryptionType(i);
            
            String encType;
            switch (encryption) {
                case WIFI_AUTH_OPEN: encType = "OPEN"; break;
                case WIFI_AUTH_WEP: encType = "WEP"; break;
                case WIFI_AUTH_WPA_PSK: encType = "WPA"; break;
                case WIFI_AUTH_WPA2_PSK: encType = "WPA2"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: encType = "WPA/WPA2"; break;
                case WIFI_AUTH_WPA2_ENTERPRISE: encType = "WPA2-ENT"; break;
                case WIFI_AUTH_WPA3_PSK: encType = "WPA3"; break;
                default: encType = "UNKNOWN"; break;
            }
            
            Serial.printf("  %d: %s (%d dBm) [%s]", i + 1, ssid.c_str(), rssi, encType.c_str());
            
            if (ssid == WIFI_SSID) {
                Serial.print(" ← TARGET NETWORK FOUND!");
                targetFound = true;
            }
            Serial.println();
        }
        
        if (!targetFound) {
            Serial.println("\n[WiFi] ❌ Target network \"" WIFI_SSID "\" NOT FOUND!");
            Serial.println("[WiFi] Please check:");
            Serial.println("  - SSID spelling (case sensitive)");
            Serial.println("  - Network is broadcasting (not hidden)");
            Serial.println("  - Using 2.4GHz (ESP32 doesn't support 5GHz)");
        } else {
            Serial.println("\n[WiFi] ✅ Target network found, proceeding...");
        }
    }
    
    WiFi.scanDelete();
}

// ========================
// Wi-Fi SETUP (VULNERABLE)
// ========================
void setupWiFi() {
    Serial.println("\n[WiFi] Starting connection...");
    Serial.println("[WiFi] SSID: \"" WIFI_SSID "\"");
    Serial.printf("[WiFi] Password length: %d characters\n", strlen(WIFI_PASSWORD));
    Serial.println("[WARNING] ⚠️  Using hardcoded credentials!");
    
    // Set WiFi LED to blink while connecting
    setLED(1, LED_BLINK_FAST_STATE);
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting WiFi...");
    display.println("SSID: " WIFI_SSID);
    display.display();
    
    // Disconnect any previous connection and clear config
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1000);
    
    // Set WiFi mode and begin connection
    WiFi.mode(WIFI_STA);
    
    // Force WPA2 mode (not WPA3) for better compatibility
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;  // Force WPA2
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;  // Don't require PMF
    
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    
    // Optional: Set specific WiFi configuration for better compatibility
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    
    // Start connection
    WiFi.begin();
    
    Serial.println("[WiFi] Connecting");
    unsigned long startAttempt = millis();
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && 
           millis() - startAttempt < WIFI_TIMEOUT) {
        delay(500);
        Serial.print(".");
        updateLEDs(); // Keep LED blinking during connection
        
        attempts++;
        if (attempts % 10 == 0) {
            wl_status_t status = WiFi.status();
            Serial.printf("\n[WiFi] Attempt %d, Status: %d ", attempts, status);
            
            switch(status) {
                case WL_IDLE_STATUS:     Serial.println("(IDLE)"); break;
                case WL_NO_SSID_AVAIL:   Serial.println("(NO_SSID - Network not found!)"); break;
                case WL_SCAN_COMPLETED:  Serial.println("(SCAN_COMPLETED)"); break;
                case WL_CONNECTED:       Serial.println("(CONNECTED)"); break;
                case WL_CONNECT_FAILED:  Serial.println("(CONNECT_FAILED - Wrong password?)"); break;
                case WL_CONNECTION_LOST: Serial.println("(CONNECTION_LOST)"); break;
                case WL_DISCONNECTED:    Serial.println("(DISCONNECTED)"); break;
                default:                 Serial.println("(UNKNOWN)"); break;
            }
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] ✅ Connected successfully!");
        Serial.print("[WiFi] IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("[WiFi] MAC Address: ");
        Serial.println(WiFi.macAddress());
        Serial.print("[WiFi] Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
        Serial.print("[WiFi] Gateway: ");
        Serial.println(WiFi.gatewayIP());
        Serial.print("[WiFi] DNS: ");
        Serial.println(WiFi.dnsIP());
        
        // WiFi connected - solid green LED
        setLED(1, LED_ON);
        
        // Update display
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("WiFi Connected!");
        display.print("IP: ");
        display.println(WiFi.localIP());
        display.print("RSSI: ");
        display.print(WiFi.RSSI());
        display.println(" dBm");
        display.display();
        delay(3000);
        
    } else {
        Serial.println("\n[WiFi] ❌ Connection FAILED!");
        wl_status_t finalStatus = WiFi.status();
        Serial.printf("[WiFi] Final Status: %d ", finalStatus);
        
        switch(finalStatus) {
            case WL_NO_SSID_AVAIL:
                Serial.println("(Network not found)");
                Serial.println("[WiFi] Solutions:");
                Serial.println("  - Check SSID spelling: \"" WIFI_SSID "\"");
                Serial.println("  - Make sure network is broadcasting");
                Serial.println("  - Try 2.4GHz network (not 5GHz only)");
                break;
            case WL_CONNECT_FAILED:
                Serial.println("(Authentication failed)");
                Serial.println("[WiFi] Solutions:");
                Serial.println("  - Check password: \"" WIFI_PASSWORD "\"");
                Serial.println("  - Try WPA2 instead of WPA3");
                Serial.println("  - Check MAC filtering");
                break;
            default:
                Serial.println("(Unknown error)");
                Serial.println("[WiFi] General solutions:");
                Serial.println("  - Move closer to router");
                Serial.println("  - Restart router");
                Serial.println("  - Check network capacity");
                break;
        }
        
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("WiFi Failed!");
        display.println("Check:");
        display.println("- SSID/Password");
        display.println("- Network type");
        display.display();
        
        // WiFi failed - blink red LED
        setLED(1, LED_OFF);
        setLED(3, LED_BLINK_SLOW_STATE);
    }
}

// ========================
// SENSOR SETUP
// ========================
void setupSensors() {
    Serial.println("\n[Sensors] Initializing...");
    
    // DHT11
    dht.begin();
    Serial.println("[DHT11] ✅ Initialized");
    
    // LDR (MH Light Sensor Module)
    pinMode(LDR_PIN, INPUT);
    
    // Configure ADC for better accuracy on ESP32-S3
    analogReadResolution(12);        // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db);  // 0-3.3V range
    
    Serial.println("[LDR] ✅ Initialized (GPIO 1 - ADC1_CH0)");
    Serial.println("[Sensors] All sensors ready");
}

// ========================
// DISPLAY SETUP
// ========================
void setupDisplay() {
    Serial.println("\n[OLED] Initializing display...");
    
    // Initialize I2C first
    if (!Wire.begin(I2C_SDA, I2C_SCL)) {
        Serial.println("[I2C] ❌ Failed to initialize I2C bus");
        Serial.println("[I2C] Check wiring and pin configuration");
        return;
    }
    Serial.printf("[I2C] ✅ Initialized: SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
    
    // Try to initialize display with timeout
    bool displayFound = false;
    for (int attempts = 0; attempts < 3; attempts++) {
        if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            displayFound = true;
            break;
        }
        Serial.printf("[OLED] Attempt %d failed, retrying...\n", attempts + 1);
        delay(500);
    }
    
    if (!displayFound) {
        Serial.println("[OLED] ❌ ERROR - Display not found!");
        Serial.println("[OLED] Check wiring:");
        Serial.println("  VCC → 3.3V");
        Serial.println("  GND → GND");
        Serial.println("  SDA → GPIO14");
        Serial.println("  SCL → GPIO13");
        return;
    }
    
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP32-S3");
    display.println("Smart Home");
    display.println("IoT Security");
    display.println("Demo");
    display.display();
    delay(2000);
    
    Serial.println("[OLED] ✅ Initialized successfully (I2C: 0x3C)");
}

// ========================
// READ SENSORS
// ========================
void readSensors() {
    // Read DHT11
    currentData.temperature = dht.readTemperature();
    currentData.humidity = dht.readHumidity();
    
    // Read LDR (MH Light Sensor Module)
    int ldrValue = analogRead(LDR_PIN);
    // Convert ADC value to light percentage (0-100%)
    currentData.light = map(ldrValue, 0, 4095, 0, 100);
    
    // Timestamp
    currentData.timestamp = millis();
    
    // Validate sensor readings (BASIC SECURITY)
    bool tempValid = !isnan(currentData.temperature) && 
                     currentData.temperature >= TEMP_MIN && 
                     currentData.temperature <= TEMP_MAX;
    
    bool humiValid = !isnan(currentData.humidity) && 
                     currentData.humidity >= HUMIDITY_MIN && 
                     currentData.humidity <= HUMIDITY_MAX;
    
    bool lightValid = currentData.light >= LIGHT_MIN && 
                      currentData.light <= LIGHT_MAX;
    
    // Print to serial
    if (!tempValid || !humiValid) {
        Serial.println("[DHT11] ⚠️  Read ERROR or OUT OF RANGE!");
        Serial.printf("[DHT11] Temp: %.1f°C (Valid: %s), Humidity: %.1f%% (Valid: %s)\n", 
                      currentData.temperature, tempValid ? "YES" : "NO",
                      currentData.humidity, humiValid ? "YES" : "NO");
    } else {
        Serial.printf("[DHT11] 🌡️  Temp: %.1f°C, Humidity: %.1f%%\n", 
                      currentData.temperature, currentData.humidity);
    }
    
    if (!lightValid) {
        Serial.printf("[LDR] ⚠️  Raw: %d, Light: %.0f%% (OUT OF RANGE!)\n", ldrValue, currentData.light);
    } else {
        Serial.printf("[LDR] 💡 Raw: %d, Light: %.0f%%\n", ldrValue, currentData.light);
    }
    
    // Update LED effects based on sensor readings
    handleLEDEffects();
}

// ========================
// DISPLAY DATA ON OLED - MODERN UI
// ========================

// Draw a progress bar with border
void drawProgressBar(int x, int y, int width, int height, int percentage) {
    percentage = constrain(percentage, 0, 100);
    int fillWidth = (width - 2) * percentage / 100;
    
    // Draw border
    display.drawRect(x, y, width, height, SSD1306_WHITE);
    
    // Draw fill
    display.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
}

// Draw header with icon and title
void drawHeader(const char* title) {
    // Top line
    display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
    
    // Title
    display.setTextSize(1);
    display.setCursor(0, 1);
    display.print(title);
    
    // Animated dot indicator
    int dotX = 120 - (animationFrame % 3) * 4;
    display.fillCircle(dotX, 5, 2, SSD1306_WHITE);
}

// Draw footer with status info
void drawFooter() {
    // Bottom line
    display.drawLine(0, 54, 127, 54, SSD1306_WHITE);
    
    // Uptime
    unsigned long uptime = millis() / 1000;
    int hours = uptime / 3600;
    int mins = (uptime % 3600) / 60;
    int secs = uptime % 60;
    
    display.setTextSize(1);
    display.setCursor(0, 56);
    display.printf("%02d:%02d:%02d", hours, mins, secs);
    
    // WiFi indicator
    if (WiFi.status() == WL_CONNECTED) {
        display.drawBitmap(110, 56, icon_wifi, 8, 8, SSD1306_WHITE);
    } else {
        // Blink WiFi icon when disconnected
        if (animationFrame % 4 < 2) {
            display.drawBitmap(110, 56, icon_alert, 8, 8, SSD1306_WHITE);
        }
    }
}

// Main sensor display screen
void displayMainScreen() {
    display.clearDisplay();
    drawHeader("SMART HOME");
    
    int yPos = 14;
    int iconX = 2;
    int labelX = 14;
    int valueX = 70;
    int barX = 62;
    
    // Temperature
    display.drawBitmap(iconX, yPos, icon_temp, 8, 8, SSD1306_WHITE);
    display.setCursor(labelX, yPos);
    display.print("Temp");
    display.setCursor(valueX, yPos);
    if (!isnan(currentData.temperature)) {
        display.printf("%.1fC", currentData.temperature);
    } else {
        display.print("--.-C");
    }
    int tempPercent = map(constrain(currentData.temperature, 0, 50), 0, 50, 0, 100);
    drawProgressBar(barX, yPos + 9, PROGRESS_BAR_WIDTH, 4, tempPercent);
    
    yPos += 16;
    
    // Humidity
    display.drawBitmap(iconX, yPos, icon_humidity, 8, 8, SSD1306_WHITE);
    display.setCursor(labelX, yPos);
    display.print("Humi");
    display.setCursor(valueX, yPos);
    if (!isnan(currentData.humidity)) {
        display.printf("%.0f%%", currentData.humidity);
    } else {
        display.print("--%");
    }
    drawProgressBar(barX, yPos + 9, PROGRESS_BAR_WIDTH, 4, (int)currentData.humidity);
    
    yPos += 16;
    
    // Light
    display.drawBitmap(iconX, yPos, icon_light, 8, 8, SSD1306_WHITE);
    display.setCursor(labelX, yPos);
    display.print("Lght");
    display.setCursor(valueX, yPos);
    display.printf("%.0f%%", currentData.light);
    drawProgressBar(barX, yPos + 9, PROGRESS_BAR_WIDTH, 4, (int)currentData.light);
    
    drawFooter();
    display.display();
}

// System stats screen
void displayStatsScreen() {
    display.clearDisplay();
    drawHeader("SYSTEM INFO");
    
    int yPos = 14;
    
    // Device ID
    display.drawBitmap(2, yPos, icon_server, 8, 8, SSD1306_WHITE);
    display.setCursor(14, yPos);
    display.print("ID: ");
    display.print(DEVICE_ID);
    
    yPos += 12;
    
    // Memory
    display.drawBitmap(2, yPos, icon_clock, 8, 8, SSD1306_WHITE);
    display.setCursor(14, yPos);
    display.printf("Mem: %dKB", ESP.getFreeHeap() / 1024);
    
    yPos += 12;
    
    // CPU
    display.setCursor(14, yPos);
    display.printf("CPU: %dMHz", ESP.getCpuFreqMHz());
    
    yPos += 12;
    
    // Readings count
    static unsigned long readingCount = 0;
    readingCount++;
    display.setCursor(14, yPos);
    display.printf("Reads: %lu", readingCount);
    
    drawFooter();
    display.display();
}

// WiFi info screen
void displayWiFiScreen() {
    display.clearDisplay();
    drawHeader("NETWORK");
    
    int yPos = 14;
    
    display.drawBitmap(2, yPos, icon_wifi, 8, 8, SSD1306_WHITE);
    
    if (WiFi.status() == WL_CONNECTED) {
        display.setCursor(14, yPos);
        display.print("CONNECTED");
        
        yPos += 12;
        display.setCursor(4, yPos);
        display.print(WiFi.localIP());
        
        yPos += 10;
        display.setCursor(4, yPos);
        display.printf("RSSI: %d dBm", WiFi.RSSI());
        
        // Signal strength bar
        int rssi = WiFi.RSSI();
        int signalPercent = map(constrain(rssi, -90, -30), -90, -30, 0, 100);
        yPos += 10;
        display.setCursor(4, yPos);
        display.print("Signal:");
        drawProgressBar(50, yPos, 70, 6, signalPercent);
        
    } else {
        display.setCursor(14, yPos);
        display.print("DISCONNECTED");
        
        yPos += 14;
        display.setCursor(4, yPos);
        display.print("Reconnecting...");
        
        // Animated loading dots
        for (int i = 0; i <= animationFrame % 4; i++) {
            display.print(".");
        }
    }
    
    drawFooter();
    display.display();
}

// Alert screen (shown when there are issues)
void displayAlertScreen() {
    display.clearDisplay();
    
    // Flashing border for alerts
    if (animationFrame % 2 == 0) {
        display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
        display.drawRect(1, 1, 126, 62, SSD1306_WHITE);
    }
    
    display.drawBitmap(60, 8, icon_alert, 8, 8, SSD1306_WHITE);
    
    display.setTextSize(1);
    display.setCursor(45, 20);
    display.print("ALERT!");
    
    display.setCursor(10, 32);
    
    // Check for issues
    bool hasIssue = false;
    
    if (isnan(currentData.temperature) || isnan(currentData.humidity)) {
        display.print("Sensor Error!");
        hasIssue = true;
    } else if (currentData.temperature > 40) {
        display.print("HIGH TEMP!");
        hasIssue = true;
    } else if (currentData.humidity > 80) {
        display.print("HIGH HUMIDITY!");
        hasIssue = true;
    } else if (WiFi.status() != WL_CONNECTED) {
        display.print("WiFi Lost!");
        hasIssue = true;
    }
    
    if (!hasIssue) {
        display.setCursor(25, 32);
        display.print("All Systems OK");
        display.drawBitmap(60, 45, icon_heart, 8, 8, SSD1306_WHITE);
    }
    
    display.display();
}

// Startup animation
void displayStartupAnimation() {
    // Logo animation
    for (int i = 0; i < 128; i += 8) {
        display.clearDisplay();
        
        // Draw expanding rectangle
        int rectSize = min(i, 40);
        display.drawRect(64 - rectSize, 32 - rectSize/2, rectSize*2, rectSize, SSD1306_WHITE);
        
        display.display();
        delay(20);
    }
    
    // Text fade in
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(15, 20);
    display.print("ESP32-S3");
    display.setTextSize(1);
    display.setCursor(25, 45);
    display.print("Smart Home IoT");
    display.display();
    delay(1500);
    
    // Loading bar animation
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(35, 25);
    display.print("Loading...");
    
    for (int i = 0; i <= 100; i += 5) {
        display.fillRect(14, 40, i + 4, 10, SSD1306_BLACK);
        drawProgressBar(14, 40, 100, 10, i);
        display.display();
        delay(50);
    }
    
    delay(500);
}

// Main display update function
void updateDisplay() {
    unsigned long currentTime = millis();
    
    // Update animation frame
    if (currentTime - lastAnimationFrame >= ANIMATION_INTERVAL) {
        lastAnimationFrame = currentTime;
        animationFrame++;
        screenNeedsUpdate = true;
    }
    
    // Auto-cycle through screens
    if (currentTime - lastScreenChange >= SCREEN_CHANGE_INTERVAL) {
        lastScreenChange = currentTime;
        currentScreen = (DisplayScreen)((currentScreen + 1) % SCREEN_COUNT);
        screenNeedsUpdate = true;
    }
    
    // Only update display when needed
    if (!screenNeedsUpdate) return;
    screenNeedsUpdate = false;
    
    switch (currentScreen) {
        case SCREEN_MAIN:
            displayMainScreen();
            break;
        case SCREEN_STATS:
            displayStatsScreen();
            break;
        case SCREEN_WIFI:
            displayWiFiScreen();
            break;
        case SCREEN_ALERT:
            displayAlertScreen();
            break;
        default:
            displayMainScreen();
            break;
    }
}

// Legacy function - now calls updateDisplay
void displayData() {
    screenNeedsUpdate = true;
    updateDisplay();
}

// ========================
// SEND DATA VIA HTTP (VULNERABLE - PLAINTEXT)
// ========================
void sendDataHTTP() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[HTTP] ❌ WiFi not connected!");
        setLED(3, LED_BLINK_FAST_STATE); // Alert LED
        return;
    }
    
    // Create JSON payload
    DynamicJsonDocument doc(1024);
    doc["device_id"] = DEVICE_ID;
    doc["temperature"] = currentData.temperature;
    doc["humidity"] = currentData.humidity;
    doc["light"] = currentData.light;
    doc["timestamp"] = currentData.timestamp;
    
    String jsonData;
    serializeJson(doc, jsonData);
    
    Serial.println("\n[HTTP] 📤 Sending data...");
    Serial.println("[WARNING] ⚠️  Using HTTP (plaintext - vulnerable to MITM)!");
    Serial.println("[HTTP] URL: " API_URL);
    Serial.println("[HTTP] Payload: " + jsonData);
    
    // VULNERABLE: HTTP instead of HTTPS
    http.begin(wifiClient, API_URL);
    
    // Add headers for better compatibility
    http.addHeader("Content-Type", "application/json");
    http.addHeader("User-Agent", "ESP32-S3/1.0");
    http.addHeader("Accept", "application/json");
    http.addHeader("Connection", "close");
    
    // VULNERABLE: API key in header (plaintext)
    http.addHeader("Authorization", "Bearer " API_KEY);
    http.addHeader("X-API-Key", API_KEY);
    
    // Set timeout and connection settings
    http.setTimeout(10000);  // 10 seconds timeout
    http.setReuse(false);    // Don't reuse connections
    
    int httpCode = http.POST(jsonData);
    
    if (httpCode > 0) {
        Serial.printf("[HTTP] ✅ Response code: %d\n", httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
            String response = http.getString();
            Serial.println("[HTTP] Response: " + response);
            setLED(1, LED_ON); // WiFi LED solid green on success
        }
    } else if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
        Serial.println("[HTTP] ❌ Connection refused - Server not running?");
        Serial.println("[HTTP] 💡 Start backend: python simple_server.py");
        setLED(3, LED_BLINK_FAST_STATE); // Alert LED
    } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
        Serial.println("[HTTP] ❌ Connection lost - Network issue?");
        setLED(3, LED_BLINK_SLOW_STATE);
    } else if (httpCode == HTTPC_ERROR_READ_TIMEOUT) {
        Serial.println("[HTTP] ❌ Read timeout - Server too slow?");
        setLED(3, LED_BLINK_SLOW_STATE);
    } else {
        Serial.printf("[HTTP] ❌ Error: %s (Code: %d)\n", 
                     http.errorToString(httpCode).c_str(), httpCode);
        Serial.println("[HTTP] Troubleshooting:");
        Serial.println("  1. Check Windows Firewall (allow Python)");
        Serial.println("  2. Try: curl http://192.168.137.1:5000/api/devices");
        Serial.println("  3. Restart backend server");
        Serial.println("  4. Check antivirus blocking connections");
        setLED(3, LED_BLINK_FAST_STATE);
    }
    
    http.end();
}

// ========================
// SYSTEM INFORMATION
// ========================
void printSystemInfo() {
    Serial.println("\n========== SYSTEM INFO ==========");
    Serial.print("Chip Model: ");
    Serial.println(ESP.getChipModel());
    Serial.print("Chip Revision: ");
    Serial.println(ESP.getChipRevision());
    Serial.print("CPU Frequency: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");
    Serial.print("Flash Size: ");
    Serial.print(ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println(" MB");
    Serial.print("Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.println("================================\n");
}

// ========================
// SENSOR HEALTH CHECK
// ========================
void checkSensorHealth() {
    Serial.println("\n[HEALTH] 🏥 Checking system status...");
    
    // Check DHT11
    float testTemp = dht.readTemperature();
    float testHumi = dht.readHumidity();
    
    if (isnan(testTemp) || isnan(testHumi)) {
        Serial.println("[HEALTH] DHT11: ❌ FAILED");
        setLED(3, LED_BLINK_SLOW_STATE); // Alert LED
    } else {
        Serial.println("[HEALTH] DHT11: ✅ OK");
    }
    
    // Check LDR
    int ldrTest = analogRead(LDR_PIN);
    if (ldrTest < 0 || ldrTest > 4095) {
        Serial.println("[HEALTH] LDR: ❌ FAILED");
    } else {
        Serial.printf("[HEALTH] LDR: ✅ OK (Raw: %d)\n", ldrTest);
    }
    
    // Check WiFi
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[HEALTH] WiFi: ✅ OK (RSSI: %d dBm)\n", WiFi.RSSI());
    } else {
        Serial.println("[HEALTH] WiFi: ❌ FAILED - Attempting reconnect...");
        setupWiFi();
    }
    
    // MQTT check removed (HTTP only mode)
    Serial.println("[HEALTH] MQTT: ⏭️  SKIPPED (HTTP only mode)");
}

// ========================
// LED SETUP
// ========================
void setupLEDs() {
    Serial.println("\n[LEDs] 💡 Initializing...");
    
    // Initialize all LED pins
    for (int i = 0; i < NUM_LEDS; i++) {
        pinMode(leds[i].pin, OUTPUT);
        digitalWrite(leds[i].pin, LOW);
        Serial.printf("[LED] GPIO%d initialized\n", leds[i].pin);
    }
    
    // Startup LED sequence
    Serial.println("[LEDs] Running startup sequence...");
    for (int i = 0; i < NUM_LEDS; i++) {
        digitalWrite(leds[i].pin, HIGH);
        delay(200);
        digitalWrite(leds[i].pin, LOW);
        delay(100);
    }
    
    // Set status LED to indicate system starting
    setLED(0, LED_BLINK_SLOW_STATE);
    
    Serial.println("[LEDs] ✅ Initialization complete");
}

// ========================
// LED CONTROL FUNCTIONS
// ========================
void setLED(int ledIndex, LEDState state) {
    if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
        leds[ledIndex].state = state;
        leds[ledIndex].lastToggle = millis();
        
        // Immediate action for ON/OFF states
        if (state == LED_ON) {
            digitalWrite(leds[ledIndex].pin, HIGH);
            leds[ledIndex].currentState = true;
        } else if (state == LED_OFF) {
            digitalWrite(leds[ledIndex].pin, LOW);
            leds[ledIndex].currentState = false;
        }
    }
}

void setLEDBrightness(int ledIndex, int brightness) {
    if (ledIndex >= 0 && ledIndex < NUM_LEDS) {
        leds[ledIndex].brightness = constrain(brightness, 0, 255);
        
        // Use PWM for brightness control
        if (leds[ledIndex].state == LED_ON) {
            analogWrite(leds[ledIndex].pin, leds[ledIndex].brightness);
        }
    }
}

void updateLEDs() {
    unsigned long currentTime = millis();
    
    for (int i = 0; i < NUM_LEDS; i++) {
        switch (leds[i].state) {
            case LED_BLINK_FAST_STATE:
                if (currentTime - leds[i].lastToggle >= LED_BLINK_FAST) {
                    leds[i].currentState = !leds[i].currentState;
                    digitalWrite(leds[i].pin, leds[i].currentState ? HIGH : LOW);
                    leds[i].lastToggle = currentTime;
                }
                break;
                
            case LED_BLINK_SLOW_STATE:
                if (currentTime - leds[i].lastToggle >= LED_BLINK_SLOW) {
                    leds[i].currentState = !leds[i].currentState;
                    digitalWrite(leds[i].pin, leds[i].currentState ? HIGH : LOW);
                    leds[i].lastToggle = currentTime;
                }
                break;
                
            case LED_FADE: {
                // Simple fade effect using PWM
                int fadeValue = (sin(currentTime / 500.0) + 1) * 127.5;
                analogWrite(leds[i].pin, fadeValue);
                break;
            }
                
            default:
                // LED_ON and LED_OFF are handled in setLED()
                break;
        }
    }
}

// ========================
// LED EFFECTS HANDLER
// ========================
void handleLEDEffects() {
    // Temperature alert
    if (currentData.temperature > TEMP_MAX - 10 || 
        currentData.temperature < TEMP_MIN + 10) {
        setLED(3, LED_BLINK_FAST_STATE); // Alert LED - fast blink on temperature warning
    } else {
        setLED(3, LED_OFF);
    }
    
    // Light level indicator
    if (currentData.light < 20) {
        setLED(4, LED_ON); // Control LED solid on when dark
    } else if (currentData.light > 80) {
        setLED(4, LED_BLINK_SLOW_STATE); // Blink slow when very bright
    } else {
        setLED(4, LED_OFF);
    }
    
    // System status LED (removed MQTT dependency)
    if (WiFi.status() == WL_CONNECTED) {
        setLED(0, LED_FADE); // Fade effect when WiFi connected
    } else {
        setLED(0, LED_BLINK_FAST_STATE); // Fast blink when WiFi issues
    }
}