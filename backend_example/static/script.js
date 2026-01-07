/**
 * ============================================
 * ESP32 SMART HOME DASHBOARD - JavaScript
 * Modern Dashboard with Real-time Updates
 * ============================================
 */

// ============================================
// 1. CONSTANTS & CONFIGURATION
// ============================================
const CONFIG = {
    API_BASE: '',
    REFRESH_INTERVAL: 10000,        // 10 seconds
    CHART_REFRESH_INTERVAL: 30000,  // 30 seconds
    CHART_MAX_POINTS: 20,
    LOCALE: 'vi-VN'
};

const SENSOR_THRESHOLDS = {
    temperature: {
        min: 0, max: 50,
        warningLow: 10, warningHigh: 35
    },
    humidity: {
        min: 0, max: 100,
        warningLow: 30, warningHigh: 70
    },
    light: {
        min: 0, max: 100,
        warningLow: 10, warningHigh: 90
    }
};

// ============================================
// 2. UTILITY FUNCTIONS
// ============================================
const Utils = {
    /**
     * Format timestamp to readable string
     */
    formatTime(timestamp, short = false) {
        if (!timestamp) return '--';
        
        let date;
        
        // Handle different timestamp formats
        if (typeof timestamp === 'string') {
            // If it's a string like "2026-01-04 22:12:30"
            date = new Date(timestamp);
        } else if (typeof timestamp === 'number') {
            // If it's a number (milliseconds or seconds)
            if (timestamp < 10000000000) {
                // Likely seconds, convert to milliseconds
                date = new Date(timestamp * 1000);
            } else {
                // Already milliseconds
                date = new Date(timestamp);
            }
        } else {
            return '--';
        }
        
        // Check if date is valid
        if (isNaN(date.getTime())) {
            return '--';
        }
        
        if (short) {
            return date.toLocaleTimeString(CONFIG.LOCALE, {
                hour12: false,
                hour: '2-digit',
                minute: '2-digit'
            });
        }
        
        return date.toLocaleString(CONFIG.LOCALE, {
            year: 'numeric',
            month: 'short',
            day: 'numeric',
            hour: '2-digit',
            minute: '2-digit',
            second: '2-digit'
        });
    },

    /**
     * Format uptime from milliseconds
     */
    formatUptime(ms) {
        const seconds = Math.floor(ms / 1000);
        const hours = Math.floor(seconds / 3600);
        const minutes = Math.floor((seconds % 3600) / 60);
        return `${hours}h ${minutes}m`;
    },

    /**
     * Calculate percentage for progress bar
     */
    calculatePercentage(value, min, max) {
        if (value === null || value === undefined) return 0;
        const percentage = ((value - min) / (max - min)) * 100;
        return Math.max(0, Math.min(100, percentage));
    },

    /**
     * Determine sensor status based on value and thresholds
     */
    getSensorStatus(value, thresholds) {
        if (value === null || value === undefined) return { status: 'waiting', text: 'Đang chờ dữ liệu...' };
        if (value < thresholds.min || value > thresholds.max) return { status: 'error', text: 'Ngoài phạm vi' };
        if (value < thresholds.warningLow || value > thresholds.warningHigh) return { status: 'warning', text: 'Cảnh báo' };
        return { status: 'normal', text: 'Bình thường' };
    }
};

// ============================================
// 3. API SERVICE
// ============================================
const ApiService = {
    async fetchDevices() {
        const response = await fetch(`${CONFIG.API_BASE}/api/devices`);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        return response.json();
    },

    async fetchLatestSensorData(deviceId) {
        const response = await fetch(`${CONFIG.API_BASE}/api/sensor/latest/${deviceId}`);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        return response.json();
    },

    async fetchSensorHistory(deviceId, limit = CONFIG.CHART_MAX_POINTS) {
        const response = await fetch(`${CONFIG.API_BASE}/api/sensor/history/${deviceId}?limit=${limit}`);
        if (!response.ok) throw new Error(`HTTP ${response.status}`);
        return response.json();
    }
};

