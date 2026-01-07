# 🔐 PHÂN TÍCH RỦI RO BẢO MẬT IoT

## Hệ Thống ESP32 Smart Home - Phân Loại Theo 3 Lớp

---

## 📊 Tổng Quan Rủi Ro

| Lớp | Số Rủi Ro | Mức Độ Nghiêm Trọng |
|-----|-----------|---------------------|
| **Device Layer** | 5 | 🔴 Critical - High |
| **Network Layer** | 5 | 🔴 Critical - High |
| **Backend Layer** | 5 | 🟠 High - Medium |
| **Tổng cộng** | **15** | - |

---

## 🔴 DEVICE LAYER (Lớp Thiết Bị)

### Rủi Ro D1: Hardcoded Credentials
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🔴 CRITICAL (10/10) |
| **Mô tả** | WiFi password, API keys được lưu trực tiếp trong source code |
| **Vị trí** | `include/config.h` - dòng 10-21 |
| **Bằng chứng** | `#define WIFI_PASSWORD "12345678"` |
| **Tấn công** | Firmware dumping → strings command → Extract credentials |
| **Hậu quả** | Attacker truy cập mạng WiFi, giả mạo API requests |
| **Khắc phục** | Sử dụng ESP32 NVS (Non-Volatile Storage) với encryption |

```cpp
// VULNERABLE CODE (config.h)
#define WIFI_SSID "Bang"
#define WIFI_PASSWORD "12345678"
#define API_KEY "sk_live_51234567890abcdef"
```

---

### Rủi Ro D2: Debug Information Exposure
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (8/10) |
| **Mô tả** | Serial output expose thông tin nhạy cảm về hệ thống |
| **Vị trí** | `src/main.cpp` - Serial.println() calls |
| **Bằng chứng** | Log IP address, API endpoints, sensor data qua UART |
| **Tấn công** | Kết nối USB → Serial monitor → Thu thập thông tin |
| **Hậu quả** | Lộ cấu trúc hệ thống, IP addresses, data patterns |
| **Khắc phục** | Disable debug mode trong production, sử dụng DEBUG_SERIAL flag |

```
// Serial Output Example (EXPOSED)
📶 [WiFi] IP Address: 192.168.137.100
📡 [HTTP] Sending to: http://192.168.137.1:5000/api/sensor/data
🔑 [API] Using key: sk_live_512345...
```

---

### Rủi Ro D3: Physical Sensor Manipulation
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (7/10) |
| **Mô tả** | Cảm biến LDR có thể bị can thiệp vật lý để inject false data |
| **Vị trí** | GPIO 1 - MH Light Sensor (LDR) |
| **Bằng chứng** | Che/chiếu sáng LDR → Thay đổi light reading từ 0-100% |
| **Tấn công** | Physical access → Manipulate light source → Inject fake data |
| **Hậu quả** | Sai lệch dữ liệu, kích hoạt automation sai, tấn công DoS logic |
| **Khắc phục** | Sensor correlation, anomaly detection, tamper-evident enclosure |

---

### Rủi Ro D4: Firmware Dumping via UART/JTAG
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🔴 CRITICAL (9/10) |
| **Mô tả** | Firmware có thể bị extract qua USB/UART interface |
| **Vị trí** | ESP32-S3 USB-C port |
| **Bằng chứng** | `esptool.py read_flash 0x0 0x400000 firmware.bin` |
| **Tấn công** | Physical access → esptool.py → Firmware analysis → Reverse engineering |
| **Hậu quả** | Lộ toàn bộ source code, credentials, logic |
| **Khắc phục** | Enable ESP32 Flash Encryption + Secure Boot |

```bash
# Attack Command
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 firmware.bin
strings firmware.bin | grep -E "(password|key|ssid)"
```

---

### Rủi Ro D5: No Input Validation on Sensors
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟡 MEDIUM (6/10) |
| **Mô tả** | Dữ liệu sensor không được validate đầy đủ trước khi gửi |
| **Vị trí** | `src/main.cpp` - readSensors() function |
| **Bằng chứng** | Có range check nhưng không có anomaly detection |
| **Tấn công** | Manipulate sensor → Send edge-case values → Bypass validation |
| **Hậu quả** | False data trong database, trigger wrong automation |
| **Khắc phục** | Implement statistical anomaly detection, multi-sensor correlation |

---

## 🌐 NETWORK LAYER (Lớp Mạng)

