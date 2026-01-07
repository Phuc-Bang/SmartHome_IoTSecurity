# 🛡️ BIỆN PHÁP PHÒNG CHỐNG - IoT SECURITY

## Countermeasures cho ESP32 Smart Home System

---

## 📊 Tổng Quan Biện Pháp Phòng Chống

| ID | Biện Pháp | Layer | Ngăn Chặn Attack Chain | Hiệu Quả |
|----|-----------|-------|------------------------|----------|
| CM1 | Flash Encryption & Secure Boot | Device | AC1, AC5 (Step 1-2) | 🔴 Critical |
| CM2 | HTTPS/TLS Implementation | Network | AC2, AC4, AC5 (Step 2-3) | 🔴 Critical |
| CM3 | API Authentication (JWT) | Backend | AC2, AC4 (Step 3) | 🔴 Critical |
| CM4 | Parameterized SQL Queries | Backend | AC4 (Step 3) | 🔴 Critical |
| CM5 | Input Validation & Sanitization | Backend | AC2, AC4 (Step 3) | 🟠 High |
| CM6 | Sensor Anomaly Detection | Device | AC3 (Step 3-4) | 🟠 High |
| CM7 | Rate Limiting | Backend | AC2, AC4 (Step 3) | 🟡 Medium |
| CM8 | Network Segmentation | Network | AC1, AC5 (Step 2) | 🟠 High |

---

## 🔒 CM1: Flash Encryption & Secure Boot

### Mô Tả
Mã hóa toàn bộ flash memory của ESP32 và enable secure boot để ngăn chặn firmware tampering.

### Ngăn Chặn Attack Chain

```
Attack Chain 1: Credential Extraction
─────────────────────────────────────
STEP 1: Physical Access     → ✅ VẪN CÓ THỂ
STEP 2: Firmware Dump       → 🛡️ CHẶN TẠI ĐÂY (encrypted data unreadable)
STEP 3: Extract Credentials → 🚫 KHÔNG THỂ (data encrypted)
STEP 4: Network Compromise  → 🚫 KHÔNG THỂ

Attack Chain 5: Full System Compromise
──────────────────────────────────────
STEP 1: Firmware Extraction → 🛡️ CHẶN TẠI ĐÂY (encrypted, useless)
STEP 2: Lateral Movement    → 🚫 KHÔNG THỂ (no credentials)
STEP 3: Privilege Escalation→ 🚫 KHÔNG THỂ
STEP 4: Full Compromise     → 🚫 KHÔNG THỂ
```

### Implementation

```bash
# 1. Generate encryption key
espsecure.py generate_flash_encryption_key my_flash_key.bin

# 2. Burn key vào eFuse (ONE-TIME OPERATION!)
espefuse.py --port COM3 burn_key BLOCK_KEY0 my_flash_key.bin XTS_AES_128_KEY

# 3. Enable flash encryption
espefuse.py --port COM3 burn_efuse SPI_BOOT_CRYPT_CNT 1

# 4. Enable Secure Boot v2
espefuse.py --port COM3 burn_efuse SECURE_BOOT_EN 1
```

```cpp
// platformio.ini - Enable encryption
build_flags = 
    -DCONFIG_FLASH_ENCRYPTION_ENABLED=1
    -DCONFIG_SECURE_BOOT_V2_ENABLED=1
```

### Kết Quả
| Trước | Sau |
|-------|-----|
| `strings firmware.bin` → Credentials visible | `strings firmware.bin` → Encrypted garbage |
| Attacker đọc được WiFi password | Attacker chỉ thấy random bytes |

---

## 🔐 CM2: HTTPS/TLS Implementation

### Mô Tả
Chuyển từ HTTP plaintext sang HTTPS với TLS 1.2+ và certificate pinning.

### Ngăn Chặn Attack Chain

