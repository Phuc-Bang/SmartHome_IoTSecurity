    #!/usr/bin/env python3
"""
Simple HTTP Server for ESP32 Smart Home IoT Security Demo
WARNING: This is a VULNERABLE server for educational purposes only!
"""

from flask import Flask, request, jsonify, render_template, send_from_directory
import json
import datetime
import sqlite3
import os

app = Flask(__name__)

# Create database if not exists
def init_db():
    conn = sqlite3.connect('sensor_data.db')
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS sensor_readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT,
            temperature REAL,
            humidity REAL,
            light REAL,
            timestamp INTEGER,
            received_at TEXT
        )
    ''')
    conn.commit()
    conn.close()

@app.route('/api/sensor/data', methods=['POST'])
def receive_sensor_data():
    """
    VULNERABLE endpoint - No authentication, no input validation
    """
    try:
        # Get JSON data
        data = request.get_json()
        
        # VULNERABLE: No authentication check
        print(f"[RECEIVED] Data from device: {data}")
        
        # VULNERABLE: No input validation
        device_id = data.get('device_id', 'unknown')
        temperature = data.get('temperature', 0)
        humidity = data.get('humidity', 0)
        light = data.get('light', 0)
        timestamp = data.get('timestamp', 0)
        
        # Store in database (VULNERABLE: SQL injection possible)
        conn = sqlite3.connect('sensor_data.db')
        cursor = conn.cursor()
        
        # VULNERABLE: Direct string interpolation
        query = f"""
            INSERT INTO sensor_readings 
            (device_id, temperature, humidity, light, timestamp, received_at)
            VALUES ('{device_id}', {temperature}, {humidity}, {light}, {timestamp}, '{datetime.datetime.now()}')
        """
        
        cursor.execute(query)
        conn.commit()
        conn.close()
        
        return jsonify({
            'status': 'success',
            'message': 'Data received',
            'device_id': device_id
        }), 200
        
    except Exception as e:
        print(f"[ERROR] {str(e)}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500

@app.route('/api/sensor/latest/<device_id>', methods=['GET'])
def get_latest_data(device_id):
    """
    Get latest sensor data for a device
    """
    try:
        conn = sqlite3.connect('sensor_data.db')
        cursor = conn.cursor()
        
        # VULNERABLE: SQL injection possible
        query = f"SELECT * FROM sensor_readings WHERE device_id = '{device_id}' ORDER BY id DESC LIMIT 1"
        cursor.execute(query)
        
        row = cursor.fetchone()
        conn.close()
        
        if row:
            return jsonify({
                'id': row[0],
                'device_id': row[1],
                'temperature': row[2],
                'humidity': row[3],
                'light': row[4],
                'timestamp': row[5],
                'received_at': row[6]
            }), 200
        else:
            return jsonify({'message': 'No data found'}), 404
            
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/api/devices', methods=['GET'])
def list_devices():
    """
    List all devices that have sent data
    """
    try:
        conn = sqlite3.connect('sensor_data.db')
        cursor = conn.cursor()
        
        cursor.execute("SELECT DISTINCT device_id FROM sensor_readings")
        devices = [row[0] for row in cursor.fetchall()]
        conn.close()
        
        return jsonify({'devices': devices}), 200
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/')
def index():
    """
    Modern dashboard homepage
    """
    return render_template('index.html')

@app.route('/dashboard')
def dashboard():
    """
    Redirect to modern dashboard
    """
    return render_template('index.html')

@app.route('/api/sensor/history/<device_id>')
def get_sensor_history(device_id):
    """
    Get sensor history for charts
    """
    try:
        limit = request.args.get('limit', 50)
        
        conn = sqlite3.connect('sensor_data.db')
        cursor = conn.cursor()
        
        query = f"""
            SELECT temperature, humidity, light, received_at 
            FROM sensor_readings 
            WHERE device_id = '{device_id}' 
            ORDER BY id DESC 
            LIMIT {limit}
        """
        cursor.execute(query)
        
        rows = cursor.fetchall()
        conn.close()
        
        # Reverse to get chronological order
        rows.reverse()
        
        history = []
        for row in rows:
            history.append({
                'temperature': row[0],
                'humidity': row[1],
                'light': row[2],
                'timestamp': row[3]
            })
        
        return jsonify({'history': history}), 200
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/static/<path:filename>')
def static_files(filename):
    """
    Serve static files
    """
    return send_from_directory('static', filename)

if __name__ == '__main__':
    print("=" * 50)
    print("ESP32 Smart Home - VULNERABLE Backend Server")
    print("WARNING: For educational purposes only!")
    print("=" * 50)
    
    # Initialize database
    init_db()
    
    # Run server (VULNERABLE: Debug mode, all interfaces)
    app.run(host='0.0.0.0', port=5000, debug=True)