### Rủi Ro N1: HTTP Plaintext Communication
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🔴 CRITICAL (10/10) |
| **Mô tả** | Tất cả HTTP traffic không được mã hóa (plaintext) |
| **Vị trí** | `config.h` - API_URL sử dụng http:// |
| **Bằng chứng** | `#define API_URL "http://192.168.137.1:5000/..."` |
| **Tấn công** | Wireshark/tcpdump → Capture packets → Read all data |
| **Hậu quả** | Lộ sensor data, API keys, device fingerprinting |
| **Khắc phục** | Chuyển sang HTTPS với TLS 1.2+, certificate pinning |

```
// Wireshark Capture Example
POST /api/sensor/data HTTP/1.1
{"device_id":"esp32_001","temperature":25.5,"api_key":"sk_live_..."}
```

---

### Rủi Ro N2: Man-in-the-Middle (MITM) Attack
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🔴 CRITICAL (9/10) |
| **Mô tả** | Không có certificate verification, dễ bị MITM |
| **Vị trí** | HTTP communication giữa ESP32 và Backend |
| **Bằng chứng** | Không sử dụng WiFiClientSecure, không verify certificate |
| **Tấn công** | ARP spoofing → Intercept traffic → Modify/inject data |
| **Hậu quả** | Data tampering, credential theft, command injection |
| **Khắc phục** | HTTPS + Certificate Pinning trong firmware |

```
[ESP32] ←→ [Attacker] ←→ [Backend]
         ↑
    Intercept & Modify
```

---

### Rủi Ro N3: WiFi Credential Sniffing
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (8/10) |
| **Mô tả** | WPA2 password có thể bị crack nếu weak, traffic visible |
| **Vị trí** | WiFi connection từ ESP32 |
| **Bằng chứng** | Password "12345678" - weak và common |
| **Tấn công** | Aircrack-ng → Capture handshake → Dictionary attack |
| **Hậu quả** | Truy cập mạng nội bộ, sniff all IoT traffic |
| **Khắc phục** | Strong WPA3 password, network segmentation, IoT VLAN |

---

### Rủi Ro N4: No Network Segmentation
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (7/10) |
| **Mô tả** | ESP32 và Backend trên cùng network, không isolation |
| **Vị trí** | 192.168.137.x subnet |
| **Bằng chứng** | Cùng subnet, không firewall rules |
| **Tấn công** | Compromise ESP32 → Lateral movement → Attack other devices |
| **Hậu quả** | Full network compromise, pivot to other systems |
| **Khắc phục** | VLAN separation, firewall rules, IoT isolated network |

---

### Rủi Ro N5: MQTT Insecure Configuration (Disabled nhưng có config)
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟡 MEDIUM (5/10) |
| **Mô tả** | MQTT config sử dụng port 1883 (plaintext), password weak |
| **Vị trí** | `config.h` - MQTT settings |
| **Bằng chứng** | `MQTT_PORT 1883`, `MQTT_PASSWORD "mqtt_password"` |
| **Tấn công** | Nếu enable MQTT → Subscribe all topics → Publish fake commands |
| **Hậu quả** | Device control hijacking, data injection |
| **Khắc phục** | Sử dụng MQTTS (port 8883) với TLS, strong authentication |

---

## 🖥️ BACKEND LAYER (Lớp Backend)

### Rủi Ro B1: No API Authentication
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🔴 CRITICAL (10/10) |
| **Mô tả** | API endpoints hoàn toàn public, không cần authentication |
| **Vị trí** | `simple_server.py` - tất cả routes |
| **Bằng chứng** | Không có @login_required, token validation |
| **Tấn công** | Truy cập trực tiếp API → Đọc/ghi dữ liệu tùy ý |
| **Hậu quả** | Unauthorized data access, data injection, DoS |
| **Khắc phục** | Implement JWT authentication, API key validation |

```bash
# Anyone can access - NO AUTH REQUIRED
curl http://192.168.137.1:5000/api/devices
curl http://192.168.137.1:5000/api/sensor/latest/esp32_001
```

---

### Rủi Ro B2: SQL Injection Vulnerability
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🔴 CRITICAL (9/10) |
| **Mô tả** | Sử dụng f-string để build SQL queries |
| **Vị trí** | `simple_server.py` - line 57-60, 90, 155-160 |
| **Bằng chứng** | `f"INSERT INTO ... VALUES ('{device_id}', ...)"` |
| **Tấn công** | Inject SQL via device_id → Drop tables, extract data |
| **Hậu quả** | Database compromise, data theft, data destruction |
| **Khắc phục** | Sử dụng parameterized queries với ? placeholders |

```python
# VULNERABLE CODE
query = f"SELECT * FROM sensor_readings WHERE device_id = '{device_id}'"

# Attack payload
device_id = "' OR '1'='1' --"
```

