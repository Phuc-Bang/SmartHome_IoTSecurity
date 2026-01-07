# BÁO CÁO PHÂN TÍCH BẢO MẬT IoT - ESP32 SMART HOME
## Chuyên gia An toàn thông tin IoT

---

## 1. MÔ TẢ HỆ THỐNG IoT

Hệ thống Smart Home sử dụng ESP32-S3 làm vi điều khiển trung tâm, tích hợp cảm biến DHT11 (nhiệt độ/độ ẩm) và mô-đun cảm biến ánh sáng quang điện trở MH (LDR) để thu thập dữ liệu môi trường. ESP32-S3 hiển thị thông tin trên màn hình OLED 0.96" và truyền dữ liệu qua Wi-Fi đến backend server thông qua HTTP API và MQTT broker. Hệ thống hoạt động theo chu kỳ 10 giây, đọc cảm biến và gửi dữ liệu JSON lên cloud để lưu trữ và hiển thị trên dashboard. Firmware được lập trình với Arduino framework, sử dụng các thư viện chuẩn cho kết nối mạng và xử lý cảm biến analog.

---

## 2. PHÂN TÍCH KIẾN TRÚC HỆ THỐNG

### 2.1 Device Layer
- **ESP32-S3**: Vi điều khiển chính, xử lý logic, kết nối Wi-Fi
- **DHT11**: Cảm biến nhiệt độ và độ ẩm (GPIO 4)
- **MH Light Sensor (LDR)**: Mô-đun cảm biến ánh sáng quang điện trở (GPIO 1)
- **OLED 0.96"**: Màn hình hiển thị dữ liệu (I2C: SDA=GPIO14, SCL=GPIO13)
- **5x LED**: Đèn báo trạng thái (GPIO 2,5,6,7,8)
- **Firmware**: Arduino framework với các thư viện WiFi, HTTPClient, PubSubClient

### 2.2 Network Layer
- **Wi-Fi 802.11**: Kết nối không dây đến router gia đình
- **HTTP Protocol**: Truyền dữ liệu qua port 80 (plaintext)
- **MQTT Protocol**: Pub/Sub messaging qua port 1883 (plaintext)
- **Router/Gateway**: Điểm trung gian kết nối Internet

### 2.3 Backend/Cloud Layer
- **API Server**: REST API endpoint nhận dữ liệu sensor (port 5000)
- **MQTT Broker**: Message broker xử lý MQTT traffic
- **Database**: Lưu trữ dữ liệu sensor và metadata
- **Dashboard**: Web interface hiển thị dữ liệu và điều khiển

### 2.4 Luồng Dữ Liệu

**Device → Backend:**
1. ESP32-S3 đọc cảm biến DHT11 và mô-đun LDR MH
2. Tạo JSON payload với device_id, temperature, humidity, light (%), timestamp
3. Gửi HTTP POST đến API endpoint với Bearer token
4. Đồng thời publish MQTT message đến topic "home/sensor/esp32_001"

**Backend → Device:**
1. Backend có thể gửi lệnh điều khiển qua MQTT
2. ESP32-S3 subscribe các topic control để nhận lệnh
3. Firmware xử lý lệnh và thực hiện hành động tương ứng

---

## 3. SƠ ĐỒ LOGIC HỆ THỐNG IoT

```
[ESP32-S3 Device]
├── DHT11 (GPIO 4)
├── MH LDR Sensor (GPIO 1)
├── OLED (I2C: SDA=14, SCL=13)
├── 5x Status LEDs (GPIO 2,5,6,7,8)
└── Wi-Fi Module
    │
    ▼ (HTTP/MQTT - Plaintext)
[Home Router/Gateway]
    │
    ▼ (Internet)
[Backend Infrastructure]
├── API Server (Port 5000)
├── MQTT Broker (Port 1883)
├── Database
└── Web Dashboard

Attack Surface Points:
• Firmware/UART Interface
• Wi-Fi Credentials & Traffic
• HTTP API Communication
• MQTT Broker Communication
• Backend API Endpoints
• Analog Sensor Manipulation
• LED Control Commands (MQTT)
```

---

## 4. XÁC ĐỊNH ATTACK SURFACE

### 4.1 Các Điểm Tấn Công Tiềm Năng

1. **Firmware ESP32-S3**: Hardcoded credentials, debug interfaces
2. **UART/USB Interface**: Serial debugging port, firmware dumping
3. **Wi-Fi Credentials**: Plaintext storage trong firmware
4. **HTTP Communication**: Unencrypted data transmission
5. **MQTT Communication**: No TLS encryption, plaintext authentication
6. **API Endpoints**: Authentication bypass, injection attacks
7. **Physical Access**: Device tampering, side-channel attacks
8. **Analog Sensor Manipulation**: LDR signal interference, spoofing

---

## 5. LIỆT KÊ RỦI RO BẢO MẬT

### 5.1 Device/Firmware Layer
1. **Hardcoded Credentials**: Wi-Fi password, API keys lưu trong firmware
2. **Firmware Dumping**: Có thể extract firmware qua UART/JTAG
3. **Debug Interface**: Serial port expose thông tin nhạy cảm
4. **Memory Corruption**: Buffer overflow trong xử lý JSON/string
5. **Analog Signal Manipulation**: LDR sensor có thể bị can thiệp bằng ánh sáng ngoài

### 5.2 Network/Transport Layer
6. **Plaintext Communication**: HTTP và MQTT không mã hóa
7. **Wi-Fi Sniffing**: Traffic có thể bị intercept trên mạng không dây
8. **Man-in-the-Middle**: Attacker có thể chèn vào giữa device và server

