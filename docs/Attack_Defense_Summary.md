# 📋 TÓM TẮT: TẤN CÔNG & PHÒNG THỦ IoT

## ESP32 Smart Home Security - Attack vs Defense Matrix

---

## 🎯 Bảng Tổng Hợp Nhanh

| # | Tấn Công | Phòng Thủ Tương Ứng |
|---|----------|---------------------|
| 1 | Firmware Dumping → Credential Extraction | Flash Encryption + Secure Boot |
| 2 | HTTP Traffic Sniffing | HTTPS/TLS + Certificate Pinning |
| 3 | Unauthorized API Access | JWT Authentication |
| 4 | SQL Injection | Parameterized Queries |
| 5 | Fake Data Injection | Input Validation + Anomaly Detection |
| 6 | Physical Sensor Manipulation | Sensor Anomaly Detection |
| 7 | DoS / Brute Force | Rate Limiting |
| 8 | Lateral Movement | Network Segmentation |

---

## ⚔️ Chi Tiết Tấn Công & Phòng Thủ

### 1️⃣ Credential Extraction Attack

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Dump firmware via USB → Extract hardcoded WiFi/API credentials | **Giải pháp**: ESP32 Flash Encryption + Secure Boot |
| **Command**: `esptool.py read_flash` → `strings firmware.bin \| grep password` | **Kết quả**: Firmware encrypted, strings unreadable |
| **Impact**: Full network access | **Chặn tại**: Step 1-2 (Entry/Exploitation) |

```
ATTACK:  [USB Access] → [Firmware Dump] → [Strings Extract] → [Network Access]
                              ↓
DEFENSE:              🛡️ FLASH ENCRYPTION (data unreadable)
```

---

### 2️⃣ HTTP Traffic Interception

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Sniff HTTP plaintext traffic → Steal sensor data & API keys | **Giải pháp**: HTTPS/TLS 1.2+ với Certificate Pinning |
| **Tool**: Wireshark, tcpdump | **Kết quả**: Traffic encrypted, MITM blocked |
| **Impact**: Data theft, credential exposure | **Chặn tại**: Step 2-3 (Recon/Exploitation) |

```
ATTACK:  [Network Access] → [Wireshark Capture] → [Read Plaintext] → [Steal Data]
                                    ↓
DEFENSE:                    🛡️ HTTPS/TLS (encrypted traffic)
```

---

### 3️⃣ Unauthorized API Access

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Access public API endpoints → Inject/retrieve data | **Giải pháp**: JWT Authentication required |
| **Command**: `curl POST /api/sensor/data` → 200 OK | **Kết quả**: `curl POST` → 401 Unauthorized |
| **Impact**: Data manipulation | **Chặn tại**: Step 3 (Exploitation) |

```
ATTACK:  [Find API] → [No Auth Check] → [Inject Data] → [Corrupt Database]
                            ↓
DEFENSE:            🛡️ JWT TOKEN REQUIRED (401 Unauthorized)
```

---

### 4️⃣ SQL Injection Attack

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Inject SQL via device_id → Extract/destroy database | **Giải pháp**: Parameterized Queries với ? placeholders |
| **Payload**: `' OR '1'='1` hoặc `'; DROP TABLE--` | **Kết quả**: Payload treated as literal string |
| **Impact**: Full database compromise | **Chặn tại**: Step 3 (Exploitation) |

```
ATTACK:  [Find Input] → [Test SQLi] → [Execute Payload] → [DB Compromise]
                              ↓
DEFENSE:              🛡️ PARAMETERIZED QUERY (payload = data, not code)
```

**Code Fix:**
```python
# ❌ Vulnerable:  f"SELECT * FROM t WHERE id = '{user_input}'"
# ✅ Secure:     cursor.execute("SELECT * FROM t WHERE id = ?", (user_input,))
```

---

### 5️⃣ Fake Data Injection

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Send invalid sensor data (temp=999°C) | **Giải pháp**: Input Validation + Range Checking |
| **Payload**: `{"temperature": 999.9}` | **Kết quả**: 400 - "temperature out of range (-40 to 80)" |
| **Impact**: Corrupt analytics, wrong automation | **Chặn tại**: Step 3-4 (Exploitation/Impact) |

