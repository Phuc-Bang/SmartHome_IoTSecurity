# 🏗️ SƠ ĐỒ HỆ THỐNG IoT - CHECKLIST

## 📊 Tổng Quan Kiến Trúc 3 Lớp

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              DEVICE LAYER                                    │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │                        ESP32-S3 DevKit-C-1                          │    │
│  │  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐  ┌─────────┐  │    │
│  │  │ DHT11   │  │ LDR     │  │ OLED    │  │ 5 LEDs  │  │ WiFi    │  │    │
│  │  │ GPIO 4  │  │ GPIO 1  │  │ I2C     │  │GPIO2-8  │  │ Module  │  │    │
│  │  └─────────┘  └─────────┘  └─────────┘  └─────────┘  └────┬────┘  │    │
│  └───────────────────────────────────────────────────────────┼───────┘    │
└──────────────────────────────────────────────────────────────┼─────────────┘
                                                               │
                              ▼ WiFi 802.11 (2.4GHz)           │
┌──────────────────────────────────────────────────────────────┼─────────────┐
│                            NETWORK LAYER                      │             │
│  ┌─────────────────────────────────────────────────────────┐ │             │
│  │                   Home Router/Gateway                    │◄┘             │
│  │                   (192.168.x.x subnet)                   │               │
│  └─────────────────────────────────────────────────────────┘               │
│                              │                                              │
│              ┌───────────────┼───────────────┐                             │
│              ▼               ▼               ▼                             │
│       ┌──────────┐    ┌──────────┐    ┌──────────┐                        │
│       │HTTP:5000 │    │MQTT:1883 │    │ Internet │                        │
│       │(Plaintext)│    │(Disabled)│    │  Access  │                        │
│       └────┬─────┘    └──────────┘    └──────────┘                        │
└────────────┼───────────────────────────────────────────────────────────────┘
             │
             ▼