### 5.3 Backend/Cloud Layer
9. **API Authentication Bypass**: Weak token validation
10. **Injection Attacks**: SQL injection qua sensor data
11. **MQTT Broker Security**: No authentication, open subscriptions

---

## 6. ÁP DỤNG KILL CHAIN

### 6.1 Reconnaissance (Trinh sát)
**Mục tiêu**: Thu thập thông tin về hệ thống
- **Network Scanning**: Nmap scan tìm ESP32 devices trên mạng
- **Wi-Fi Monitoring**: Airodump-ng capture Wi-Fi traffic
- **Physical Sensor Attack**: Sử dụng đèn LED mạnh để làm sai lệch giá trị LDR
- **Firmware Analysis**: Binwalk extract firmware từ device
- **OSINT**: Tìm kiếm thông tin về API endpoints, default credentials

### 6.2 Exploitation (Khai thác)
**Mục tiêu**: Tận dụng điểm yếu để xâm nhập
- **Credential Extraction**: Strings command trên firmware dump
- **Sensor Spoofing**: Manipulate LDR bằng cách che/chiếu sáng để inject false data
- **Traffic Interception**: Wireshark capture HTTP/MQTT plaintext
- **API Abuse**: Sử dụng stolen API key để inject fake data
- **MQTT Hijacking**: Subscribe/publish unauthorized messages

### 6.3 Impact (Tác động)
**Mục tiêu**: Gây thiệt hại cho hệ thống
- **Data Manipulation**: Inject false sensor readings
- **Service Disruption**: DoS attack trên MQTT broker
- **Privacy Breach**: Access sensitive home environment data
- **Lateral Movement**: Sử dụng compromised device để tấn công mạng nội bộ

---

## 7. XÂY DỰNG ATTACK CHAIN

### Bước 1: Entry Point - Firmware Dumping
```bash
# Kết nối UART để dump firmware
esptool.py --port COM3 read_flash 0x0 0x400000 firmware_dump.bin
```

### Bước 2: Credential Extraction
```bash
# Tìm hardcoded credentials trong firmware
strings firmware_dump.bin | grep -E "(password|key|ssid|api)"
# Output: HomeNetwork2024, MySecurePass123, sk_live_51234567890abcdef
```

### Bước 3: Network Infiltration
```bash
# Sử dụng Wi-Fi credentials để join network
# Sniff traffic để lấy API endpoints và MQTT broker info
tcpdump -i wlan0 host 192.168.1.100
```

### Bước 4: Backend Compromise
```bash
# Sử dụng stolen API key để inject malicious data
curl -X POST http://192.168.1.100:5000/api/sensor/data \
  -H "Authorization: Bearer sk_live_51234567890abcdef" \
  -H "Content-Type: application/json" \
  -d '{"device_id":"esp32_001","temperature":999,"humidity":999,"light":100}'
```

---

## 8. ĐỀ XUẤT BIỆN PHÁP PHÒNG CHỐNG

### 8.1 Device-Level Security
1. **Secure Boot & Encryption**: Enable ESP32 flash encryption và secure boot
   - *Chặn*: Firmware dumping và tampering (Kill Chain: Recon)
   
2. **Credential Management**: Sử dụng NVS encrypted storage thay vì hardcode
   - *Chặn*: Credential extraction (Attack Chain: Bước 2)

### 8.2 Network-Level Security  
3. **TLS/SSL Implementation**: Chuyển sang HTTPS và MQTTS
   - *Chặn*: Traffic interception và MITM (Kill Chain: Exploitation)
   
4. **Certificate Pinning**: Pin server certificates trong firmware
   - *Chặn*: Certificate spoofing attacks (Attack Chain: Bước 3)

### 8.3 Backend-Level Security
5. **API Rate Limiting**: Implement rate limiting và request validation
   - *Chặn*: API abuse và DoS attacks (Kill Chain: Impact)
   
6. **Input Sanitization**: Validate tất cả sensor data input
   - *Chặn*: Injection attacks (Attack Chain: Bước 4)

### 8.4 Additional Measures
7. **Sensor Validation**: Implement range checking và anomaly detection cho LDR readings
8. **Device Authentication**: Implement mutual TLS authentication
9. **Monitoring & Logging**: Real-time anomaly detection
10. **OTA Security**: Signed firmware updates với rollback protection

---

## 9. BẰNG CHỨNG LAB

### 9.1 Firmware Analysis Demonstration
```bash
# Dump firmware từ ESP32-S3
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 esp32_firmware.bin

# Extract strings để tìm credentials
strings esp32_firmware.bin | grep -A5 -B5 "WIFI_SSID\|API_KEY\|MQTT"

# Kết quả (đã che thông tin nhạy cảm):
HomeNetwork****
MySecurePass***
sk_live_512345*****
192.168.1.***
mqtt_password***
```

### 9.2 Liên Kết Với Rủi Ro
Bằng chứng này chứng minh **rủi ro hardcoded credentials** trong firmware, tương ứng với **Attack Chain Bước 2** - credential extraction. Attacker có thể dễ dàng lấy được Wi-Fi password, API key và MQTT credentials để thực hiện các bước tấn công tiếp theo.

---

## KẾT LUẬN

Hệ thống ESP32 Smart Home hiện tại có nhiều lỗ hổng bảo mật nghiêm trọng ở tất cả các lớp. Việc triển khai các biện pháp bảo mật được đề xuất là cần thiết để bảo vệ hệ thống khỏi các cuộc tấn công IoT phổ biến.