---

### Rủi Ro B3: No Input Validation/Sanitization
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (8/10) |
| **Mô tả** | Dữ liệu từ ESP32 được accept mà không validate |
| **Vị trí** | `simple_server.py` - receive_sensor_data() |
| **Bằng chứng** | `temperature = data.get('temperature', 0)` - no range check |
| **Tấn công** | Send invalid data → Store garbage → Corrupt analytics |
| **Hậu quả** | Bad data in DB, wrong analytics, system confusion |
| **Khắc phục** | Validate ranges (-40-80°C, 0-100% humidity/light) |

```python
# Current code - NO VALIDATION
temperature = data.get('temperature', 0)  # Could be 99999 or -99999
```

---

### Rủi Ro B4: No Rate Limiting
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (7/10) |
| **Mô tả** | API không giới hạn số request, dễ bị DoS |
| **Vị trí** | `simple_server.py` - all endpoints |
| **Bằng chứng** | Không có Flask-Limiter hoặc rate limiting middleware |
| **Tấn công** | Flood API với requests → Server overload → DoS |
| **Hậu quả** | Service unavailable, resource exhaustion |
| **Khắc phục** | Implement Flask-Limiter (e.g., 10 requests/minute) |

```bash
# DoS Attack
while true; do curl -X POST http://target:5000/api/sensor/data -d '{}'; done
```

---

### Rủi Ro B5: Debug Mode in Production
| Thuộc Tính | Chi Tiết |
|------------|----------|
| **Mức độ** | 🟠 HIGH (7/10) |
| **Mô tả** | Flask chạy với debug=True, expose werkzeug debugger |
| **Vị trí** | `simple_server.py` - line 201 |
| **Bằng chứng** | `app.run(host='0.0.0.0', port=5000, debug=True)` |
| **Tấn công** | Trigger error → Access debugger → Execute arbitrary code |
| **Hậu quả** | Remote Code Execution (RCE), full server compromise |
| **Khắc phục** | Set `debug=False` trong production |

---

## 📈 Ma Trận Rủi Ro Tổng Hợp

| ID | Rủi Ro | Layer | Mức Độ | Khả Năng | Tác Động |
|----|--------|-------|--------|----------|----------|
| D1 | Hardcoded Credentials | Device | 🔴 Critical | Cao | Cao |
| D2 | Debug Info Exposure | Device | 🟠 High | Trung bình | Trung bình |
| D3 | Physical Sensor Manipulation | Device | 🟠 High | Trung bình | Cao |
| D4 | Firmware Dumping | Device | 🔴 Critical | Cao | Cao |
| D5 | No Sensor Validation | Device | 🟡 Medium | Thấp | Trung bình |
| N1 | HTTP Plaintext | Network | 🔴 Critical | Cao | Cao |
| N2 | MITM Attack | Network | 🔴 Critical | Trung bình | Cao |
| N3 | WiFi Credential Sniffing | Network | 🟠 High | Trung bình | Cao |
| N4 | No Network Segmentation | Network | 🟠 High | Cao | Cao |
| N5 | MQTT Insecure Config | Network | 🟡 Medium | Thấp | Trung bình |
| B1 | No API Authentication | Backend | 🔴 Critical | Cao | Cao |
| B2 | SQL Injection | Backend | 🔴 Critical | Cao | Cao |
| B3 | No Input Validation | Backend | 🟠 High | Cao | Trung bình |
| B4 | No Rate Limiting | Backend | 🟠 High | Cao | Trung bình |
| B5 | Debug Mode Production | Backend | 🟠 High | Trung bình | Cao |

---

## 🛡️ Ưu Tiên Khắc Phục

### Ưu tiên 1 (Khẩn cấp):
1. **B2** - SQL Injection → Chuyển sang parameterized queries
2. **N1** - HTTP Plaintext → Implement HTTPS/TLS
3. **B1** - No Auth → Add JWT authentication

### Ưu tiên 2 (Cao):
4. **D1** - Hardcoded Credentials → NVS encrypted storage
5. **D4** - Firmware Dumping → Enable Flash Encryption
6. **B5** - Debug Mode → Set debug=False

### Ưu tiên 3 (Trung bình):
7. **N2, N3** - Network security → HTTPS + Certificate Pinning
8. **B3, B4** - Input validation + Rate limiting
9. **D3, D5** - Sensor validation + Anomaly detection

---

> **Ngày tạo**: 07/01/2026  
> **Dự án**: ESP32 Smart Home IoT Security Analysis  
> **Mục đích**: Giáo dục về An toàn thông tin IoT
