# 🌤️ Weather Station Project

A comprehensive weather station project for senseBox:edu that collects and displays various environmental measurements.

## 📋 Features

- **Temperature & Humidity** monitoring using HDC1000 sensor
- **Pressure & Altitude** measurements with DPS310 sensor
- **Light Intensity** detection using LTR329/TSL45315 sensor
- **UV Radiation** monitoring with VEML6070 sensor
- **Data Logging** to SD card
- **Web Interface** for real-time data access
- **NTP Time Synchronization**
- **WiFi Connectivity**

## 🛠️ Hardware Requirements

- senseBox:edu
- HDC1000 Temperature & Humidity Sensor
- DPS310 Pressure Sensor
- LTR329/TSL45315 Light Sensor
- VEML6070 UV Sensor
- SD Card Module
- WiFi Shield

## 📦 Installation

1. Clone or download this repository
2. Open the project in Arduino IDE
3. Install required libraries:
   - senseBoxIO (included with senseBox board package)
   - Adafruit_HDC1000
   - Adafruit_DPS310
   - LTR329
   - VEML6070
   - WiFi101
   - NTPClient
   - SD

## ⚙️ Configuration

1. Open `main.ino` in Arduino IDE
2. Configure your WiFi credentials:
   ```cpp
   const char* WIFI_SSID = "YOUR_WIFI_SSID";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   ```
3. Data logging interval is set to 5 minutes:
   ```cpp
   const unsigned long DATA_SAVE_INTERVAL = 300000;  // 5 minutes
   ```

## 📤 Upload Instructions

1. Connect your senseBox:edu to your computer
2. Select "senseBox MCU" as the board in Arduino IDE
3. Select the correct port
4. Click the upload button or press Ctrl+U (Cmd+U on Mac)

## 📊 Data Access

Once the weather station is running, you can access the data through:

- **Live Data**: Connect to the device's IP address in your web browser (displayed in Serial Monitor)
- **Historical Data**: Access the dashboard at `http://[device-ip]/dashboard`
- **Raw Data**: Check the SD card's `data.txt` file

## 🔧 Troubleshooting

If you encounter issues:

1. **SD Card Not Initializing**:
   - Check if the SD card is properly inserted
   - Ensure the SD card is formatted as FAT32
   - Verify the SD card module connections

2. **WiFi Connection Issues**:
   - Check if the WiFi shield is properly connected
   - Verify WiFi credentials
   - Check Serial Monitor for connection status and IP address

## 📁 Project Structure

```
├── main.ino              # Main program file
└── src/
    ├── sensors/         # Sensor management
    │   ├── SensorManager.h
    │   └── SensorManager.cpp
    ├── utils/           # Utility functions
    │   ├── TimeManager.h
    │   └── TimeManager.cpp
    └── web/             # Web server functionality
        ├── WebServer.h
        └── WebServer.cpp
```

## 🤝 Contributing

Feel free to submit issues and enhancement requests!

## 📝 License

This project is licensed under the Apache 2.0 License - see the LICENSE file for details.