```
Attack Chain 2: Data Injection
──────────────────────────────
STEP 1: Network Access      → ✅ VẪN CÓ THỂ
STEP 2: API Discovery       → 🛡️ CHẶN TẠI ĐÂY (traffic encrypted)
STEP 3: Data Injection      → 🚫 KHÔNG THỂ (certificate validation fails)
STEP 4: Dashboard Manip.    → 🚫 KHÔNG THỂ

Attack Chain 4: SQL Injection
─────────────────────────────
STEP 1: API Access          → ✅ VẪN CÓ THỂ (HTTPS public)
STEP 2: SQLi Detection      → 🛡️ KHÓAN (encrypted, harder to analyze)
STEP 3: SQL Injection       → ⚠️ VẪN CÓ THỂ (cần kết hợp CM4)
STEP 4: DB Compromise       → ⚠️ VẪN CÓ THỂ

Attack Chain 5: Full System Compromise
──────────────────────────────────────
STEP 2: Network Infiltration→ 🛡️ CHẶN TẠI ĐÂY (can't sniff traffic)
STEP 3: Backend Exploitation→ 🚫 KHÓAN (MITM không thể)
```

### Implementation

**ESP32 (Client):**
```cpp
#include <WiFiClientSecure.h>

// Root CA Certificate
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSj...\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure client;

void setupHTTPS() {
    client.setCACert(root_ca);
    // Optional: Certificate Pinning
    client.setFingerprint("AA:BB:CC:DD:EE:FF:00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD");
}

void sendDataHTTPS() {
    if (client.connect("192.168.137.1", 443)) {
        client.println("POST /api/sensor/data HTTP/1.1");
        // ... encrypted communication
    }
}
```

**Backend (Server):**
```python
# Generate self-signed certificate
# openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365

from flask import Flask
import ssl

app = Flask(__name__)

if __name__ == '__main__':
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain('cert.pem', 'key.pem')
    app.run(host='0.0.0.0', port=443, ssl_context=context)
```

### Kết Quả
| Trước (HTTP) | Sau (HTTPS) |
|--------------|-------------|
| Wireshark thấy: `{"temperature":25.5}` | Wireshark thấy: `Encrypted Application Data` |
| MITM attack dễ dàng | MITM bị chặn bởi certificate validation |

---

## 🔑 CM3: API Authentication (JWT)

### Mô Tả
Implement JSON Web Token (JWT) authentication cho tất cả API endpoints.

### Ngăn Chặn Attack Chain

```
Attack Chain 2: Data Injection
──────────────────────────────
STEP 1: Network Access      → ✅ VẪN CÓ THỂ
STEP 2: API Discovery       → ✅ VẪN CÓ THỂ (endpoints visible)
STEP 3: Data Injection      → 🛡️ CHẶN TẠI ĐÂY (401 Unauthorized)
STEP 4: Dashboard Manip.    → 🚫 KHÔNG THỂ

Attack Chain 4: SQL Injection
─────────────────────────────
STEP 1: API Access          → ✅ VẪN CÓ THỂ
STEP 2: SQLi Detection      → ✅ VẪN CÓ THỂ
STEP 3: SQL Injection       → 🛡️ CHẶN TẠI ĐÂY (401 - no valid token)
STEP 4: DB Compromise       → 🚫 KHÔNG THỂ
```

### Implementation

**Backend:**
```python
from flask import Flask, request, jsonify
from flask_jwt_extended import JWTManager, jwt_required, create_access_token
import datetime

app = Flask(__name__)
app.config['JWT_SECRET_KEY'] = 'super-secret-key-change-in-production'
jwt = JWTManager(app)

# Device registration - returns JWT token
@app.route('/api/auth/register', methods=['POST'])
def register_device():
    device_id = request.json.get('device_id')
    device_secret = request.json.get('device_secret')
    
    # Validate device credentials
    if validate_device(device_id, device_secret):
        token = create_access_token(
            identity=device_id,
            expires_delta=datetime.timedelta(days=30)
        )
        return jsonify({'token': token}), 200
    return jsonify({'error': 'Invalid credentials'}), 401

# Protected endpoint - requires JWT
@app.route('/api/sensor/data', methods=['POST'])
@jwt_required()  # 🛡️ REQUIRES VALID TOKEN
def receive_sensor_data():
    # Only authenticated devices can send data
    data = request.get_json()
    # ... process data
    return jsonify({'status': 'success'}), 200
```

