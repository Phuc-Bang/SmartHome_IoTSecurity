# 🏠 ESP32 Smart Home IoT Security

<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-S3-blue?style=for-the-badge&logo=espressif)
![PlatformIO](https://img.shields.io/badge/PlatformIO-5.0+-orange?style=for-the-badge&logo=platformio)
![Python](https://img.shields.io/badge/Python-3.9+-green?style=for-the-badge&logo=python)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

**Dự án phân tích bảo mật IoT cho hệ thống Smart Home**

[Tính năng](#-tính-năng) • [Cài đặt](#-cài-đặt-nhanh) • [Tài liệu](#-tài-liệu) • [Bảo mật](#-phân-tích-bảo-mật)

</div>

---

## 🎯 Giới Thiệu

Dự án demo hệ thống Smart Home với **ESP32-S3**, tích hợp phân tích bảo mật IoT. Hệ thống chứa các **lỗ hổng có chủ đích** để phục vụ mục đích giáo dục về an toàn thông tin.

> ⚠️ **CẢNH BÁO**: Không sử dụng trong môi trường production!

---

## ✨ Tính Năng

| Tính Năng | Mô Tả |
|-----------|-------|
| 🌡️ **Sensor Monitoring** | Đọc nhiệt độ, độ ẩm (DHT11) và ánh sáng (LDR) |
| 📺 **OLED Display** | Hiển thị real-time trên màn hình SSD1306 |
| 🌐 **Web Dashboard** | Giao diện web hiện đại với biểu đồ Chart.js |
| 💾 **Data Storage** | Lưu trữ SQLite với REST API |
| 💡 **LED Control** | 5 LED báo trạng thái hệ thống |
| 🔐 **Security Analysis** | Phân tích 15+ lỗ hổng bảo mật |

---

## 🔧 Phần Cứng

### Sơ đồ kết nối

```
┌─────────────────────────────────────────────────────────┐
│                    ESP32-S3 DevKit                       │
├─────────────────────────────────────────────────────────┤
│                                                          │
│   DHT11          LDR Module        OLED SSD1306         │
│   ┌───┐          ┌───┐             ┌────────┐           │
│   │VCC│→3.3V     │VCC│→3.3V        │VCC     │→3.3V      │
│   │GND│→GND      │GND│→GND         │GND     │→GND       │
│   │DAT│→GPIO4    │AO │→GPIO1       │SDA     │→GPIO14    │
│   └───┘          └───┘             │SCL     │→GPIO13    │
│                                    └────────┘           │
│                                                          │
│   LEDs (với điện trở 220Ω)                              │
│   ├── Status  (Blue)   → GPIO 2                         │
│   ├── WiFi    (Green)  → GPIO 5                         │
│   ├── Sensor  (Yellow) → GPIO 6                         │
│   ├── Alert   (Red)    → GPIO 7                         │
│   └── Control (White)  → GPIO 8                         │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

---

## 🚀 Cài Đặt Nhanh

### 1️⃣ Clone Repository
```bash
git clone https://github.com/Phuc-Bang/SmartHome_IoTSecurity.git
cd SmartHome_IoTSecurity
```

### 2️⃣ Cấu hình WiFi
Chỉnh sửa `include/config.h`:
```cpp
#define WIFI_SSID     "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define API_URL       "http://YOUR_SERVER_IP:5000/api/sensor/data"
```

### 3️⃣ Upload Firmware
```bash
# PlatformIO
pio run --target upload
pio device monitor
```

### 4️⃣ Khởi động Backend
```bash
cd backend_example
pip install -r requirements.txt
python simple_server.py
```

### 5️⃣ Mở Dashboard
Truy cập: `http://localhost:5000`

---

## 📁 Cấu Trúc Dự Án

```
SmartHome_IoTSecurity/
├── 📁 src/                    # Firmware ESP32
│   └── main.cpp
├── 📁 include/                # Cấu hình
│   └── config.h
├── 📁 backend_example/        # Flask Server
│   ├── simple_server.py
│   ├── templates/
│   └── static/
├── 📁 docs/                   # Tài liệu bảo mật
│   ├── Detailed_Attack_Defense_Guide.md  ⭐
│   ├── FINAL_REPORT.md
│   ├── DEMO_SCRIPT.md
│   ├── attack_chains/
│   ├── countermeasures/
│   └── security_risks/
└── 📁 scripts/                # Scripts hỗ trợ
```

---

## 📚 Tài Liệu

| Tài Liệu | Mô Tả |
|----------|-------|
| [**Detailed Attack & Defense Guide**](docs/Detailed_Attack_Defense_Guide.md) | ⭐ Hướng dẫn chi tiết tấn công-phòng thủ với Wireshark, Nmap, Postman |
| [**Attack Chains**](docs/attack_chains/IoT_Attack_Chains.md) | 5 chuỗi tấn công từ Entry → Impact |
| [**Countermeasures**](docs/countermeasures/IoT_Countermeasures.md) | 8 biện pháp phòng chống |
| [**Security Risks**](docs/security_risks/IoT_Security_Risks_Analysis.md) | 15 rủi ro theo 3 lớp |
| [**Demo Script**](docs/DEMO_SCRIPT.md) | Script thuyết trình |

---

## 🔐 Phân Tích Bảo Mật

### Kiến trúc 3 lớp

```
┌──────────────────┐
│   DEVICE LAYER   │  ESP32 + Sensors + LEDs
├──────────────────┤
│  NETWORK LAYER   │  WiFi + HTTP/MQTT
├──────────────────┤
│  BACKEND LAYER   │  Flask + SQLite + Dashboard
└──────────────────┘
```

### Lỗ hổng chính (Có chủ đích)

| # | Lỗ hổng | Mức độ | Layer |
|---|---------|--------|-------|
| 1 | Hardcoded Credentials | 🔴 Critical | Device |
| 2 | HTTP Plaintext | 🔴 Critical | Network |
| 3 | No API Authentication | 🔴 Critical | Backend |
| 4 | SQL Injection | 🔴 Critical | Backend |
| 5 | No Input Validation | 🟠 High | Backend |

### Attack vs Defense

| Tấn Công | Phòng Thủ |
|----------|-----------|
| Firmware Dump | Flash Encryption |
| HTTP Sniffing | HTTPS/TLS |
| API Abuse | JWT Authentication |
| SQL Injection | Parameterized Queries |
| DoS Attack | Rate Limiting |

> 📖 Xem chi tiết: [Detailed Attack & Defense Guide](docs/Detailed_Attack_Defense_Guide.md)

---

## 🛠️ Công Cụ Sử Dụng

| Công Cụ | Mục Đích |
|---------|----------|
| **Wireshark** | Phân tích HTTP traffic |
| **Nmap** | Scan network & ports |
| **Postman** | Test API endpoints |
| **esptool.py** | Dump firmware |

---

## ⚠️ Lưu Ý Quan Trọng

> 🎓 **Chỉ sử dụng cho mục đích giáo dục!**

1. ❌ Không deploy trong production
2. ❌ Không sử dụng credentials mặc định
3. ✅ Implement các biện pháp bảo mật trước khi triển khai thực tế
4. ✅ Đọc kỹ tài liệu phân tích bảo mật

---

## 📝 License

MIT License - Chỉ sử dụng cho mục đích giáo dục và nghiên cứu.

---

<div align="center">

**Made with ❤️ for IoT Security Education**

</div>