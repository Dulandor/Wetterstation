# Weather Station Project

A modular Arduino-based weather station for senseBox:edu that provides comprehensive environmental monitoring with web interface and data logging capabilities.

## Key Features

### Multi-Sensor Environmental Monitoring
- **Temperature & Humidity**: HDC1000 sensor with ±0.2°C accuracy
- **Atmospheric Pressure**: DPS310 sensor with altitude calculation
- **Light Intensity**: Dual sensor support (LTR329/TSL45315) with automatic detection
- **UV Radiation**: VEML6070 sensor for UV index monitoring

### Connectivity & Data Access
- **WiFi Connectivity**: Automatic connection with retry mechanism
- **Web Server**: Real-time data dashboard accessible via browser
- **NTP Time Sync**: Accurate timestamping for all measurements
- **RESTful API**: JSON endpoints for data integration

### Data Management
- **SD Card Logging**: CSV format with configurable intervals (default: 5 minutes)
- **Historical Data**: Web-accessible historical data viewer
- **Data Structure**: Timestamped sensor readings with metadata

### Modular Architecture
- **SensorManager**: Centralized sensor initialization and data collection
- **TimeManager**: NTP synchronization and timestamp management
- **WebServer**: HTTP request handling and dashboard serving

## Hardware Requirements

### Core Components
- **senseBox:edu** (Arduino-compatible microcontroller)
- **WiFi101 Shield** or compatible WiFi module
- **MicroSD Card** (FAT32 formatted, Class 10 recommended)

### Sensors
- **HDC1000**: Temperature & Humidity (I2C: 0x40)
- **DPS310**: Pressure & Altitude (I2C: 0x77)
- **LTR329** or **TSL45315**: Light Intensity (I2C: 0x29/0x39)
- **VEML6070**: UV Radiation (I2C: 0x38/0x39)

### Connections
- All sensors connect via I2C bus (SDA/SCL)
- SD card uses SPI interface (CS pin 28)
- WiFi shield uses dedicated SPI pins

## Installation & Setup

### 1. Library Dependencies
Install these libraries via Arduino Library Manager:
```
senseBoxIO           # senseBox board support
Adafruit_HDC1000     # Temperature/Humidity sensor
Adafruit_DPS310      # Pressure sensor
LTR329               # Light sensor (alternative)
VEML6070             # UV sensor
WiFi101              # WiFi connectivity
NTPClient            # Network time protocol
SD                   # SD card support
```

### 2. Configuration
Edit the configuration constants in `main.ino`:
```cpp
// WiFi Settings (REQUIRED)
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Data Logging Interval
const unsigned long DATA_SAVE_INTERVAL = 300000;  // 5 minutes

// Connection Settings
const int MAX_CONNECTION_ATTEMPTS = 3;
const int SD_CS_PIN = 28;
```

### 3. Upload Process
1. Connect senseBox:edu via USB
2. Select **"senseBox MCU"** in Arduino IDE
3. Choose correct COM port
4. Upload sketch (Ctrl+U / Cmd+U)

## Web Interface

### Accessing the Dashboard
1. Monitor Serial output for IP address assignment
2. Open browser and navigate to: `http://[DEVICE_IP]`
3. Available endpoints:
   - `/` - Live sensor dashboard
   - `/dashboard` - Historical data viewer
   - `/api/data` - JSON API for current readings

### Data Format
```json
{
  "timestamp": "DD.MM.YYYY;HH:MM:SS",
  "temperature": 25.5,
  "humidity": 60.0,
  "pressure": 1013.25,
  "altitude": 150.0,
  "light": 500,
  "uv": 3
}
```

## Project Architecture

```
Weather-Station/
├── main.ino                 # Main application entry point
├── README.md                # This documentation
├── LICENSE                  # Apache 2.0 license
└── src/
    ├── sensors/
    │   ├── SensorManager.h   # Sensor interface definitions
    │   └── SensorManager.cpp # Sensor implementation & data collection
    ├── utils/
    │   ├── TimeManager.h     # Time synchronization interface
    │   └── TimeManager.cpp   # NTP client implementation
    └── web/
        ├── WebServer.h       # HTTP server interface
        └── WebServer.cpp     # Web dashboard & API implementation
```

### Component Responsibilities

**SensorManager** (`src/sensors/`)
- Sensor initialization and health monitoring
- Unified data collection from all sensors
- Automatic light sensor type detection (LTR329/TSL45315)
- I2C communication handling

**TimeManager** (`src/utils/`)
- NTP server synchronization
- Timestamp generation for data logging
- Time zone handling and formatting

**WebServer** (`src/web/`)
- HTTP request routing and response handling
- Real-time dashboard serving
- Historical data API endpoints
- JSON data serialization

## Troubleshooting

### Common Issues

**SD Card Initialization Failed**
```
Solution:
- Verify SD card is FAT32 formatted
- Check CS pin connection (pin 28)
- Try different SD card (Class 10 recommended)
- Ensure proper 3.3V power supply
```

**WiFi Connection Issues**
```
Solution:
- Verify SSID and password in code
- Check WiFi shield connections
- Monitor Serial output for connection attempts
- Ensure 2.4GHz network (5GHz not supported)
```

**Sensor Not Detected**
```
Solution:
- Check I2C connections (SDA/SCL)
- Verify sensor power (3.3V)
- Use I2C scanner to detect addresses
- Check for conflicting I2C addresses
```

**Time Synchronization Failed**
```
Solution:
- Ensure internet connectivity
- Check firewall settings (NTP port 123)
- Verify NTP server accessibility
- Monitor Serial output for sync status
```

### Debug Mode
Enable verbose logging by monitoring Serial output at 9600 baud:
```cpp
Serial.begin(9600);
// Watch for initialization status messages
```

## Contributing

Feel free to submit issues and enhancement requests!

## License

This project is licensed under the Apache 2.0 License - see the LICENSE file for details.
