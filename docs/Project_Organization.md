# 📁 THỐNG KÊ VÀ TỔ CHỨC THƯ MỤC DỰ ÁN

## ESP32 Smart Home IoT Security - Project Structure

---

## ✅ FILE QUAN TRỌNG (GIỮ LẠI)

### 📂 Root Directory
| File | Kích thước | Mô tả | Quan trọng |
|------|------------|-------|------------|
| `README.md` | 7.2 KB | Hướng dẫn chính | ⭐⭐⭐ |
| `platformio.ini` | 1.1 KB | Cấu hình PlatformIO | ⭐⭐⭐ |
| `.gitignore` | 94 B | Git ignore rules | ⭐⭐⭐ |

### 📂 src/ (Firmware)
| File | Mô tả | Quan trọng |
|------|-------|------------|
| `main.cpp` | Code chính ESP32 (1136 dòng) | ⭐⭐⭐ |

### 📂 include/ (Headers)
| File | Mô tả | Quan trọng |
|------|-------|------------|
| `config.h` | Cấu hình WiFi, pins, thresholds | ⭐⭐⭐ |
| `README` | Hướng dẫn include | ⭐ |

### 📂 backend_example/ (Backend)
| File | Mô tả | Quan trọng |
|------|-------|------------|
| `simple_server.py` | Flask server | ⭐⭐⭐ |
| `requirements.txt` | Python dependencies | ⭐⭐⭐ |
| `README.md` | Hướng dẫn backend | ⭐⭐ |
| `templates/index.html` | Dashboard HTML | ⭐⭐⭐ |
| `static/style.css` | CSS styling | ⭐⭐⭐ |
| `static/script.js` | JavaScript logic | ⭐⭐⭐ |

### 📂 docs/ (Tài liệu - MỚI TẠO)
| File/Folder | Mô tả | Quan trọng |
|-------------|-------|------------|
| `Detailed_Attack_Defense_Guide.md` | Hướng dẫn chi tiết tấn công-phòng thủ | ⭐⭐⭐ |
| `Attack_Defense_Summary.md` | Tóm tắt tấn công-phòng thủ | ⭐⭐ |
| `checklists/` | Checklists dự án | ⭐⭐ |
| `security_risks/` | Phân tích rủi ro | ⭐⭐ |
| `attack_chains/` | Chuỗi tấn công | ⭐⭐ |
| `countermeasures/` | Biện pháp phòng chống | ⭐⭐ |

---

## ❌ FILE NÊN XÓA (THỪA/TRÙNG LẶP)

### Đề Xuất Xóa:
| File | Lý do | Hành động |
|------|-------|-----------|
| `.pio/` | Build cache, tự động tạo lại | 🗑️ **XÓA** (đã trong .gitignore) |
| `.vscode/` | IDE settings cá nhân | 🗑️ **XÓA** (đã trong .gitignore) |
| `backend_example/sensor_data.db` | Database demo, tự tạo lại | 🗑️ **XÓA** |
| `FINAL_REPORT.md` | Trùng với docs/ | ⚠️ **DI CHUYỂN** → docs/ |
| `IoT_Security_Analysis_Report.md` | Trùng với docs/security_risks/ | ⚠️ **DI CHUYỂN** → docs/ |
| `DEMO_SCRIPT.md` | Trùng với docs/ | ⚠️ **DI CHUYỂN** → docs/ |
| `QUICK_START.md` | Có thể gộp vào README.md | ⚠️ **XEM XÉT** |
| `demo_security_test.py` | Script demo | ⚠️ **DI CHUYỂN** → scripts/ |
| `fix_firewall.bat` | Script hỗ trợ | ⚠️ **DI CHUYỂN** → scripts/ |
| `lib/README` | File mặc định PlatformIO | ⭐ Giữ nguyên |
| `test/README` | File mặc định PlatformIO | ⭐ Giữ nguyên |

---

## 🔧 CẬP NHẬT .gitignore