```
ATTACK:  [API Access] → [Send Fake Data] → [Data Accepted] → [Dashboard Shows 999°C]
                              ↓
DEFENSE:              🛡️ INPUT VALIDATION (400 Bad Request)
```

---

### 6️⃣ Physical Sensor Spoofing

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Cover/illuminate LDR → Inject false light readings | **Giải pháp**: Anomaly Detection + Rate-of-Change Check |
| **Method**: Cover LDR → Light=0%, LED flashlight → Light=100% | **Kết quả**: ⚠️ Warning: "Rapid change detected" |
| **Impact**: Wrong automation triggers | **Chặn tại**: Step 4 (Impact) |

```
ATTACK:  [Physical Access] → [Manipulate Sensor] → [False Reading] → [Wrong Automation]
                                                          ↓
DEFENSE:                                          🛡️ ANOMALY DETECTION (flagged)
```

---

### 7️⃣ DoS / Brute Force Attack

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Flood API with requests → Server overload | **Giải pháp**: Rate Limiting (10 req/min) |
| **Command**: `while true; do curl POST /api; done` | **Kết quả**: 429 Too Many Requests after limit |
| **Impact**: Service unavailable | **Chặn tại**: Step 3 (Exploitation) |

```
ATTACK:  [Target API] → [Flood Requests] → [Server Crash] → [DoS]
                              ↓
DEFENSE:              🛡️ RATE LIMITING (429 after 10 requests/min)
```

---

### 8️⃣ Lateral Movement Attack

| Tấn Công | Phòng Thủ |
|----------|-----------|
| **Mô tả**: Compromise IoT device → Attack other network devices | **Giải pháp**: Network Segmentation (IoT VLAN) |
| **Path**: ESP32 → Same subnet → Attack PCs/Servers | **Kết quả**: IoT VLAN isolated, cannot reach main network |
| **Impact**: Full network compromise | **Chặn tại**: Step 2 (Lateral Movement) |

```
ATTACK:  [Compromise ESP32] → [Scan Network] → [Attack Other Devices]
                                    ↓
DEFENSE:                    🛡️ VLAN ISOLATION (IoT ≠ Main Network)
```

---

## 📊 Ma Trận Tổng Hợp

| Attack Chain | Entry Point | Target | Defense 1 | Defense 2 | Defense 3 |
|--------------|-------------|--------|-----------|-----------|-----------|
| **AC1** Credential Extraction | USB/Physical | Credentials | Flash Encryption | Secure Boot | NVS Storage |
| **AC2** Data Injection | Network | Database | HTTPS | JWT Auth | Input Validation |
| **AC3** Sensor Spoofing | Physical | Automation | Tamper Detection | Anomaly Detection | Multi-sensor Correlation |
| **AC4** SQL Injection | API | Database | Parameterized SQL | Input Validation | Rate Limiting |
| **AC5** Full Compromise | Physical | Everything | All of above | Defense in Depth | Monitoring |

---

## ✅ Checklist Triển Khai Theo Ưu Tiên

### 🔴 Ưu Tiên Cao (Tuần 1):
- [ ] Parameterized SQL Queries
- [ ] JWT API Authentication  
- [ ] HTTPS/TLS Implementation

### 🟠 Ưu Tiên Trung Bình (Tuần 2-3):
- [ ] Flash Encryption + Secure Boot
- [ ] Input Validation
- [ ] Rate Limiting

### 🟡 Ưu Tiên Thấp (Tuần 4+):
- [ ] Anomaly Detection
- [ ] Network Segmentation
- [ ] Monitoring & Alerting

---

## 📁 Tài Liệu Liên Quan

| File | Nội Dung |
|------|----------|
| [IoT_Security_Risks_Analysis.md](./security_risks/IoT_Security_Risks_Analysis.md) | 15 Rủi ro theo 3 lớp |
| [IoT_Attack_Chains.md](./attack_chains/IoT_Attack_Chains.md) | 5 Attack Chains chi tiết |
| [IoT_Countermeasures.md](./countermeasures/IoT_Countermeasures.md) | 8 Biện pháp phòng chống |

---

> **Ngày tạo**: 07/01/2026  
> **Dự án**: ESP32 Smart Home IoT Security