// ============================================
// 4. UI CONTROLLER
// ============================================
const UIController = {
    elements: {},

    init() {
        // Cache DOM elements
        this.elements = {
            connectionStatus: document.getElementById('connectionStatus'),
            statusText: document.getElementById('statusText'),
            currentTime: document.getElementById('currentTime'),
            deviceSelect: document.getElementById('deviceSelect'),
            // Temperature
            tempValue: document.getElementById('tempValue'),
            tempPercent: document.getElementById('tempPercent'),
            tempProgress: document.getElementById('tempProgress'),
            tempStatus: document.getElementById('tempStatus'),
            tempTime: document.getElementById('tempTime'),
            // Humidity
            humidityValue: document.getElementById('humidityValue'),
            humidityPercent: document.getElementById('humidityPercent'),
            humidityProgress: document.getElementById('humidityProgress'),
            humidityStatus: document.getElementById('humidityStatus'),
            humidityTime: document.getElementById('humidityTime'),
            // Light
            lightValue: document.getElementById('lightValue'),
            lightPercent: document.getElementById('lightPercent'),
            lightProgress: document.getElementById('lightProgress'),
            lightStatus: document.getElementById('lightStatus'),
            lightTime: document.getElementById('lightTime'),
            // System
            deviceId: document.getElementById('deviceId'),
            uptime: document.getElementById('uptime'),
            dataPoints: document.getElementById('dataPoints')
        };
    },

    updateConnectionStatus(status, text) {
        this.elements.connectionStatus.className = `status-dot ${status}`;
        this.elements.statusText.textContent = text;
    },

    updateDeviceList(devices) {
        const select = this.elements.deviceSelect;
        select.innerHTML = '';

        if (devices && devices.length > 0) {
            devices.forEach(device => {
                const option = document.createElement('option');
                option.value = device;
                option.textContent = device;
                select.appendChild(option);
            });
        } else {
            select.innerHTML = '<option value="">Không tìm thấy thiết bị</option>';
        }
    },

    updateSensorCard(type, value, timestamp) {
        console.log(`🔧 Cập nhật ${type}:`, { value, timestamp, typeOfValue: typeof value }); // Debug log
        
        // Map type name to element prefix (temperature -> temp, others stay same)
        const elementPrefix = type === 'temperature' ? 'temp' : type;
        
        const thresholds = SENSOR_THRESHOLDS[type];
        const status = Utils.getSensorStatus(value, thresholds);
        
        // Convert value to number explicitly
        const numValue = (value !== null && value !== undefined) ? Number(value) : null;
        const percentage = Utils.calculatePercentage(numValue, thresholds.min, thresholds.max);

        console.log(`🔢 ${type} numValue:`, numValue, 'percentage:', percentage);

        // Update value
        const valueEl = this.elements[`${elementPrefix}Value`];
        console.log(`🎯 Element ${elementPrefix}Value:`, valueEl);
        
        if (valueEl) {
            let displayValue;
            if (numValue !== null && !isNaN(numValue)) {
                displayValue = type === 'light' ? numValue.toFixed(0) : numValue.toFixed(1);
            } else {
                displayValue = '--';
            }
            valueEl.textContent = displayValue;
            console.log(`✅ ${type} hiển thị:`, displayValue);
        } else {
            console.error(`❌ Không tìm thấy element: ${elementPrefix}Value`);
        }

        // Update progress
        const percentEl = this.elements[`${elementPrefix}Percent`];
        const progressEl = this.elements[`${elementPrefix}Progress`];
        if (percentEl) percentEl.textContent = `${percentage.toFixed(0)}%`;
        if (progressEl) progressEl.style.width = `${percentage}%`;

        // Update status
        const statusEl = this.elements[`${elementPrefix}Status`];
        if (statusEl) {
            statusEl.className = `status-row ${status.status}`;
            const icon = status.status === 'normal' ? 'circle-check' : 
                        status.status === 'warning' ? 'exclamation-triangle' : 'circle-xmark';
            statusEl.innerHTML = `<i class="fas fa-${icon}"></i><span>${status.text}</span>`;
        }

        // Update time
        const timeEl = this.elements[`${elementPrefix}Time`];
        if (timeEl) {
            const formattedTime = Utils.formatTime(timestamp);
            timeEl.textContent = `Cập nhật: ${formattedTime}`;
            console.log(`⏰ ${type} thời gian:`, formattedTime);
        }
    },

    updateSystemStats(data) {
        if (this.elements.deviceId) {
            this.elements.deviceId.textContent = data.device_id || '--';
        }
        if (this.elements.uptime && data.timestamp) {
            this.elements.uptime.textContent = Utils.formatUptime(data.timestamp);
        }
    },

    updateDataPoints(count) {
        if (this.elements.dataPoints) {
            this.elements.dataPoints.textContent = count;
        }
    },

    updateCurrentTime() {
        if (this.elements.currentTime) {
            this.elements.currentTime.textContent = new Date().toLocaleTimeString(CONFIG.LOCALE, {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
        }
    },

    clearSensorDisplays() {
        ['temperature', 'humidity', 'light'].forEach(type => {
            this.updateSensorCard(type, null, null);
        });
        if (this.elements.deviceId) this.elements.deviceId.textContent = '--';
        if (this.elements.uptime) this.elements.uptime.textContent = '--';
        if (this.elements.dataPoints) this.elements.dataPoints.textContent = '--';
    }
};

// ============================================
// 5. CHART CONTROLLER
// ============================================
const ChartController = {
    chart: null,

    init() {
        const ctx = document.getElementById('sensorChart');
        if (!ctx) return;

        this.chart = new Chart(ctx.getContext('2d'), {
            type: 'line',
            data: {
                labels: [],
                datasets: [
                    {
                        label: 'Nhiệt độ (°C)',
                        data: [],
                        borderColor: '#f87171',
                        backgroundColor: 'rgba(248, 113, 113, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        fill: true,
                        pointRadius: 4,
                        pointHoverRadius: 6
                    },
                    {
                        label: 'Độ ẩm (%)',
                        data: [],
                        borderColor: '#22d3ee',
                        backgroundColor: 'rgba(34, 211, 238, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        fill: true,
                        pointRadius: 4,
                        pointHoverRadius: 6
                    },
                    {
                        label: 'Ánh sáng (%)',
                        data: [],
                        borderColor: '#fbbf24',
                        backgroundColor: 'rgba(251, 191, 36, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        fill: true,
                        pointRadius: 4,
                        pointHoverRadius: 6
                    }
                ]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index'
                },
                plugins: {
                    legend: {
                        position: 'top',
                        labels: {
                            color: '#94a3b8',
                            padding: 20,
                            font: {
                                family: 'Inter',
                                size: 12
                            }
                        }
                    },
                    tooltip: {
                        backgroundColor: 'rgba(30, 41, 59, 0.95)',
                        titleColor: '#f1f5f9',
                        bodyColor: '#94a3b8',
                        borderColor: 'rgba(148, 163, 184, 0.2)',
                        borderWidth: 1,
                        padding: 12,
                        cornerRadius: 8,
                        displayColors: true,
                        titleFont: {
                            family: 'Inter',
                            weight: 600
                        },
                        bodyFont: {
                            family: 'Inter'
                        }
                    }
                },
                scales: {
                    x: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Thời gian',
                            color: '#64748b'
                        },
                        grid: {
                            color: 'rgba(148, 163, 184, 0.1)'
                        },
                        ticks: {
                            color: '#64748b',
                            font: {
                                family: 'Inter'
                            }
                        }
                    },
                    y: {
                        display: true,
                        title: {
                            display: true,
                            text: 'Giá trị',
                            color: '#64748b'
                        },
                        grid: {
                            color: 'rgba(148, 163, 184, 0.1)'
                        },
                        ticks: {
                            color: '#64748b',
                            font: {
                                family: 'Inter'
                            }
                        }
                    }
                },
                animation: {
                    duration: 750,
                    easing: 'easeInOutQuart'
                }
            }
        });
    },

    update(history) {
        if (!this.chart || !history) return;

        const labels = history.map(item => Utils.formatTime(item.timestamp, true));
        const temperatures = history.map(item => item.temperature);
        const humidities = history.map(item => item.humidity);
        const lights = history.map(item => item.light);

        this.chart.data.labels = labels;
        this.chart.data.datasets[0].data = temperatures;
        this.chart.data.datasets[1].data = humidities;
        this.chart.data.datasets[2].data = lights;

        this.chart.update('none');
    }
};