```gitignore
# PlatformIO
.pio/
.vscode/

# Python
__pycache__/
*.pyc
*.pyo
venv/
.env

# Database
*.db
*.sqlite

# IDE
.idea/
*.swp
*.swo

# OS
.DS_Store
Thumbs.db

# Build
build/
*.bin
```

---

## 📂 CẤU TRÚC ĐỀ XUẤT (SAU KHI TỔ CHỨC LẠI)

```
ESP32_SmartHome_IoTSecurity/
├── 📄 README.md                    # Hướng dẫn chính
├── 📄 platformio.ini               # PlatformIO config
├── 📄 .gitignore                   # Git ignore
│
├── 📁 src/                         # Firmware ESP32
│   └── main.cpp
│
├── 📁 include/                     # Header files
│   └── config.h
│
├── 📁 backend/                     # Backend Server (đổi tên từ backend_example)
│   ├── simple_server.py
│   ├── requirements.txt
│   ├── README.md
│   ├── 📁 templates/
│   │   └── index.html
│   └── 📁 static/
│       ├── style.css
│       └── script.js
│
├── 📁 docs/                        # Tất cả tài liệu
│   ├── Detailed_Attack_Defense_Guide.md    # ⭐ MAIN GUIDE
│   ├── Attack_Defense_Summary.md
│   ├── FINAL_REPORT.md             # ← Di chuyển từ root
│   ├── DEMO_SCRIPT.md              # ← Di chuyển từ root
│   ├── IoT_Security_Analysis_Report.md     # ← Di chuyển từ root
│   ├── 📁 checklists/
│   ├── 📁 security_risks/
│   ├── 📁 attack_chains/
│   └── 📁 countermeasures/
│
├── 📁 scripts/                     # Scripts hỗ trợ (MỚI)
│   ├── demo_security_test.py       # ← Di chuyển từ root
│   └── fix_firewall.bat            # ← Di chuyển từ root
│
├── 📁 lib/                         # PlatformIO libraries
│   └── README
│
└── 📁 test/                        # PlatformIO tests
    └── README
```

---

## 🚀 COMMANDS ĐỂ TỔ CHỨC LẠI

### Windows PowerShell:
```powershell
cd E:\IoT\Ly_Thuyet\ATTT\BTT\ESP32_SmartHome_IoTSecurity

# 1. Tạo thư mục scripts
mkdir scripts

# 2. Di chuyển scripts vào thư mục scripts
Move-Item demo_security_test.py scripts/
Move-Item fix_firewall.bat scripts/

# 3. Di chuyển reports vào docs
Move-Item FINAL_REPORT.md docs/
Move-Item DEMO_SCRIPT.md docs/
Move-Item IoT_Security_Analysis_Report.md docs/

# 4. Xóa database (sẽ tự tạo lại)
Remove-Item backend_example/sensor_data.db

# 5. Đổi tên backend_example thành backend (optional)
Rename-Item backend_example backend

# 6. Cập nhật .gitignore
# (Copy nội dung mới vào .gitignore)

# 7. Kiểm tra lại cấu trúc
tree /F
```

---

## 📊 TÓM TẮT

| Hành động | Số lượng |
|-----------|----------|
| ✅ Files giữ lại | 15 files chính |
| 🗑️ Files xóa | 2 (cache, db) |
| ⚠️ Files di chuyển | 5 files |
| 📁 Thư mục mới | 1 (scripts/) |

---

## ✅ CHECKLIST TRƯỚC KHI PUSH GITHUB

- [ ] Xóa `.pio/` (hoặc đảm bảo trong .gitignore)
- [ ] Xóa `sensor_data.db`
- [ ] Di chuyển scripts vào `scripts/`
- [ ] Di chuyển reports vào `docs/`
- [ ] Cập nhật `.gitignore`
- [ ] Kiểm tra không có credentials thật trong `config.h`
- [ ] Commit và push

```bash
git add .
git commit -m "Reorganize project structure for security analysis report"
git push origin main
```
