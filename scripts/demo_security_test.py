#!/usr/bin/env python3
"""
Demo Security Testing - ESP32 Smart Home IoT
Thực hiện các test bảo mật cơ bản để chứng minh lỗ hổng
"""

import requests
import json
import time
import socket
import subprocess
import sys

def demo_hardcoded_credentials():
    """Demo 1: Hardcoded credentials trong source code"""
    print("🔍 DEMO 1: HARDCODED CREDENTIALS")
    print("=" * 50)
    
    # Đọc file config.h để show hardcoded credentials
    try:
        with open('include/config.h', 'r') as f:
            content = f.read()
            
        print("📁 File: include/config.h")
        print("⚠️  Phát hiện hardcoded credentials:")
        
        lines = content.split('\n')
        for line in lines:
            if 'WIFI_SSID' in line or 'WIFI_PASSWORD' in line or 'API_KEY' in line:
                print(f"   {line.strip()}")
                
        print("\n💥 RỦI RO: Attacker có thể extract credentials từ firmware!")
        print("🛡️  KHUYẾN NGHỊ: Sử dụng encrypted storage (NVS)")
        
    except Exception as e:
        print(f"❌ Lỗi đọc file: {e}")

def demo_plaintext_http():
    """Demo 2: HTTP plaintext transmission"""
    print("\n🔍 DEMO 2: PLAINTEXT HTTP TRANSMISSION")
    print("=" * 50)
    
    # Gửi request và show plaintext data
    test_data = {
        "device_id": "esp32_001_HACKED",
        "temperature": 999.9,  # Fake data
        "humidity": 999.9,     # Fake data  
        "light": 0,            # Fake data
        "timestamp": int(time.time() * 1000)
    }
    
    headers = {
        "Content-Type": "application/json",
        "Authorization": "Bearer sk_live_51234567890abcdef",  # Hardcoded API key
        "X-API-Key": "sk_live_51234567890abcdef"
    }
    
    print("📤 Gửi fake data qua HTTP (plaintext):")
    print(f"   URL: http://192.168.137.1:5000/api/sensor/data")
    print(f"   Data: {json.dumps(test_data)}")
    print(f"   API Key: {headers['Authorization']}")
    
    try:
        response = requests.post(
            "http://192.168.137.1:5000/api/sensor/data",
            json=test_data,
            headers=headers,
            timeout=10
        )
        
        if response.status_code == 200:
            print("✅ Fake data được chấp nhận!")
            print(f"📥 Response: {response.text}")
            print("\n💥 RỦI RO: Không có validation, attacker có thể inject fake data!")
            print("🛡️  KHUYẾN NGHỊ: Implement HTTPS + input validation")
        else:
            print(f"❌ Request failed: {response.status_code}")
            
    except Exception as e:
        print(f"❌ Lỗi kết nối: {e}")

def demo_api_abuse():
    """Demo 3: API abuse without authentication"""
    print("\n🔍 DEMO 3: API ABUSE - NO AUTHENTICATION")
    print("=" * 50)
    
    # Test các endpoint không cần authentication
    endpoints = [
        "/api/devices",
        "/api/sensor/latest/esp32_001",
        "/"
    ]
    
    base_url = "http://192.168.137.1:5000"
    
    for endpoint in endpoints:
        try:
            response = requests.get(f"{base_url}{endpoint}", timeout=5)
            print(f"📡 {endpoint}: HTTP {response.status_code}")
            
            if response.status_code == 200:
                if endpoint == "/api/devices":
                    data = response.json()
                    print(f"   📊 Devices exposed: {data.get('devices', [])}")
                elif "latest" in endpoint:
                    data = response.json()
                    print(f"   🌡️  Latest data: Temp={data.get('temperature')}°C")
                    
        except Exception as e:
            print(f"❌ {endpoint}: {e}")
    
    print("\n💥 RỦI RO: API endpoints không có authentication!")
    print("🛡️  KHUYẾN NGHỊ: Implement API authentication & authorization")

def demo_sensor_spoofing():
    """Demo 4: Sensor spoofing simulation"""
    print("\n🔍 DEMO 4: SENSOR SPOOFING SIMULATION")
    print("=" * 50)
    
    print("💡 PHYSICAL ATTACK: LDR Sensor Manipulation")
    print("   - Attacker có thể dùng đèn LED mạnh để làm sai lệch giá trị LDR")
    print("   - Hoặc che kín sensor để tạo ra giá trị 0%")
    print("   - ESP32 sẽ đọc và gửi dữ liệu sai lệch lên server")
    
    # Simulate spoofed sensor data
    spoofed_readings = [
        {"light": 0, "attack": "Che kín sensor (darkness attack)"},
        {"light": 100, "attack": "Chiếu đèn LED mạnh (brightness attack)"},
        {"light": 50, "attack": "Điều khiển ánh sáng để tạo pattern"}
    ]
    
    for reading in spoofed_readings:
        print(f"   🎯 {reading['attack']}: Light = {reading['light']}%")
    
    print("\n💥 RỦI RO: Sensor analog có thể bị manipulate vật lý!")
    print("🛡️  KHUYẾN NGHỊ: Sensor validation + anomaly detection")

def demo_network_reconnaissance():
    """Demo 5: Network reconnaissance"""
    print("\n🔍 DEMO 5: NETWORK RECONNAISSANCE")
    print("=" * 50)
    
    print("🔍 NETWORK SCANNING SIMULATION:")
    
    # Check if ESP32 IP is reachable
    esp32_ip = "192.168.137.14"
    server_ip = "192.168.137.1"
    
    print(f"   📍 Target ESP32: {esp32_ip}")
    print(f"   📍 Target Server: {server_ip}")
    
    # Simulate port scanning
    common_ports = [80, 443, 1883, 5000, 8080]
    
    for port in common_ports:
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(1)
            result = sock.connect_ex((server_ip, port))
            sock.close()
            
            if result == 0:
                print(f"   ✅ Port {port}: OPEN")
            else:
                print(f"   ❌ Port {port}: CLOSED")
                
        except Exception as e:
            print(f"   ❓ Port {port}: ERROR")
    
    print("\n💥 RỦI RO: Open ports có thể bị scan và exploit!")
    print("🛡️  KHUYẾN NGHỊ: Firewall rules + port security")

def main():
    print("🚨 ESP32 SMART HOME - SECURITY VULNERABILITY DEMO")
    print("⚠️  CHỈ SỬ DỤNG CHO MỤC ĐÍCH GIÁO DỤC!")
    print("=" * 60)
    
    # Chạy các demo
    demo_hardcoded_credentials()
    demo_plaintext_http()
    demo_api_abuse()
    demo_sensor_spoofing()
    demo_network_reconnaissance()
    
    print("\n" + "=" * 60)
    print("📋 TÓM TẮT CÁC LỖ HỔNG ĐÃ DEMO:")
    print("1. ❌ Hardcoded credentials trong source code")
    print("2. ❌ HTTP plaintext transmission")
    print("3. ❌ API endpoints không authentication")
    print("4. ❌ Sensor có thể bị manipulate vật lý")
    print("5. ❌ Network services có thể bị reconnaissance")
    
    print("\n🛡️  BIỆN PHÁP KHUYẾN NGHỊ:")
    print("• Implement HTTPS/TLS encryption")
    print("• Sử dụng encrypted credential storage")
    print("• Add API authentication & rate limiting")
    print("• Implement sensor data validation")
    print("• Configure proper firewall rules")
    print("• Add anomaly detection & monitoring")

if __name__ == "__main__":
    main()