// ============================================
// 6. MAIN DASHBOARD CLASS
// ============================================
class SmartHomeDashboard {
    constructor() {
        this.currentDevice = null;
        this.updateInterval = null;
        this.chartUpdateInterval = null;
        this.lastChartUpdate = 0;

        this.init();
    }

    async init() {
        console.log('🏠 Smart Home Dashboard đang khởi tạo...');

        // Initialize UI Controller
        UIController.init();

        // Initialize Chart
        ChartController.init();

        // Setup event listeners
        this.setupEventListeners();

        // Start time update
        this.startTimeUpdate();

        // Load devices
        await this.loadDevices();

        // Start auto-refresh
        this.startAutoRefresh();

        console.log('✅ Dashboard khởi tạo thành công!');
    }

    setupEventListeners() {
        // Device select change
        const deviceSelect = document.getElementById('deviceSelect');
        if (deviceSelect) {
            deviceSelect.addEventListener('change', async (e) => {
                this.currentDevice = e.target.value;
                if (this.currentDevice) {
                    await this.loadSensorData();
                    await this.updateCharts();
                }
            });
        }

        // Handle visibility change
        document.addEventListener('visibilitychange', () => {
            if (document.hidden) {
                this.stopAutoRefresh();
            } else {
                this.startAutoRefresh();
            }
        });
    }