**ESP32:**
```cpp
String jwtToken = ""; // Stored securely in NVS

void authenticateDevice() {
    HTTPClient http;
    http.begin("https://192.168.137.1/api/auth/register");
    http.addHeader("Content-Type", "application/json");
    
    String payload = "{\"device_id\":\"esp32_001\",\"device_secret\":\"secret123\"}";
    int httpCode = http.POST(payload);
    
    if (httpCode == 200) {
        // Parse and store JWT token
        jwtToken = parseToken(http.getString());
        saveTokenToNVS(jwtToken);
    }
}

void sendDataWithAuth() {
    HTTPClient http;
    http.begin("https://192.168.137.1/api/sensor/data");
    http.addHeader("Authorization", "Bearer " + jwtToken);  // 🛡️ JWT TOKEN
    http.addHeader("Content-Type", "application/json");
    
    http.POST(sensorData);
}
```

### Kết Quả
| Trước | Sau |
|-------|-----|
| `curl POST /api/sensor/data` → 200 OK | `curl POST /api/sensor/data` → 401 Unauthorized |
| Anyone can inject data | Only devices with valid JWT can send data |

---

## 💉 CM4: Parameterized SQL Queries

### Mô Tả
Thay thế string concatenation bằng parameterized queries để ngăn SQL injection.

### Ngăn Chặn Attack Chain

```
Attack Chain 4: SQL Injection Exploitation
──────────────────────────────────────────
STEP 1: API Access          → ✅ VẪN CÓ THỂ
STEP 2: SQLi Detection      → ✅ VẪN CÓ THỂ (test payloads)
STEP 3: SQL Injection       → 🛡️ CHẶN TẠI ĐÂY (payload treated as data)
STEP 4: DB Compromise       → 🚫 KHÔNG THỂ

Payload "' OR '1'='1" được treat như literal string, không phải SQL code!
```

### Implementation

**TRƯỚC (Vulnerable):**
```python
# ❌ VULNERABLE - String concatenation
@app.route('/api/sensor/latest/<device_id>')
def get_latest_data(device_id):
    query = f"SELECT * FROM sensor_readings WHERE device_id = '{device_id}'"
    cursor.execute(query)  # SQLi possible!
```

**SAU (Secure):**
```python
# ✅ SECURE - Parameterized query
@app.route('/api/sensor/latest/<device_id>')
def get_latest_data(device_id):
    query = "SELECT * FROM sensor_readings WHERE device_id = ?"
    cursor.execute(query, (device_id,))  # SQLi blocked!

# ✅ SECURE - For INSERT
@app.route('/api/sensor/data', methods=['POST'])
def receive_sensor_data():
    data = request.get_json()
    
    query = """
        INSERT INTO sensor_readings 
        (device_id, temperature, humidity, light, timestamp, received_at)
        VALUES (?, ?, ?, ?, ?, ?)
    """
    cursor.execute(query, (
        data.get('device_id'),
        data.get('temperature'),
        data.get('humidity'),
        data.get('light'),
        data.get('timestamp'),
        datetime.datetime.now()
    ))
```

### Kết Quả
| Attack Payload | Trước | Sau |
|----------------|-------|-----|
| `' OR '1'='1` | Returns ALL records | Returns 0 records (no match) |
| `'; DROP TABLE--` | Table dropped! | Treated as literal device_id |
| `' UNION SELECT * FROM users--` | Data leaked | Query fails (no match) |

---

## ✅ CM5: Input Validation & Sanitization

### Mô Tả
Validate tất cả input data theo expected ranges và formats trước khi xử lý.

### Ngăn Chặn Attack Chain

