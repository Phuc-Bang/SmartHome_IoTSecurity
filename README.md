# ESP32 Smart Home IoT Security Analysis Project

## Mô tả dự án
Dự án phân tích bảo mật IoT cho hệ thống Smart Home sử dụng ESP32-S3 với các cảm biến môi trường.

## Thành phần phần cứng
- **ESP32-S3 DevKit-C-1**: Vi điều khiển chính
- **DHT11**: Cảm biến nhiệt độ và độ ẩm
- **MH Light Sensor (LDR)**: Mô-đun cảm biến ánh sáng quang điện trở
- **OLED 0.96" (SSD1306)**: Màn hình hiển thị I2C
- **5x LED**: Đèn báo trạng thái và điều khiển
- **Breadboard và dây nối**

## Sơ đồ kết nối

### DHT11
- VCC → 3.3V
- GND → GND  
- DATA → GPIO 4

### MH Light Sensor (LDR)
- VCC → 3.3V
- GND → GND
- AO → GPIO 1 (ADC1_CH0)

### OLED SSD1306
- VCC → 3.3V
- GND → GND
- SDA → GPIO 14
- SCL → GPIO 13

### LED Connections
- **Status LED (Blue)**: GPIO 2 → 220Ω resistor → LED → GND
- **WiFi LED (Green)**: GPIO 5 → 220Ω resistor → LED → GND  
- **Sensor LED (Yellow)**: GPIO 6 → 220Ω resistor → LED → GND
- **Alert LED (Red)**: GPIO 7 → 220Ω resistor → LED → GND
- **Control LED (White)**: GPIO 8 → 220Ω resistor → LED → GND

## Cài đặt và biên dịch

### Yêu cầu
- PlatformIO IDE hoặc Arduino IDE
- ESP32 board package
- Các thư viện được liệt kê trong `platformio.ini`

### Các bước cài đặt
1. Clone repository này
2. Mở project trong PlatformIO
3. Cấu hình Wi-Fi credentials trong `include/config.h`
4. Build và upload lên ESP32-S3

```bash
# Sử dụng PlatformIO CLI
pio run --target upload
pio device monitor
```

## 🔧 Khắc phục sự cố HTTP Connection

### Cấu hình hiện tại (HTTP-Only Mode)
- **WiFi Network**: "Phuc Bang"
- **ESP32 IP**: 192.168.99.195 (expected)
- **Server IP**: 192.168.99.85 (expected)
- **Backend Port**: 5000
- **Mode**: HTTP-only (MQTT disabled)

### Lỗi thường gặp và cách khắc phục

#### 1. "Connection reset by peer" Error
**Nguyên nhân**: Windows Firewall chặn kết nối từ ESP32

**Giải pháp**:
```bash
# Chạy script sửa firewall (Run as Administrator)
fix_firewall.bat

# Hoặc tạm thời tắt Windows Firewall
# Kiểm tra antivirus software
```

#### 2. "Connection refused" Error
**Nguyên nhân**: Backend server chưa chạy

**Giải pháp**:
```bash
# Khởi động server
cd backend_example
python simple_server.py

# Kiểm tra server đang lắng nghe port 5000
netstat -an | findstr :5000
```

#### 3. WiFi Connection Issues
**Nguyên nhân**: Sai tên mạng hoặc mật khẩu

**Giải pháp**:
- Kiểm tra tên mạng: "Phuc Bang"
- Kiểm tra mật khẩu trong `include/config.h`
- Đảm bảo mạng 2.4GHz (không phải 5GHz)
- Kiểm tra ESP32 nhận IP 192.168.99.195

#### 4. Kiểm tra kết nối mạng
```bash
# Test kết nối tổng thể
python test_network.py

# Test server thủ công
curl http://192.168.99.85:5000/api/devices

# Ping ESP32 (nếu đã kết nối)
ping 192.168.99.195
```

### Serial Output mong đợi
```
📶 [WiFi] Starting connection...
✅ [WiFi] Connected successfully!
🌐 [WiFi] IP Address: 192.168.99.195
📊 ========== Sensor Reading ==========
🌡️ [SENSORS] Reading data...
✅ [DHT11] Temp: 25.5°C, Humidity: 60.2%
💡 [LDR] Raw: 2048, Light: 50%
📡 [HTTP] Preparing to send data...
✅ [HTTP] Response code: 200 (SUCCESS)
📥 [HTTP] Response: {"status":"success","device_id":"esp32_001"}
```