    startTimeUpdate() {
        UIController.updateCurrentTime();
        setInterval(() => UIController.updateCurrentTime(), 1000);
    }

    async loadDevices() {
        try {
            UIController.updateConnectionStatus('connecting', 'Đang tải...');

            const data = await ApiService.fetchDevices();

            UIController.updateDeviceList(data.devices);

            if (data.devices && data.devices.length > 0) {
                this.currentDevice = data.devices[0];
                document.getElementById('deviceSelect').value = this.currentDevice;

                await this.loadSensorData();
                await this.updateCharts();

                UIController.updateConnectionStatus('connected', 'Đã kết nối');
            } else {
                UIController.updateConnectionStatus('error', 'Không có thiết bị');
            }
        } catch (error) {
            console.error('❌ Lỗi tải thiết bị:', error);
            UIController.updateConnectionStatus('error', 'Lỗi kết nối');
            UIController.updateDeviceList([]);
        }
    }

    async loadSensorData() {
        if (!this.currentDevice) return;

        try {
            const data = await ApiService.fetchLatestSensorData(this.currentDevice);
            
            console.log('📊 Dữ liệu nhận được:', data); // Debug log

            // Ensure we have valid data
            const temperature = data.temperature !== null && data.temperature !== undefined ? data.temperature : null;
            const humidity = data.humidity !== null && data.humidity !== undefined ? data.humidity : null;
            const light = data.light !== null && data.light !== undefined ? data.light : null;
            
            // Use received_at or timestamp for time
            const timestamp = data.received_at || data.timestamp || new Date().toISOString();

            // Update sensor cards
            UIController.updateSensorCard('temperature', temperature, timestamp);
            UIController.updateSensorCard('humidity', humidity, timestamp);
            UIController.updateSensorCard('light', light, timestamp);

            // Update system stats
            UIController.updateSystemStats(data);

            UIController.updateConnectionStatus('connected', 'Đã kết nối');

        } catch (error) {
            console.error('❌ Lỗi tải dữ liệu cảm biến:', error);
            UIController.updateConnectionStatus('error', 'Lỗi dữ liệu');
            UIController.clearSensorDisplays();
        }
    }

    async updateCharts() {
        if (!this.currentDevice) return;

        try {
            const data = await ApiService.fetchSensorHistory(this.currentDevice);
            const history = data.history || [];

            ChartController.update(history);
            UIController.updateDataPoints(history.length);

            this.lastChartUpdate = Date.now();
            console.log(`📊 Biểu đồ đã cập nhật với ${history.length} điểm dữ liệu`);

        } catch (error) {
            console.error('❌ Lỗi cập nhật biểu đồ:', error);
        }
    }

    startAutoRefresh() {
        // Clear existing intervals
        this.stopAutoRefresh();

        // Refresh sensor data
        this.updateInterval = setInterval(async () => {
            await this.loadSensorData();

            // Update charts every 30 seconds
            if (Date.now() - this.lastChartUpdate >= CONFIG.CHART_REFRESH_INTERVAL) {
                await this.updateCharts();
            }
        }, CONFIG.REFRESH_INTERVAL);

        console.log(`🔄 Tự động làm mới đã bật (${CONFIG.REFRESH_INTERVAL / 1000}s)`);
    }

    stopAutoRefresh() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
            console.log('⏹️ Đã dừng tự động làm mới');
        }
    }
}

// ============================================
// 7. GLOBAL FUNCTIONS (for onclick handlers)
// ============================================
function updateCharts() {
    if (window.dashboard) {
        window.dashboard.updateCharts();
    }
}

function exportData() {
    // TODO: Implement data export
    alert('Chức năng xuất dữ liệu đang được phát triển!');
}

// ============================================
// 8. INITIALIZATION
// ============================================
document.addEventListener('DOMContentLoaded', () => {
    window.dashboard = new SmartHomeDashboard();
});

// Console branding
console.log(`
🏠 ESP32 Smart Home Dashboard
🔧 Phân tích bảo mật IoT
⚠️  Bản Demo - Chứa lỗ hổng có chủ đích
📊 Giám sát cảm biến real-time với UI hiện đại
`);

// Export for debugging
window.SmartHomeDashboard = SmartHomeDashboard;
window.Utils = Utils;
window.ApiService = ApiService;