```
Attack Chain 2: Data Injection
──────────────────────────────
STEP 1: Network Access      → ✅ VẪN CÓ THỂ
STEP 2: API Discovery       → ✅ VẪN CÓ THỂ
STEP 3: Data Injection      → 🛡️ CHẶN TẠI ĐÂY (invalid data rejected)
STEP 4: Dashboard Manip.    → 🚫 KHÔNG THỂ

Attack Chain 3: Physical Sensor Spoofing
────────────────────────────────────────
STEP 1-3: Physical Manip.   → ✅ VẪN CÓ THỂ
STEP 4: False Data Stored   → 🛡️ CHẶN TẠI ĐÂY (anomaly detected)
```

### Implementation

```python
from flask import Flask, request, jsonify
import re

# Validation constants
VALID_RANGES = {
    'temperature': (-40, 80),      # DHT11/22 range
    'humidity': (0, 100),          # Percentage
    'light': (0, 100),             # Percentage
}

DEVICE_ID_PATTERN = re.compile(r'^[a-zA-Z0-9_]{3,20}$')

def validate_sensor_data(data):
    """Validate sensor data before processing"""
    errors = []
    
    # 1. Validate device_id format
    device_id = data.get('device_id', '')
    if not DEVICE_ID_PATTERN.match(device_id):
        errors.append(f"Invalid device_id format: {device_id}")
    
    # 2. Validate numeric ranges
    for field, (min_val, max_val) in VALID_RANGES.items():
        value = data.get(field)
        if value is not None:
            try:
                value = float(value)
                if not (min_val <= value <= max_val):
                    errors.append(f"{field} out of range: {value} (expected {min_val}-{max_val})")
            except (TypeError, ValueError):
                errors.append(f"{field} must be a number: {value}")
    
    # 3. Validate timestamp
    timestamp = data.get('timestamp')
    if timestamp:
        try:
            ts = int(timestamp)
            # Check if timestamp is reasonable (not too old, not future)
            now = int(time.time())
            if ts < now - 86400 or ts > now + 300:  # Within 24h past, 5min future
                errors.append(f"Invalid timestamp: {ts}")
        except:
            errors.append(f"Invalid timestamp format: {timestamp}")
    
    return errors

@app.route('/api/sensor/data', methods=['POST'])
def receive_sensor_data():
    data = request.get_json()
    
    # 🛡️ VALIDATE INPUT
    errors = validate_sensor_data(data)
    if errors:
        return jsonify({
            'status': 'error',
            'message': 'Validation failed',
            'errors': errors
        }), 400
    
    # Process valid data...
    return jsonify({'status': 'success'}), 200
```

### Kết Quả
| Malicious Input | Response |
|-----------------|----------|
| `{"temperature": 999.9}` | 400 - "temperature out of range" |
| `{"device_id": "'; DROP TABLE--"}` | 400 - "Invalid device_id format" |
| `{"humidity": "abc"}` | 400 - "humidity must be a number" |
| `{"temperature": 25.5, "humidity": 60}` | 200 - Success |

---

## 📊 CM6: Sensor Anomaly Detection

### Mô Tả
Phát hiện bất thường trong sensor readings để chặn physical manipulation.

### Ngăn Chặn Attack Chain

```
Attack Chain 3: Physical Sensor Spoofing
────────────────────────────────────────
STEP 1: Physical Access     → ✅ VẪN CÓ THỂ
STEP 2: Identify Sensor     → ✅ VẪN CÓ THỂ
STEP 3: Manipulate Sensor   → ✅ VẪN CÓ THỂ
STEP 4: False Data Accepted → 🛡️ CHẶN TẠI ĐÂY (anomaly flagged)
```

### Implementation