┌────────────────────────────────────────────────────────────────────────────┐
│                            BACKEND LAYER                                    │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                     Flask Server (Port 5000)                          │  │
│  │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │  │
│  │  │ REST API    │  │ SQLite DB   │  │ Templates   │  │ Static Files│  │  │
│  │  │ Endpoints   │  │ sensor_data │  │ index.html  │  │ CSS/JS      │  │  │
│  │  └─────────────┘  └─────────────┘  └─────────────┘  └─────────────┘  │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
│                                    │                                        │
│                                    ▼                                        │
│  ┌──────────────────────────────────────────────────────────────────────┐  │
│  │                        Web Dashboard                                   │  │
│  │         Browser ←→ Real-time Charts ←→ Sensor Cards                   │  │
│  └──────────────────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────────────────┘
```

---

## ✅ 1. DEVICE LAYER (Lớp Thiết Bị)

### 1.1 Phần Cứng ESP32-S3
| STT | Thành Phần | Pin/Interface | Trạng Thái | Mô Tả |
|-----|------------|---------------|------------|-------|
| 1 | ESP32-S3 DevKit-C-1 | - | ✅ | Vi điều khiển chính, 240MHz dual-core |
| 2 | DHT11 | GPIO 4 | ✅ | Cảm biến nhiệt độ (-40~80°C) & độ ẩm (0-100%) |
| 3 | MH LDR Sensor | GPIO 1 (ADC1) | ✅ | Cảm biến ánh sáng quang điện trở (0-100%) |
| 4 | OLED SSD1306 | I2C (SDA=14, SCL=13) | ✅ | Màn hình 128x64, hiển thị data |
| 5 | Status LED | GPIO 2 | ✅ | Blue - Trạng thái hệ thống |
| 6 | WiFi LED | GPIO 5 | ✅ | Green - Kết nối WiFi |
| 7 | Sensor LED | GPIO 6 | ✅ | Yellow - Hoạt động sensor |
| 8 | Alert LED | GPIO 7 | ✅ | Red - Cảnh báo |
| 9 | Control LED | GPIO 8 | ✅ | White - Điều khiển từ xa |

### 1.2 Firmware (main.cpp)
| STT | Chức Năng | File | Trạng Thái |
|-----|-----------|------|------------|
| 1 | WiFi Connection | main.cpp:setupWiFi() | ✅ |
| 2 | Sensor Reading | main.cpp:readSensors() | ✅ |
| 3 | OLED Display | main.cpp:displayData() | ✅ |
| 4 | HTTP Data Sending | main.cpp:sendDataHTTP() | ✅ |
| 5 | LED Control | main.cpp:updateLEDs() | ✅ |
| 6 | Health Monitoring | main.cpp:checkSensorHealth() | ✅ |
| 7 | WiFi Scanning | main.cpp:scanWiFiNetworks() | ✅ |

### 1.3 Cấu Hình (config.h)
| STT | Parameter | Giá Trị Hiện Tại | Trạng Thái |
|-----|-----------|------------------|------------|
| 1 | WIFI_SSID | "Bang" | ✅ |
| 2 | WIFI_PASSWORD | "12345678" | ✅ |
| 3 | API_URL | http://192.168.137.1:5000/api/sensor/data | ✅ |
| 4 | DEVICE_ID | "esp32_001" | ✅ |
| 5 | SENSOR_INTERVAL | 10000ms (10s) | ✅ |

---

## ✅ 2. NETWORK LAYER (Lớp Mạng)

### 2.1 Kết Nối WiFi
| STT | Thành Phần | Giá Trị | Trạng Thái |
|-----|------------|---------|------------|
| 1 | Protocol | WiFi 802.11 b/g/n | ✅ |
| 2 | Frequency | 2.4 GHz | ✅ |
| 3 | Security | WPA2-Personal | ✅ |
| 4 | IP Mode | DHCP | ✅ |
| 5 | Timeout | 20000ms | ✅ |

### 2.2 HTTP Communication
| STT | Endpoint | Method | Mô Tả | Trạng Thái |
|-----|----------|--------|-------|------------|
| 1 | /api/sensor/data | POST | Gửi dữ liệu sensor | ✅ |
| 2 | /api/sensor/latest/{id} | GET | Lấy data mới nhất | ✅ |
| 3 | /api/sensor/history/{id} | GET | Lấy lịch sử | ✅ |
| 4 | /api/devices | GET | Liệt kê devices | ✅ |
| 5 | / | GET | Web dashboard | ✅ |

### 2.3 Data Format (JSON)
```json
{
  "device_id": "esp32_001",
  "temperature": 25.5,
  "humidity": 60.2,
  "light": 50.0,
  "timestamp": 1704412800
}
```
| Field | Type | Range | Trạng Thái |
|-------|------|-------|------------|
| device_id | string | - | ✅ |
| temperature | float | -40 ~ 80°C | ✅ |
| humidity | float | 0 ~ 100% | ✅ |
| light | float | 0 ~ 100% | ✅ |
| timestamp | integer | Unix time | ✅ |

### 2.4 MQTT (Disabled)
| STT | Thành Phần | Giá Trị | Trạng Thái |
|-----|------------|---------|------------|
| 1 | MQTT_BROKER | 192.168.137.1 | ⏸️ Disabled |
| 2 | MQTT_PORT | 1883 | ⏸️ Disabled |
| 3 | MQTT_TOPIC | home/sensor/esp32_001 | ⏸️ Disabled |
| 4 | ENABLE_MQTT | false | ✅ |

---

## ✅ 3. BACKEND LAYER (Lớp Backend)

### 3.1 Flask Server (simple_server.py)
| STT | Thành Phần | Cấu Hình | Trạng Thái |
|-----|------------|----------|------------|
| 1 | Host | 0.0.0.0 (all interfaces) | ✅ |
| 2 | Port | 5000 | ✅ |
| 3 | Debug Mode | True | ✅ |
| 4 | Database | SQLite (sensor_data.db) | ✅ |

### 3.2 Database Schema
```sql
CREATE TABLE sensor_readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    device_id TEXT,
    temperature REAL,
    humidity REAL,
    light REAL,
    timestamp INTEGER,
    received_at TEXT
);
```
| Column | Type | Trạng Thái |
|--------|------|------------|
| id | INTEGER (PK) | ✅ |
| device_id | TEXT | ✅ |
| temperature | REAL | ✅ |
| humidity | REAL | ✅ |
| light | REAL | ✅ |
| timestamp | INTEGER | ✅ |
| received_at | TEXT | ✅ |

### 3.3 REST API Endpoints
| STT | Route | Method | Function | Trạng Thái |
|-----|-------|--------|----------|------------|
| 1 | POST /api/sensor/data | POST | receive_sensor_data() | ✅ |
| 2 | GET /api/sensor/latest/<id> | GET | get_latest_data() | ✅ |
| 3 | GET /api/sensor/history/<id> | GET | get_sensor_history() | ✅ |
| 4 | GET /api/devices | GET | list_devices() | ✅ |
| 5 | GET / | GET | index() | ✅ |
| 6 | GET /dashboard | GET | dashboard() | ✅ |
| 7 | GET /static/<path> | GET | static_files() | ✅ |

### 3.4 Web Dashboard
| STT | File | Dòng Code | Mô Tả | Trạng Thái |
|-----|------|-----------|-------|------------|
| 1 | index.html | 413 | HTML structure | ✅ |
| 2 | style.css | 1102 | Modern dark theme | ✅ |
| 3 | script.js | 637 | Real-time updates | ✅ |

### 3.5 Dashboard Features
| STT | Feature | Trạng Thái |
|-----|---------|------------|
| 1 | Temperature card với progress bar | ✅ |
| 2 | Humidity card với progress bar | ✅ |
| 3 | Light card với progress bar | ✅ |
| 4 | System status card | ✅ |
| 5 | Chart.js real-time biểu đồ | ✅ |
| 6 | Device selector dropdown | ✅ |
| 7 | Auto-refresh (10s interval) | ✅ |
| 8 | Connection status indicator | ✅ |
| 9 | Security analysis section | ✅ |

---

## 🔄 4. LUỒNG DỮ LIỆU (Data Flow)

### 4.1 Device → Backend (Uplink)
```
[ESP32] → [Read Sensors] → [Create JSON] → [HTTP POST] → [Flask API] → [SQLite]
```
| Bước | Mô Tả | Interval | Trạng Thái |
|------|-------|----------|------------|
| 1 | ESP32 đọc DHT11 & LDR | 10s | ✅ |
| 2 | Tạo JSON payload | - | ✅ |
| 3 | HTTP POST đến /api/sensor/data | - | ✅ |
| 4 | Flask nhận và parse JSON | - | ✅ |
| 5 | Lưu vào SQLite database | - | ✅ |
| 6 | Trả về response JSON | - | ✅ |

### 4.2 Backend → Dashboard (Display)
```
[Browser] → [Fetch API] → [Flask] → [SQLite] → [JSON Response] → [Update UI]
```
| Bước | Mô Tả | Interval | Trạng Thái |
|------|-------|----------|------------|
| 1 | Dashboard load page | - | ✅ |
| 2 | JavaScript fetch /api/devices | - | ✅ |
| 3 | Fetch /api/sensor/latest/{id} | 10s | ✅ |
| 4 | Fetch /api/sensor/history/{id} | 30s | ✅ |
| 5 | Update sensor cards | - | ✅ |
| 6 | Update Chart.js | - | ✅ |

---

## ⚠️ 5. ATTACK SURFACE (Điểm Tấn Công)

### Theo Layer:
| Layer | Attack Vector | Vulnerability | Trạng Thái |
|-------|---------------|---------------|------------|
| Device | Firmware Dumping | Hardcoded credentials | ⚠️ Vulnerable |
| Device | Serial/UART | Debug info exposure | ⚠️ Vulnerable |
| Device | Physical | LDR manipulation | ⚠️ Vulnerable |
| Network | WiFi Sniffing | Plaintext traffic | ⚠️ Vulnerable |
| Network | MITM | No HTTPS/TLS | ⚠️ Vulnerable |
| Backend | API Abuse | No authentication | ⚠️ Vulnerable |
| Backend | SQL Injection | No input validation | ⚠️ Vulnerable |

---

## 📋 Tóm Tắt Trạng Thái

| Layer | Tổng Items | Hoàn Thành | Phần Trăm |
|-------|------------|------------|-----------|
| **Device Layer** | 21 | 21 | 100% ✅ |
| **Network Layer** | 18 | 18 | 100% ✅ |
| **Backend Layer** | 24 | 24 | 100% ✅ |
| **TỔNG CỘNG** | **63** | **63** | **100%** ✅ |

> **Ghi chú**: Hệ thống đã hoàn thành đầy đủ các thành phần cơ bản. Các lỗ hổng bảo mật được thiết kế **có chủ đích** cho mục đích giáo dục về IoT Security.
