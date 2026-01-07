# Backend Server với Giao Diện Web Hiện Đại

## Mô tả
Server backend Flask với giao diện web hiện đại, responsive và sinh động cho hệ thống ESP32 Smart Home.

⚠️ **CẢNH BÁO**: Server này chứa các lỗ hổng bảo mật có chủ ý để phục vụ mục đích giáo dục!

## Tính năng giao diện

### 🎨 Modern UI/UX
- **Responsive Design**: Tương thích mọi thiết bị (desktop, tablet, mobile)
- **Real-time Updates**: Cập nhật dữ liệu tự động mỗi 10 giây
- **Interactive Charts**: Biểu đồ thời gian thực với Chart.js
- **Status Indicators**: Hiển thị trạng thái kết nối và cảm biến
- **Gradient Background**: Thiết kế hiện đại với màu sắc gradient
- **Smooth Animations**: Hiệu ứng chuyển động mượt mà

### 📊 Dashboard Features
- **Sensor Cards**: Hiển thị nhiệt độ, độ ẩm, ánh sáng với icon và màu sắc
- **System Status**: Thông tin device ID, uptime, số lượng data points
- **Historical Charts**: Biểu đồ xu hướng dữ liệu cảm biến
- **Security Analysis**: Phân tích lỗ hổng và khuyến nghị bảo mật
- **Device Selector**: Chọn thiết bị để xem dữ liệu

## Cài đặt

```bash
# Cài đặt Python dependencies
pip install -r requirements.txt

# Chạy server
python simple_server.py
```

Server sẽ chạy trên `http://localhost:5000`

## Cấu trúc thư mục

```
backend_example/
├── simple_server.py          # Flask server chính
├── requirements.txt          # Python dependencies
├── sensor_data.db           # SQLite database (tự động tạo)
├── templates/
│   └── index.html           # Template HTML chính
└── static/
    ├── style.css            # CSS hiện đại với animations
    └── script.js            # JavaScript cho real-time updates
```

## API Endpoints

### POST /api/sensor/data
Nhận dữ liệu từ ESP32
```json
{
    "device_id": "esp32_001",
    "temperature": 25.2,
    "humidity": 60.1,
    "light": 50,
    "timestamp": 1234567890
}
```

### GET /api/sensor/latest/{device_id}
Lấy dữ liệu mới nhất của device

### GET /api/sensor/history/{device_id}
Lấy lịch sử dữ liệu cho biểu đồ (query param: `limit`)

### GET /api/devices
Liệt kê tất cả devices đã gửi dữ liệu

### GET / hoặc /dashboard
Dashboard web hiện đại với real-time monitoring

## Screenshots

### Desktop View
- Modern gradient background
- Card-based sensor display
- Real-time charts với Chart.js
- Security analysis section

### Mobile View
- Fully responsive design
- Touch-friendly interface
- Optimized for small screens

## Công nghệ sử dụng

### Backend
- **Flask**: Web framework
- **SQLite**: Database
- **Jinja2**: Template engine

### Frontend
- **HTML5**: Semantic markup
- **CSS3**: Modern styling với Flexbox/Grid
- **JavaScript ES6+**: Real-time functionality
- **Chart.js**: Interactive charts
- **Font Awesome**: Icons

### Design Features
- **CSS Variables**: Consistent theming
- **CSS Grid/Flexbox**: Modern layouts
- **CSS Animations**: Smooth transitions
- **Responsive Design**: Mobile-first approach
- **Modern Typography**: Inter font family

## Lỗ hổng bảo mật (Có chủ ý)

1. **No Authentication**: Không có xác thực API
2. **SQL Injection**: Truy vấn SQL không được sanitize
3. **No Input Validation**: Không validate dữ liệu đầu vào
4. **Debug Mode**: Flask chạy ở debug mode
5. **Bind All Interfaces**: Server bind 0.0.0.0 thay vì localhost

## Sử dụng với ESP32

1. Chạy server trên máy tính
2. Cập nhật IP address trong `config.h` của ESP32
3. Upload code lên ESP32
4. Xem dữ liệu tại `http://localhost:5000/dashboard`