```python
import statistics
from collections import deque

class AnomalyDetector:
    def __init__(self, window_size=10, threshold_std=3):
        self.history = {
            'temperature': deque(maxlen=window_size),
            'humidity': deque(maxlen=window_size),
            'light': deque(maxlen=window_size)
        }
        self.threshold_std = threshold_std
    
    def check_anomaly(self, sensor_type, value):
        """Check if value is anomaly based on historical data"""
        history = self.history[sensor_type]
        
        if len(history) < 5:
            # Not enough data, accept value
            history.append(value)
            return False, "Insufficient history"
        
        mean = statistics.mean(history)
        std = statistics.stdev(history) if len(history) > 1 else 1
        
        # Z-score calculation
        z_score = abs(value - mean) / std if std > 0 else 0
        
        if z_score > self.threshold_std:
            return True, f"Anomaly detected: z-score={z_score:.2f}, expected ~{mean:.1f}"
        
        history.append(value)
        return False, "Normal"
    
    def check_rate_of_change(self, sensor_type, value):
        """Check if value changed too quickly"""
        history = self.history[sensor_type]
        
        if len(history) == 0:
            return False, "First reading"
        
        last_value = history[-1]
        
        # Max expected change per reading (10 seconds)
        max_changes = {
            'temperature': 5,   # Max 5°C change in 10 seconds
            'humidity': 10,     # Max 10% change in 10 seconds
            'light': 50         # Max 50% change in 10 seconds (light can change fast)
        }
        
        change = abs(value - last_value)
        if change > max_changes[sensor_type]:
            return True, f"Rapid change detected: {last_value} → {value}"
        
        return False, "Normal rate of change"

detector = AnomalyDetector()

@app.route('/api/sensor/data', methods=['POST'])
def receive_sensor_data():
    data = request.get_json()
    warnings = []
    
    for sensor in ['temperature', 'humidity', 'light']:
        value = data.get(sensor)
        if value:
            # 🛡️ Check for statistical anomaly
            is_anomaly, msg = detector.check_anomaly(sensor, value)
            if is_anomaly:
                warnings.append(f"{sensor}: {msg}")
            
            # 🛡️ Check for rapid change
            is_rapid, msg = detector.check_rate_of_change(sensor, value)
            if is_rapid:
                warnings.append(f"{sensor}: {msg}")
    
    if warnings:
        # Log warning but still accept (or reject based on policy)
        log_security_event("ANOMALY_DETECTED", warnings)
        return jsonify({
            'status': 'warning',
            'message': 'Anomaly detected',
            'warnings': warnings
        }), 202
    
    return jsonify({'status': 'success'}), 200
```

### Kết Quả
| Sensor Manipulation | Detection |
|--------------------|-----------|
| Light: 50% → 0% (LDR covered) | ⚠️ "Rapid change detected" |
| Temp: 25°C → 45°C (sensor heated) | ⚠️ "Anomaly: z-score=4.5" |
| Normal fluctuation: 25°C → 26°C | ✅ Accepted |

---

## ⏱️ CM7: Rate Limiting

### Mô Tả
Giới hạn số lượng requests per IP/device để ngăn chặn DoS và brute force.

### Ngăn Chặn Attack Chain

```
Attack Chain 2 & 4: Data Injection / SQL Injection
──────────────────────────────────────────────────
STEP 3: Exploitation attempts → 🛡️ RATE LIMITED
- Attacker chỉ có thể thử 10 requests/phút
- Brute force SQLi payloads bị chặn sau vài attempts
- DoS attack bị neutralized
```

### Implementation

```python
from flask import Flask
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address

app = Flask(__name__)

limiter = Limiter(
    app,
    key_func=get_remote_address,
    default_limits=["100 per hour"]
)

# 🛡️ Strict limit for data submission
@app.route('/api/sensor/data', methods=['POST'])
@limiter.limit("10 per minute")  # Max 10 requests/min per IP
def receive_sensor_data():
    # ... process data
    pass

# 🛡️ Very strict limit for auth endpoints
@app.route('/api/auth/register', methods=['POST'])
@limiter.limit("5 per hour")  # Prevent brute force
def register_device():
    # ... authenticate
    pass

# 🛡️ Moderate limit for read endpoints
@app.route('/api/sensor/latest/<device_id>')
@limiter.limit("30 per minute")
def get_latest_data(device_id):
    # ... return data
    pass
```

### Kết Quả
| Attack | Response |
|--------|----------|
| 11th request in 1 minute | 429 Too Many Requests |
| DoS flood attack | Blocked after limit reached |
| SQLi brute force | Limited to 10 attempts/min |

