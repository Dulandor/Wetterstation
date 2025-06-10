/*
 * Weather Station Project
 * 
 * This program controls a weather station with the following sensors:
 * - Temperature and Humidity (HDC1000)
 * - Pressure (DPS310)
 * - Light Intensity (LTR329 or TSL45315)
 * - UV Radiation (VEML6070)
 * 
 * Features:
 * - Data logging to SD card
 * - Web server for real-time data access
 * - NTP time synchronization
 * - WiFi connectivity
 * 
 * Author: Leander
 * Version: 1.0
 */

// ===== INCLUDES =====
#include <Arduino.h>
#include <senseBoxIO.h>
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Forward declarations
class Adafruit_HDC1000;
class Adafruit_DPS310;
class LTR329;
class VEML6070;

#include "src/sensors/SensorManager.h"
#include "src/utils/TimeManager.h"
#include "src/web/WebServer.h"

// ===== CONSTANTS =====
const int SD_CS_PIN = 28;                          // SD card chip select pin
const char* WIFI_SSID = "YOUR_WIFI_SSID";          // Replace with your WiFi network name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";  // Replace with your WiFi password
const unsigned long DATA_SAVE_INTERVAL = 300000;   // Data logging interval: 5 minutes in milliseconds
const int MAX_CONNECTION_ATTEMPTS = 3;             // Maximum WiFi connection retry attempts
const unsigned long WIFI_STATUS_LOG_INTERVAL = 60000; // WiFi status logging interval: 1 minute in milliseconds

// ===== GLOBAL OBJECTS =====
// Sensor instances
static Adafruit_HDC1000 hdc;   // Temperature & humidity sensor
static Adafruit_DPS310 dps;    // Pressure sensor
static LTR329 ltr;             // Light sensor (alternative)
static VEML6070 veml;          // UV radiation sensor

// Network time protocol setup
static WiFiUDP ntpUDP;
static NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000);  // UTC+1, update every minute

// Manager instances for modular architecture
static SensorManager sensorManager(hdc, dps, veml, ltr);
static TimeManager timeManager(timeClient);
static WebServer webServer(sensorManager, timeManager);

// Timing variables
static unsigned long lastWiFiStatusLog = 0;

// ===== SETUP =====
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  while (!Serial && millis() < 3000) {  // Wait up to 3 seconds for serial connection
    delay(10);
  }
  Serial.println("\nWeather Station Initialization");
  
  // Initialize I2C communication for sensors
  Wire.begin();
  
  // Initialize SD card for data logging
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
  } else {
    Serial.println("SD card initialized");
    // Create CSV file with headers if it doesn't exist
    if (!SD.exists("data.txt")) {
      File dataFile = SD.open("data.txt", FILE_WRITE);
      if (dataFile) {
        dataFile.println("timestamp,temperature,humidity,pressure,altitude,light,uv");
        dataFile.close();
        Serial.println("Created data.txt with headers");
      }
    }
  }

  // Initialize WiFi connection with retry mechanism
  Serial.println("Checking WiFi shield...");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("No WiFi shield found!");
  } else {
    Serial.println("WiFi shield detected");
    // Attempt WiFi connection with multiple retries
    for (int i = 0; i < MAX_CONNECTION_ATTEMPTS; i++) {
      Serial.print("WiFi connection attempt ");
      Serial.print(i + 1);
      Serial.print(" of ");
      Serial.println(MAX_CONNECTION_ATTEMPTS);
      
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      unsigned long startAttemptTime = millis();
      
      // Wait up to 15 seconds for connection
      while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
        delay(500);
        Serial.print(".");
      }
      Serial.println();
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected");
        // Display assigned IP address
        IPAddress ip = WiFi.localIP();
        String ipStr = String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
        Serial.print("IP Address: ");
        Serial.println(ipStr);
        // Start web server and sync time
        webServer.begin();
        timeManager.syncTime();
        break;
      }
      
      delay(5000);  // Wait before next attempt
    }
  }

  if (sensorManager.initialize()) {
    Serial.println("All sensors initialized successfully");
  } else {
    Serial.println("Some sensors failed to initialize");
  }
  
  Serial.println("Setup complete");
}

void loop() {
  // Check WiFi connection and reconnect if necessary
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost, attempting reconnect...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long reconnectStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - reconnectStart < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi reconnected successfully");
      IPAddress ip = WiFi.localIP();
      Serial.print("IP Address: ");
      Serial.println(ip);
      timeManager.syncTime();  // Resync time after reconnection
    } else {
      Serial.println("\nWiFi reconnection failed");
    }
  }
   
  // Log WiFi status periodically for monitoring
  if (millis() - lastWiFiStatusLog >= WIFI_STATUS_LOG_INTERVAL) {
    lastWiFiStatusLog = millis();
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("WiFi Status: Connected (IP: ");
      Serial.print(WiFi.localIP());
      Serial.print(", RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm)");
    } else {
      Serial.println("WiFi Status: Disconnected");
    }
  }
   
  // Update time synchronization periodically
  timeManager.update();
  
  // Save sensor data at specified intervals
  if (millis() % DATA_SAVE_INTERVAL < 1000) {
    SensorData data = sensorManager.readData();
    data.timestamp = timeManager.getCurrentTimestamp();
    
    // Write data to SD card in CSV format
    File dataFile = SD.open("data.txt", FILE_WRITE);
    if (dataFile) {
      dataFile.print(timeManager.getFormattedTimestamp());
      dataFile.print(",");
      dataFile.print(data.temperature);
      dataFile.print(",");
      dataFile.print(data.humidity);
      dataFile.print(",");
      dataFile.print(data.pressure);
      dataFile.print(",");
      dataFile.print(data.altitude);
      dataFile.print(",");
      dataFile.print(data.light);
      dataFile.print(",");
      dataFile.println(data.uv);
      dataFile.close();
      Serial.println("Data saved to SD card");
    }
  }
  
  // Handle incoming web requests
  webServer.handleClient();
  
  delay(100);  // Small delay to prevent overwhelming the system
}