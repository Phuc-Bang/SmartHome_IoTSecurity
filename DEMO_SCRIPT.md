# 🎬 SCRIPT DEMO ESP32 SMART HOME IoT SECURITY

## 📋 Chuẩn bị Demo (5 phút)

### Thiết bị cần thiết:
- ✅ ESP32-S3 đã kết nối và hoạt động
- ✅ Backend server đang chạy (http://192.168.99.85:5000)
- ✅ Web dashboard mở sẵn
- ✅ Serial monitor ESP32
- ✅ Terminal với script demo

### Kiểm tra trước khi demo:
```bash
# 1. Kiểm tra ESP32 hoạt động
pio device monitor

# 2. Kiểm tra backend server
curl http://192.168.99.85:5000/api/devices

# 3. Mở dashboard
# http://192.168.99.85:5000
```

---

## 🎯 PHẦN 1: GIỚI THIỆU HỆ THỐNG (3 phút)

### 1.1 Mô tả hệ thống
> "Chào mọi người, hôm nay tôi sẽ demo hệ thống ESP32 Smart Home và phân tích các lỗ hổng bảo mật IoT"

**Show:**
- ESP32-S3 với các sensor đã kết nối
- OLED hiển thị dữ liệu real-time
- LEDs báo trạng thái

**Giải thích:**
- DHT11: Đo nhiệt độ, độ ẩm
- MH LDR: Cảm biến ánh sáng
- OLED: Hiển thị dữ liệu
- 5 LEDs: Báo trạng thái hệ thống

### 1.2 Kiến trúc hệ thống
**Show diagram:**
```
ESP32-S3 → WiFi → Backend Server → Web Dashboard
```

**Giải thích luồng dữ liệu:**
1. ESP32 đọc sensor mỗi 10 giây
2. Gửi dữ liệu qua HTTP (plaintext)
3. Backend lưu vào database
4. Dashboard hiển thị real-time

---

## 🎯 PHẦN 2: DEMO HOẠT ĐỘNG BÌNH THƯỜNG (2 phút)

### 2.1 Show Serial Monitor
```
📊 ========== Sensor Reading ==========
🌡️ [SENSORS] Reading data...
✅ [DHT11] Temp: 25.5°C, Humidity: 60.2%
💡 [LDR] Raw: 2048, Light: 50%
📡 [HTTP] Preparing to send data...
✅ [HTTP] Response code: 200 (SUCCESS)
```

### 2.2 Show Web Dashboard
- Biểu đồ nhiệt độ, độ ẩm, ánh sáng
- Dữ liệu cập nhật real-time
- Danh sách devices

### 2.3 Tương tác với sensor
- Thổi vào DHT11 → nhiệt độ tăng
- Che LDR → ánh sáng giảm
- LEDs thay đổi theo sensor

---

## 🎯 PHẦN 3: PHÂN TÍCH BẢO MẬT (10 phút)

### 3.1 Demo Hardcoded Credentials (2 phút)
```bash
# Show source code
cat include/config.h
```

**Giải thích:**
> "Như các bạn thấy, WiFi password và API key được hardcode trực tiếp trong source code. Đây là lỗ hổng nghiêm trọng!"

**Show:**
```cpp
#define WIFI_SSID "Phuc Bang"
#define WIFI_PASSWORD "12345678"
#define API_KEY "sk_live_51234567890abcdef"
```

### 3.2 Demo HTTP Plaintext Attack (3 phút)
```bash
# Chạy security demo
python demo_security_test.py
```

**Highlight:**
- Fake data được inject thành công
- Không có validation
- API key bị expose

**Show dashboard:**
- Device "esp32_001_HACKED" xuất hiện
- Dữ liệu fake (999.9°C) được hiển thị

### 3.3 Demo Physical Sensor Attack (2 phút)
**Thực hành:**
1. Che kín LDR sensor → Light = 0%
2. Chiếu đèn phone vào LDR → Light = 100%
3. Show dashboard cập nhật dữ liệu sai

**Giải thích:**
> "Attacker có thể manipulate sensor vật lý để inject dữ liệu sai mà không cần hack firmware"

### 3.4 Demo API Abuse (2 phút)
```bash
# Test API endpoints
curl http://192.168.99.85:5000/api/devices
curl http://192.168.99.85:5000/api/sensor/latest/esp32_001
```

**Show:**
- Tất cả endpoints đều public
- Không cần authentication
- Có thể access dữ liệu của tất cả devices

### 3.5 Network Reconnaissance (1 phút)
**Show port scan results:**
```
Port 5000: OPEN ← Backend server
Port 1883: CLOSED ← MQTT (disabled)
```

---

## 🎯 PHẦN 4: KILL CHAIN & ATTACK SCENARIOS (5 phút)

### 4.1 Attack Chain Demo
**Bước 1: Reconnaissance**
```bash
# Network scan
nmap -sS 192.168.99.0/24
```

**Bước 2: Credential Extraction**
```bash
# Firmware analysis (simulation)
strings firmware.bin | grep -E "(password|key|ssid)"
```

**Bước 3: Exploitation**
```bash
# Inject fake data
curl -X POST http://192.168.99.85:5000/api/sensor/data \
  -H "Authorization: Bearer sk_live_51234567890abcdef" \
  -d '{"device_id":"HACKED","temperature":999}'
```

**Bước 4: Impact**
- Show fake data trên dashboard
- Explain potential damage

### 4.2 Real-world Impact
**Giải thích tác động:**
- Privacy breach: Biết được lịch sinh hoạt gia đình
- Data manipulation: Làm sai lệch hệ thống tự động
- Lateral movement: Tấn công các thiết bị khác trong mạng
- Physical security: Biết khi nào nhà không có người

---

## 🎯 PHẦN 5: BIỆN PHÁP PHÒNG CHỐNG (3 phút)

### 5.1 Device Level
- ✅ Flash encryption
- ✅ Secure boot
- ✅ Encrypted credential storage
- ✅ Input validation

### 5.2 Network Level
- ✅ HTTPS/TLS encryption
- ✅ Certificate pinning
- ✅ VPN tunneling
- ✅ Network segmentation

### 5.3 Backend Level
- ✅ API authentication
- ✅ Rate limiting
- ✅ Input sanitization
- ✅ Anomaly detection

### 5.4 Physical Level
- ✅ Sensor validation
- ✅ Tamper detection
- ✅ Secure enclosure
- ✅ Multiple sensor correlation

---

## 🎯 PHẦN 6: KẾT LUẬN (2 phút)

### 6.1 Tóm tắt lỗ hổng
1. ❌ Hardcoded credentials
2. ❌ Plaintext communication
3. ❌ No authentication
4. ❌ Physical sensor manipulation
5. ❌ No input validation

### 6.2 Bài học rút ra
> "IoT security cần được thiết kế từ đầu, không phải là afterthought. Mỗi layer đều cần có biện pháp bảo mật riêng."

### 6.3 Q&A
Chuẩn bị trả lời các câu hỏi về:
- Chi tiết kỹ thuật implementation
- Cost/benefit của các biện pháp bảo mật
- Real-world attack cases
- Compliance requirements

---

## 📝 NOTES CHO PRESENTER

### Timing:
- **Tổng thời gian:** 30 phút
- **Demo:** 20 phút
- **Q&A:** 10 phút

### Tips:
- Chuẩn bị backup slides nếu demo fail
- Test tất cả commands trước khi present
- Có sẵn screenshots cho các bước quan trọng
- Explain technical terms cho audience không chuyên

### Backup Plans:
- Nếu ESP32 disconnect: Dùng recorded video
- Nếu backend crash: Dùng static screenshots
- Nếu network issue: Demo offline với local data

### Key Messages:
1. IoT devices thường có nhiều lỗ hổng bảo mật
2. Attackers có thể exploit từ nhiều vectors khác nhau
3. Defense in depth là cần thiết
4. Security phải được thiết kế từ đầu, không phải bolt-on