---

## 🌐 CM8: Network Segmentation

### Mô Tả
Tách biệt IoT devices vào VLAN riêng, hạn chế lateral movement.

### Ngăn Chặn Attack Chain

```
Attack Chain 1: Credential Extraction → Network Compromise
──────────────────────────────────────────────────────────
STEP 1-3: Credential extracted  → ✅ VẪN CÓ THỂ
STEP 4: Network Compromise      → 🛡️ GIỚI HẠN (chỉ access IoT VLAN)

Attack Chain 5: Full System Compromise
──────────────────────────────────────
STEP 2: Lateral Movement        → 🛡️ CHẶN TẠI ĐÂY (VLAN isolation)
- Attacker access IoT VLAN → Cannot reach main network
- Cannot attack PCs, servers, other sensitive devices
```

### Implementation

```
Network Topology (Segmented):
─────────────────────────────

                    ┌─────────────┐
                    │   Router    │
                    │ 192.168.1.1 │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
    ┌──────▼──────┐ ┌──────▼──────┐ ┌──────▼──────┐
    │  VLAN 10    │ │  VLAN 20    │ │  VLAN 30    │
    │ Main Network│ │ IoT Devices │ │   Servers   │
    │192.168.10.x │ │192.168.20.x │ │192.168.30.x │
    └─────────────┘ └─────────────┘ └─────────────┘
         │               │               │
    ┌────▼────┐    ┌────▼────┐    ┌────▼────┐
    │   PCs   │    │  ESP32  │    │ Backend │
    │ Laptops │    │ Sensors │    │ Database│
    └─────────┘    └─────────┘    └─────────┘

Firewall Rules:
- VLAN 20 → VLAN 30: ALLOW (IoT → Backend API only, port 443)
- VLAN 20 → VLAN 10: DENY  (IoT cannot reach main network)
- VLAN 10 → VLAN 20: ALLOW (Admin can manage IoT)
```

---

## 📈 Ma Trận Phòng Chống vs Attack Chains

| Biện Pháp | AC1 | AC2 | AC3 | AC4 | AC5 | Bước Chặn |
|-----------|-----|-----|-----|-----|-----|-----------|
| **CM1** Flash Encryption | ✅ | - | - | - | ✅ | Step 1-2 (Entry/Exploitation) |
| **CM2** HTTPS/TLS | - | ✅ | - | ⚠️ | ✅ | Step 2-3 (Recon/Exploitation) |
| **CM3** JWT Auth | - | ✅ | - | ✅ | - | Step 3 (Exploitation) |
| **CM4** Parameterized SQL | - | - | - | ✅ | - | Step 3 (Exploitation) |
| **CM5** Input Validation | - | ✅ | ⚠️ | ✅ | - | Step 3-4 (Exploitation/Impact) |
| **CM6** Anomaly Detection | - | - | ✅ | - | - | Step 4 (Impact) |
| **CM7** Rate Limiting | - | ✅ | - | ✅ | - | Step 3 (Exploitation) |
| **CM8** Network Segmentation | ✅ | - | - | - | ✅ | Step 2 (Lateral Movement) |

**Legend:** ✅ Chặn hoàn toàn | ⚠️ Giảm thiểu | - Không áp dụng

---

## 🎯 Ưu Tiên Triển Khai

### Phase 1 - Critical (Tuần 1-2):
1. **CM4** - Parameterized SQL Queries
2. **CM3** - API Authentication
3. **CM2** - HTTPS/TLS

### Phase 2 - High (Tuần 3-4):
4. **CM1** - Flash Encryption
5. **CM5** - Input Validation
6. **CM8** - Network Segmentation

### Phase 3 - Medium (Tuần 5-6):
7. **CM6** - Anomaly Detection
8. **CM7** - Rate Limiting

---

> **Ngày tạo**: 07/01/2026  
> **Dự án**: ESP32 Smart Home IoT Security Analysis  
> **Mục đích**: Giáo dục về An toàn thông tin IoT
