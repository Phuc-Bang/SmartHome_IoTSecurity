@echo off
echo ========================================
echo ESP32 Smart Home - Windows Firewall Fix
echo ========================================
echo.
echo This script will help fix Windows Firewall issues
echo that may block ESP32 HTTP connections.
echo.
echo IMPORTANT: Run as Administrator!
echo.
pause

echo.
echo 1. Adding Python to Windows Firewall...
netsh advfirewall firewall add rule name="Python HTTP Server" dir=in action=allow protocol=TCP localport=5000
netsh advfirewall firewall add rule name="Python HTTP Server Out" dir=out action=allow protocol=TCP localport=5000

echo.
echo 2. Adding rule for ESP32 connections...
netsh advfirewall firewall add rule name="ESP32 HTTP Client" dir=in action=allow protocol=TCP remoteip=192.168.137.0/24
netsh advfirewall firewall add rule name="ESP32 HTTP Client Out" dir=out action=allow protocol=TCP remoteip=192.168.137.0/24

echo.
echo 3. Testing server connectivity...
echo Testing if port 5000 is accessible...
netstat -an | findstr :5000

echo.
echo 4. Checking current firewall status...
netsh advfirewall show allprofiles state

echo.
echo ========================================
echo Firewall rules added successfully!
echo ========================================
echo.
echo Next steps:
echo 1. Start the backend server: python backend_example/simple_server.py
echo 2. Test with: curl http://192.168.137.1:5000/api/devices
echo 3. Upload and run ESP32 code
echo.
echo If still having issues:
echo - Check antivirus software
echo - Temporarily disable Windows Defender Firewall
echo - Verify ESP32 IP is 192.168.137.14
echo - Verify server IP is 192.168.137.1
echo.
pause