### Backend Server Setup
```bash
# Cài đặt dependencies
cd backend_example
pip install -r requirements.txt

# Khởi động server
python simple_server.py

# Truy cập dashboard
# http://192.168.99.85:5000
```

## Cấu hình

### Wi-Fi Settings
Chỉnh sửa file `include/config.h`:
```cpp
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"
```

### API & MQTT Settings
```cpp
#define API_URL "http://your-server:5000/api/sensor/data"
#define MQTT_BROKER "your-mqtt-broker-ip"
```

## Tính năng

### Chức năng chính
- Đọc cảm biến DHT11 (nhiệt độ, độ ẩm)
- Đọc cảm biến ánh sáng LDR
- Hiển thị dữ liệu trên OLED
- Gửi dữ liệu qua HTTP API
- Publish dữ liệu qua MQTT
- Nhận lệnh điều khiển từ MQTT
- Điều khiển 5 LED báo trạng thái và cảnh báo
- LED effects: ON/OFF, Blink, Fade

### LED Status Indicators
- **Status LED (GPIO 2)**: Trạng thái hệ thống tổng quát
- **WiFi LED (GPIO 5)**: Kết nối WiFi (Xanh lá = connected)
- **Sensor LED (GPIO 6)**: Hoạt động cảm biến (Vàng = reading)
- **Alert LED (GPIO 7)**: Cảnh báo (Đỏ = temperature/humidity alert)
- **Control LED (GPIO 8)**: Điều khiển từ xa (Trắng = MQTT commands)

### Tính năng bảo mật (Demo)
- Sensor data validation
- Health monitoring
- System information logging
- MQTT command handling
- LED status indicators
- Remote LED control (VULNERABLE - no authentication)

## Lỗ hổng bảo mật (Có chủ ý)

⚠️ **CẢNH BÁO**: Code này chứa các lỗ hổng bảo mật có chủ ý để phục vụ mục đích giáo dục!

### Các lỗ hổng chính:
1. **Hardcoded Credentials**: Wi-Fi password, API keys
2. **Plaintext Communication**: HTTP thay vì HTTPS
3. **Unencrypted MQTT**: Port 1883 thay vì 8883 (TLS)
4. **No Input Validation**: MQTT commands không được validate
5. **Debug Information Exposure**: Thông tin hệ thống được log

## Phân tích bảo mật

Xem file `IoT_Security_Analysis_Report.md` để có báo cáo phân tích bảo mật đầy đủ bao gồm:
- Kiến trúc hệ thống
- Attack surface analysis
- Kill chain analysis
- Biện pháp phòng chống

## Serial Monitor Output

Khi chạy, hệ thống sẽ output thông tin debug qua Serial:
```
========================================
ESP32-S3 Smart Home - VULNERABLE VERSION
Device ID: esp32_001
========================================

[WiFi] Connecting to: HomeNetwork2024
[WARNING] Using hardcoded credentials!
[WiFi] Connected!
[WiFi] IP Address: 192.168.1.150

--- Sensor Reading ---
[DHT11] Temp: 25.2°C, Humidity: 60.1%
[LDR] Raw: 2048, Light: 50%
[HTTP] Sending data...
[MQTT] Publishing data...
```

## Cấu trúc thư mục
```
ESP32_SmartHome_IoTSecurity/
├── src/
│   └── main.cpp                    # Code chính
├── include/
│   └── config.h                    # Cấu hình hệ thống
├── platformio.ini                  # Cấu hình PlatformIO
├── IoT_Security_Analysis_Report.md # Báo cáo phân tích bảo mật
└── README.md                       # File này
```

## Lưu ý quan trọng

1. **Chỉ sử dụng cho mục đích giáo dục**
2. **Không deploy trong môi trường production**
3. **Thay đổi tất cả credentials mặc định**
4. **Implement các biện pháp bảo mật được đề xuất trong báo cáo**

## License
MIT License - Chỉ sử dụng cho mục đích giáo dục và nghiên cứu.